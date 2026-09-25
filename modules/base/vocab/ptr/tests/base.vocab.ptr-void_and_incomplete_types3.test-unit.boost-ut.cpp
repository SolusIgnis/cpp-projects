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
    concept dereferenceable = requires(T t) { *t; };

    template<typename T>
    concept arrow_accessible = requires(T t) { t.operator->(); };
} //namespace

//NOLINTBEGIN(readability-function-size, readability-function-cognitive-complexity)
//NOLINTNEXTLINE(bugprone-exception-escape): Test framework.
int main()
{
    //============================================================
    // `void` support
    //============================================================

    "type aliases are correct for `void`"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
                using t = ConcretePtr<void>;

                expect(that % std::same_as<typename t::element_type, void>);
                expect(that % std::same_as<typename t::value_type, void>);
                expect(that % std::same_as<typename t::address_type, void*>);
                expect(that % std::same_as<typename t::difference_type, std::ptrdiff_t>);
            }
        });
    };

    "void specialization supports type erasure"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
                std::int32_t x{};

                const auto typed = base::vocab::pointer_to<ConcretePtr>(x);
                const ConcretePtr<void> erased{typed};

                expect(that % erased.get() == std::addressof(x));
            }
        });
    };

    "void specialization disables dereference operators"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
                expect(that % dereferenceable<ConcretePtr<base_type>>);
                expect(that % arrow_accessible<ConcretePtr<base_type>>);

                expect(that % !dereferenceable<ConcretePtr<void>>);
                expect(that % !arrow_accessible<ConcretePtr<void>>);
            }
        });
    };

    "construction from void raw pointer is explicit"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
                expect(that % !std::convertible_to<void*, ConcretePtr<void>>);
                expect(that % std::constructible_from<ConcretePtr<void>, void*>);
            }
        });
    };

    "construction from void smart pointer is explicit"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
                expect(that % !std::convertible_to<trivial_smart_ptr<void>&, ConcretePtr<void>>);
                expect(that % std::constructible_from<ConcretePtr<void>, trivial_smart_ptr<void>&>);
            }
        });
    };

    "void pointer constructs implicitly from typed pointer"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
                expect(that % std::convertible_to<ConcretePtr<std::int32_t>, ConcretePtr<void>>);
                expect(that % !std::convertible_to<ConcretePtr<void>, ConcretePtr<std::int32_t>>);

                const std::int32_t value{42};
                const auto typed_ptr = base::vocab::pointer_to<ConcretePtr>(value);

                // Should be implicit (convertible)
                const auto takes_void = [](ConcretePtr<const void> ptr) { return ptr.get(); };
                expect(that % takes_void(typed_ptr) == std::addressof(value));
            }
        });
    };

    //NOLINTBEGIN(misc-const-correctness): Readability suffers with const correctness in this test.
    "static_pointer_cast converts static pointee type to and from void"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
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
        });
    };
    //NOLINTEND(misc-const-correctness)

    "void pointer is equality comparable"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
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
        });
    };

    //============================================================
    // Incomplete types
    //============================================================

    struct incomplete_type;

    "incomplete type support"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            if constexpr (pointer_test_traits<ConcretePtr>::allows_pointer_binding) {
                expect(that % base::meta::concepts::instantiable_with<ConcretePtr, incomplete_type>);

                //NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, readability-magic-numbers): Test requires a fabricated pointer value to an incomplete type.
                auto* const raw = reinterpret_cast<incomplete_type*>(0x1234);

                const ConcretePtr<incomplete_type> ptr{raw};

                expect(eq(ptr.get(), raw));
            }
        });
    };

    struct incomplete_type {
        std::int32_t value;
    };

    "incomplete type becomes usable after completion"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            //NOLINTNEXTLINE(readability-magic-numbers): Test fixture needs a meaningless number.
            incomplete_type obj{42};

            const auto ptr = base::vocab::pointer_to<ConcretePtr>(obj);

            expect(eq(ptr->value, obj.value));
            expect(eq((*ptr).value, obj.value));
        });
    };
}

//NOLINTEND(readability-function-size, readability-function-cognitive-complexity)
