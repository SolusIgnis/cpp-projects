// SPDX-License-Identifier: Apache-2.0
// Unit tests for base.functional.overload 

/*
 * NOTE: This is probably "overtesting". The implementation of the system under test is trivial and
 *       idiomatic. It is more clearly "correct by inspection" than the tests. However, there is
 *       still value in demonstrating correctness, serving as an executable specification, and
 *       guarding against regressions. The tests here ensure that no "clever" changes inadvertently
 *       interfere with normal overload resolution.
 */

import base.functional.overload;

import base.vocab.ptr;
import ut;
import std;

using namespace ut;
using namespace std::literals;
using base::functional::overload;
using base::vocab::alias_ptr;

suite overload_tests = [] mutable {
    "overload{...} produces an invocable object"_test = [] mutable {
        constexpr int expected = 42;
        
        auto overloaded = overload{[](int i){ return i; }};
        
        auto result = std::invoke(overloaded, expected);
        expect(eq(result, expected));
    };
    
    "overload{...} produces a resolvable overload set"_test = [] mutable {
        constexpr int expected_int = 42;
        
        constexpr std::string_view expected_sv = "42"sv;
        
        auto overloaded = overload{
                              [](int i){ return i; },
                              [](std::string_view str_v){ return str_v; }
                          };
        
        auto result_int = overloaded(expected_int);
        expect(eq(result_int, expected_int));
        
        auto result_sv = overloaded(expected_sv);
        expect(eq(result_sv, expected_sv));
    };
    
    "overload{...} resolves by arity"_test = [] mutable {
        constexpr int arg1 = 40;
        constexpr int arg2 = 2;
        constexpr int expected1 = -arg1;
        constexpr int expected2 = arg1 + arg2;
        
        auto overloaded = overload{
                              [](auto i, auto j){ return i + j; },
                              [](auto i){ return -i; }
                          };
        int result2 = overloaded(arg1, arg2);
        expect(eq(result2, expected2));
        
        int result1 = overloaded(arg1);
        expect(eq(result1, expected1));
    };
    
    "overload{...} composes overload sets of multiple multi-overload bases"_test = [] mutable {
        struct f1 {
            std::string operator()(int)    { return "int"s; }
            std::string operator()(double) { return "double"s; }
        };
    
        struct f2 {
            std::string operator()(std::string)      { return "string"s; }
            std::string operator()(std::string_view) { return "string view"s; }
        };
    
        auto overloaded = overload{f1{}, f2{}};
    
        expect(eq(overloaded(1), "int"s));
        expect(eq(overloaded(3.14), "double"s));
        expect(eq(overloaded("hello"s), "string"s));
        expect(eq(overloaded("view"sv), "string view"s));
    };
    
    "overload{...} preserves value category"_test = [] mutable {
        struct functor {
            enum class val_cat : std::uint8_t { lval, clval, rval };
            
            std::uint8_t operator()(this       functor&)  { return std::to_underlying(val_cat::lval); }
            std::uint8_t operator()(this const functor&)  { return std::to_underlying(val_cat::clval); }
            std::uint8_t operator()(this       functor&&) { return std::to_underlying(val_cat::rval); }
            std::uint8_t operator()(this const functor&&) = delete;
        };
    
        auto overloaded = overload{functor{}};
        const auto const_overloaded = overloaded;
    
        using enum functor::val_cat;
        expect(eq(overloaded(), std::to_underlying(lval)));
        expect(eq(const_overloaded(), std::to_underlying(clval)));
        expect(eq(std::move(overloaded)(), std::to_underlying(rval)));
    };
    
    "overload{...} integrates with std::visit"_test = [] mutable {
        using var_t = std::variant<int, std::string, double>;
        
        auto visitor = overload{
            [](int)                { return "int"s; },
            [](const std::string&) { return "string"s; },
            [](double)             { return "double"s; }
        };

        expect(eq(std::visit(visitor, var_t{42}), "int"s));
        expect(eq(std::visit(visitor, var_t{"hello"s}), "string"s));
        expect(eq(std::visit(visitor, var_t{3.14}), "double"s));
    };
    
    "overload{...} preserves ambiguity across identical signatures"_test = [] {
        auto overloaded = overload{[](int){ return 1; },
                                   [](int){ return 2; }};
    
        expect(eq(std::invocable<decltype(overloaded), int>, false));
    };
    
    "overload{...} preserves ambiguity and overload ranking across multiple aggregated callables"_test = [] mutable {
        struct f1 {
            auto operator()(int)         { return "f1 int"s; }
            auto operator()(double)      { return "f1 double"s; }
            auto operator()(const char*) { return "f1 const char*"s; }
        };
    
        struct f2 {
            auto operator()(int)         { return "f2 int"s; }
            auto operator()(std::string) { return "f2 string"s; }
        };
    
        auto overloaded = overload{
                              f1{},
                              f2{},
                              [](double) mutable { return "lambda double"s; }, //mutable => non-const operator()
                              [](const char*)    { return "lambda const char*"s; }    //const operator()
                          };
    
        // ambiguous: f1(int) vs f2(int)
        expect(eq(std::invocable<decltype(overloaded), int>, false));
        
        // ambiguous: f1(double) vs lambda(double) [both non-const]
        expect(eq(std::invocable<decltype(overloaded), double>, false));
        
        // unambiguous: only f2(std::string) [const char* is not a match]
        expect(eq(std::invocable<decltype(overloaded), std::string>, true));
        expect(eq(std::invoke(overloaded, "std::string"s), "f2 string"s));
        
        // unambiguous: 1) non-const f1 beats const lambda [better implicit object parameter binding],
        //              2) and f1(const char*) beats f2(std::string) [conversion is a worse match]
        expect(eq(std::invocable<decltype(overloaded), const char*>, true));
        expect(eq(std::invoke(overloaded, "c-string"), "f1 const char*"s));
    };
    
    "overload{...} with deduced `this` lambda sees derived object identity"_test = [] mutable {
        auto overloaded = overload{
            []<typename SelfT>(this SelfT&&, auto) {
                if constexpr (std::is_const_v<std::remove_reference_t<SelfT>>) {
                    return "const"s;
                } else {
                    return "non-const"s;
                }
            },
            [](this const auto&, std::string arg) { return arg; }
        };

        const auto& const_ov = overloaded;

        // Deduced `self` reflects the `const`-ness of the `overload` object (derived type).
        expect(eq(overloaded(0), "non-const"s));
        expect(eq(const_ov(0), "const"s));

        // Explicit `std::string` parameter is a better match than template parameter
        expect(eq(const_ov("foo"s), "foo"s));

        // Template wins: better object parameter binding (non-`const` vs `const`) outweighs non-template preference
        expect(eq(overloaded("foo"s), "non-const"s));
    };
    
    "overload{...} supports simple recursion"_test = [] mutable  {
        auto factorial_tester = [](int n, int expected) {
            // fail fast for ill-formed test.
            if (n < 1) throw std::logic_error("factorial test runner requires n >= 1");

            int steps = 0;
            
            auto factorial = overload{
                [&steps](this auto& self, int n) -> int {
                    ++steps;
                    if (n <= 1) return 1;
                    return n * self(n - 1);
                }
            };
            
            expect(eq(factorial(n), expected));
            expect(eq(steps, n));
        };
        
        constexpr int num1 = 5;
        constexpr int expected1 = 120; // 5 * 4 * 3 * 2 * 1
        factorial_tester(num1, expected1);
    
        constexpr int num2 = 4;
        constexpr int expected2 = 24; // 4 * 3 * 2 * 1
        factorial_tester(num2, expected2);
        
        constexpr int num3 = 1;
        constexpr int expected3 = 1; // sanity check
        factorial_tester(num3, expected3);
    };

    "overload{...} supports recursive multi-overload dispatch (binary tree)"_test = [] mutable {
        struct node {
            std::variant<int, std::tuple<alias_ptr<node>, alias_ptr<node>>> value;
        };
        
        auto tree_sum = overload{
            [](int val){ return val; },
            []<typename T>(this auto& self, alias_ptr<T> ptr) {
                if (!ptr) throw std::logic_error("test tree node holds null pointer");
                return self(*ptr);
            },
            []<typename T>(this auto& self, std::tuple<T, T> children) {
                auto [left, right] = children;
                return self(left) + self(right);
            },
            [](this auto& self, node next) {
                return std::visit(self, next.value);
            }
        };
        
        int i = 0;
        
        node leaf1{++i};
        node leaf2{++i};
        node leaf3{++i};
        node leaf4{++i};
        node leaf5{++i};
        
        node branch1{std::tuple{&leaf2, &leaf3}};
        node branch2{std::tuple{&leaf1, &branch1}};
        node branch3{std::tuple{&leaf4, &leaf5}};
        
        node tree{std::tuple{&branch2, &branch3}};
        
        const int expected = (i * (i + 1)) / 2;
        expect(eq(std::visit(tree_sum, tree.value), expected));
    };
};

int main() {}
