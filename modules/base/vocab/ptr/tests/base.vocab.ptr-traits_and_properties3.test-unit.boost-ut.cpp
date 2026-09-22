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
    // Template Constraint Validation
    //============================================================

    "template instantiation checks"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            using base::meta::concepts::instantiable_with;

            expect(that % instantiable_with<ConcretePtr, std::int32_t>);
            expect(that % instantiable_with<ConcretePtr, std::int32_t*>);
            expect(that % instantiable_with<ConcretePtr, std::map<std::string, std::vector<std::int32_t>>>);

            expect(eq(instantiable_with<ConcretePtr, void>, pointer_test_traits<ConcretePtr>::permits_void_pointee));

            expect(that % !instantiable_with<ConcretePtr, std::int32_t&>);
            expect(that % !instantiable_with<ConcretePtr, std::int32_t&&>);
            expect(that % !instantiable_with<ConcretePtr, void(std::int32_t)>);
            expect(that % !instantiable_with<ConcretePtr, void (&)(std::int32_t)>);
            expect(that % !instantiable_with<ConcretePtr, void (*)(std::int32_t, float)>);
            expect(that % !instantiable_with<ConcretePtr, void (**)(std::string, std::int32_t)>);
            expect(that % !instantiable_with<ConcretePtr, void (*******)(std::int32_t)>);
        });
    };

    //============================================================
    // Triviality & ABI properties
    //============================================================

    "triviality"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            const auto test_impl = []<typename Pointee> {
                expect(that % std::is_standard_layout_v<ConcretePtr<Pointee>>)
                    << "is_standard_layout_v(" << reflection::type_name<ConcretePtr<Pointee>>() << ")";
                expect(that % std::is_trivially_copyable_v<ConcretePtr<Pointee>>)
                    << "is_trivially_copyable_v(" << reflection::type_name<ConcretePtr<Pointee>>() << ")";
                expect(that % std::is_trivially_destructible_v<ConcretePtr<Pointee>>)
                    << "is_trivially_destructible_v(" << reflection::type_name<ConcretePtr<Pointee>>() << ")";
                expect(that % std::is_trivially_copy_constructible_v<ConcretePtr<Pointee>>)
                    << "is_trivially_copy_constructible_v(" << reflection::type_name<ConcretePtr<Pointee>>() << ")";
                expect(that % std::is_trivially_move_constructible_v<ConcretePtr<Pointee>>)
                    << "is_trivially_move_constructible_v(" << reflection::type_name<ConcretePtr<Pointee>>() << ")";
                expect(that % std::is_trivially_copy_assignable_v<ConcretePtr<Pointee>>)
                    << "is_trivially_copy_assignable_v(" << reflection::type_name<ConcretePtr<Pointee>>() << ")";
                expect(that % std::is_trivially_move_assignable_v<ConcretePtr<Pointee>>)
                    << "is_trivially_move_assignable_v(" << reflection::type_name<ConcretePtr<Pointee>>() << ")";
                expect(that % std::is_nothrow_constructible_v<ConcretePtr<Pointee>, Pointee&>)
                    << "is_nothrow_constructible_v(" << reflection::type_name<ConcretePtr<Pointee>>() << ", "
                    << reflection::type_name<Pointee> << "&)";
                expect(that % std::is_nothrow_swappable_v<ConcretePtr<Pointee>>)
                    << "is_nothrow_swappable_v(" << reflection::type_name<ConcretePtr<Pointee>>() << ")";
            };

            test_impl.template operator()<std::int32_t>();
            test_impl.template operator()<std::map<std::string, std::vector<std::int32_t>>>();
        });
    };

    "size and alignment match raw pointers"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            using simple_t = std::int32_t;

            expect(eq(sizeof(ConcretePtr<simple_t>), sizeof(simple_t*)));
            expect(eq(alignof(ConcretePtr<simple_t>), alignof(simple_t*)));

            using complex_t = std::map<std::string, std::vector<std::int32_t>>;

            expect(eq(sizeof(ConcretePtr<complex_t>), sizeof(complex_t*)));
            expect(eq(alignof(ConcretePtr<complex_t>), alignof(complex_t*)));
        });
    };

    //============================================================
    // Type properties
    //============================================================

    "type aliases are correct"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            using ptr_t  = ConcretePtr<const std::int32_t>;
            using traits = std::pointer_traits<ptr_t>;

            expect(that % std::same_as<typename traits::pointer, ptr_t>);
            expect(that % std::same_as<typename ptr_t::element_type, const std::int32_t>);
            expect(that % std::same_as<typename traits::element_type, typename ptr_t::element_type>);
            expect(that % std::same_as<typename ptr_t::value_type, std::int32_t>);
            expect(that % std::same_as<typename ptr_t::address_type, const std::int32_t*>);
            expect(that % std::same_as<typename ptr_t::reference, const std::int32_t&>);
            expect(that % std::same_as<typename ptr_t::rvalue_reference, const std::int32_t&&>);
            expect(that % std::same_as<typename ptr_t::difference_type, std::ptrdiff_t>);
            expect(that % std::same_as<typename traits::difference_type, typename ptr_t::difference_type>);
        });
    };

    "`rebind` metafunctions preserve the pointer template while changing the pointee type"_test = [] mutable {
        test_each_pointer_type_with([]<template<typename> typename ConcretePtr> {
            using pointee1 = std::int32_t;
            using pointee2 = const std::map<std::string, std::vector<std::int32_t>>;

            expect(that % std::same_as<typename ConcretePtr<pointee1>::template rebind<pointee2>, ConcretePtr<pointee2>>);
            expect(that % std::same_as<typename std::pointer_traits<ConcretePtr<pointee1>>::template rebind<pointee2>, ConcretePtr<pointee2>>);
        });
    };
}
