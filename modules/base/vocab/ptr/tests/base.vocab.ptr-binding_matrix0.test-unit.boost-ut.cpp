// SPDX-License-Identifier: Apache-2.0
// Parameterized unit tests for base.vocab.ptr

import base.vocab.ptr;
import boost.ut;
import std;

import base.meta.concepts;

#include "base.vocab.ptr-common_fixtures.boost-ut.hpp"

using namespace boost::ext::ut;

namespace {
    struct ref_tag;
    struct ptr_tag;
    struct smart_ptr_tag;

    template<typename T, typename Tag>
    struct source_category;

    template<typename T>
    struct source_category<T, ref_tag> {
        using type = std::add_lvalue_reference_t<T>;
    };

    template<typename T>
    struct source_category<T, ptr_tag> {
        using type = std::add_pointer_t<T>;
    };

    template<typename T>
    struct source_category<T, smart_ptr_tag> {
        using type = trivial_smart_ptr<T>&;
    };

    template<typename T, typename Tag>
    using source_t = source_category<T, Tag>::type;

    template<typename, typename, bool, bool, bool>
    struct binding_parameters {};
} //namespace

//NOLINTBEGIN(readability-function-size, readability-function-cognitive-complexity)
//NOLINTNEXTLINE(bugprone-exception-escape): Test framework.
int main()
{
    //============================================================
    // Construction (Initial Binding) / Assignment (Rebinding)
    //============================================================

    "pointer binding according to policies"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        should("bind") =
            []<typename Pointee, typename SourceTag, bool IsConstructibleFrom, bool IsConvertibleFrom, bool IsAssignableFrom>(
                binding_parameters<Pointee, SourceTag, IsConstructibleFrom, IsConvertibleFrom, IsAssignableFrom>
            ) mutable {
                //Explicitly constructible unless removing qualifier
                expect(eq(std::constructible_from<ConcretePtr<Pointee>, source_t<Pointee, SourceTag>>, IsConstructibleFrom));
                expect(that % !std::constructible_from<ConcretePtr<Pointee>, source_t<const Pointee, SourceTag>>);
                expect(that % !std::constructible_from<ConcretePtr<Pointee>, source_t<volatile Pointee, SourceTag>>);
                expect(that % !std::constructible_from<ConcretePtr<Pointee>, source_t<const volatile Pointee, SourceTag>>);
                expect(eq(std::constructible_from<ConcretePtr<const Pointee>, source_t<Pointee, SourceTag>>, IsConstructibleFrom));
                expect(eq(std::constructible_from<ConcretePtr<const Pointee>, source_t<const Pointee, SourceTag>>, IsConstructibleFrom));
                expect(that % !std::constructible_from<ConcretePtr<const Pointee>, source_t<volatile Pointee, SourceTag>>);
                expect(that % !std::constructible_from<ConcretePtr<const Pointee>, source_t<const volatile Pointee, SourceTag>>);
                expect(eq(std::constructible_from<ConcretePtr<volatile Pointee>, source_t<Pointee, SourceTag>>, IsConstructibleFrom));
                expect(that % !std::constructible_from<ConcretePtr<volatile Pointee>, source_t<const Pointee, SourceTag>>);
                expect(eq(std::constructible_from<ConcretePtr<volatile Pointee>, source_t<volatile Pointee, SourceTag>>, IsConstructibleFrom));
                expect(that % !std::constructible_from<ConcretePtr<volatile Pointee>, source_t<const volatile Pointee, SourceTag>>);
                expect(eq(std::constructible_from<ConcretePtr<const volatile Pointee>, source_t<Pointee, SourceTag>>, IsConstructibleFrom));
                expect(eq(std::constructible_from<ConcretePtr<const volatile Pointee>, source_t<const Pointee, SourceTag>>, IsConstructibleFrom));
                expect(eq(std::constructible_from<ConcretePtr<const volatile Pointee>, source_t<volatile Pointee, SourceTag>>, IsConstructibleFrom));
                expect(eq(std::constructible_from<ConcretePtr<const volatile Pointee>, source_t<const volatile Pointee, SourceTag>>, IsConstructibleFrom));

                //Implicitly convertible unless removing qualifier
                expect(eq(std::convertible_to<source_t<Pointee, SourceTag>, ConcretePtr<Pointee>>, IsConvertibleFrom));
                expect(that % !std::convertible_to<source_t<const Pointee, SourceTag>, ConcretePtr<Pointee>>);
                expect(that % !std::convertible_to<source_t<volatile Pointee, SourceTag>, ConcretePtr<Pointee>>);
                expect(that % !std::convertible_to<source_t<const volatile Pointee, SourceTag>, ConcretePtr<Pointee>>);
                expect(eq(std::convertible_to<source_t<Pointee, SourceTag>, ConcretePtr<const Pointee>>, IsConvertibleFrom));
                expect(eq(std::convertible_to<source_t<const Pointee, SourceTag>, ConcretePtr<const Pointee>>, IsConvertibleFrom));
                expect(that % !std::convertible_to<source_t<volatile Pointee, SourceTag>, ConcretePtr<const Pointee>>);
                expect(that % !std::convertible_to<source_t<const volatile Pointee, SourceTag>, ConcretePtr<const Pointee>>);
                expect(eq(std::convertible_to<source_t<Pointee, SourceTag>, ConcretePtr<volatile Pointee>>, IsConvertibleFrom));
                expect(that % !std::convertible_to<source_t<const Pointee, SourceTag>, ConcretePtr<volatile Pointee>>);
                expect(eq(std::convertible_to<source_t<volatile Pointee, SourceTag>, ConcretePtr<volatile Pointee>>, IsConvertibleFrom));
                expect(that % !std::convertible_to<source_t<const volatile Pointee, SourceTag>, ConcretePtr<volatile Pointee>>);
                expect(eq(std::convertible_to<source_t<Pointee, SourceTag>, ConcretePtr<const volatile Pointee>>, IsConvertibleFrom));
                expect(eq(std::convertible_to<source_t<const Pointee, SourceTag>, ConcretePtr<const volatile Pointee>>, IsConvertibleFrom));
                expect(eq(std::convertible_to<source_t<volatile Pointee, SourceTag>, ConcretePtr<const volatile Pointee>>, IsConvertibleFrom));
                expect(eq(std::convertible_to<source_t<const volatile Pointee, SourceTag>, ConcretePtr<const volatile Pointee>>, IsConvertibleFrom));

                //Assignable unless removing qualifier or rebinding from reference
                expect(eq(std::is_assignable_v<ConcretePtr<Pointee>&, source_t<Pointee, SourceTag>>, IsAssignableFrom));
                expect(that % !std::is_assignable_v<ConcretePtr<Pointee>&, source_t<const Pointee, SourceTag>>);
                expect(that % !std::is_assignable_v<ConcretePtr<Pointee>&, source_t<volatile Pointee, SourceTag>>);
                expect(that % !std::is_assignable_v<ConcretePtr<Pointee>&, source_t<const volatile Pointee, SourceTag>>);
                expect(eq(std::is_assignable_v<ConcretePtr<const Pointee>&, source_t<Pointee, SourceTag>>, IsAssignableFrom));
                expect(eq(std::is_assignable_v<ConcretePtr<const Pointee>&, source_t<const Pointee, SourceTag>>, IsAssignableFrom));
                expect(that % !std::is_assignable_v<ConcretePtr<const Pointee>&, source_t<volatile Pointee, SourceTag>>);
                expect(that % !std::is_assignable_v<ConcretePtr<const Pointee>&, source_t<const volatile Pointee, SourceTag>>);
                expect(eq(std::is_assignable_v<ConcretePtr<volatile Pointee>&, source_t<Pointee, SourceTag>>, IsAssignableFrom));
                expect(that % !std::is_assignable_v<ConcretePtr<volatile Pointee>&, source_t<const Pointee, SourceTag>>);
                expect(eq(std::is_assignable_v<ConcretePtr<volatile Pointee>&, source_t<volatile Pointee, SourceTag>>, IsAssignableFrom));
                expect(that % !std::is_assignable_v<ConcretePtr<volatile Pointee>&, source_t<const volatile Pointee, SourceTag>>);
                expect(eq(std::is_assignable_v<ConcretePtr<const volatile Pointee>&, source_t<Pointee, SourceTag>>, IsAssignableFrom));
                expect(eq(std::is_assignable_v<ConcretePtr<const volatile Pointee>&, source_t<const Pointee, SourceTag>>, IsAssignableFrom));
                expect(eq(std::is_assignable_v<ConcretePtr<const volatile Pointee>&, source_t<volatile Pointee, SourceTag>>, IsAssignableFrom));
                expect(eq(std::is_assignable_v<ConcretePtr<const volatile Pointee>&, source_t<const volatile Pointee, SourceTag>>, IsAssignableFrom));
            }
            | std::tuple{
                //Reference binding is explicit when allowed
                binding_parameters<std::int32_t, ref_tag, pointer_test_traits<ConcretePtr>::allows_reference_binding, false, false>{},

                //Pointer binding allows implicit conversion
                binding_parameters<std::int32_t, ptr_tag, pointer_test_traits<ConcretePtr>::allows_pointer_binding, pointer_test_traits<ConcretePtr>::allows_pointer_binding, pointer_test_traits<ConcretePtr>::allows_pointer_binding>{},
                binding_parameters<std::int32_t, smart_ptr_tag, pointer_test_traits<ConcretePtr>::allows_pointer_binding, pointer_test_traits<ConcretePtr>::allows_pointer_binding, pointer_test_traits<ConcretePtr>::allows_pointer_binding>{},
            };
    } | pointers_to_test;
}

//NOLINTEND(readability-function-size, readability-function-cognitive-complexity)
