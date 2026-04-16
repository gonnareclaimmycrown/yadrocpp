#include <cassert>
#include <iostream>
#include <memory>
#include <string_view>
#include <vector>
#include <limits>

struct int_tree_t {
    int value;
    std::unique_ptr<int_tree_t> left;
    std::unique_ptr<int_tree_t> right;

    int_tree_t(int v, std::unique_ptr<int_tree_t> l = nullptr, std::unique_ptr<int_tree_t> r = nullptr)
        : value(v), left(std::move(l)), right(std::move(r)) {}
};

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

struct parse_state {
    std::string_view::const_iterator pos;
    std::string_view::const_iterator end;    
    
    char peek() const { return pos != end ? *pos : '\0'; }
    void advance() { if (pos != end) ++pos; }
    void skip_whitespace() { while (pos != end && (*pos == ' ' || *pos == '\t')) ++pos; }

    void expect(char expected) {
        skip_whitespace();
        if (peek() != expected) {
            throw std::invalid_argument(std::string("Syntax error: expected '") + expected + "', got, '" + peek() + "'");
        }
        advance();
    }

    int parse_number() {
        skip_whitespace();
        auto start = pos;
        if (peek() == '-' || peek() == '+') advance();
        while (pos != end && peek() >= '0' && peek() <= '9') advance();
        if (start == pos) throw std::invalid_argument("Expected number");
        return parse_int(std::string_view(&*start, pos - start));
    }
};

std::unique_ptr<int_tree_t> parse_node(parse_state& state) {
    state.skip_whitespace();
    if (state.peek() == '<') {
        state.advance(); // skip '<'
        state.skip_whitespace();
        
        // Parse left subtree (may be empty)
        std::unique_ptr<int_tree_t> left = nullptr;
        if (state.peek() != '|') {
            left = parse_node(state);
        }
        
        // Parse value
        state.expect('|');
        int value = state.parse_number();
        state.expect('|');
        state.skip_whitespace();
        
        // Parse right subtree (may be empty)
        std::unique_ptr<int_tree_t> right = nullptr;
        if (state.peek() != '>') {
            right = parse_node(state);
        }
        state.expect('>');
        
        return std::make_unique<int_tree_t>(value, std::move(left), std::move(right));
    } else {
        return std::make_unique<int_tree_t>(state.parse_number());
    }
}

std::unique_ptr<int_tree_t> operator""_ti(const char* str, size_t len) {
    parse_state state{str, str + len};
    auto tree = parse_node(state);

    state.skip_whitespace();
    if (state.pos != state.end) {
        throw std::invalid_argument("Syntax error: unexpected characters at the end");
    }

    return tree;
}

void fine_print_tree(const int_tree_t* node, int indent = 0) {
    if (!node) return;
    fine_print_tree(node->right.get(), indent + 4);
    for (int i = 0; i < indent; ++i) std::cout << ' ';
    std::cout << node->value << '\n';
    fine_print_tree(node->left.get(), indent + 4);
}

int main() {
    try {
        auto const t = "<<<<|0|>|1|<|2|>>|3|>|7|<<|9|>|11|<|13|>>>"_ti;
        fine_print_tree(t.get());
        
        std::cout << "--------------------\n";

        auto const t2 = "< < | -5 | > | 10 | 42 >"_ti;
        fine_print_tree(t2.get());

    } catch (const std::exception& e) {
        std::cerr << "Parser error: " << e.what() << '\n';
    }

    return 0;
}
