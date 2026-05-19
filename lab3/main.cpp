#include <iostream>
#include <string>
#include <type_traits>

// container for all the data types
template<typename... Ts> struct List {};

// number, need arithmetic type
template<typename T, T V> 
requires std::is_arithmetic_v<T>
struct num {
    using Type = T;
    static constexpr T value = V;
};

// markers for operators
struct op_add {};
struct op_sub {};
struct op_mul {};
struct op_div {};
struct op_neg {};
struct open {};
struct close {};

// evaluation implementation
struct Error {};

template<typename Stack, typename Input> struct EvalRPN {
    using type = Error;
};

// helper for division to avoid nested explicit specialization 
template<bool IsZero, typename TL, typename TR, typename StackRest, typename RestIn>
struct DivSelector;

template<typename TL, TL VL, typename TR, TR VR, typename... StackRest, typename... RestIn>
struct DivSelector<true, num<TL, VL>, num<TR, VR>, List<StackRest...>, List<RestIn...>> {
    using type = Error;
};

template<typename TL, TL VL, typename TR, TR VR, typename... StackRest, typename... RestIn>
struct DivSelector<false, num<TL, VL>, num<TR, VR>, List<StackRest...>, List<RestIn...>> {
    using new_num = num<decltype(VL / VR), VL / VR>;
    using type = typename EvalRPN<List<new_num, StackRest...>, List<RestIn...>>::type;
};

// ideal case 
template<typename Result> 
struct EvalRPN<List<Result>, List<>> {
    using type = Result;
};

// if input is a number push it to the stack
template<typename... StackTs, typename T, T V, typename... RestIn>
struct EvalRPN<List<StackTs...>, List<num<T, V>, RestIn...>> {
    using type = typename EvalRPN<List<num<T, V>, StackTs...>, List<RestIn...>>::type;     
};

// unary minus, only one number needed 
template<typename T, T V, typename... StackRest, typename... RestIn>
struct EvalRPN<List<num<T, V>, StackRest...>, List<op_neg, RestIn...>> {
    using new_num = num<T, -V>;
    using type = typename EvalRPN<List<new_num, StackRest...>, List<RestIn...>>::type;
};

// sum, two numbers needed
template<typename TL, TL VL, typename TR, TR VR, typename... StackRest, typename... RestIn>
struct EvalRPN<List<num<TL, VL>, num<TR, VR>, StackRest...>, List<op_add, RestIn...>> {
    using new_num = num<decltype(VL + VR), VL + VR>;
    using type = typename EvalRPN<List<new_num, StackRest...>, List<RestIn...>>::type;
};

// sub, two numbers needed, opposite order
template<typename TL, TL VL, typename TR, TR VR, typename... StackRest, typename... RestIn>
struct EvalRPN<List<num<TR, VR>, num<TL, VL>, StackRest...>, List<op_sub, RestIn...>> {
    using new_num = num<decltype(VL - VR), VL - VR>;
    using type = typename EvalRPN<List<new_num, StackRest...>, List<RestIn...>>::type;
};

// mul, two numbers needed
template<typename TL, TL VL, typename TR, TR VR, typename... StackRest, typename... RestIn>
struct EvalRPN<List<num<TR, VR>, num<TL, VL>, StackRest...>, List<op_mul, RestIn...>> {
    using new_num = num<decltype(VL * VR), VL * VR>;
    using type = typename EvalRPN<List<new_num, StackRest...>, List<RestIn...>>::type;
};

template<typename TL, TL VL, typename TR, TR VR, typename... StackRest, typename... RestIn>
struct EvalRPN<List<num<TR, VR>, num<TL, VL>, StackRest...>, List<op_div, RestIn...>> {
    // using the helper here
    using type = typename DivSelector<VR == 0, num<TL, VL>, num<TR, VR>, List<StackRest...>, List<RestIn...>>::type;
};

// precedences
template<typename Op> struct Prec { static constexpr int value = 0; };
template<> struct Prec<op_add> { static constexpr int value = 1; };
template<> struct Prec<op_sub> { static constexpr int value = 1; };
template<> struct Prec<op_mul> { static constexpr int value = 2; };
template<> struct Prec<op_div> { static constexpr int value = 2; };
template<> struct Prec<op_neg> { static constexpr int value = 3; };

template<typename In, typename OpStack, typename Out> struct ShuntingYard {using type = Error; };

// stack is empty, input is empty, just out
template<typename... OutOps>
struct ShuntingYard<List<>, List<>, List<OutOps...>> { using type = List<OutOps...>; };

// operations left in the stack -> out
template<typename TopOp, typename... RestOps, typename... OutOps>
struct ShuntingYard<List<>, List<TopOp, RestOps...>, List<OutOps...>> {
    using type = typename ShuntingYard<List<>, List<RestOps...>, List<OutOps..., TopOp>>::type;
};

// input is a number -> out
template<typename T, T V, typename... RestIn, typename... OpStackTs, typename... OutOps>
struct ShuntingYard<List<num<T, V>, RestIn...>, List<OpStackTs...>, List<OutOps...>> {
    using type = typename ShuntingYard<List<RestIn...>, List<OpStackTs...>, List<OutOps..., num<T, V>>>::type;
};

// input is ( -> push to operations
template<typename... RestIn, typename... OpStackTs, typename... OutOps>
struct ShuntingYard<List<open, RestIn...>, List<OpStackTs...>, List<OutOps...>> {
    using type = typename ShuntingYard<List<RestIn...>, List<open, OpStackTs...>, List<OutOps...>>::type;
};

// pop until ( 
template<typename In, typename OpStack, typename Out> struct PopUntilOpen { using type = Error; };

// if meet ( just pop it
template<typename In, typename... RestOps, typename... OutOps>
struct PopUntilOpen<In, List<open, RestOps...>, List<OutOps...>> {
    using type = typename ShuntingYard<In, List<RestOps...>, List<OutOps...>>::type;
};

// if meet operator -> out and look for (
template<typename In, typename TopOp, typename... RestOps, typename... OutOps>
struct PopUntilOpen<In, List<TopOp, RestOps...>, List<OutOps...>> {
    using type = typename PopUntilOpen<In, List<RestOps...>, List<OutOps..., TopOp>>::type;
};

// if meet ) -> helper
template<typename... RestIn, typename... OpStackTs, typename... OutOps>
struct ShuntingYard<List<close, RestIn...>, List<OpStackTs...>, List<OutOps...>> {
    using type = typename PopUntilOpen<List<RestIn...>, List<OpStackTs...>, List<OutOps...>>::type;
};

// process operators
template<typename Op, typename In, typename OpStack, typename Out> struct HandleOp;

// helper for HandleOp to avoid nested explicit specialization 
template<bool Pop, typename Op, typename In, typename OpStack, typename Out>
struct HandleOpSelector;

// pop TopOp -> out, Op waits
template<typename Op, typename In, typename TopOp, typename... RestOps, typename... OutOps>
struct HandleOpSelector<true, Op, In, List<TopOp, RestOps...>, List<OutOps...>> {
    using type = typename HandleOp<Op, In, List<RestOps...>, List<OutOps..., TopOp>>::type;
};

// push Op to stack 
template<typename Op, typename In, typename TopOp, typename... RestOps, typename... OutOps>
struct HandleOpSelector<false, Op, In, List<TopOp, RestOps...>, List<OutOps...>> {
    using type = typename ShuntingYard<In, List<Op, TopOp, RestOps...>, List<OutOps...>>::type;
};

// stack is empty -> push operator
template<typename Op, typename In, typename... OutOps>
struct HandleOp<Op, In, List<>, List<OutOps...>> {
    using type = typename ShuntingYard<In, List<Op>, List<OutOps...>>::type;
};

// stack is not empty -> compare precedence
template<typename Op, typename In, typename TopOp, typename... RestOps, typename... OutOps>
struct HandleOp<Op, In, List<TopOp, RestOps...>, List<OutOps...>> {
    // Pop if TopOp >= Op  
    static constexpr bool pop_it = Prec<TopOp>::value >= Prec<Op>::value;

    // using the helper here
    using type = typename HandleOpSelector<pop_it, Op, In, List<TopOp, RestOps...>, List<OutOps...>>::type;
};

// meeting operator -> helper
template<typename Op, typename... RestIn, typename... OpStackTs, typename... OutOps>
struct ShuntingYard<List<Op, RestIn...>, List<OpStackTs...>, List<OutOps...>> {
    using type = typename HandleOp<Op, List<RestIn...>, List<OpStackTs...>, List<OutOps...>>::type;
};

template<typename... Tokens>
struct expr {
    
    using rpn = typename ShuntingYard<List<Tokens...>, List<>, List<>>::type;

    using res = typename EvalRPN<List<>, rpn>::type;
    static constexpr auto eval() {
        static_assert(!std::is_same_v<res, Error>, "Evaluation error");
        return res::value;
    }
};

template<typename Expr> struct is_valid_expression { static constexpr bool value = false; };

template<typename... Tokens> struct is_valid_expression<expr<Tokens...>> {
    using rpn = typename ShuntingYard<List<Tokens...>, List<>, List<>>::type;
    using res = typename EvalRPN<List<>, rpn>::type;
    static constexpr bool value = !std::is_same_v<res, Error>;
};
template<typename T> struct TokenStr;
template<typename T, T V> struct TokenStr<num<T, V>> { static std::string get() { return std::to_string(V); } };
template<> struct TokenStr<op_add> { static std::string get() { return " + "; } };
template<> struct TokenStr<op_sub> { static std::string get() { return " - "; } };
template<> struct TokenStr<op_mul> { static std::string get() { return " * "; } };
template<> struct TokenStr<op_div> { static std::string get() { return " / "; } };
template<> struct TokenStr<op_neg> { static std::string get() { return "-"; } };
template<> struct TokenStr<open>   { static std::string get() { return "("; } };
template<> struct TokenStr<close>  { static std::string get() { return ")"; } };

template<typename... Tokens>
std::string to_string(expr<Tokens...>) {
    return (TokenStr<Tokens>::get() + ...);
}

int main() {
    using E1 = expr<num<int, 5>, op_add, num<int, 3>, op_mul, num<int, 2>>;
    static_assert(E1::eval() == 11);
    std::cout << "Test 1: " << to_string(E1{}) << " = " << E1::eval() << " (Expected: 11)\n";

    using E2 = expr<open, num<int, 5>, op_add, num<int, 3>, close, op_mul, num<int, 2>>;
    static_assert(E2::eval() == 16);
    std::cout << "Test 2: " << to_string(E2{}) << " = " << E2::eval() << " (Expected: 16)\n";

    using E3 = expr<num<double, 3.5>, op_mul, num<int, 2>>;
    static_assert(E3::eval() == 7.0);
    std::cout << "Test 3: " << to_string(E3{}) << " = " << E3::eval() << " (Expected: 7.0)\n";

    using E4 = expr<num<int, 10>, op_sub, open, num<int, 5>, op_add, op_neg, num<int, 2>, close>;
    static_assert(E4::eval() == 7);
    std::cout << "Test 4: " << to_string(E4{}) << " = " << E4::eval() << " (Expected: 7)\n";

    using E5 = expr<num<int, 10>, op_div, num<int, 0>>;
    static_assert(is_valid_expression<E5>::value == false);
    std::cout << "Test 5: " << to_string(E5{}) << " is valid? " << (is_valid_expression<E5>::value ? "Yes" : "No (Division by zero)") << "\n";

    using E6 = expr<open, num<int, 10>, close, close>;
    static_assert(is_valid_expression<E6>::value == false);
    std::cout << "Test 6: (10)) is valid? " << (is_valid_expression<E6>::value ? "Yes" : "No (Syntax error)") << "\n";

    return 0;
}