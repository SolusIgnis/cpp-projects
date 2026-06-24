// SPDX-License-Identifier: Apache-2.0
// Unit tests for base.vocab.ptr

import base.vocab.ptr;
import ut;
import std;

import base.meta.concepts;

using namespace ut;

namespace {
    template<template<typename> typename Ptr>
    struct pointer_test_traits_base;

    template<>
    struct pointer_test_traits_base<base::vocab::ptr::dependency_ptr> {
        static constexpr bool is_nullable              = false;
        static constexpr bool has_arithmetic_traversal = false;
        static constexpr bool allows_pointer_binding   = false;
        static constexpr bool allows_reference_binding = true;
    };

    template<>
    struct pointer_test_traits_base<base::vocab::ptr::required_ptr> {
        static constexpr bool is_nullable              = false;
        static constexpr bool has_arithmetic_traversal = false;
        static constexpr bool allows_pointer_binding   = true;
        static constexpr bool allows_reference_binding = true;
    };

    template<>
    struct pointer_test_traits_base<base::vocab::ptr::alias_ptr> {
        static constexpr bool is_nullable              = true;
        static constexpr bool has_arithmetic_traversal = false;
        static constexpr bool allows_pointer_binding   = true;
        static constexpr bool allows_reference_binding = true;
    };

    template<>
    struct pointer_test_traits_base<base::vocab::ptr::cursor_ptr> {
        static constexpr bool is_nullable              = false;
        static constexpr bool has_arithmetic_traversal = true;
        static constexpr bool allows_pointer_binding   = true;
        static constexpr bool allows_reference_binding = true;
    };

    template<template<typename> typename Ptr>
    struct pointer_test_traits : pointer_test_traits_base<Ptr> {
        static constexpr bool permits_void_pointee = !pointer_test_traits_base<Ptr>::has_arithmetic_traversal && pointer_test_traits_base<Ptr>::allows_pointer_binding;
    };

    template<typename Lambda>
    constexpr void test_each_pointer_type_with(Lambda&& test_impl)
    {
        test_impl.template operator()<base::vocab::ptr::dependency_ptr>();
       // test_impl.template operator()<base::vocab::ptr::required_ptr>();
       // test_impl.template operator()<base::vocab::ptr::alias_ptr>();
       // test_impl.template operator()<base::vocab::ptr::cursor_ptr>();
    }

    template<typename T>
    concept HasAddition = requires(T t) { t + 1; } || requires(T t) { 1 + t; };

    template<typename T>
    concept HasSubtraction = requires(T t) { t - 1; };

    template<typename T>
    concept HasDifference = requires(T t) { t - t; };

    template<typename T>
    concept HasPreIncrement = requires(T t) { ++t; };

    template<typename T>
    concept HasPostIncrement = requires(T t) { t++; };

    template<typename T>
    concept HasPreDecrement = requires(T t) { --t; };

    template<typename T>
    concept HasPostDecrement = requires(T t) { t--; };

    template<typename T>
    concept Dereferenceable = requires(T t) { *t; };

    template<typename T>
    concept ArrowAccessible = requires(T t) { t.operator->(); };

    struct base_type {
        int value{0};
    };

    struct derived_type : base_type {
        int extra{42};
    };

    template<typename T>
    struct trivial_smart_ptr {
        T* address{};

        T* get() const { return address; }
    };

    suite concrete_pointer_parameterized_tests = [] mutable {
        //============================================================
        // Template Constraint Validation
        //============================================================

        "template instantiation checks"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                using base::meta::concepts::InstantiableWith;
                expect(eq(InstantiableWith<ConcretePtr, std::int32_t>, true));
                expect(eq(InstantiableWith<ConcretePtr, std::int32_t*>, true));
                expect(eq(InstantiableWith<ConcretePtr, std::map<std::string, std::vector<std::int32_t>>>, true));

                expect(eq(InstantiableWith<ConcretePtr, void>, pointer_test_traits<ConcretePtr>::permits_void_pointee));

                expect(eq(InstantiableWith<ConcretePtr, std::int32_t&>, false));
                expect(eq(InstantiableWith<ConcretePtr, std::int32_t&&>, false));
                expect(eq(InstantiableWith<ConcretePtr, void(int)>, false));
                expect(eq(InstantiableWith<ConcretePtr, void (&)(int)>, false));
                expect(eq(InstantiableWith<ConcretePtr, void (*)(int, float)>, false));
                expect(eq(InstantiableWith<ConcretePtr, void (**)(std::string, int)>, false));
                expect(eq(InstantiableWith<ConcretePtr, void (*******)(int)>, false));
            });
        };

        //============================================================
        // Triviality & ABI properties
        //============================================================

        "triviality"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                auto test_impl = []<typename Pointee>(){
                    expect(eq(std::is_standard_layout_v<ConcretePtr<Pointee>>, true));
                    expect(eq(std::is_trivially_copyable_v<ConcretePtr<Pointee>>, true));
                    expect(eq(std::is_trivially_destructible_v<ConcretePtr<Pointee>>, true));
                    expect(eq(std::is_trivially_copy_constructible_v<ConcretePtr<Pointee>>, true));
                    expect(eq(std::is_trivially_move_constructible_v<ConcretePtr<Pointee>>, true));
                    expect(eq(std::is_trivially_copy_assignable_v<ConcretePtr<Pointee>>, true));
                    expect(eq(std::is_trivially_move_assignable_v<ConcretePtr<Pointee>>, true));
                    expect(eq(std::is_nothrow_constructible_v<ConcretePtr<Pointee>, Pointee&>, true));
                    expect(eq(std::is_nothrow_swappable_v<ConcretePtr<Pointee>>, true));
                };

                test_impl.template operator()<std::int32_t>();
                test_impl.template operator()<std::map<std::string, std::vector<std::int32_t>>>();
            });
        };

        "size and alignment match raw pointers"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                using simple_t = std::int32_t;

                expect(eq(sizeof(ConcretePtr<simple_t>) == sizeof(simple_t*), true));
                expect(eq(alignof(ConcretePtr<simple_t>) == alignof(simple_t*), true));

                using complex_t = std::map<std::string, std::vector<std::int32_t>>;

                expect(eq(sizeof(ConcretePtr<complex_t>) == sizeof(complex_t*), true));
                expect(eq(alignof(ConcretePtr<complex_t>) == alignof(complex_t*), true));
            });
        };

        //============================================================
        // Type properties
        //============================================================

        "type aliases are correct"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                using T = ConcretePtr<const std::int32_t>;

                constexpr bool element = std::same_as<typename T::element_type, const std::int32_t>;
                constexpr bool value   = std::same_as<typename T::value_type, std::int32_t>;
                constexpr bool pointer = std::same_as<typename T::address_type, const std::int32_t*>;
                constexpr bool lref    = std::same_as<typename T::reference, const std::int32_t&>;
                constexpr bool rref    = std::same_as<typename T::rvalue_reference, const std::int32_t&&>;
                constexpr bool ptrdiff = std::same_as<typename T::difference_type, std::ptrdiff_t>;

                expect(eq(element, true));
                expect(eq(value, true));
                expect(eq(pointer, true));
                expect(eq(lref, true));
                expect(eq(rref, true));
                expect(eq(ptrdiff, true));
            });
        };

        //============================================================
        // Construction (Initial Binding) / Assignment (Rebinding)
        //============================================================

        "bindable from nullptr according to nullability policy"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                expect(eq(std::constructible_from<ConcretePtr<std::int32_t>, std::nullptr_t>, pointer_test_traits<ConcretePtr>::is_nullable));
                expect(eq(std::is_assignable_v<ConcretePtr<std::int32_t>&, std::nullptr_t>, pointer_test_traits<ConcretePtr>::is_nullable));
            });
        };

        "pointer binding according to policies"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                auto verify_binding_operations = []<typename Pointee, typename Source, bool IsConstructibleFrom, bool IsConvertibleFrom>(){
                    //Explicitly constructible unless removing qualifier
                    expect(eq(std::constructible_from<ConcretePtr<Pointee>, Source>, IsConstructibleFrom));
                    expect(eq(std::constructible_from<ConcretePtr<Pointee>, const Source>, false));
                    expect(eq(std::constructible_from<ConcretePtr<Pointee>, volatile Source>, false));
                    expect(eq(std::constructible_from<ConcretePtr<Pointee>, const volatile Source>, false));
                    expect(eq(std::constructible_from<ConcretePtr<const Pointee>, Source>, IsConstructibleFrom));
                    expect(eq(std::constructible_from<ConcretePtr<const Pointee>, const Source>, IsConstructibleFrom));
                    expect(eq(std::constructible_from<ConcretePtr<const Pointee>, volatile Source>, false));
                    expect(eq(std::constructible_from<ConcretePtr<const Pointee>, const volatile Source>, false));
                    expect(eq(std::constructible_from<ConcretePtr<volatile Pointee>, Source>, IsConstructibleFrom));
                    expect(eq(std::constructible_from<ConcretePtr<volatile Pointee>, const Source>, false));
                    expect(eq(std::constructible_from<ConcretePtr<volatile Pointee>, volatile Source>, IsConstructibleFrom));
                    expect(eq(std::constructible_from<ConcretePtr<volatile Pointee>, const volatile Source>, false));
                    expect(eq(std::constructible_from<ConcretePtr<const volatile Pointee>, Source>, IsConstructibleFrom));
                    expect(eq(std::constructible_from<ConcretePtr<const volatile Pointee>, const Source>, IsConstructibleFrom));
                    expect(eq(std::constructible_from<ConcretePtr<const volatile Pointee>, volatile Source>, IsConstructibleFrom));
                    expect(eq(std::constructible_from<ConcretePtr<const volatile Pointee>, const volatile Source>, IsConstructibleFrom));

                    //Implicitly convertible unless removing qualifier
                    expect(eq(std::convertible_to<Source, ConcretePtr<Pointee>>, IsConvertibleFrom));
                    expect(eq(std::convertible_to<const Source, ConcretePtr<Pointee>>, false));
                    expect(eq(std::convertible_to<volatile Source, ConcretePtr<Pointee>>, false));
                    expect(eq(std::convertible_to<const volatile Source, ConcretePtr<Pointee>>, false));
                    expect(eq(std::convertible_to<Source, ConcretePtr<const Pointee>>, IsConvertibleFrom));
                    expect(eq(std::convertible_to<const Source, ConcretePtr<const Pointee>>, IsConvertibleFrom));
                    expect(eq(std::convertible_to<volatile Source, ConcretePtr<const Pointee>>, false));
                    expect(eq(std::convertible_to<const volatile Source, ConcretePtr<const Pointee>>, false));
                    expect(eq(std::convertible_to<Source, ConcretePtr<volatile Pointee>>, IsConvertibleFrom));
                    expect(eq(std::convertible_to<const Source, ConcretePtr<volatile Pointee>>, false));
                    expect(eq(std::convertible_to<volatile Source, ConcretePtr<volatile Pointee>>, IsConvertibleFrom));
                    expect(eq(std::convertible_to<const volatile Source, ConcretePtr<volatile Pointee>>, false));
                    expect(eq(std::convertible_to<Source, ConcretePtr<const volatile Pointee>>, IsConvertibleFrom));
                    expect(eq(std::convertible_to<const Source, ConcretePtr<const volatile Pointee>>, IsConvertibleFrom));
                    expect(eq(std::convertible_to<volatile Source, ConcretePtr<const volatile Pointee>>, IsConvertibleFrom));
                    expect(eq(std::convertible_to<const volatile Source, ConcretePtr<const volatile Pointee>>, IsConvertibleFrom));

                    //Assignable unless removing qualifier
                    expect(eq(std::is_assignable_v<ConcretePtr<Pointee>&, Source>, IsConstructibleFrom));
                    expect(eq(std::is_assignable_v<ConcretePtr<Pointee>&, const Source>, false));
                    expect(eq(std::is_assignable_v<ConcretePtr<Pointee>&, volatile Source>, false));
                    expect(eq(std::is_assignable_v<ConcretePtr<Pointee>&, const volatile Source>, false));
                    expect(eq(std::is_assignable_v<ConcretePtr<const Pointee>&, Source>, IsConstructibleFrom));
                    expect(eq(std::is_assignable_v<ConcretePtr<const Pointee>&, const Source>, IsConstructibleFrom));
                    expect(eq(std::is_assignable_v<ConcretePtr<const Pointee>&, volatile Source>, false));
                    expect(eq(std::is_assignable_v<ConcretePtr<const Pointee>&, const volatile Source>, false));
                    expect(eq(std::is_assignable_v<ConcretePtr<volatile Pointee>&, Source>, IsConstructibleFrom));
                    expect(eq(std::is_assignable_v<ConcretePtr<volatile Pointee>&, const Source>, false));
                    expect(eq(std::is_assignable_v<ConcretePtr<volatile Pointee>&, volatile Source>, IsConstructibleFrom));
                    expect(eq(std::is_assignable_v<ConcretePtr<volatile Pointee>&, const volatile Source>, false));
                    expect(eq(std::is_assignable_v<ConcretePtr<const volatile Pointee>&, Source>, IsConstructibleFrom));
                    expect(eq(std::is_assignable_v<ConcretePtr<const volatile Pointee>&, const Source>, IsConstructibleFrom));
                    expect(eq(std::is_assignable_v<ConcretePtr<const volatile Pointee>&, volatile Source>, IsConstructibleFrom));
                    expect(eq(std::is_assignable_v<ConcretePtr<const volatile Pointee>&, const volatile Source>, IsConstructibleFrom));
                };

                //Reference binding is explicit when allowed
                verify_binding_operations.template operator()<std::int32_t, std::int32_t&, pointer_test_traits<ConcretePtr>::allows_reference_binding, false>();

                //Pointer binding allows implicit conversion
                //verify_binding_operations.template operator()<std::int32_t, std::int32_t*, pointer_test_traits<ConcretePtr>::allows_pointer_binding, pointer_test_traits<ConcretePtr>::allows_pointer_binding>();
                //verify_binding_operations.template operator()<std::int32_t, trivial_smart_ptr<std::int32_t>&, pointer_test_traits<ConcretePtr>::allows_pointer_binding, pointer_test_traits<ConcretePtr>::allows_pointer_binding>();
            });
        };

        "not bindable from rvalue"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                expect(eq(std::constructible_from<ConcretePtr<std::int32_t>, std::int32_t>, false));
                expect(eq(std::constructible_from<ConcretePtr<const std::int32_t>, std::int32_t>, false));
                expect(eq(std::constructible_from<ConcretePtr<const std::int32_t>, const std::int32_t>, false));

                expect(eq(std::constructible_from<ConcretePtr<std::int32_t>, std::int32_t&&>, false));
                expect(eq(std::constructible_from<ConcretePtr<const std::int32_t>, std::int32_t&&>, false));
                expect(eq(std::constructible_from<ConcretePtr<const std::int32_t>, const std::int32_t&&>, false));

                expect(eq(std::is_assignable_v<ConcretePtr<std::int32_t>&, std::int32_t&&>, false));
                expect(eq(std::is_assignable_v<ConcretePtr<const std::int32_t>&, std::int32_t&&>, false));
                expect(eq(std::is_assignable_v<ConcretePtr<const std::int32_t>&, const std::int32_t&&>, false));
            });
        };

        //============================================================
        // Pointer semantics
        //============================================================

        
    };
} //namespace

int main() {}
