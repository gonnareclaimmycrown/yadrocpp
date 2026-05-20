#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <stdexcept>
#include <utility> 

template <typename T>
class BoundedQueue {
    public:
        explicit BoundedQueue(size_t capacity) : capacity_(capacity) {
            if (capacity == 0) {
                throw std::invalid_argument("Capacity must be greater than 0");
            }
        }

        template <typename U>
        bool push(U&& item) {
            std::unique_lock<std::mutex> lock(mutex_);
            not_full_.wait(lock, [this] { return queue_.size() < capacity_ || stopped_; });
            if (stopped_) {
                return false;
            }
            queue_.emplace(std::forward<U>(item));
            lock.unlock();
            not_empty_.notify_one();
            return true;
        }

        template <typename U>
        bool try_push(U&& item) {
            std::unique_lock<std::mutex> lock(mutex_);
            if (stopped_ || queue_.size() >= capacity_) {
                return false;
            }
            queue_.emplace(std::forward<U>(item));
            lock.unlock();
            not_empty_.notify_one();
            return true;
        }

        std::optional<T> pop(){
            std::unique_lock<std::mutex> lock(mutex_);
            not_empty_.wait(lock, [this] { return !queue_.empty() || stopped_; });
            if (stopped_ && queue_.empty()) {
                return std::nullopt;
            }
            T item = std::move(queue_.front());
            queue_.pop();
            lock.unlock();
            not_full_.notify_one();
            return item;
        }

        std::optional<T> try_pop() {
            std::unique_lock<std::mutex> lock(mutex_);
            if (queue_.empty()) {
                return std::nullopt;
            }
            T item = std::move(queue_.front());
            queue_.pop();
            lock.unlock();
            not_full_.notify_one();
            return item;
        }

        void stop() {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                stopped_ = true;
            }
            not_empty_.notify_all();
            not_full_.notify_all();
        }
    private:
        std::queue<T> queue_;
        const size_t capacity_;
        std::mutex mutex_;
        std::condition_variable not_full_;
        std::condition_variable not_empty_;
        bool stopped_ = false;
};