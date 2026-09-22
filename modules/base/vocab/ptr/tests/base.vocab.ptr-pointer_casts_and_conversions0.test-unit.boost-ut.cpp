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
    // Pointer Casting & Lifetime Transmutation
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

    "static_pointer_cast converts static pointee type up and down inheritance hierarchies"_test =
        []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
            derived_type object;

            ConcretePtr<derived_type> source = base::vocab::pointer_to<ConcretePtr>(object);
            auto result1                     = static_pointer_cast<base_type>(source);

            expect(that % std::same_as<decltype(result1), ConcretePtr<base_type>>);
            expect(that % result1.get() == std::addressof(object));

            auto result2 = static_pointer_cast<derived_type>(result1);

            expect(that % std::same_as<decltype(result2), ConcretePtr<derived_type>>);
            expect(that % result2.get() == std::addressof(object));
        }
        | pointers_to_test;

    "static_pointer_cast preserves cv-qualifications"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        const derived_type object;

        ConcretePtr<const derived_type> source = base::vocab::pointer_to<ConcretePtr>(object);
        auto result                            = static_pointer_cast<base_type>(source);

        expect(that % std::same_as<decltype(result), ConcretePtr<const base_type>>);
    } | pointers_to_test;

    "static_pointer_cast preserves null state"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
            ConcretePtr<derived_type> source{nullptr};

            const auto result = static_pointer_cast<base_type>(source);

            expect(that % result == nullptr);
        }
    } | pointers_to_test;

    "dynamic_pointer_cast performs multiple-inheritance upcasts"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        derived_type value;

        auto source = base::vocab::pointer_to<ConcretePtr>(value);

        auto result_1 = dynamic_pointer_cast<mixin_1>(source);
        auto result_2 = dynamic_pointer_cast<mixin_2>(source);

        expect(that % std::same_as<decltype(result_1), ConcretePtr<mixin_1>>);
        expect(eq(result_1.get(), dynamic_cast<mixin_1*>(std::addressof(value))));

        expect(that % std::same_as<decltype(result_2), ConcretePtr<mixin_2>>);
        expect(eq(result_2.get(), dynamic_cast<mixin_2*>(std::addressof(value))));
    } | pointers_to_test;

    "dynamic_pointer_cast performs successful downcasts"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        derived_type value;

        ConcretePtr<base_type> source = base::vocab::pointer_to<ConcretePtr>(value);

        auto result = dynamic_pointer_cast<derived_type>(source);

        expect(that % std::same_as<decltype(result), ConcretePtr<derived_type>>);
        expect(eq(result.get(), std::addressof(value)));
        expect(eq(result->extra, value.extra));
    } | pointers_to_test;

    "dynamic_pointer_cast handles failed downcasts according to policy"_test =
        []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
            struct wrong_derived : base_type {};
            wrong_derived sentinel{};
            auto result = base::vocab::pointer_to<ConcretePtr>(sentinel);

            derived_type value;

            ConcretePtr<base_type> source = base::vocab::pointer_to<ConcretePtr>(value);

            const auto fail_to_cast = [&] { result = dynamic_pointer_cast<wrong_derived>(source); };

            if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                expect(nothrow(fail_to_cast));
                expect(that % result.get() == nullptr);
            } else {
                expect(throws<std::bad_cast>(fail_to_cast));
                expect(that % result.get() == std::addressof(sentinel));
            }
        }
        | pointers_to_test;

    "dynamic_pointer_cast preserves cv-qualifications"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        const derived_type object;

        ConcretePtr<const derived_type> source = base::vocab::pointer_to<ConcretePtr>(object);

        auto result = dynamic_pointer_cast<base_type>(source);

        expect(that % std::same_as<decltype(result), ConcretePtr<const base_type>>);
    } | pointers_to_test;

    "dynamic_pointer_cast preserves null state"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
            ConcretePtr<derived_type> source{nullptr};

            auto result = dynamic_pointer_cast<base_type>(source);

            expect(that % result == nullptr);
        }
    } | pointers_to_test;

    //NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast): Testing `reinterpret_pointer_cast` in terms of `reinterpret_cast`.
    "reinterpret_pointer_cast views objects as raw bytes"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        struct object {
            std::int32_t x;
            double y;
        };

        object value{};

        auto source       = base::vocab::pointer_to<ConcretePtr>(value);
        auto result_bytes = reinterpret_pointer_cast<std::byte>(source);
        auto result_chars = reinterpret_pointer_cast<char>(source);

        expect(that % std::same_as<decltype(result_bytes), ConcretePtr<std::byte>>);
        expect(eq(result_bytes.get(), reinterpret_cast<std::byte*>(std::addressof(value))));

        expect(that % std::same_as<decltype(result_chars), ConcretePtr<char>>);
        expect(eq(result_chars.get(), reinterpret_cast<char*>(std::addressof(value))));
    } | pointers_to_test;

    "reinterpret_pointer_cast alters how a pointer sees its pointee type"_test =
        []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
            struct origin_t {
                std::int32_t foo;
                char bar;
                double baz;
            };
            struct target_t {
                std::int64_t foo;
                double bar;
                std::uint8_t baz;
            };

            origin_t value{};

            auto source = base::vocab::pointer_to<ConcretePtr>(value);
            //WARNING: Using this result pointer's stored address potentially invokes undefined behavior.
            auto result = reinterpret_pointer_cast<target_t>(source);

            expect(that % std::same_as<decltype(result), ConcretePtr<target_t>>);
            expect(eq(result.get(), reinterpret_cast<target_t*>(std::addressof(value))));
        }
        | pointers_to_test;

    "reinterpret_pointer_cast preserves cv-qualifications"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        const derived_type object;

        ConcretePtr<const derived_type> source = base::vocab::pointer_to<ConcretePtr>(object);

        auto result = reinterpret_pointer_cast<base_type>(source);

        expect(that % std::same_as<decltype(result), ConcretePtr<const base_type>>);
    } | pointers_to_test;

    "reinterpret_pointer_cast preserves null state"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
            ConcretePtr<std::int32_t> source{nullptr};

            auto result = reinterpret_pointer_cast<std::byte>(source);

            expect(that % result.get() == nullptr);
        }
    } | pointers_to_test;
    //NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

#if defined(__cpp_lib_start_lifetime_as)
    "start_lifetime_as alters pointee type"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        struct origin_t {
            std::int32_t foo;
            std::int32_t bar;
            std::int32_t baz;
            double qux;
        };
        struct target_t {
            std::int32_t x;
            std::int32_t y;
            std::int32_t z;
            double velocity;
        };

        const origin_t value    = {.foo = 1, .bar = 3, .baz = 5, .qux = 2.0};
        const origin_t expected = value;

        auto source = base::vocab::pointer_to<ConcretePtr>(value);
        auto result = start_lifetime_as<target_t>(source);

        expect(that % std::same_as<decltype(result), ConcretePtr<const target_t>>);
        expect(eq(result.get(), reinterpret_cast<target_t*>(std::addressof(value))));
        expect(eq(result->x, expected.foo));
        expect(eq(result->y, expected.bar));
        expect(eq(result->z, expected.baz));
        expect(eq(result->velocity, expected.qux));
    } | pointers_to_test;
#else
    //NOLINTNEXTLINE(clang-diagnostic-#warnings)
    #warning "std::start_lifetime_as not defined. Tests skipped."
#endif
    //NOLINTEND(misc-const-correctness)

    //============================================================
    // `void` support
    //============================================================

    "type aliases are correct for `void`"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
            using ptr_t = ConcretePtr<void>;

            expect(that % std::same_as<typename ptr_t::element_type, void>);
            expect(that % std::same_as<typename ptr_t::value_type, void>);
            expect(that % std::same_as<typename ptr_t::address_type, void*>);
            expect(that % std::same_as<typename ptr_t::difference_type, std::ptrdiff_t>);
        }
    } | pointers_to_test;

    "void specialization supports type erasure"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
            std::int32_t x{};

            const auto typed = base::vocab::pointer_to<ConcretePtr>(x);
            const ConcretePtr<void> erased{typed};

            expect(that % erased.get() == std::addressof(x));
        }
    } | pointers_to_test;

    "void specialization disables dereference operators"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
            expect(that % dereferenceable<ConcretePtr<base_type>>);
            expect(that % arrow_accessible<ConcretePtr<base_type>>);

            expect(that % !dereferenceable<ConcretePtr<void>>);
            expect(that % !arrow_accessible<ConcretePtr<void>>);
        }
    } | pointers_to_test;

    "construction from void raw pointer is explicit"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
            expect(that % !std::convertible_to<void*, ConcretePtr<void>>);
            expect(that % std::constructible_from<ConcretePtr<void>, void*>);
        }
    } | pointers_to_test;

    "construction from void smart pointer is explicit"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
            expect(that % !std::convertible_to<trivial_smart_ptr<void>&, ConcretePtr<void>>);
            expect(that % std::constructible_from<ConcretePtr<void>, trivial_smart_ptr<void>&>);
        }
    } | pointers_to_test;

    "void pointer constructs implicitly from typed pointer"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
            expect(that % std::convertible_to<ConcretePtr<std::int32_t>, ConcretePtr<void>>);
            expect(that % !std::convertible_to<ConcretePtr<void>, ConcretePtr<std::int32_t>>);

            const std::int32_t value{42};
            const auto typed_ptr = base::vocab::pointer_to<ConcretePtr>(value);

            // Should be implicit (convertible)
            const auto takes_void = [](ConcretePtr<const void> ptr) { return ptr.get(); };
            expect(that % takes_void(typed_ptr) == std::addressof(value));
        }
    } | pointers_to_test;

    //NOLINTBEGIN(misc-const-correctness): Readability suffers with const correctness in this test.
    "static_pointer_cast converts static pointee type to and from void"_test =
        []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
            if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
                std::int32_t object = 0;

                auto source  = base::vocab::pointer_to<ConcretePtr>(object);
                auto result1 = static_pointer_cast<void>(source);

                expect(that % std::same_as<decltype(result1), ConcretePtr<void>>);
                expect(that % result1.get() == std::addressof(object));

                auto result2 = static_pointer_cast<std::int32_t>(result1);

                expect(that % std::same_as<decltype(result2), ConcretePtr<std::int32_t>>);
                expect(that % result2.get() == std::addressof(object));
            }
        }
        | pointers_to_test;
    //NOLINTEND(misc-const-correctness)

    "void pointer is equality comparable"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee && pointer_test_traits<ConcretePtr>::allows_pointer_binding) {
            const std::int32_t value{42};
            const auto typed_ptr                = base::vocab::pointer_to<ConcretePtr>(value);
            const std::int32_t* const typed_raw = std::addressof(value);
            const void* const erased_raw        = std::addressof(value);

            const ConcretePtr<const void> erased_ptr1{typed_raw};
            const ConcretePtr<const void> erased_ptr2{typed_ptr};

            expect(that % erased_ptr1 == erased_ptr2);
            expect(that % erased_ptr1 == typed_ptr);
            expect(that % erased_ptr1 == erased_raw);
            expect(that % erased_ptr1 == typed_raw);
        }
    } | pointers_to_test;

    //============================================================
    // Incomplete types
    //============================================================

    struct incomplete_type;

    "incomplete type support"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        if constexpr (pointer_test_traits<ConcretePtr>::allows_pointer_binding) {
            expect(that % base::meta::concepts::instantiable_with<ConcretePtr, incomplete_type>);

            //NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, readability-magic-numbers): Test requires a fabricated pointer value to an incomplete type.
            auto* const raw = reinterpret_cast<incomplete_type*>(0x1234);

            const ConcretePtr<incomplete_type> ptr{raw};

            expect(eq(ptr.get(), raw));
        }
    } | pointers_to_test;

    struct incomplete_type {
        std::int32_t value;
    };

    "incomplete type becomes usable after completion"_test = []<template<typename> typename ConcretePtr>(template_tag<ConcretePtr>) mutable {
        //NOLINTNEXTLINE(readability-magic-numbers): Test fixture needs a meaningless number.
        incomplete_type obj{42};

        const auto ptr = base::vocab::pointer_to<ConcretePtr>(obj);

        expect(eq(ptr->value, obj.value));
        expect(eq((*ptr).value, obj.value));
    } | pointers_to_test;
}
