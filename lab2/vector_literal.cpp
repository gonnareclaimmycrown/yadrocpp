#include <cassert>
#include <cstddef>
#include <array>
#include <iostream>
#include <vector>
#include <string_view>
#include <limits>

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

std::vector<int> operator""_vi(const char* str, size_t len) {
    std::string_view sv(str, len);
    std::vector<int> result;
    std::string_view delimiters = " ,";
    size_t start = sv.find_first_not_of(delimiters);
    while (start != std::string_view::npos) {
        size_t end = sv.find_first_of(delimiters, start);
        std::string_view token = sv.substr(start, end - start);
        result.push_back(parse_int(token));
        start = sv.find_first_not_of(delimiters, end);
    }
    return result;
}

int main() {
    auto const xs = "10, 20, 30, 40"_vi;
    for (int x : xs) {                       
        std::cout << x << " ";                    
    }                                             

    std::cout << std::endl;

    return 0;
}

