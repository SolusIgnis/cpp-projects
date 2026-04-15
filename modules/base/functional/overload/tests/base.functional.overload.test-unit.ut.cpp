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

import ut;
import std;

using namespace ut;
using namespace std::literals;
using base::functional::overload;

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
    
    "overload{...} preserves ambiguity across multiple aggregated callables"_test = [] mutable {
        struct f1 {
            auto operator()(int)    { return "f1 int"s; }
            auto operator()(double) { return "f1 double"s; }
        };
    
        struct f2 {
            auto operator()(int)         { return "f2 int"s; }
            auto operator()(std::string) { return "f2 string"s; }
        };
    
        auto overloaded = overload{f1{},
                                   f2{},
                                   [](double num){ return std::format("lambda double {}", num); }};
    
        expect(eq(std::invocable<decltype(overloaded), int>, false));
        expect(eq(std::invocable<decltype(overloaded), double>, false));
        expect(eq(std::invocable<decltype(overloaded), std::string>, true));
    };
};

int main() {}
