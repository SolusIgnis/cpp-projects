// SPDX-License-Identifier: Apache-2.0
// Parameterized unit tests for base.vocab.ptr

import base.vocab.ptr;
import boost.ut;
import std;

import base.meta.concepts;

#include "base.vocab.ptr-common_fixtures.boost-ut.hpp"

using namespace boost::ext::ut;

namespace {
    template<typename T>
    concept has_addition = requires(T t) { t + 1; } || requires(T t) { 1 + t; };

    template<typename T>
    concept has_subtraction = requires(T t) { t - 1; };

    template<typename T>
    concept has_difference = requires(T t) { t - t; };

    template<typename T>
    concept has_pre_increment = requires(T t) { ++t; };

    template<typename T>
    concept has_post_increment = requires(T t) { t++; };

    template<typename T>
    concept has_pre_decrement = requires(T t) { --t; };

    template<typename T>
    concept has_post_decrement = requires(T t) { t--; };
} //namespace

//NOLINTNEXTLINE(bugprone-exception-escape): Test framework.
int main()
{
    //============================================================
    // Arithmetic operations
    //============================================================

    "pointer arithmetic operations according to policy"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        using t = ConcretePtr<std::int32_t>;

        expect(eq(has_addition<t>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
        expect(eq(has_subtraction<t>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
        expect(eq(has_difference<t>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
        expect(eq(has_pre_increment<t>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
        expect(eq(has_post_increment<t>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
        expect(eq(has_pre_decrement<t>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
        expect(eq(has_post_decrement<t>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
    } | pointers_to_test;

    "input_or_output_iterator according to policy"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        //note: all other iterator concepts subsume this one and thus are implicitly false when it is false
        expect(eq(std::input_or_output_iterator<ConcretePtr<std::int32_t>>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
    } | pointers_to_test;

    //============================================================
    // Traversal identity
    //============================================================

    //NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access, cppcoreguidelines-pro-bounds-array-to-pointer-decay): Testing pointer arithmetic and indexing operations.
    "pointer arithmetic preserves native traversal semantics"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        if constexpr (pointer_test_traits<ConcretePtr>::has_arithmetic_traversal) {
            //NOLINTNEXTLINE(readability-magic-numbers, modernize-avoid-c-arrays): Test fixture.
            constexpr std::int32_t values[] = {10, 20, 30, 40};

            constexpr std::ptrdiff_t step = 2;

            const auto ptr = base::vocab::pointer_to<ConcretePtr>(values[0]);

            const auto advanced = ptr + step;
            auto clone          = ptr;

            expect(eq(clone, ptr));
            expect(neq(clone, advanced));

            clone += step;

            expect(neq(clone, ptr));
            expect(eq(clone, advanced));

            expect(eq(*advanced, values[step]));
            expect(eq(advanced.get(), values + step));
            expect(that % ptr < advanced);
            expect(that % advanced > ptr);
            expect(that % advanced >= (values + (step / 2)));
        }
    } | pointers_to_test;

    "difference matches raw pointer semantics"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        if constexpr (pointer_test_traits<ConcretePtr>::has_arithmetic_traversal) {
            //NOLINTNEXTLINE(readability-magic-numbers, modernize-avoid-c-arrays): Test fixture.
            std::int32_t values[] = {10, 20, 30, 40};

            constexpr std::ptrdiff_t first_index = 0;
            constexpr std::ptrdiff_t last_index  = 3;

            const auto first = base::vocab::pointer_to<ConcretePtr>(values[first_index]);
            const auto last  = base::vocab::pointer_to<ConcretePtr>(values[last_index]);

            expect(eq(last - first, last_index - first_index));
        }
    } | pointers_to_test;

    //NOLINTBEGIN(bugprone-argument-comment): Matchers lhs/rhs.
    "increment and decrement traverse correctly"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        if constexpr (pointer_test_traits<ConcretePtr>::has_arithmetic_traversal) {
            //NOLINTNEXTLINE(readability-magic-numbers, modernize-avoid-c-arrays): Test fixture.
            std::int32_t values[] = {1, 2, 3};

            auto ptr = base::vocab::pointer_to<ConcretePtr>(values[0]);

            ++ptr;
            expect(eq(*ptr, 2));

            ptr++;
            expect(eq(*ptr, 3));

            --ptr;
            expect(eq(*ptr, 2));

            ptr--;
            expect(eq(*ptr, 1));
        }
    } | pointers_to_test;
    //NOLINTEND(bugprone-argument-comment)

    "subscript matches raw pointer indexing"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        if constexpr (pointer_test_traits<ConcretePtr>::has_arithmetic_traversal) {
            //NOLINTNEXTLINE(readability-magic-numbers, modernize-avoid-c-arrays): Test fixture.
            std::int32_t values[] = {5, 6, 7, 8};

            const auto ptr = base::vocab::pointer_to<ConcretePtr>(values[0]);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
            expect(eq(ptr[0], values[0]));
            expect(eq(ptr[1], values[1]));
            expect(eq(ptr[2], values[2]));
#pragma GCC diagnostic pop
        }
    } | pointers_to_test;

    "mixed raw and cursor arithmetic produce identical addresses"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        if constexpr (pointer_test_traits<ConcretePtr>::has_arithmetic_traversal) {
            //NOLINTNEXTLINE(readability-magic-numbers, modernize-avoid-c-arrays): Test fixture.
            std::int32_t values[] = {1, 2, 3, 4};

            const auto ptr = base::vocab::pointer_to<ConcretePtr>(values[0]);

            expect(eq((ptr + 3).get(), values + 3));
            expect(eq((3 + ptr).get(), values + 3));
        }
    } | pointers_to_test;
    //NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
}
