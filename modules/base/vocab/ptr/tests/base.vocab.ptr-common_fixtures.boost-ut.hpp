// SPDX-License-Identifier: Apache-2.0
// Common fixtures for unit tests for base.vocab.ptr

#pragma once

//NOTE: This header is only included in single-TU test runners.
namespace {
    template<template<typename> typename>
    struct template_tag {};

    constexpr std::tuple pointers_to_test{
        template_tag<base::vocab::ptr::dependency_ptr>{},
        template_tag<base::vocab::ptr::required_ptr>{},
        template_tag<base::vocab::ptr::alias_ptr>{},
        template_tag<base::vocab::ptr::cursor_ptr>{},
        template_tag<base::vocab::ptr::iterator_ptr>{},
    };

    template<template<typename> typename Ptr>
    struct pointer_test_traits_base;

    template<>
    struct pointer_test_traits_base<base::vocab::ptr::dependency_ptr> {
        static constexpr bool is_nullable              = false;
        static constexpr bool has_arithmetic_traversal = false;
        static constexpr bool allows_pointer_binding   = false;
        static constexpr bool allows_reference_binding = true;
    };

    template<>
    struct pointer_test_traits_base<base::vocab::ptr::required_ptr> {
        static constexpr bool is_nullable              = false;
        static constexpr bool has_arithmetic_traversal = false;
        static constexpr bool allows_pointer_binding   = true;
        static constexpr bool allows_reference_binding = true;
    };

    template<>
    struct pointer_test_traits_base<base::vocab::ptr::alias_ptr> {
        static constexpr bool is_nullable              = true;
        static constexpr bool has_arithmetic_traversal = false;
        static constexpr bool allows_pointer_binding   = true;
        static constexpr bool allows_reference_binding = true;
    };

    template<>
    struct pointer_test_traits_base<base::vocab::ptr::cursor_ptr> {
        static constexpr bool is_nullable              = false;
        static constexpr bool has_arithmetic_traversal = true;
        static constexpr bool allows_pointer_binding   = true;
        static constexpr bool allows_reference_binding = true;
    };

    template<>
    struct pointer_test_traits_base<base::vocab::ptr::iterator_ptr> {
        static constexpr bool is_nullable              = true;
        static constexpr bool has_arithmetic_traversal = true;
        static constexpr bool allows_pointer_binding   = true;
        static constexpr bool allows_reference_binding = true;
    };

    template<template<typename> typename Ptr>
    struct pointer_test_traits : pointer_test_traits_base<Ptr> {
        static constexpr bool permits_void_pointee =
            !pointer_test_traits_base<Ptr>::has_arithmetic_traversal && pointer_test_traits_base<Ptr>::allows_pointer_binding;
    };

    //NOLINTNEXTLINE(cppcoreguidelines-special-member-functions): Trivial fixture.
    struct mixin_1 {
        virtual ~mixin_1() = default;
    };

    //NOLINTNEXTLINE(cppcoreguidelines-special-member-functions): Trivial fixture.
    struct mixin_2 {
        virtual ~mixin_2() = default;
    };

    //NOLINTNEXTLINE(cppcoreguidelines-special-member-functions): Trivial fixture.
    struct base_type : mixin_1,
                       mixin_2 {
        ~base_type() override = default;
        std::int32_t value{0};
    };

    //NOLINTNEXTLINE(cppcoreguidelines-special-member-functions): Trivial fixture.
    struct derived_type : base_type {
        ~derived_type() override = default;
        std::int32_t extra{42};
    };

    union union_type {
        std::int32_t value;
        std::int16_t irrelevant;
    };

    template<typename T>
    struct trivial_smart_ptr {
        T* address{};

        [[nodiscard]] T* get() const { return address; }

        T* operator->() const { return address; }
    };
} //namespace
