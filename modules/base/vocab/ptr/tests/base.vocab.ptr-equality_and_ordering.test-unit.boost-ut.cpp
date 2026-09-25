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
    // Equality and Ordering
    //============================================================

    "equality compares pointer identity"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        expect(that % std::equality_comparable<ConcretePtr<std::int32_t>>);

        std::int32_t x = 1;
        std::int32_t y = 1;

        const auto ptr1 = base::vocab::pointer_to<ConcretePtr, const std::int32_t>(x);
        const auto ptr2 = base::vocab::pointer_to<ConcretePtr>(x);
        const auto ptr3 = base::vocab::pointer_to<ConcretePtr>(y);

        expect(that % ptr1 == ptr2);
        expect(that % ptr1 != ptr3);
        expect(that % ptr2 != ptr3);
    } | pointers_to_test;

    "nullable comparisons with nullptr"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        if constexpr (pointer_test_traits<ConcretePtr>::allows_pointer_binding && pointer_test_traits<ConcretePtr>::is_nullable) {
            std::int32_t value{};

            const ConcretePtr<std::int32_t> bound{std::addressof(value)};
            const ConcretePtr<std::int32_t> null{nullptr};

            //Expecting both operator== and operator!= to be synthesized correctly
            expect(that % !(bound == nullptr));
            expect(that % !(nullptr == bound));

            expect(that % (bound != nullptr));
            expect(that % (nullptr != bound));

            expect(that % (null == nullptr));
            expect(that % (nullptr == null));

            expect(that % !(null != nullptr));
            expect(that % !(nullptr != null));
        }
    } | pointers_to_test;

    "null pointers of same pointer type compare equal"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
            const ConcretePtr<std::int32_t> lhs{nullptr};
            const ConcretePtr<const std::int32_t> rhs{nullptr};

            expect(that % (lhs == rhs));
            expect(that % !(lhs != rhs));
        }
    } | pointers_to_test;

    "ordering comparisons according to policy"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        expect(eq(std::three_way_comparable<ConcretePtr<std::int32_t>>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
        expect(eq(std::three_way_comparable_with<ConcretePtr<std::int32_t>, std::int32_t*>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
        expect(eq(std::three_way_comparable<ConcretePtr<base_type>>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
        expect(eq(std::three_way_comparable_with<ConcretePtr<base_type>, ConcretePtr<derived_type>>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
        expect(eq(std::three_way_comparable_with<ConcretePtr<base_type>, derived_type*>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
        expect(eq(std::three_way_comparable_with<base_type*, ConcretePtr<derived_type>>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
    } | pointers_to_test;
}
