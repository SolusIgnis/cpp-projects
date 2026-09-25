// SPDX-License-Identifier: Apache-2.0
// Parameterized unit tests for base.vocab.ptr

import base.vocab.ptr;
import boost.ut;
import std;

import base.meta.concepts;

#include "base.vocab.ptr-common_fixtures.qlibs-ut.hpp"

using namespace boost::ext::ut;

//NOLINTNEXTLINE(bugprone-exception-escape): Test framework.
int main()
{
    //============================================================
    // Swap
    //============================================================

    "swap exchanges bindings"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            const std::int32_t value1 = 1;
            const std::int32_t value2 = 2;

            auto lhs = base::vocab::pointer_to<ConcretePtr>(value1);
            auto rhs = base::vocab::pointer_to<ConcretePtr>(value2);

            using std::swap;
            swap(lhs, rhs);

            expect(eq(lhs.get(), std::addressof(value2)));
            expect(eq(rhs.get(), std::addressof(value1)));

            expect(eq(*lhs, value2));
            expect(eq(*rhs, value1));
        });
    };

    //============================================================
    // Constant Expression Usage
    //============================================================

    "constexpr construction and dereference"_test = [] {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            static constexpr std::int32_t value = 42;

            constexpr auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

            expect(eq(*ptr, value));
        });
    };

    //NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access, cppcoreguidelines-pro-bounds-array-to-pointer-decay): Testing pointer arithmetic and indexing operations.
    "constexpr arithmetic"_test = [] {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            if constexpr (pointer_test_traits<ConcretePtr>::has_arithmetic_traversal) {
                //NOLINTNEXTLINE(readability-magic-numbers, modernize-avoid-c-arrays): Test fixture.
                static constexpr std::int32_t values[] = {2, 4, 6};

                constexpr auto ptr = base::vocab::pointer_to<ConcretePtr>(values[0]);

                constexpr auto next = ptr + 1;
                expect(eq(*next, values[1]));
            }
        });
    };
    //NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-bounds-avoid-unchecked-container-access, cppcoreguidelines-pro-bounds-array-to-pointer-decay)

    "constexpr get and boolean conversion"_test = [] {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            static constexpr std::int32_t value = 7;

            constexpr auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

            expect(eq(ptr.get(), std::addressof(value)));
            expect(that % static_cast<bool>(ptr) == true);
        });
    };

    "constexpr equality"_test = [] {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            static constexpr std::int32_t value = 11;

            constexpr auto ptr1 = base::vocab::pointer_to<ConcretePtr>(value);
            constexpr auto ptr2 = base::vocab::pointer_to<ConcretePtr>(value);

            expect(that % ptr1 == ptr2);
        });
    };

    "constexpr rebinding"_test = [] {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            static constexpr std::int32_t value1 = 1;
            static constexpr std::int32_t value2 = 2;

            constexpr auto rebound = std::invoke([] {
                auto ptr = base::vocab::pointer_to<ConcretePtr>(value1);
                ptr      = ConcretePtr{value2};
                return ptr;
            });

            expect(eq(*rebound, value2));
            expect(eq(rebound.get(), std::addressof(value2)));
        });
    };

    "constexpr swap"_test = [] {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            static constexpr std::int32_t value1 = 1;
            static constexpr std::int32_t value2 = 2;

            constexpr auto swapped = std::invoke([] {
                auto lhs = base::vocab::pointer_to<ConcretePtr>(value1);
                auto rhs = base::vocab::pointer_to<ConcretePtr>(value2);

                using std::swap;
                swap(lhs, rhs);

                return std::pair{lhs, rhs};
            });

            expect(eq(*swapped.first, value2));
            expect(eq(*swapped.second, value1));
        });
    };

    //============================================================
    // Hash Support
    //============================================================

    "hash matches raw pointer hash"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            std::int32_t value{};

            const auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

            const auto ptr_hash = std::hash<ConcretePtr<std::int32_t>>{}(ptr);
            const auto raw_hash = std::hash<std::int32_t*>{}(std::addressof(value));

            expect(eq(ptr_hash, raw_hash));
        });
    };

    "equal pointers produce equal hashes"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            std::int32_t value{};

            const auto lhs = base::vocab::pointer_to<ConcretePtr, std::int32_t>(value);
            const auto rhs = base::vocab::pointer_to<ConcretePtr, const std::int32_t>(value);

            const auto lhs_hash = std::hash<ConcretePtr<std::int32_t>>{}(lhs);
            const auto rhs_hash = std::hash<ConcretePtr<const std::int32_t>>{}(rhs);

            expect(that % lhs == rhs);
            expect(that % lhs_hash == rhs_hash);
        });
    };

    //============================================================
    // Formatting and output stream support
    //============================================================

    "std::formatter formats as raw pointer"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            std::int32_t value{};

            const auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

            const auto formatted_ptr = std::format("{}", ptr);
            const auto formatted_raw = std::format<void*>("{}", std::addressof(value));

            expect(eq(formatted_ptr, formatted_raw));
        });
    };

    "std::formatter formats null equivalently to raw pointer"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                ConcretePtr<std::int32_t> ptr{nullptr};

                const auto formatted_ptr = std::format("{}", ptr);
                const auto formatted_raw = std::format<void*>("{}", nullptr);

                expect(eq(formatted_ptr, formatted_raw));
            }
        });
    };

    "std::formatter supports cv-qualified element types"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            const std::int32_t value{};

            const auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

            const auto formatted_ptr = std::format("{}", ptr);
            const auto formatted_raw = std::format<const void*>("{}", std::addressof(value));

            expect(eq(formatted_ptr, formatted_raw));
        });
    };

    "ostream insertion outputs raw pointer representation"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            std::int32_t value{};

            const auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

            std::ostringstream ptr_stream;
            std::ostringstream raw_stream;

            ptr_stream << ptr;
            raw_stream << std::addressof(value);

            expect(eq(ptr_stream.str(), raw_stream.str()));
        });
    };

    "ostream insertion outputs null equivalently to raw pointer"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                const ConcretePtr<std::int32_t> ptr{nullptr};

                std::ostringstream ptr_stream;
                std::ostringstream raw_stream;

                ptr_stream << ptr;
                raw_stream << static_cast<std::int32_t*>(nullptr);

                expect(eq(ptr_stream.str(), raw_stream.str()));
            }
        });
    };

    //NOLINTBEGIN(cppcoreguidelines-pro-type-const-cast): Raw pointer stream inserters lack support for `volatile` pointees, so a `const_cast` to remove the qualifier is required to stream the address value.
    "ostream insertion supports cv-qualified element types"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            volatile std::int32_t value{};

            const auto ptr = base::vocab::pointer_to<ConcretePtr>(value);

            std::ostringstream ptr_stream;
            std::ostringstream raw_stream;

            ptr_stream << ptr;
            raw_stream << const_cast<std::add_pointer_t<std::remove_volatile_t<std::remove_pointer_t<decltype(std::addressof(value))>>>>(
                std::addressof(value)
            );

            expect(eq(ptr_stream.str(), raw_stream.str()));
        });
    };
    //NOLINTEND(cppcoreguidelines-pro-type-const-cast)
}
