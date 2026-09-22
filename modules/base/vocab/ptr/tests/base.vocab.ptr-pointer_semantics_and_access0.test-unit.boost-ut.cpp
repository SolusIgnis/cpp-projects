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

    //============================================================
    // CV-correctness propagation
    //============================================================

    "const element forbids mutation through dereference"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        const std::int32_t value{};
        const auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

        expect(that % !std::is_assignable_v<decltype(*ptr), std::int32_t>) << "Compile-time: *ptr must NOT be assignable";
    } | pointers_to_test;

    "const pointer prevents rebinding but not mutation"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        constexpr auto initial{5};
        constexpr auto expected{10};

        auto value     = initial;
        const auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

        *ptr = expected;

        expect(that % !std::is_assignable_v<decltype(ptr)&, const decltype(ptr)&>) << "Compile-time: ptr must NOT be assignable";

        expect(neq(value, initial));
        expect(eq(value, expected));
    } | pointers_to_test;

    "`address_type` nested type preserves top-level const"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        using ptr_t = ConcretePtr<const std::int32_t>;

        expect(that % std::same_as<typename ptr_t::address_type, const std::int32_t*>);
    } | pointers_to_test;

    "`reference` nested type preserves const"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        using ptr_t = ConcretePtr<const std::int32_t>;

        expect(that % std::same_as<typename ptr_t::reference, const std::int32_t&>);
    } | pointers_to_test;

    //NOLINTBEGIN(misc-const-correctness): Readability suffers with const correctness in this test.
    "qualification climbing construction and assignment"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        std::int32_t value{};
        std::int32_t other{};
        auto mutable_ptr = base::vocab::pointer_to<ConcretePtr, std::int32_t>(value);
        auto const_ptr   = base::vocab::pointer_to<ConcretePtr, const std::int32_t>(other);

        //Qualification climbing (Assignment)
        const_ptr = mutable_ptr;
        expect(that % const_ptr.get() == mutable_ptr.get());

        //Qualification climbing (Construction)
        ConcretePtr<const std::int32_t> const_copy{mutable_ptr};
        expect(that % const_copy.get() == mutable_ptr.get());
    } | pointers_to_test;
    //NOLINTEND(misc-const-correctness)

    "volatile qualifier preservation"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        //NOLINTNEXTLINE(readability-magic-numbers): Test fixture needs a meaningless number.
        volatile std::int32_t hardware_register = 0xAA;

        const auto ptr = base::vocab::pointer_to<ConcretePtr>(hardware_register);

        //Ensure the raw pointer retrieved is also volatile
        expect(that % std::same_as<decltype(ptr.get()), volatile std::int32_t*>);

        //Ensure conversion to raw pointer preserves volatile
        volatile std::int32_t* raw = ptr;
        expect(eq(raw, std::addressof(hardware_register)));

        //Ensure dereference preserves volatile
        //NOLINTNEXTLINE(misc-const-correctness): It would be missing the point.
        decltype(auto) dereferenced = *ptr;
        expect(that % std::is_volatile_v<std::remove_reference_t<decltype(dereferenced)>>);
        expect(that % (dereferenced == hardware_register));
    } | pointers_to_test;

    //============================================================
    // Interoperability with raw pointer APIs
    //============================================================

    "implicit conversion works with raw pointer API"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        const auto takes_ptr = [](const std::int32_t* iptr) { return *iptr; };

        //NOLINTNEXTLINE(readability-magic-numbers): Test fixture needs a meaningless number.
        std::int32_t value = 3;
        const auto ptr     = base::vocab::pointer_to<ConcretePtr>(value);

        expect(eq(takes_ptr(ptr), value));
    } | pointers_to_test;

    "get() works with raw pointer API"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        const auto takes_ptr = [](const std::int32_t* iptr) { return *iptr; };

        //NOLINTNEXTLINE(readability-magic-numbers): Test fixture needs a meaningless number.
        std::int32_t value = 4;
        const auto ptr     = base::vocab::pointer_to<ConcretePtr>(value);

        expect(eq(takes_ptr(ptr.get()), value));
    } | pointers_to_test;

    //NOLINTBEGIN(modernize-avoid-c-arrays, cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access, cppcoreguidelines-pro-bounds-array-to-pointer-decay): Testing interactions with C Arrays, including pointer arithmetic and indexing operations.
    "not constructible, convertible, nor assignable from C-array decay"_test =
        []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
            //NOLINTNEXTLINE(readability-magic-numbers): Test fixture.
            std::int32_t array[3] = {0, 1, 2};

            //ConcretePtr<std::int32_t> should_fail{array};

            expect(that % !std::convertible_to<decltype(array), ConcretePtr<std::int32_t>>);
            expect(that % !std::constructible_from<ConcretePtr<std::int32_t>, decltype(array)>);
            expect(that % !std::is_assignable_v<ConcretePtr<std::int32_t>&, decltype(array)>);

            expect(that % std::constructible_from<ConcretePtr<std::int32_t>, decltype(array[0])>);

            const auto ptr = base::vocab::pointer_to<ConcretePtr>(array[1]);

            //Ensure binding to the element is equivalent to expected array-to-pointer decay with pointer offset arithmetic
            expect(eq(ptr.get(), array + 1));
        }
        | pointers_to_test;

    "not constructible, convertible, nor assignable from C-array decay when pointing to an array"_test =
        []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
            //NOLINTBEGIN(readability-magic-numbers): Test fixture.
            std::int32_t array[3][3] = {
                {0, 1, 2},
                {3, 4, 5},
                {6, 7, 8},
            };
            //NOLINTEND(readability-magic-numbers)

            //ConcretePtr<std::int32_t[3]> should_fail{array};

            expect(that % !std::convertible_to<decltype(array), ConcretePtr<std::int32_t[3]>>);
            expect(that % !std::constructible_from<ConcretePtr<std::int32_t[3]>, decltype(array)>);
            expect(that % !std::is_assignable_v<ConcretePtr<std::int32_t[3]>&, decltype(array)>);

            expect(that % std::constructible_from<ConcretePtr<std::int32_t[3]>, decltype(array[0])>);

            const auto ptr = base::vocab::pointer_to<ConcretePtr, std::int32_t[3]>(array[1]);

            //Ensure binding to the element is equivalent to expected array-to-pointer decay with pointer offset arithmetic
            expect(eq(ptr.get(), array + 1));
        }
        | pointers_to_test;
    //NOLINTEND(modernize-avoid-c-arrays, cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
}
