#include <string_view>
#include <array>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <iostream>

constexpr int parse_int(std::string_view sv) {
    if (sv.empty()) throw std::invalid_argument("parse_int: empty string");
    size_t i = 0;
    int sign = 1;
    long long result = 0;
    if (sv[i] == '-') {
        sign = -1;
        ++i;
    } else if (sv[i] == '+') {
        ++i;
    }
    if (i == sv.size()) throw std::invalid_argument("parse_int: no digits");
    for (; i < sv.size(); ++i) {
        char c = sv[i];
        if (c < '0' || c > '9') throw std::invalid_argument("parse_int: invalid character");
        result = result * 10 + (c - '0');
        if (sign == 1 && result > std::numeric_limits<int>::max()) {
            throw std::out_of_range("parse_int: overflow");
        }
        if (sign == -1 && -result < std::numeric_limits<int>::min()) {
            throw std::out_of_range("parse_int: underflow");
        }
    }
    return static_cast<int>(sign * result);
}

constexpr std::array<std::string_view, 5> interesting = {
    "init", "open", "read", "write", "close"
};

constexpr bool is_interesting(std::string_view name) {
    for (std::string_view s : interesting) {
        if (s == name) {
            return true;
        }
    }
    return false;
}

static_assert(is_interesting("read"), "read is interesting");
static_assert(!is_interesting("get_time"), "get_time is NOT interesting");
static_assert(parse_int("42") == 42);
static_assert(parse_int("-123") == -123);

int main() {
    return 0;
}
