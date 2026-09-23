// SPDX-License-Identifier: Apache-2.0
// Parameterized unit tests for base.vocab.ptr

import base.vocab.ptr;
import boost.ut;
import std;

import base.meta.concepts;

#include "base.vocab.ptr-common_fixtures.boost-ut.hpp"

using namespace boost::ext::ut;

//NOLINTBEGIN(readability-function-size, readability-function-cognitive-complexity)
//NOLINTNEXTLINE(bugprone-exception-escape): Test framework.
int main()
{
    //============================================================
    // Construction (Initial Binding) / Assignment (Rebinding)
    //============================================================

    "pointer binding according to policies"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            const auto
                verify_binding_operations = [] < typename Pointee,
                typename SourceTag, bool IsConstructibleFrom, bool IsConvertibleFrom,
                bool IsAssignableFrom = IsConstructibleFrom && !std::same_as<SourceTag, ref_tag> > {
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
                };

            //Reference binding is explicit when allowed
            verify_binding_operations.template operator()<std::int32_t, ref_tag, pointer_test_traits<ConcretePtr>::allows_reference_binding, false>();

            //Pointer binding allows implicit conversion
            verify_binding_operations.template
                operator()<std::int32_t, ptr_tag, pointer_test_traits<ConcretePtr>::allows_pointer_binding, pointer_test_traits<ConcretePtr>::allows_pointer_binding>();
            verify_binding_operations.template
                operator()<std::int32_t, smart_ptr_tag, pointer_test_traits<ConcretePtr>::allows_pointer_binding, pointer_test_traits<ConcretePtr>::allows_pointer_binding>();
        });
    };
}

//NOLINTEND(readability-function-size, readability-function-cognitive-complexity)
