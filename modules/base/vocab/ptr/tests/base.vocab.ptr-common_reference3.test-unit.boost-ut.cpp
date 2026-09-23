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
    // Common Reference
    //============================================================

    "basic_common_reference preserves concrete pointer type with cv-qualifications"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            expect(that % std::common_reference_with<ConcretePtr<std::int32_t>, ConcretePtr<const std::int32_t>>);
            expect(that % std::same_as<std::common_reference_t<ConcretePtr<std::int32_t>, ConcretePtr<const std::int32_t>>, ConcretePtr<const std::int32_t>>);

            expect(that % std::common_reference_with<ConcretePtr<std::int32_t>, ConcretePtr<volatile std::int32_t>>);
            expect(that % std::same_as<std::common_reference_t<ConcretePtr<std::int32_t>, ConcretePtr<volatile std::int32_t>>, ConcretePtr<volatile std::int32_t>>);

            expect(that % std::common_reference_with<ConcretePtr<const std::int32_t>, ConcretePtr<const volatile std::int32_t>>);
            expect(
                that % std::same_as<std::common_reference_t<ConcretePtr<const std::int32_t>, ConcretePtr<const volatile std::int32_t>>, ConcretePtr<const volatile std::int32_t>>
            );

            expect(that % std::common_reference_with<ConcretePtr<volatile std::int32_t>, ConcretePtr<const volatile std::int32_t>>);
            expect(
                that % std::same_as<std::common_reference_t<ConcretePtr<volatile std::int32_t>, ConcretePtr<const volatile std::int32_t>>, ConcretePtr<const volatile std::int32_t>>
            );

            expect(that % std::common_reference_with<ConcretePtr<volatile std::int32_t>, ConcretePtr<const std::int32_t>>);
            expect(that % std::same_as<std::common_reference_t<ConcretePtr<volatile std::int32_t>, ConcretePtr<const std::int32_t>>, ConcretePtr<const volatile std::int32_t>>);
        });
    };

    "basic_common_reference uses reference-to-pointer value category propagation"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            static_assert(
                std::same_as<std::common_reference_t<std::int32_t*&, const std::int32_t*&>, const std::int32_t*>, "Sanity check for raw pointer common_reference_t<T*&, const T*&> -> const T*"
            );
            expect(that % std::common_reference_with<ConcretePtr<std::int32_t>&, ConcretePtr<const std::int32_t>&>);
            expect(that % std::same_as<std::common_reference_t<ConcretePtr<std::int32_t>&, ConcretePtr<const std::int32_t>&>, ConcretePtr<const std::int32_t>>);

            static_assert(
                std::same_as<std::common_reference_t<std::int32_t*&&, const std::int32_t*&&>, const std::int32_t*>,
                "Sanity check for raw pointer common_reference_t<T*&&, const T*&&> -> const T*"
            );
            expect(that % std::common_reference_with<ConcretePtr<std::int32_t>&&, ConcretePtr<const std::int32_t>&&>);
            expect(that % std::same_as<std::common_reference_t<ConcretePtr<std::int32_t>&&, ConcretePtr<const std::int32_t>&&>, ConcretePtr<const std::int32_t>>);

            static_assert(
                std::same_as<std::common_reference_t<const std::int32_t*&, std::int32_t*&&>, const std::int32_t* const&>,
                "Sanity check for raw pointer common_reference_t<const T*&, T*&&> -> const T* const &"
            );
            expect(that % std::common_reference_with<ConcretePtr<const std::int32_t>&, ConcretePtr<std::int32_t>&&>);
            expect(that % std::same_as<std::common_reference_t<ConcretePtr<const std::int32_t>&, ConcretePtr<std::int32_t>&&>, const ConcretePtr<const std::int32_t>&>);

            static_assert(
                std::same_as<std::common_reference_t<std::int32_t* const&, std::int32_t*&&>, std::int32_t* const&>,
                "Sanity check for raw pointer common_reference_t<T* const &, T*&&> -> T* const &"
            );
            expect(that % std::common_reference_with<const ConcretePtr<std::int32_t>&, ConcretePtr<const std::int32_t>&&>);
            expect(that % std::same_as<std::common_reference_t<const ConcretePtr<std::int32_t>&, ConcretePtr<const std::int32_t>&&>, const ConcretePtr<const std::int32_t>&>);
        });
    };

    "basic_common_reference matches raw pointer common_reference"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            expect(that % std::common_reference_with<ConcretePtr<std::int32_t>, ConcretePtr<const volatile std::int32_t>>);
            expect(
                that
                % std::same_as<
                    std::common_reference_t<ConcretePtr<std::int32_t>, ConcretePtr<const volatile std::int32_t>>,
                    ConcretePtr<std::remove_pointer_t<std::remove_cvref_t<std::common_reference_t<std::int32_t*, const volatile std::int32_t*>>>>
                >
            );
        });
    };

    "vocabulary pointer and raw pointer share raw pointer common reference"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            expect(that % std::common_reference_with<ConcretePtr<std::int32_t>, std::int32_t*>);
            expect(that % std::same_as<std::common_reference_t<ConcretePtr<std::int32_t>, std::int32_t*>, std::int32_t*>);

            expect(that % std::common_reference_with<std::int32_t*, ConcretePtr<std::int32_t>>);
            expect(that % std::same_as<std::common_reference_t<std::int32_t*, ConcretePtr<std::int32_t>>, std::int32_t*>);
        });
    };

    "common_reference supports covariance"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            expect(that % std::common_reference_with<ConcretePtr<derived_type>, ConcretePtr<base_type>>);
            expect(that % std::same_as<std::common_reference_t<ConcretePtr<derived_type>, ConcretePtr<base_type>>, ConcretePtr<base_type>>);

            expect(that % std::common_reference_with<ConcretePtr<const derived_type>, ConcretePtr<base_type>>);
            expect(that % std::same_as<std::common_reference_t<ConcretePtr<const derived_type>, ConcretePtr<base_type>>, ConcretePtr<const base_type>>);

            expect(that % std::common_reference_with<ConcretePtr<derived_type>, ConcretePtr<const base_type>>);
            expect(that % std::same_as<std::common_reference_t<ConcretePtr<derived_type>, ConcretePtr<const base_type>>, ConcretePtr<const base_type>>);

            expect(that % std::common_reference_with<ConcretePtr<const derived_type>, ConcretePtr<volatile base_type>>);
            expect(that % std::same_as<std::common_reference_t<ConcretePtr<const derived_type>, ConcretePtr<volatile base_type>>, ConcretePtr<const volatile base_type>>);

            expect(that % std::common_reference_with<ConcretePtr<derived_type>, base_type*>);
            expect(that % std::same_as<std::common_reference_t<ConcretePtr<derived_type>, base_type*>, base_type*>);

            expect(that % std::common_reference_with<derived_type*, ConcretePtr<base_type>>);
            expect(that % std::same_as<std::common_reference_t<derived_type*, ConcretePtr<base_type>>, base_type*>);

            expect(that % std::common_reference_with<ConcretePtr<const derived_type>, volatile base_type*>);
            expect(that % std::same_as<std::common_reference_t<ConcretePtr<const derived_type>, volatile base_type*>, const volatile base_type*>);
        });
    };

    "common_type preserves const qualification"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            using common_t = std::common_type_t<ConcretePtr<std::int32_t>, ConcretePtr<const std::int32_t>>;

            expect(that % std::same_as<common_t, ConcretePtr<const std::int32_t>>);
        });
    };

    "common_reference preserves const qualification"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            using common_ref = std::common_reference_t<ConcretePtr<std::int32_t>, ConcretePtr<const std::int32_t>>;

            expect(that % std::same_as<common_ref, ConcretePtr<const std::int32_t>>);
        });
    };
}
