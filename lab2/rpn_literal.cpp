#include <cassert>
#include <cstddef>
#include <array>
#include <iostream>
#include <vector>
#include <string_view>
#include <limits>
#include <stdexcept>

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

constexpr int pow_int(int base, int exp) {
    if (exp < 0) throw std::invalid_argument("pow_int: negative exponent not supported");
    if (exp == 0) return 1;
    if (base == 0) return 0;
    long long result = 1;
    long long l_base = base;
    for (int i = 0; i < exp; ++i) {
        result *= l_base;
        if (result > std::numeric_limits<int>::max()) {
            throw std::overflow_error("pow_int: integer overflow");
        }
        if (result < std::numeric_limits<int>::min()) {
            throw std::underflow_error("pow_int: integer underflow");
        }
    }
    return static_cast<int>(result);
}

constexpr int apply_op(int a, int b, char op) {
    long long la = a;
    long long lb = b;
    long long result = 0;
    switch (op) {
        case '+': result = la + lb; break;
        case '-': result = la - lb; break;
        case '*': result = la * lb; break;
        case '/': 
            if (b == 0) throw std::domain_error("apply_op: division by zero");
            if (a == std::numeric_limits<int>::min() && b == -1) {
                throw std::overflow_error("apply_op: integer overflow");
            }
            result = la / lb;
            break;
        case '%': return a % b;
            if (b == 0) throw std::domain_error("apply_op: modulo by zero");
            if (a == std::numeric_limits<int>::min() && b == -1) {
                throw std::overflow_error("apply_op: integer overflow");
            }
            result = la % lb;
            break;
        case '^': return pow_int(a, b);
        default:  throw "Unknown operator";
    }

    if (result > std::numeric_limits<int>::max()) {
        throw std::overflow_error("apply_op: integer overflow");
    } 
    if (result < std::numeric_limits<int>::min()) {
        throw std::underflow_error("apply_op: integer underflow");
    }
    return static_cast<int>(result);
}

constexpr int evaluate_rpn(std::string_view sv) {
    std::array<int, 64> stack {};
    size_t stack_size = 0;
    size_t start = 0;
    for (size_t i = 0; i <= sv.size(); ++i) {
        if (i == sv.size() || sv[i] == ' ' || sv[i] == ',') {
            if(i > start) {
                std::string_view token = sv.substr(start, i - start);
                if (token == "+" || token == "-" || token == "*" ||
                token == "/" || token == "%" || token == "^") {
                    if (stack_size < 2) throw std::invalid_argument("evaluate_rpn: insufficient operands");
                    int b = stack[--stack_size];
                    int a = stack[--stack_size];
                    stack[stack_size++] = apply_op(a, b, token[0]);
                } else {
                    if (stack_size >= stack.size()) throw std::overflow_error("evaluate_rpn: stack overflow");
                    stack[stack_size++] = parse_int(token);
                }
            }
            start = i + 1;
        }
    }
    if (stack_size == 0) throw std::invalid_argument("evaluate_rpn: empty expression");
    if (stack_size > 1) throw std::invalid_argument("evaluate_rpn: too many operands");

    return stack[0];
}

constexpr int operator""_rpn(const char* str, size_t len) {
    return evaluate_rpn(std::string_view(str, len));
}

int main() {
    static_assert(("2 3 +"_rpn) == 5, "Math failed");
    static_assert(("2 3 + 4 5 + *"_rpn) == 45, "Math failed");
    static_assert(("10 3 /"_rpn) == 3, "Math failed");
    static_assert(("2 3 ^"_rpn) == 8, "Math failed");
    static_assert(("5 1 2 + 4 * + 3 -"_rpn) == 14, "Math failed");
    static_assert(("-5 10 +"_rpn) == 5, "Negative numbers failed");

    try {
        std::string runtime_input = "10 0 /";
        int result = evaluate_rpn(runtime_input);
        std::cout << result << "\n";
    } catch (const std::exception& e) {
        std::cout << "Runtime error caught: " << e.what() << "\n";
    }

    return 0;
}
