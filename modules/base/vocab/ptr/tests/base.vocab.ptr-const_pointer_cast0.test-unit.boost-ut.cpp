// SPDX-License-Identifier: Apache-2.0
// Parameterized unit tests for base.vocab.ptr

import base.vocab.ptr;
import boost.ut;
import std;

import base.meta.concepts;

#include "base.vocab.ptr-common_fixtures.boost-ut.hpp"

using namespace boost::ext::ut;

namespace {
    template<typename...>
    struct type_list {};
} //namespace

//NOLINTNEXTLINE(bugprone-exception-escape): Test framework.
int main()
{
    //============================================================
    // Const Pointer Casting
    //============================================================

    //NOLINTBEGIN(misc-const-correctness): Readability suffers with const correctness in these tests.
    "const_pointer_cast alters pointee cv-qualifications"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        should("alter") =
            []<typename Source, typename Destination>(type_list<Source, Destination>) mutable {
                Source value{};

                auto source = base::vocab::pointer_to<ConcretePtr>(value);
                auto result = const_pointer_cast<Destination>(source);

                expect(that % std::same_as<decltype(result), ConcretePtr<Destination>>);
                expect(that % result.get() == std::addressof(value));
            }
            | std::tuple{
                type_list<std::int32_t, std::int32_t>{},
                type_list<std::int32_t, const std::int32_t>{},
                type_list<std::int32_t, volatile std::int32_t>{},
                type_list<std::int32_t, const volatile std::int32_t>{},
                type_list<const std::int32_t, std::int32_t>{},
                type_list<const std::int32_t, const std::int32_t>{},
                type_list<const std::int32_t, volatile std::int32_t>{},
                type_list<const std::int32_t, const volatile std::int32_t>{},
                type_list<volatile std::int32_t, std::int32_t>{},
                type_list<volatile std::int32_t, const std::int32_t>{},
                type_list<volatile std::int32_t, volatile std::int32_t>{},
                type_list<volatile std::int32_t, const volatile std::int32_t>{},
                type_list<const volatile std::int32_t, std::int32_t>{},
                type_list<const volatile std::int32_t, const std::int32_t>{},
                type_list<const volatile std::int32_t, volatile std::int32_t>{},
                type_list<const volatile std::int32_t, const volatile std::int32_t>{},
            };
    } | pointers_to_test;

    "const_pointer_cast preserves null state"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
            ConcretePtr<std::int32_t> source{nullptr};

            const auto result = const_pointer_cast<const std::int32_t>(source);

            expect(that % result == nullptr);
        }
    } | pointers_to_test;
    //NOLINTEND(misc-const-correctness): Readability suffers with const correctness in these tests.
}
