// SPDX-License-Identifier: Apache-2.0
// Parameterized unit tests for base.vocab.ptr

import base.vocab.ptr;
import ut;
import std;

import base.meta.concepts;

#include "base.vocab.ptr-common_fixtures.boost-ut.hpp"

using namespace ut;

//NOLINTNEXTLINE(bugprone-exception-escape): Test framework.
int main()
{
    //============================================================
    // Pointer Casting & Lifetime Transmutation
    //============================================================

    //NOLINTBEGIN(misc-const-correctness): Readability suffers with const correctness in these tests.
        "const_pointer_cast alters pointee cv-qualifications"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                const auto test_cast = []<typename Source, typename Destination> {
                    Source value{};

                    auto source = base::vocab::pointer_to<ConcretePtr>(value);
                    auto result = const_pointer_cast<Destination>(source);

                    expect(eq(std::same_as<decltype(result), ConcretePtr<Destination>>, true));
                    expect(eq(result.get() == std::addressof(value), true));
                };

                test_cast.template operator()<std::int32_t, std::int32_t>();
                test_cast.template operator()<std::int32_t, const std::int32_t>();
                test_cast.template operator()<std::int32_t, volatile std::int32_t>();
                test_cast.template operator()<std::int32_t, const volatile std::int32_t>();
                test_cast.template operator()<const std::int32_t, std::int32_t>();
                test_cast.template operator()<const std::int32_t, const std::int32_t>();
                test_cast.template operator()<const std::int32_t, volatile std::int32_t>();
                test_cast.template operator()<const std::int32_t, const volatile std::int32_t>();
                test_cast.template operator()<volatile std::int32_t, std::int32_t>();
                test_cast.template operator()<volatile std::int32_t, const std::int32_t>();
                test_cast.template operator()<volatile std::int32_t, volatile std::int32_t>();
                test_cast.template operator()<volatile std::int32_t, const volatile std::int32_t>();
                test_cast.template operator()<const volatile std::int32_t, std::int32_t>();
                test_cast.template operator()<const volatile std::int32_t, const std::int32_t>();
                test_cast.template operator()<const volatile std::int32_t, volatile std::int32_t>();
                test_cast.template operator()<const volatile std::int32_t, const volatile std::int32_t>();
            });
        };

        "const_pointer_cast preserves null state"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
                if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                    ConcretePtr<std::int32_t> source{nullptr};

                    const auto result = const_pointer_cast<const std::int32_t>(source);

                    expect(eq(result == nullptr, true));
                }
            });
        };
    };
    //NOLINTEND(misc-const-correctness)
}
