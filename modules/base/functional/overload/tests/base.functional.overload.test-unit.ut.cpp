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
    
    "overload{...} resolves by arity"_test_test = [] mutable {
        constexpr int arg1 = 40;
        constexpr int arg2 = 2;
        constexpr int expected1 = -arg1;
        constexpr int expected2 = arg1 + arg2;
        
        constexpr std::string_view expected_sv = "42"sv;
        
        auto overloaded = overload{
                              [](auto i, auto j){ return i + j; },
                              [](auto i){ return -i; }
                          };
        int result2 = overloaded(arg1, arg2);
        expect(eq(result2, expected2));
        
        int result1 = overloaded(arg1);
        expect(eq(result1, expected1));
    };
    
    "overload{...} preserves value category"_test = [] mutable {
        struct functor {
            enum class val_cat : std::uint8_t { lval, clval, rval };
            
            int operator()(this       functor&)  { return std::to_underlying(val_cat::lval); }
            int operator()(this const functor&)  { return std::to_underlying(val_cat::clval); }
            int operator()(this       functor&&) { return std::to_underlying(val_cat::rval); }
            int operator()(this const functor&&) = delete;
        };
    
        auto overloaded = overload{functor{}};
        const auto const_overloaded = overloaded;
    
        using enum functor::val_cat;
        expect(eq(overloaded(), std::to_underlying(lval)));
        expect(eq(const_overloaded(), std::to_underlying(clval)));
        expect(eq(std::move(overloaded)(), std::to_underlying(rval)));
    };
    
    "base.functional.overload example 1"_test = [] mutable {
        //NOLINTBEGIN(readability-magic-numbers)
        std::variant<int, float, std::string> v = "Hello World";
    
        std::visit(overload {
            [](int i) { std::cout << "Integer: " << i << "\n"; },
            [](float f) { std::cout << "Float: " << f << "\n"; },
            [](const std::string& s) { std::cout << "String: " << s << "\n"; }
        }, v);
        //NOLINTEND(readability-magic-numbers)
    };
};

int main() {}
