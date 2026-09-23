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
    // Covariance
    //============================================================

    "construct base from derived pointer"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            derived_type d_obj;
            const auto d_ptr = base::vocab::pointer_to<ConcretePtr>(d_obj);

            const ConcretePtr<base_type> b_ptr{d_ptr};

            expect(that % b_ptr.get() == std::addressof(d_obj));
        });
    };

    "construct base from derived reference"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            derived_type d_obj;

            const auto b_ptr = base::vocab::pointer_to<ConcretePtr, base_type>(d_obj);

            expect(that % b_ptr.get() == std::addressof(d_obj));
        });
    };

    "assign base from derived pointer"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            derived_type d_obj;
            const auto d_ptr = base::vocab::pointer_to<ConcretePtr>(d_obj);

            base_type b_obj;
            auto b_ptr = base::vocab::pointer_to<ConcretePtr>(b_obj);

            b_ptr = d_ptr;

            expect(that % b_ptr.get() == std::addressof(d_obj));
        });
    };

    "rebind base from derived reference"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                derived_type d_obj;

                base_type b_obj;
                ConcretePtr<base_type> b_ptr{b_obj};

                b_ptr = ConcretePtr{d_obj};

                expect(that % b_ptr.get() == std::addressof(d_obj));
            }
        });
    };

    "assign const base from derived pointer"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            derived_type d_obj;
            const auto d_ptr = base::vocab::pointer_to<ConcretePtr>(d_obj);

            const base_type b_obj;
            auto b_ptr = base::vocab::pointer_to<ConcretePtr, const base_type>(b_obj);

            b_ptr = d_ptr;

            expect(that % b_ptr.get() == std::addressof(d_obj));
        });
    };

    "rebind const base from derived reference"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                derived_type d_obj;

                const base_type b_obj;
                ConcretePtr<const base_type> b_ptr{b_obj};

                b_ptr = ConcretePtr{d_obj};

                expect(that % b_ptr.get() == std::addressof(d_obj));
            }
        });
    };

    "covariant equality comparison"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            expect(that % std::equality_comparable_with<ConcretePtr<base_type>, ConcretePtr<derived_type>>);
            expect(that % std::equality_comparable_with<ConcretePtr<base_type>, derived_type*>);
            expect(that % std::equality_comparable_with<base_type*, ConcretePtr<derived_type>>);
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

    //============================================================
    // Nullability-based exception throwing
    //============================================================

    "constructing from null raw pointer throws according to policy"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            if constexpr (pointer_test_traits<ConcretePtr>::allows_pointer_binding) {
                const std::int32_t value{42};
                const std::int32_t* const bound_source{std::addressof(value)};
                const std::int32_t* const null_source{};

                expect(nothrow([&] {
                    const ConcretePtr<const std::int32_t> ptr{bound_source};

                    expect(eq(*ptr, value));
                    expect(eq(ptr.get(), bound_source));
                }));

                const auto null_init = [&] {
                    const ConcretePtr<const std::int32_t> ptr{null_source};
                    expect(eq(ptr.get(), null_source)); //Skipped when construction throws.
                };

                if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                    expect(nothrow(null_init));
                } else {
                    expect(throws<std::invalid_argument>(null_init));
                }
            }
        });
    };

    "constructing from null smart pointer throws according to policy"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            if constexpr (pointer_test_traits<ConcretePtr>::allows_pointer_binding) {
                const std::int32_t value{42};
                const trivial_smart_ptr<const std::int32_t> bound_source{std::addressof(value)};
                const trivial_smart_ptr<const std::int32_t> null_source{};

                expect(nothrow([&] {
                    const ConcretePtr<const std::int32_t> ptr{bound_source};

                    expect(eq(*ptr, value));
                    expect(eq(ptr.get(), bound_source.get()));
                }));

                const auto null_init = [&] {
                    const ConcretePtr<const std::int32_t> ptr{null_source};
                    expect(eq(ptr.get(), null_source.get())); //Skipped when construction throws.
                };

                if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                    expect(nothrow(null_init));
                } else {
                    expect(throws<std::invalid_argument>(null_init));
                }
            }
        });
    };

    "assigning from null raw pointer throws according to policy"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            if constexpr (pointer_test_traits<ConcretePtr>::allows_pointer_binding) {
                const std::int32_t value{42};
                const std::int32_t other{};

                const std::int32_t* const bound_source{std::addressof(value)};
                const std::int32_t* const null_source{nullptr};

                ConcretePtr<const std::int32_t> ptr{other};

                expect(nothrow([&] {
                    ptr = bound_source;

                    expect(eq(*ptr, value));
                    expect(eq(ptr.get(), bound_source));
                }));

                const auto null_assign = [&] { ptr = null_source; };

                if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                    expect(nothrow(null_assign));

                    //Assignment successfully modifies stored address.
                    expect(eq(ptr.get(), null_source));
                } else {
                    expect(throws<std::invalid_argument>(null_assign));

                    //Invariant preserved after failed assignment
                    expect(eq(*ptr, *bound_source));
                    expect(eq(ptr.get(), bound_source));
                }
            }
        });
    };

    "assigning from null smart pointer throws according to policy"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            if constexpr (pointer_test_traits<ConcretePtr>::allows_pointer_binding) {
                const std::int32_t value{42};
                const std::int32_t other{};

                const trivial_smart_ptr<const std::int32_t> bound_source{std::addressof(value)};
                const trivial_smart_ptr<std::int32_t> null_source{};

                ConcretePtr<const std::int32_t> ptr{other};

                expect(nothrow([&] {
                    ptr = bound_source;

                    expect(eq(*ptr, value));
                    expect(eq(ptr.get(), bound_source.get()));
                }));

                const auto null_assign = [&] { ptr = null_source; };

                if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                    expect(nothrow(null_assign));

                    //Assignment successfully modifies stored address.
                    expect(eq(ptr.get(), null_source.get()));
                } else {
                    expect(throws<std::invalid_argument>(null_assign));

                    //Invariant preserved after failed assignment
                    expect(eq(*ptr, value));
                    expect(eq(ptr.get(), bound_source.get()));
                }
            }
        });
    };
}

//NOLINTEND(readability-function-size, readability-function-cognitive-complexity)
