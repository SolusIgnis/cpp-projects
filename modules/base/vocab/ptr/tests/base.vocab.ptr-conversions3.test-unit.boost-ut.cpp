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
    // Conversions
    //============================================================

    "implicit conversion to raw pointer"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            using test_type = std::int32_t;

            expect(that % std::convertible_to<ConcretePtr<test_type>, test_type*>);
            expect(that % std::convertible_to<ConcretePtr<test_type>, const test_type*>);
            expect(that % std::convertible_to<ConcretePtr<test_type>, volatile test_type*>);
            expect(that % std::convertible_to<ConcretePtr<test_type>, const volatile test_type*>);
            expect(that % !std::convertible_to<ConcretePtr<const test_type>, test_type*>);
            expect(that % std::convertible_to<ConcretePtr<const test_type>, const test_type*>);
            expect(that % !std::convertible_to<ConcretePtr<const test_type>, volatile test_type*>);
            expect(that % std::convertible_to<ConcretePtr<const test_type>, const volatile test_type*>);
            expect(that % !std::convertible_to<ConcretePtr<volatile test_type>, test_type*>);
            expect(that % !std::convertible_to<ConcretePtr<volatile test_type>, const test_type*>);
            expect(that % std::convertible_to<ConcretePtr<volatile test_type>, volatile test_type*>);
            expect(that % std::convertible_to<ConcretePtr<volatile test_type>, const volatile test_type*>);
            expect(that % !std::convertible_to<ConcretePtr<const volatile test_type>, test_type*>);
            expect(that % !std::convertible_to<ConcretePtr<const volatile test_type>, const test_type*>);
            expect(that % !std::convertible_to<ConcretePtr<const volatile test_type>, volatile test_type*>);
            expect(that % std::convertible_to<ConcretePtr<const volatile test_type>, const volatile test_type*>);
        });
    };

    "conversion to raw pointer preserves address"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            const std::int32_t value{};
            const auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

            const std::int32_t* raw = ptr;

            expect(eq(raw, std::addressof(value)));
        });
    };

    "contextual boolean conversion is supported"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            expect(that % std::constructible_from<bool, ConcretePtr<std::int32_t>>);

            const std::int32_t value{};
            const auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

            bool converted{false};
            if (ptr) {
                converted = true;
            }

            expect(that % static_cast<bool>(ptr) == true);
            expect(that % !ptr == false);
            expect(that % converted == true);
        });
    };
}
