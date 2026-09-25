// SPDX-License-Identifier: Apache-2.0
// Parameterized unit tests for base.vocab.ptr

import base.vocab.ptr;
import boost.ut;
import std;

import base.meta.concepts;

#include "base.vocab.ptr-common_fixtures.qlibs-ut.hpp"

using namespace boost::ext::ut;

namespace {
    template<typename T>
    concept arrow_accessible = requires(T t) { t.operator->(); };
} //namespace

//NOLINTNEXTLINE(bugprone-exception-escape): Test framework.
int main()
{
    //============================================================
    // Pointer semantics
    //============================================================

    "`pointer_to` forms a valid pointer instance whose `get` returns its stored address"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            using pointee_t = std::int32_t;
            using pointer_t = const ConcretePtr<pointee_t>;

            pointee_t obj{};

            const auto class_ptr = pointer_t::pointer_to(obj);
            const auto trait_ptr = std::pointer_traits<pointer_t>::pointer_to(obj);
            const auto free_ptr  = base::vocab::ptr::pointer_to<ConcretePtr>(obj);

            expect(that % std::same_as<decltype(class_ptr), pointer_t>);
            expect(that % std::same_as<decltype(trait_ptr), pointer_t>);
            expect(that % std::same_as<decltype(free_ptr), pointer_t>);

            expect(eq(class_ptr.get(), std::addressof(obj)));
            expect(eq(trait_ptr.get(), std::addressof(obj)));
            expect(eq(free_ptr.get(), std::addressof(obj)));
        });
    };

    "`to_address` returns stored address as raw pointer"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            using pointee_t = const std::int32_t;
            using pointer_t = ConcretePtr<pointee_t>;

            pointee_t obj{};

            const auto ptr = pointer_t::pointer_to(obj);

            expect(that % std::same_as<decltype(std::pointer_traits<pointer_t>::to_address(ptr)), typename pointer_t::address_type>);
            expect(that % std::same_as<decltype(std::to_address(ptr)), typename pointer_t::address_type>);

            expect(eq(std::pointer_traits<pointer_t>::to_address(ptr), std::addressof(obj)));
            expect(eq(std::to_address(ptr), std::addressof(obj)));
        });
    };

    "operator* dereferences correctly"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            const auto value = 55;
            const auto ptr   = base::vocab::pointer_to<ConcretePtr>(value);

            expect(eq(*ptr, value));
        });
    };

    //NOLINTBEGIN(readability-magic-numbers): Test fixture needs a meaningless number.
    //NOLINTBEGIN(cppcoreguidelines-pro-type-union-access): Testing union access.
    "operator-> provides member access"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            expect(that % !arrow_accessible<ConcretePtr<std::uint8_t>>);
            expect(that % !arrow_accessible<ConcretePtr<std::float_round_style>>);
            expect(that % !arrow_accessible<ConcretePtr<std::memory_order>>);
            expect(that % !arrow_accessible<ConcretePtr<std::byte>>);
            expect(that % arrow_accessible<ConcretePtr<base_type>>);
            expect(that % arrow_accessible<ConcretePtr<union_type>>);

            base_type c_obj;
            c_obj.value     = 123;
            const auto ptr1 = base::vocab::pointer_to<ConcretePtr>(c_obj);

            union_type u_obj{};
            u_obj.value     = 321;
            const auto ptr2 = base::vocab::pointer_to<ConcretePtr>(u_obj);

            expect(eq(ptr1->value, c_obj.value));
            expect(eq(ptr2->value, u_obj.value));
        });
    };
    //NOLINTEND(cppcoreguidelines-pro-type-union-access)
    //NOLINTEND(readability-magic-numbers)
}
