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
}
