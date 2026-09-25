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
using base::vocab::alias_ptr;

//NOLINTNEXTLINE(bugprone-exception-escape): Test framework.
int main()
{
    //NOLINTBEGIN(performance-unnecessary-value-param, performance-move-const-arg): Value categories are selected for overload resolution testing.
    "overload{...} supports simple recursion"_test = [] mutable {
        constexpr auto test_arguments = std::array{
            std::pair{5, 120}, // 5 * 4 * 3 * 2 * 1
            std::pair{4,  24}, // 4 * 3 * 2 * 1
            std::pair{1,   1}, // sanity check
        };

        should("recurse") = [](auto test_parameter) {
            const auto [num, expected] = test_parameter;
            // fail fast for ill-formed test.
            if (num < 1) {
                throw std::logic_error("factorial test runner requires num >= 1");
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

            expect(eq(factorial(num), expected)) << std::format("factorial({}) result", num);
            expect(eq(steps, num)) << "step count";
        } | test_arguments;
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

        const node tree{
            std::tuple{&branch2, &branch3},
        };

        // There should thus be `i` leaf nodes, and their sum the sum of the first `i` counting numbers.
        expect(eq(tree_sum(tree), sum_to(i))) << "sum";
        expect(eq(tree_count(tree), i)) << "leaf node count";
    };
    //NOLINTEND(performance-unnecessary-value-param, performance-move-const-arg)
}
