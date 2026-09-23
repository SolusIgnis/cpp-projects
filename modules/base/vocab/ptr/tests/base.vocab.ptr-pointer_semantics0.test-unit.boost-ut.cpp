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
    // Pointer semantics
    //============================================================

    "`pointer_to` forms a valid pointer instance whose `get` returns its stored address"_test =
        []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
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
        }
        | pointers_to_test;

    "`to_address` returns stored address as raw pointer"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        using pointee_t = const std::int32_t;
        using pointer_t = ConcretePtr<pointee_t>;

        pointee_t obj{};

        const auto ptr = pointer_t::pointer_to(obj);

        expect(that % std::same_as<decltype(std::pointer_traits<pointer_t>::to_address(ptr)), typename pointer_t::address_type>);
        expect(that % std::same_as<decltype(std::to_address(ptr)), typename pointer_t::address_type>);

        expect(eq(std::pointer_traits<pointer_t>::to_address(ptr), std::addressof(obj)));
        expect(eq(std::to_address(ptr), std::addressof(obj)));
    } | pointers_to_test;

    "operator* dereferences correctly"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        const auto value = 55;
        const auto ptr   = base::vocab::pointer_to<ConcretePtr>(value);

        expect(eq(*ptr, value));
    } | pointers_to_test;

    //NOLINTBEGIN(readability-magic-numbers): Test fixture needs a meaningless number.
    //NOLINTBEGIN(cppcoreguidelines-pro-type-union-access): Testing union access.
    "operator-> provides member access"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
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
    } | pointers_to_test;
    //NOLINTEND(cppcoreguidelines-pro-type-union-access)
    //NOLINTEND(readability-magic-numbers)

    "implicit conversion to raw pointer"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
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
    } | pointers_to_test;

    "conversion to raw pointer preserves address"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        const std::int32_t value{};
        const auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

        const std::int32_t* raw = ptr;

        expect(eq(raw, std::addressof(value)));
    } | pointers_to_test;

    "contextual boolean conversion is supported"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
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
    } | pointers_to_test;
}
