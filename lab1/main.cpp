#include <iostream>
#include <vector>
#include <algorithm> 
#include <cstring>
#include <cassert>
#include <cstdint>

class bitset {
private:
    uint64_t* data_;
    size_t bit_capacity_;
    size_t block_count_;
    size_t used_blocks_;

    static size_t calculate_blocks(size_t bits) {
        return (bits + 63) / 64;
    }

public:
    //rule of five
    bitset() : data_(nullptr), bit_capacity_(0), block_count_(0), used_blocks_(0) {}

    explicit bitset(size_t initial_capacity) {
        if (initial_capacity == 0) {
            data_ = nullptr;
            bit_capacity_ = 0;
            block_count_ = 0;
            used_blocks_ = 0;
        }
        else {
            block_count_ = calculate_blocks(initial_capacity);
            bit_capacity_ = block_count_ * 64;
            data_ = new uint64_t[block_count_]();
            used_blocks_ = 0;
        }
    }

    ~bitset() {
        delete[] data_;
    }

    bitset(const bitset& other) : bit_capacity_(other.bit_capacity_), block_count_(other.block_count_), used_blocks_(other.used_blocks_) {
        if (block_count_ > 0) {
            data_ = new uint64_t[block_count_];
            std::memcpy(data_, other.data_, block_count_ * sizeof(uint64_t));
        }
        else {
            data_ = nullptr;
        }
    }

    bitset& operator=(const bitset& other) {
        if (this != &other) {
            uint64_t* new_data = nullptr;
            if (other.block_count_ > 0) {
                new_data = new uint64_t[other.block_count_];
                std::memcpy(new_data, other.data_, other.block_count_ * sizeof(uint64_t));
            }
            delete[] data_;
            data_ = new_data;
            bit_capacity_ = other.bit_capacity_;
            block_count_ = other.block_count_;
            used_blocks_ = other.used_blocks_;
        }
        return *this;
    }

    bitset(bitset&& other) noexcept : data_(other.data_), bit_capacity_(other.bit_capacity_), block_count_(other.block_count_), used_blocks_(other.used_blocks_) {
        other.data_ = nullptr;
        other.block_count_ = 0;
        other.bit_capacity_ = 0;
        other.used_blocks_ = 0;
    }

    bitset& operator=(bitset&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            bit_capacity_ = other.bit_capacity_;
            block_count_ = other.block_count_;
            used_blocks_ = other.used_blocks_;

            other.data_ = nullptr;
            other.bit_capacity_ = 0;
            other.block_count_ = 0;
            other.used_blocks_ = 0;
        }

        return *this;
    }

    // main methods
    void set(size_t k, bool b) {
        if (k >= bit_capacity_) {
            if (!b) {
                return;
            }
            size_t new_capacity = std::max(bit_capacity_ * 2, k + 1);
            size_t new_blocks = calculate_blocks(new_capacity);

            uint64_t* new_data = new uint64_t[new_blocks]();
            if (data_) {
                std::memcpy(new_data, data_, block_count_ * sizeof(uint64_t));
                delete[] data_;
            }
            data_ = new_data;
            block_count_ = new_blocks;
            bit_capacity_ = new_blocks * 64;
        }

        size_t block_x = k / 64;
        size_t bit_x = k % 64;
        if (b) {
            data_[block_x] |= (1ULL << bit_x);
            if (block_x + 1 > used_blocks_) {
                used_blocks_ = block_x + 1;
            }
        }
        else {
            data_[block_x] &= ~(1ULL << bit_x);
        }
    }

    bool test(size_t k) const {
        if (k >= bit_capacity_) {
            return false;
        }
        return (data_[k / 64] & (1ULL << (k % 64))) != 0;
    }

    bool operator[](size_t k) const {
        return test(k);
    }

    //set theory
    bitset union_with(const bitset& other) const {
        // max because I don't want to lose 1's of the max bitset 
        size_t max_blocks = std::max(block_count_, other.block_count_);
        bitset result(max_blocks * 64);

        size_t common = std::min(block_count_, other.block_count_);
        for (size_t i = 0; i < common; ++i) {
            result.data_[i] = data_[i] | other.data_[i];
        }
        if (block_count_ > common) {
            std::memcpy(result.data_ + common, data_ + common, (block_count_ - common) * sizeof(uint64_t));
        }
        else if (other.block_count_ > common) {
            std::memcpy(result.data_ + common, other.data_ + common, (other.block_count_ - common) * sizeof(uint64_t));
        }
        result.used_blocks_ = std::max(used_blocks_, other.used_blocks_);
        return result;  
    }

    bitset intersection(const bitset& other) const {
        // intersection with nonexistent bits will result in 0 anyway so I choose min out of two sizes
        size_t min_used = std::min(used_blocks_, other.used_blocks_);
        bitset result(min_used * 64);

        for (size_t i = 0; i < min_used; ++i) {
            result.data_[i] = data_[i] & other.data_[i];
        }
        result.used_blocks_ = min_used;
        return result;
    }

    bool is_subset(const bitset& other) const {
        if (used_blocks_ > other.used_blocks_) {
            for (size_t i = other.used_blocks_; i < used_blocks_; ++i) {
                if (data_[i] != 0) {
                    return false;
                }
            }
        }

        size_t border = std::min(used_blocks_, other.used_blocks_);
        for (size_t i = 0; i < border; ++i) {
            if ((data_[i] & ~other.data_[i]) != 0) {
                return false;
            }
        }
        return true;
    }

    // helper methods
    size_t size() const {
        return bit_capacity_;
    }

    bool empty() const {
        for (size_t i = 0; i < used_blocks_; ++i) {
            if (data_[i] != 0) {
                return false;
            }
        }
        return true;
    }

    void clear() {
        std::memset(data_, 0, block_count_ * sizeof(uint64_t));
        used_blocks_ = 0;
    }

};

void run_test_suite() {
    std::cout << "Starting tests...\n";

    {
        bitset b;
        b.set(63, true);
        assert(b.size() == 64); // 63 + 1 = 64

        b.set(64, true);
        assert(b.size() >= 128);  // 64 * 2 = 128
        assert(b.test(63) && b.test(64)); // 1 and 1
    }

    {
        bitset b1(100);
        b1.set(10, true);
        b1.set(90, true);

        bitset b2 = b1;

        b1.set(10, false);
        assert(b1.test(10) == false);
        assert(b2.test(10) == true); // no double reference
    }

    {
        bitset b1;
        b1.set(500, true);
        size_t old_size = b1.size();

        bitset b2 = std::move(b1);
        assert(b2.test(500) == true);
        assert(b2.size() == old_size);
        assert(b1.size() == 0);
        assert(b1.empty() == true); // cleared old object
    }

    {
        bitset s1; s1.set(10, true); 
        s1.set(20, true);
        bitset s2; s2.set(20, true); 
        s2.set(30, true);

        bitset u = s1.union_with(s2); // {10, 20, 30}
        assert(u[10] && u[20] && u[30]);

        bitset i = s1.intersection(s2); // {20}
        assert(i[20] == true);
        assert(i[10] == false);
    }

    {
        bitset s1; s1.set(100, true); 
        s1.set(200, true); // {100, 200}
        bitset s2; s2.set(100, true); // {100}

        assert(s2.is_subset(s1) == true);
        assert(s1.is_subset(s2) == false);

        s2.set(300, true); // {100, 300}
        assert(s2.is_subset(s1) == false);
    }

    {
        bitset b(1000);
        b.set(5, true); 
        b.set(999, true);
        size_t cap = b.size();

        b.clear();
        assert(b.empty() == true);
        assert(b.size() == cap);
    }

    std::cout << "All tests passed successfully." << std::endl;
}

int main() {
    run_test_suite();
    return 0;
}