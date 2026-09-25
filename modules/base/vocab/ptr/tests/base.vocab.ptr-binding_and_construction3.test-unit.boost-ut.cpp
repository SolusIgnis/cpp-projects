// SPDX-License-Identifier: Apache-2.0
// Parameterized unit tests for base.vocab.ptr

import base.vocab.ptr;
import boost.ut;
import std;

import base.meta.concepts;

#include "base.vocab.ptr-common_fixtures.qlibs-ut.hpp"

using namespace boost::ext::ut;

//NOLINTBEGIN(readability-function-size, readability-function-cognitive-complexity)
//NOLINTNEXTLINE(bugprone-exception-escape): Test framework.
int main()
{
    //============================================================
    // Construction (Initial Binding) / Assignment (Rebinding)
    //============================================================

    "bindable from nullptr according to nullability policy"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            expect(eq(std::constructible_from<ConcretePtr<std::int32_t>, std::nullptr_t>, pointer_test_traits<ConcretePtr>::is_nullable));
            expect(eq(std::is_assignable_v<ConcretePtr<std::int32_t>&, std::nullptr_t>, pointer_test_traits<ConcretePtr>::is_nullable));
        });
    };

    "not bindable from pointee rvalue"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            expect(that % !std::constructible_from<ConcretePtr<std::int32_t>, std::int32_t>);
            expect(that % !std::constructible_from<ConcretePtr<const std::int32_t>, std::int32_t>);
            expect(that % !std::constructible_from<ConcretePtr<const std::int32_t>, const std::int32_t>);

            expect(that % !std::constructible_from<ConcretePtr<std::int32_t>, std::int32_t&&>);
            expect(that % !std::constructible_from<ConcretePtr<const std::int32_t>, std::int32_t&&>);
            expect(that % !std::constructible_from<ConcretePtr<const std::int32_t>, const std::int32_t&&>);

            expect(that % !std::is_assignable_v<ConcretePtr<std::int32_t>&, std::int32_t&&>);
            expect(that % !std::is_assignable_v<ConcretePtr<const std::int32_t>&, std::int32_t&&>);
            expect(that % !std::is_assignable_v<ConcretePtr<const std::int32_t>&, const std::int32_t&&>);
        });
    };

    "not bindable from smart pointer rvalue"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            expect(that % !std::constructible_from<ConcretePtr<std::int32_t>, trivial_smart_ptr<std::int32_t>>);
            expect(that % !std::constructible_from<ConcretePtr<const std::int32_t>, trivial_smart_ptr<std::int32_t>>);
            expect(that % !std::constructible_from<ConcretePtr<const std::int32_t>, const trivial_smart_ptr<std::int32_t>>);

            expect(that % !std::constructible_from<ConcretePtr<std::int32_t>, trivial_smart_ptr<std::int32_t>&&>);
            expect(that % !std::constructible_from<ConcretePtr<const std::int32_t>, trivial_smart_ptr<std::int32_t>&&>);
            expect(that % !std::constructible_from<ConcretePtr<const std::int32_t>, const trivial_smart_ptr<std::int32_t>&&>);

            expect(that % !std::is_assignable_v<ConcretePtr<std::int32_t>&, trivial_smart_ptr<std::int32_t>&&>);
            expect(that % !std::is_assignable_v<ConcretePtr<const std::int32_t>&, trivial_smart_ptr<std::int32_t>&&>);
            expect(that % !std::is_assignable_v<ConcretePtr<const std::int32_t>&, const trivial_smart_ptr<std::int32_t>&&>);
        });
    };

    //============================================================
    // Rebinding
    //============================================================

    "rebind via copy-assignment from reference construction"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                const std::int32_t value1{};
                const std::int32_t value2 = 2;

                ConcretePtr<const std::int32_t> ptr{value1};
                //error: ```ptr = value2;``` is deleted to prevent implicit conversions
                ptr = ConcretePtr{value2};

                expect(eq(*ptr, value2));
                expect(eq(ptr.get(), std::addressof(value2)));
            }
        });
    };

    "rebind via `reset` call with reference argument"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                const std::int32_t value1{};
                const std::int32_t value2 = 2;

                ConcretePtr<const std::int32_t> ptr{value1};
                ptr.reset(value2);

                expect(eq(*ptr, value2));
                expect(eq(ptr.get(), std::addressof(value2)));
            }
        });
    };

    "moved-from object may be rebound"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            std::int32_t value1 = 1;
            std::int32_t value2 = 2;

            auto source = base::vocab::pointer_to<ConcretePtr>(value1);
            ConcretePtr<std::int32_t> target{std::move(source)};

            //Rebind moved-from `source` to reference `value2`
            source = base::vocab::pointer_to<ConcretePtr>(value2);

            expect(eq(*target, value1));
            expect(eq(*source, value2));
        });
    };

    "rebind via `reset()` disengages the pointer"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                const std::int32_t value{};

                auto ptr = base::vocab::pointer_to<ConcretePtr>(value);
                ptr.reset();

                expect(that % !ptr == true);
            }
        });
    };

    //============================================================
    // CTAD Guide
    //============================================================

    "deduction guides work"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            constexpr std::int32_t value{};

            if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                const ConcretePtr ptr1{value};
                expect(eq(ptr1.get(), std::addressof(value)));
            }

            if constexpr (pointer_test_traits<ConcretePtr>::allows_pointer_binding) {
                const ConcretePtr ptr2{std::addressof(value)};

                trivial_smart_ptr<const std::int32_t> smart_pointer{std::addressof(value)};
                const ConcretePtr ptr3{smart_pointer};

                expect(eq(ptr2.get(), std::addressof(value)));
                expect(eq(ptr3.get(), std::addressof(value)));
            }
        });
    };
}

//NOLINTEND(readability-function-size, readability-function-cognitive-complexity)
