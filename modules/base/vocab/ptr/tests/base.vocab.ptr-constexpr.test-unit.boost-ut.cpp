// SPDX-License-Identifier: Apache-2.0
// Parameterized unit tests for base.vocab.ptr

import base.vocab.ptr;
import boost.ut;
import std;

import base.meta.concepts;

#include "base.vocab.ptr-common_fixtures.boost-ut.hpp"

using namespace boost::ext::ut;

//NOLINTNEXTLINE(bugprone-exception-escape): Test framework.
int main()
{
    //============================================================
    // Constant Expression Usage
    //============================================================

    "constexpr construction and dereference"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        static constexpr std::int32_t value = 42;

        constexpr auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

        expect(eq(*ptr, value));
    } | pointers_to_test;

    //NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access, cppcoreguidelines-pro-bounds-array-to-pointer-decay): Testing pointer arithmetic and indexing operations.
    "constexpr arithmetic"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        if constexpr (pointer_test_traits<ConcretePtr>::has_arithmetic_traversal) {
            //NOLINTNEXTLINE(readability-magic-numbers, modernize-avoid-c-arrays): Test fixture.
            static constexpr std::int32_t values[] = {2, 4, 6};

            constexpr auto ptr = base::vocab::pointer_to<ConcretePtr>(values[0]);

            constexpr auto next = ptr + 1;
            expect(eq(*next, values[1]));
        }
    } | pointers_to_test;
    //NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access, cppcoreguidelines-pro-bounds-array-to-pointer-decay)

    "constexpr get and boolean conversion"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) {
        static constexpr std::int32_t value = 7;

        constexpr auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

        expect(eq(ptr.get(), std::addressof(value)));
        expect(that % static_cast<bool>(ptr) == true);
    } | pointers_to_test;

    "constexpr equality"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) {
        static constexpr std::int32_t value = 11;

        constexpr auto ptr1 = base::vocab::pointer_to<ConcretePtr>(value);
        constexpr auto ptr2 = base::vocab::pointer_to<ConcretePtr>(value);

        expect(that % ptr1 == ptr2);
    } | pointers_to_test;

    "constexpr rebinding"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) {
        static constexpr std::int32_t value1 = 1;
        static constexpr std::int32_t value2 = 2;

        constexpr auto rebound = std::invoke([] {
            auto ptr = base::vocab::pointer_to<ConcretePtr>(value1);
            ptr      = ConcretePtr{value2};
            return ptr;
        });

        expect(eq(*rebound, value2));
        expect(eq(rebound.get(), std::addressof(value2)));
    } | pointers_to_test;

    "constexpr swap"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) {
        static constexpr std::int32_t value1 = 1;
        static constexpr std::int32_t value2 = 2;

        constexpr auto swapped = std::invoke([] {
            auto lhs = base::vocab::pointer_to<ConcretePtr>(value1);
            auto rhs = base::vocab::pointer_to<ConcretePtr>(value2);

            using std::swap;
            swap(lhs, rhs);

            return std::pair{lhs, rhs};
        });

        expect(eq(*swapped.first, value2));
        expect(eq(*swapped.second, value1));
    } | pointers_to_test;
}
