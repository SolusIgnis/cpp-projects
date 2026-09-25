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
import boost.ut;
import std;

using namespace boost::ext::ut;
using namespace std::literals;
using base::functional::overload;

//NOLINTNEXTLINE(bugprone-exception-escape): Test framework.
int main()
{
    //NOLINTBEGIN(performance-unnecessary-value-param, performance-move-const-arg): Value categories are selected for overload resolution testing.
    "overload{...} produces an invocable object"_test = [] mutable {
        constexpr std::int32_t expected = 42;

        const auto overloaded = overload{[](std::int32_t n) { return n; }};

        const auto result = std::invoke(overloaded, expected);
        expect(eq(result, expected));
    };

    "overload{...} produces a resolvable overload set"_test = [] mutable {
        constexpr std::int32_t expected_int = 42;

        constexpr std::string_view expected_sv = "42"sv;

        const auto overloaded = overload{[](std::int32_t n) { return n; }, [](std::string_view str_v) { return str_v; }};

        const auto result_int = overloaded(expected_int);
        expect(eq(result_int, expected_int));

        const auto result_sv = overloaded(expected_sv);
        expect(eq(result_sv, expected_sv));
    };

    "overload{...} resolves by arity"_test = [] mutable {
        constexpr std::int32_t arg1      = 40;
        constexpr std::int32_t arg2      = 2;
        constexpr std::int32_t expected1 = -arg1;
        constexpr std::int32_t expected2 = arg1 + arg2;

        const auto overloaded = overload{[](auto n, auto x) { return n + x; }, [](auto n) { return -n; }};

        const std::int32_t result1 = overloaded(arg1);
        expect(eq(result1, expected1));

        const std::int32_t result2 = overloaded(arg1, arg2);
        expect(eq(result2, expected2));
    };

    "overload{...} composes overload sets of multiple multi-overload bases"_test = [] mutable {
        struct fobj1 {
            std::string operator()(std::int32_t /*unused*/) { return "int"s; }
            std::string operator()(double /*unused*/) { return "double"s; }
        };

        struct fobj2 {
            std::string operator()(std::string /*unused*/) { return "string"s; }
            std::string operator()(std::string_view /*unused*/) { return "string view"s; }
        };

        auto overloaded = overload{fobj1{}, fobj2{}};

        //NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers): The types matter, but the values don't.
        //NOLINTBEGIN(bugprone-argument-comment): Matcher lhs/rhs.
        expect(eq(overloaded(1), "int"s));
        expect(eq(overloaded(3.14), "double"s));
        expect(eq(overloaded("hello"s), "string"s));
        expect(eq(overloaded("view"sv), "string view"s));
        //NOLINTEND(bugprone-argument-comment)
        //NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    };

    "overload{ overload{...}, ... } composes overload sets"_test = [] {
        const auto base = overload{[](int /*unused*/) { return "int"s; }, [](double /*unused*/) { return "double"s; }};

        const auto extended = overload{
            base,
            [](std::string /*unused*/) { return "string"s; },
            [](std::string_view /*unused*/) { return "string view"s; },
        };

        //NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers): The types matter, but the values don't.
        //NOLINTBEGIN(bugprone-argument-comment): Matcher lhs/rhs.
        expect(eq(extended(1), "int"s));
        expect(eq(extended(3.14), "double"s));
        expect(eq(extended("hello"s), "string"s));
        expect(eq(extended("view"sv), "string view"s));
        //NOLINTEND(bugprone-argument-comment)
        //NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    };

    "overload{...} preserves value category"_test = [] mutable {
        struct function_obj {
            enum class val_cat : std::uint8_t {
                lval,
                clval,
                rval,
            };

            std::uint8_t operator()(this function_obj& /*unused*/) { return std::to_underlying(val_cat::lval); }
            std::uint8_t operator()(this const function_obj& /*unused*/) { return std::to_underlying(val_cat::clval); }
            std::uint8_t operator()(this function_obj&& /*unused*/) { return std::to_underlying(val_cat::rval); }
            std::uint8_t operator()(this const function_obj&&) = delete;
        };

        auto overloaded             = overload{function_obj{}}; //NOLINT(misc-const-correctness)
        const auto const_overloaded = overloaded;

        using enum function_obj::val_cat;
        expect(eq(overloaded(), std::to_underlying(lval)));
        expect(eq(const_overloaded(), std::to_underlying(clval)));
        expect(eq(std::move(overloaded)(), std::to_underlying(rval)));
    };

    "overload{...} integrates with std::visit"_test = [] mutable {
        using var_t = std::variant<int, std::string, double>;

        const auto visitor = overload{
            [](int) { return "int"s; },
            [](const std::string&) { return "string"s; },
            [](double) { return "double"s; },
        };

        //NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers): The types matter, but the values don't.
        //NOLINTBEGIN(bugprone-argument-comment): Matcher lhs/rhs.
        expect(eq(std::visit(visitor, var_t{42}), "int"s));
        expect(eq(std::visit(visitor, var_t{"hello"s}), "string"s));
        expect(eq(std::visit(visitor, var_t{3.14}), "double"s));
        //NOLINTEND(bugprone-argument-comment)
        //NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    };

    "overload{...} preserves ambiguity across identical signatures"_test = [] {
        const auto overloaded = overload{[](int /*unused*/) { return 1; }, [](int /*unused*/) { return 2; }};

        expect(that % !std::invocable<decltype(overloaded), int>) << "overloaded is not invocable with an int";
    };

    "overload{...} preserves ambiguity and overload ranking across multiple composed and aggregated callables"_test = [] mutable {
        struct fobj1 {
            auto operator()(std::int32_t /*unused*/) { return "fobj1 int"s; }
            auto operator()(double /*unused*/) { return "fobj1 double"s; }
            auto operator()(const char* /*unused*/) { return "fobj1 const char*"s; }
        };

        //NOLINTNEXTLINE(misc-const-correctness): Non-const to test overload resolution.
        auto fobj2 = overload{
            [](int /*unused*/) mutable { return "fobj2 int"s; },            //mutable => non-const operator()
            [](std::string /*unused*/) mutable { return "fobj2 string"s; }, //mutable => non-const operator()
        };

        auto overloaded = overload{
            fobj1{},
            fobj2,
            [](double /*unused*/) mutable { return "lambda double"s; },   //mutable => non-const operator()
            [](const char* /*unused*/) { return "lambda const char*"s; }, //const operator()
        };

        //NOLINTBEGIN(bugprone-argument-comment): Matcher lhs/rhs.
        expect(that % !std::invocable<decltype(overloaded), std::int32_t>) << "ambiguous: fobj1(int) vs fobj2(int)";

        expect(that % !std::invocable<decltype(overloaded), double>) << "ambiguous: fobj1(double) vs lambda(double) [both non-const]";

        expect(that % std::invocable<decltype(overloaded), std::string>) << "unambiguous: only fobj2(std::string) [const char* is not a match]";
        expect(eq(std::invoke(overloaded, "std::string"s), "fobj2 string"s));

        expect(that % std::invocable<decltype(overloaded), const char*>)
            << "unambiguous: 1) non-const fobj1 beats const lambda [better implicit object parameter binding], 2) and fobj1(const char*) beats fobj2(std::string) [conversion is a worse match]";
        expect(eq(std::invoke(overloaded, "c-string"), "fobj1 const char*"s));
        //NOLINTEND(bugprone-argument-comment)
    };

    "overload{...} with deduced `this` lambda sees derived object identity"_test = [] mutable {
        //NOLINTNEXTLINE(misc-const-correctness)
        auto overloaded = overload{
            []<typename SelfT>(this SelfT&&, auto) {
                if constexpr (std::is_const_v<std::remove_reference_t<SelfT>>) {
                    return "const"s;
                } else {
                    return "non-const"s;
                }
            },
            [](this const auto&, std::string arg) { return arg; },
        };

        const auto& const_ov = overloaded;

        //NOLINTBEGIN(bugprone-argument-comment): Matcher lhs/rhs.
        // Deduced `self` reflects the `const`-ness of the `overload` object (derived type).
        expect(eq(overloaded(0), "non-const"s));
        expect(eq(const_ov(0), "const"s));

        expect(eq(const_ov("foo"s), "foo"s)) << "Explicit `std::string` parameter is a better match than template parameter";
        expect(eq(overloaded("foo"s), "non-const"s))
            << "Template wins: better object parameter binding (non-`const` vs `const`) outweighs non-template preference";
        //NOLINTEND(bugprone-argument-comment)
    };
    //NOLINTEND(performance-unnecessary-value-param, performance-move-const-arg)
}
