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

namespace {
    //NOLINTNEXTLINE(bugprone-throwing-static-initialization, cppcoreguidelines-avoid-non-const-global-variables): Test framework.
    suite overload_tests = [] mutable {
        //NOLINTBEGIN(performance-unnecessary-value-param): Value categories are selected for overload resolution testing.
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
                [](std::string_view /*unused*/) { return "string view"s; }
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

            expect(eq(std::invocable<decltype(overloaded), int>, false));
        };

        "overload{...} preserves ambiguity and overload ranking across multiple composed and aggregated callables"_test =
            [] mutable {
                struct fobj1 {
                    auto operator()(std::int32_t /*unused*/) { return "fobj1 int"s; }
                    auto operator()(double /*unused*/) { return "fobj1 double"s; }
                    auto operator()(const char* /*unused*/) { return "fobj1 const char*"s; }
                };

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
                // ambiguous: fobj1(int) vs fobj2(int)
                expect(eq(std::invocable<decltype(overloaded), std::int32_t>, false));

                // ambiguous: fobj1(double) vs lambda(double) [both non-const]
                expect(eq(std::invocable<decltype(overloaded), double>, false));

                // unambiguous: only fobj2(std::string) [const char* is not a match]
                expect(eq(std::invocable<decltype(overloaded), std::string>, true));
                expect(eq(std::invoke(overloaded, "std::string"s), "fobj2 string"s));

                // unambiguous: 1) non-const f1 beats const lambda [better implicit object parameter binding],
                // 2) and f1(const char*) beats f2(std::string) [conversion is a worse match]
                expect(eq(std::invocable<decltype(overloaded), const char*>, true));
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

            // Explicit `std::string` parameter is a better match than template parameter
            expect(eq(const_ov("foo"s), "foo"s));

            // Template wins: better object parameter binding (non-`const` vs `const`) outweighs non-template preference
            expect(eq(overloaded("foo"s), "non-const"s));
            //NOLINTEND(bugprone-argument-comment)
        };

        "overload{...} supports simple recursion"_test = [] mutable {
            const auto factorial_tester = [](std::int32_t n, std::int32_t expected) {
                // fail fast for ill-formed test.
                if (n < 1) {
                    throw std::logic_error("factorial test runner requires n >= 1");
                }

                std::int32_t steps = 0;

                const auto factorial = overload{
                    [&steps](this auto& self, std::int32_t n) -> std::int32_t {
                        ++steps;
                        if (n <= 1) {
                            return 1;
                        }
                        return n * self(n - 1);
                    },
                };

                expect(eq(factorial(n), expected));
                expect(eq(steps, n));
            };

            constexpr std::int32_t num1      = 5;
            constexpr std::int32_t expected1 = 120; // 5 * 4 * 3 * 2 * 1
            factorial_tester(num1, expected1);

            constexpr std::int32_t num2      = 4;
            constexpr std::int32_t expected2 = 24; // 4 * 3 * 2 * 1
            factorial_tester(num2, expected2);

            constexpr std::int32_t num3      = 1;
            constexpr std::int32_t expected3 = 1; // sanity check
            factorial_tester(num3, expected3);
        };

        "overload{...} supports composed recursive multi-overload dispatch/visitation (binary tree)"_test = [] mutable {
            // Gauss Summation Formula
            const auto sum_to = [](std::int32_t n) { return (n * (n + 1)) / 2; };

            // Recursive data structure defining a binary tree by its nodes
            struct node {
                std::variant<int, std::tuple<alias_ptr<node>, alias_ptr<node>>> value;
            };

            // Reusable recursive traversal component
            // Note: Leaf handling is NOT part of the traversal overload set.
            const auto tree_traverse = overload{
                // Pointer: Unwrap any pointers (safely).
                []<typename T>(this auto& self, alias_ptr<T> ptr) -> std::int32_t {
                    if (!ptr) {
                        throw std::logic_error("test tree node holds null pointer");
                    }
                    return self(*ptr);
                },
                // Branch: Sum the values of both children recursively.
                []<typename T>(this auto& self, const std::tuple<T, T>& children) -> std::int32_t {
                    auto [left, right] = children;
                    return self(left) + self(right);
                },
                // Node: Visit the value of a node to dispatch into a leaf or recurse into a branch.
                [](this auto& self, const node& tree_node) -> std::int32_t { return std::visit(self, tree_node.value); },
            };

            // Compose value summation leaf handling with reusable traversal component
            const auto tree_sum = overload{[](std::int32_t val) -> std::int32_t { return val; }, tree_traverse};

            // Compose fixed (counting) summation leaf handling with reusable traversal component
            const auto tree_count = overload{[](int) -> std::int32_t { return 1; }, tree_traverse};

            // Initialize the tree with the `i`th counting number for each leaf.
            std::int32_t i = 0;

            node leaf1{++i};
            node leaf2{++i};
            node leaf3{++i};
            node leaf4{++i};
            node leaf5{++i};

            node branch1{
                std::tuple{&leaf1, &leaf2},
            };
            node branch2{
                std::tuple{&leaf3, &branch1},
            };
            node branch3{
                std::tuple{&leaf4, &leaf5},
            };

            node tree{
                std::tuple{&branch2, &branch3},
            };

            // There should thus be `i` leaf nodes, and their sum the sum of the first `i` counting numbers.
            expect(eq(tree_sum(tree), sum_to(i)));
            expect(eq(tree_count(tree), i));
        };
        //NOLINTEND(performance-unnecessary-value-param)
    };
} //namespace

int main() {}
