// SPDX-License-Identifier: Apache-2.0
// Parameterized unit tests for base.vocab.ptr

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
        test_impl.template operator()<base::vocab::ptr::required_ptr>();
        test_impl.template operator()<base::vocab::ptr::alias_ptr>();
        test_impl.template operator()<base::vocab::ptr::cursor_ptr>();
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

    struct ref_tag;
    struct ptr_tag;
    struct smart_ptr_tag;
    
    template<typename T, typename Tag>
    struct source_category;
    
    template<typename T>
    struct source_category<T, ref_tag> { using type = std::add_lvalue_reference_t<T>; };
    
    template<typename T>
    struct source_category<T, ptr_tag> { using type = std::add_pointer_t<T>; };
    
    template<typename T>
    struct source_category<T, smart_ptr_tag> { using type = trivial_smart_ptr<T>; };
    
    template<typename T, typename Tag>
    using source_t = typename source_category<T, Tag>::type;
   
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
                auto verify_binding_operations = []<typename Pointee, typename SourceTag, bool IsConstructibleFrom, bool IsConvertibleFrom>(){
                    //Explicitly constructible unless removing qualifier
                    expect(eq(std::constructible_from<ConcretePtr<Pointee>, source_t<Pointee, SourceTag>>, IsConstructibleFrom));
                    expect(eq(std::constructible_from<ConcretePtr<Pointee>, source_t<const Pointee, SourceTag>>, false));
                    expect(eq(std::constructible_from<ConcretePtr<Pointee>, source_t<volatile Pointee, SourceTag>>, false));
                    expect(eq(std::constructible_from<ConcretePtr<Pointee>, source_t<const volatile Pointee, SourceTag>>, false));
                    expect(eq(std::constructible_from<ConcretePtr<const Pointee>, source_t<Pointee, SourceTag>>, IsConstructibleFrom));
                    expect(eq(std::constructible_from<ConcretePtr<const Pointee>, source_t<const Pointee, SourceTag>>, IsConstructibleFrom));
                    expect(eq(std::constructible_from<ConcretePtr<const Pointee>, source_t<volatile Pointee, SourceTag>>, false));
                    expect(eq(std::constructible_from<ConcretePtr<const Pointee>, source_t<const volatile Pointee, SourceTag>>, false));
                    expect(eq(std::constructible_from<ConcretePtr<volatile Pointee>, source_t<Pointee, SourceTag>>, IsConstructibleFrom));
                    expect(eq(std::constructible_from<ConcretePtr<volatile Pointee>, source_t<const Pointee, SourceTag>>, false));
                    expect(eq(std::constructible_from<ConcretePtr<volatile Pointee>, source_t<volatile Pointee, SourceTag>>, IsConstructibleFrom));
                    expect(eq(std::constructible_from<ConcretePtr<volatile Pointee>, source_t<const volatile Pointee, SourceTag>>, false));
                    expect(eq(std::constructible_from<ConcretePtr<const volatile Pointee>, source_t<Pointee, SourceTag>>, IsConstructibleFrom));
                    expect(eq(std::constructible_from<ConcretePtr<const volatile Pointee>, source_t<const Pointee, SourceTag>>, IsConstructibleFrom));
                    expect(eq(std::constructible_from<ConcretePtr<const volatile Pointee>, source_t<volatile Pointee, SourceTag>>, IsConstructibleFrom));
                    expect(eq(std::constructible_from<ConcretePtr<const volatile Pointee>, source_t<const volatile Pointee, SourceTag>>, IsConstructibleFrom));

                    //Implicitly convertible unless removing qualifier
                    expect(eq(std::convertible_to<source_t<Pointee, SourceTag>, ConcretePtr<Pointee>>, IsConvertibleFrom));
                    expect(eq(std::convertible_to<source_t<const Pointee, SourceTag>, ConcretePtr<Pointee>>, false));
                    expect(eq(std::convertible_to<source_t<volatile Pointee, SourceTag>, ConcretePtr<Pointee>>, false));
                    expect(eq(std::convertible_to<source_t<const volatile Pointee, SourceTag>, ConcretePtr<Pointee>>, false));
                    expect(eq(std::convertible_to<source_t<Pointee, SourceTag>, ConcretePtr<const Pointee>>, IsConvertibleFrom));
                    expect(eq(std::convertible_to<source_t<const Pointee, SourceTag>, ConcretePtr<const Pointee>>, IsConvertibleFrom));
                    expect(eq(std::convertible_to<source_t<volatile Pointee, SourceTag>, ConcretePtr<const Pointee>>, false));
                    expect(eq(std::convertible_to<source_t<const volatile Pointee, SourceTag>, ConcretePtr<const Pointee>>, false));
                    expect(eq(std::convertible_to<source_t<Pointee, SourceTag>, ConcretePtr<volatile Pointee>>, IsConvertibleFrom));
                    expect(eq(std::convertible_to<source_t<const Pointee, SourceTag>, ConcretePtr<volatile Pointee>>, false));
                    expect(eq(std::convertible_to<source_t<volatile Pointee, SourceTag>, ConcretePtr<volatile Pointee>>, IsConvertibleFrom));
                    expect(eq(std::convertible_to<source_t<const volatile Pointee, SourceTag>, ConcretePtr<volatile Pointee>>, false));
                    expect(eq(std::convertible_to<source_t<Pointee, SourceTag>, ConcretePtr<const volatile Pointee>>, IsConvertibleFrom));
                    expect(eq(std::convertible_to<source_t<const Pointee, SourceTag>, ConcretePtr<const volatile Pointee>>, IsConvertibleFrom));
                    expect(eq(std::convertible_to<source_t<volatile Pointee, SourceTag>, ConcretePtr<const volatile Pointee>>, IsConvertibleFrom));
                    expect(eq(std::convertible_to<source_t<const volatile Pointee, SourceTag>, ConcretePtr<const volatile Pointee>>, IsConvertibleFrom));

                    //Assignable unless removing qualifier
                    expect(eq(std::is_assignable_v<ConcretePtr<Pointee>&, source_t<Pointee, SourceTag>>, IsConstructibleFrom));
                    expect(eq(std::is_assignable_v<ConcretePtr<Pointee>&, source_t<const Pointee, SourceTag>>, false));
                    expect(eq(std::is_assignable_v<ConcretePtr<Pointee>&, source_t<volatile Pointee, SourceTag>>, false));
                    expect(eq(std::is_assignable_v<ConcretePtr<Pointee>&, source_t<const volatile Pointee, SourceTag>>, false));
                    expect(eq(std::is_assignable_v<ConcretePtr<const Pointee>&, source_t<Pointee, SourceTag>>, IsConstructibleFrom));
                    expect(eq(std::is_assignable_v<ConcretePtr<const Pointee>&, source_t<const Pointee, SourceTag>>, IsConstructibleFrom));
                    expect(eq(std::is_assignable_v<ConcretePtr<const Pointee>&, source_t<volatile Pointee, SourceTag>>, false));
                    expect(eq(std::is_assignable_v<ConcretePtr<const Pointee>&, source_t<const volatile Pointee, SourceTag>>, false));
                    expect(eq(std::is_assignable_v<ConcretePtr<volatile Pointee>&, source_t<Pointee, SourceTag>>, IsConstructibleFrom));
                    expect(eq(std::is_assignable_v<ConcretePtr<volatile Pointee>&, source_t<const Pointee, SourceTag>>, false));
                    expect(eq(std::is_assignable_v<ConcretePtr<volatile Pointee>&, source_t<volatile Pointee, SourceTag>>, IsConstructibleFrom));
                    expect(eq(std::is_assignable_v<ConcretePtr<volatile Pointee>&, source_t<const volatile Pointee, SourceTag>>, false));
                    expect(eq(std::is_assignable_v<ConcretePtr<const volatile Pointee>&, source_t<Pointee, SourceTag>>, IsConstructibleFrom));
                    expect(eq(std::is_assignable_v<ConcretePtr<const volatile Pointee>&, source_t<const Pointee, SourceTag>>, IsConstructibleFrom));
                    expect(eq(std::is_assignable_v<ConcretePtr<const volatile Pointee>&, source_t<volatile Pointee, SourceTag>>, IsConstructibleFrom));
                    expect(eq(std::is_assignable_v<ConcretePtr<const volatile Pointee>&, source_t<const volatile Pointee, SourceTag>>, IsConstructibleFrom));
                };

                //Reference binding is explicit when allowed
                verify_binding_operations.template operator()<std::int32_t, ref_tag, pointer_test_traits<ConcretePtr>::allows_reference_binding, false>();

                //Pointer binding allows implicit conversion
                verify_binding_operations.template operator()<std::int32_t, ptr_tag, pointer_test_traits<ConcretePtr>::allows_pointer_binding, pointer_test_traits<ConcretePtr>::allows_pointer_binding>();
                verify_binding_operations.template operator()<std::int32_t, smart_ptr_tag, pointer_test_traits<ConcretePtr>::allows_pointer_binding, pointer_test_traits<ConcretePtr>::allows_pointer_binding>();
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

        "operator* dereferences correctly"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                int value = 55;
                ConcretePtr<std::int32_t> ptr{value};

                expect(eq(*ptr, value));
            });
        };

        "operator-> provides member access"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                base_type obj{123};
                ConcretePtr<base_type> ptr{obj};

                expect(eq(ptr->value, obj.value));
            });
        };

        "implicit conversion to raw pointer"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                using test_type = std::int32_t;

                expect(eq(std::convertible_to<ConcretePtr<test_type>, test_type*>, true));
                expect(eq(std::convertible_to<ConcretePtr<test_type>, const test_type*>, true));
                expect(eq(std::convertible_to<ConcretePtr<test_type>, volatile test_type*>, true));
                expect(eq(std::convertible_to<ConcretePtr<test_type>, const volatile test_type*>, true));
                expect(eq(std::convertible_to<ConcretePtr<const test_type>, test_type*>, false));
                expect(eq(std::convertible_to<ConcretePtr<const test_type>, const test_type*>, true));
                expect(eq(std::convertible_to<ConcretePtr<const test_type>, volatile test_type*>, false));
                expect(eq(std::convertible_to<ConcretePtr<const test_type>, const volatile test_type*>, true));
                expect(eq(std::convertible_to<ConcretePtr<volatile test_type>, test_type*>, false));
                expect(eq(std::convertible_to<ConcretePtr<volatile test_type>, const test_type*>, false));
                expect(eq(std::convertible_to<ConcretePtr<volatile test_type>, volatile test_type*>, true));
                expect(eq(std::convertible_to<ConcretePtr<volatile test_type>, const volatile test_type*>, true));
                expect(eq(std::convertible_to<ConcretePtr<const volatile test_type>, test_type*>, false));
                expect(eq(std::convertible_to<ConcretePtr<const volatile test_type>, const test_type*>, false));
                expect(eq(std::convertible_to<ConcretePtr<const volatile test_type>, volatile test_type*>, false));
                expect(eq(std::convertible_to<ConcretePtr<const volatile test_type>, const volatile test_type*>, true));
            });
        };

        "conversion to raw pointer preserves address"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                const int value{};
                ConcretePtr<const std::int32_t> ptr{value};

                const std::int32_t* raw = ptr;

                expect(eq(raw, std::addressof(value)));
            });
        };

        "get returns raw pointer"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                const int value{};
                ConcretePtr<const std::int32_t> ptr{value};

                expect(eq(ptr.get(), std::addressof(value)));
            });
        };

        "contextual boolean conversion is supported"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                expect(eq(std::constructible_from<bool, ConcretePtr<std::int32_t>>, true));

                const int value{};
                ConcretePtr<const std::int32_t> ptr{value};

                bool converted{false};
                if (ptr)
                    converted = true;

                expect(eq(static_cast<bool>(ptr), true));
                expect(eq(!ptr, false));
                expect(eq(converted, true));
            });
        };

        //============================================================
        // Rebinding
        //============================================================

        "rebind via assignment from reference"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    const int a{};
                    const int b = 2;

                    ConcretePtr<const std::int32_t> ptr{a};
                    ptr = b;

                    expect(eq(*ptr, b));
                    expect(eq(ptr.get(), std::addressof(b)));
                }
            });
        };

        "rebind via `rebind` call with reference argument"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    const int a{};
                    const int b = 2;

                    ConcretePtr<const std::int32_t> ptr{a};
                    ptr.rebind(b);

                    expect(eq(*ptr, b));
                    expect(eq(ptr.get(), std::addressof(b)));
                }
            });
        };

        "rebind via `reset` call with reference argument"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    const int a{};
                    const int b = 2;

                    ConcretePtr<const std::int32_t> ptr{a};
                    ptr.reset(b);

                    expect(eq(*ptr, b));
                    expect(eq(ptr.get(), std::addressof(b)));
                }
            });
        };

        "moved-from object may be rebound"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    std::int32_t a = 1;
                    std::int32_t b = 2;

                    ConcretePtr<std::int32_t> source{a};
                    ConcretePtr<std::int32_t> target{std::move(source)};

                    //Rebind moved-from `source` to reference `b`
                    source = b;

                    expect(eq(*target, a));
                    expect(eq(*source, b));
                }
            });
        };

        "rebind via `reset()` disengages the pointer"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                    const int a{};

                    ConcretePtr<const std::int32_t> ptr{a};
                    ptr.reset();

                    expect(eq(!ptr, true));
                }
            });
        };

        //============================================================
        // Nullability-based exception throwing
        //============================================================

        "constructing from null raw pointer throws according to policy"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_pointer_binding) {
                    const std::int32_t value{42};
                    const std::int32_t* const bound_source{std::addressof(value)};
                    const std::int32_t* const null_source{};

                    bool threw_when_bound = false;
                    try {
                        const ConcretePtr<const std::int32_t> ptr{bound_source};
        
                        expect(eq(*ptr, value));
                        expect(eq(ptr.get(), bound_source));
                    } catch (...) {
                        threw_when_bound = true;
                    }

                    expect(eq(threw_when_bound, false));

                    bool threw_when_null = false;
                    bool wrong_exception = false;
                    try {
                        [[maybe_unused]] ConcretePtr<const std::int32_t> dummy_ptr{null_source};
                    } catch (const std::invalid_argument&) {
                        threw_when_null = true;
                    } catch (...) {
                        wrong_exception = true;
                    }

                    expect(eq(threw_when_null, !pointer_test_traits<ConcretePtr>::is_nullable));
                    expect(eq(wrong_exception, false));
                }
            });
        };

        "constructing from null smart pointer throws according to policy"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_pointer_binding) {
                    const std::int32_t value{42};
                    const trivial_smart_ptr<const std::int32_t> bound_source{std::addressof(value)};
                    const trivial_smart_ptr<const std::int32_t> null_source{};

                    bool threw_when_bound = false;
                    try {
                        const ConcretePtr<const std::int32_t> ptr{bound_source};
        
                        expect(eq(*ptr, value));
                        expect(eq(ptr.get(), bound_source.get()));
                    } catch (...) {
                        threw_when_bound = true;
                    }

                    expect(eq(threw_when_bound, false));

                    bool threw_when_null = false;
                    bool wrong_exception = false;
                    try {
                        [[maybe_unused]] ConcretePtr<const std::int32_t> dummy_ptr{null_source};
                    } catch (const std::invalid_argument&) {
                        threw_when_null = true;
                    } catch (...) {
                        wrong_exception = true;
                    }

                    expect(eq(threw_when_null, !pointer_test_traits<ConcretePtr>::is_nullable));
                    expect(eq(wrong_exception, false));
                }
            });
        };

        "assigning from null raw pointer throws according to policy"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_pointer_binding) {
                    const std::int32_t value{42};
                    const std::int32_t other{};

                    const std::int32_t* const bound_source{std::addressof(value)};
                    const std::int32_t* const null_source{nullptr};

                    ConcretePtr<const std::int32_t> ptr{other};

                    bool threw_when_bound = false;
                    try {
                        ptr = bound_source;
                    } catch (...) {
                        threw_when_bound = true;
                    }

                    expect(eq(threw_when_bound, false));
                    expect(eq(*ptr, *bound_source));
                    expect(eq(ptr.get(), bound_source));

                    bool threw_when_null = false;
                    bool wrong_exception = false;
                    try {
                        ptr = null_source;
                    } catch (const std::invalid_argument&) {
                        threw_when_null = true;
                    } catch (...) {
                        wrong_exception = true;
                    }

                    expect(eq(threw_when_null, !pointer_test_traits<ConcretePtr>::is_nullable));
                    expect(eq(wrong_exception, false));

                    if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                        expect(eq(ptr.get(), null_source));
                    } else {
                        //Invariant preserved after failed assignment
                        expect(eq(*ptr, *bound_source));
                        expect(eq(ptr.get(), bound_source));
                    }
                }
            });
        };

        "assigning from null smart pointer throws according to policy"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_pointer_binding) {
                    const std::int32_t value{42};
                    const std::int32_t other{};

                    const trivial_smart_ptr<const std::int32_t> bound_source{std::addressof(value)};
                    const trivial_smart_ptr<std::int32_t> null_source{};

                    ConcretePtr<const std::int32_t> ptr{other};

                    bool threw_when_bound = false;
                    try {
                        ptr = bound_source;
                    } catch (...) {
                        threw_when_bound = true;
                    }

                    expect(eq(threw_when_bound, false));
                    expect(eq(*ptr, value));
                    expect(eq(ptr.get(), bound_source.get()));

                    bool threw_when_null = false;
                    bool wrong_exception = false;
                    try {
                        ptr = null_source;
                    } catch (const std::invalid_argument&) {
                        threw_when_null = true;
                    } catch (...) {
                        wrong_exception = true;
                    }

                    expect(eq(threw_when_null, !pointer_test_traits<ConcretePtr>::is_nullable));
                    expect(eq(wrong_exception, false));

                    if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                        expect(eq(ptr.get() == null_source.get(), true));
                    } else {
                        //Invariant preserved after failed assignment
                        expect(eq(*ptr, value));
                        expect(eq(ptr.get(), bound_source.get()));
                    }
                }
            });
        };

        //============================================================
        // Swap
        //============================================================

        "swap exchanges bindings"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                const std::int32_t a = 1;
                const std::int32_t b = 2;

                ConcretePtr<const std::int32_t> lhs{a};
                ConcretePtr<const std::int32_t> rhs{b};

                using std::swap;
                swap(lhs, rhs);

                expect(eq(lhs.get(), std::addressof(b)));
                expect(eq(rhs.get(), std::addressof(a)));

                expect(eq(*lhs, b));
                expect(eq(*rhs, a));
            });
        };

        //============================================================
        // Equality semantics
        //============================================================

        "equality compares pointer identity"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    expect(eq(std::equality_comparable<ConcretePtr<std::int32_t>>, true));

                    std::int32_t x = 1;
                    std::int32_t y = 1;

                    ConcretePtr<const std::int32_t> a{x};
                    ConcretePtr<std::int32_t> b{x};
                    ConcretePtr<std::int32_t> c{y};

                    expect(eq(a == b, true));
                    expect(eq(a == c, false));
                    expect(eq(b == c, false));
                }
            });
        };

        "nullable comparisons with nullptr"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_pointer_binding && pointer_test_traits<ConcretePtr>::is_nullable) {
                    std::int32_t value{};

                    ConcretePtr<std::int32_t> bound{std::addressof(value)};
                    ConcretePtr<std::int32_t> null{nullptr};

                    //Expecting both operator== and operator!= to be synthesized correctly
                    expect(eq(bound == nullptr, false));
                    expect(eq(nullptr == bound, false));

                    expect(eq(bound != nullptr, true));
                    expect(eq(nullptr != bound, true));

                    expect(eq(null == nullptr, true));
                    expect(eq(nullptr == null, true));

                    expect(eq(null != nullptr, false));
                    expect(eq(nullptr != null, false));
                }
            });
        };

        "null pointers of same pointer type compare equal"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                    ConcretePtr<std::int32_t> lhs{nullptr};
                    ConcretePtr<const std::int32_t> rhs{nullptr};

                    expect(eq(lhs == rhs, true));
                    expect(eq(lhs != rhs, false));
                }
            });
        };

        //============================================================
        // Covariance
        //============================================================

        "construct base from derived pointer"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    derived_type d;
                    ConcretePtr<derived_type> dptr{d};

                    ConcretePtr<base_type> bptr{dptr};

                    expect(eq(bptr.get(), static_cast<base_type*>(std::addressof(d))));
                }
            });
        };

        "construct base from derived reference"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    derived_type d;

                    ConcretePtr<base_type> bptr{d};

                    expect(eq(bptr.get(), static_cast<base_type*>(std::addressof(d))));
                }
            });
        };

        "assign base from derived pointer"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    derived_type d;
                    ConcretePtr<derived_type> dptr{d};

                    base_type b;
                    ConcretePtr<base_type> bptr{b};

                    bptr = dptr;

                    expect(eq(bptr.get(), static_cast<base_type*>(std::addressof(d))));
                }
            });
        };

        "rebind base from derived reference"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    derived_type d;

                    base_type b;
                    ConcretePtr<base_type> bptr{b};

                    bptr = d;

                    expect(eq(bptr.get(), static_cast<base_type*>(std::addressof(d))));
                }
            });
        };

        "assign const base from derived pointer"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    derived_type d;
                    ConcretePtr<derived_type> dptr{d};

                    base_type b;
                    ConcretePtr<const base_type> bptr{b};

                    bptr = dptr;

                    expect(eq(bptr.get(), static_cast<const base_type*>(std::addressof(d))));
                }
            });
        };

        "rebind const base from derived reference"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    derived_type d;

                    base_type b;
                    ConcretePtr<const base_type> bptr{b};

                    bptr = d;

                    expect(eq(bptr.get(), static_cast<const base_type*>(std::addressof(d))));
                }
            });
        };

        "covariant equality comparison"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    expect(eq(std::equality_comparable_with<ConcretePtr<base_type>, ConcretePtr<derived_type>>, true));
                    expect(eq(std::equality_comparable_with<ConcretePtr<base_type>, derived_type*>, true));
                    expect(eq(std::equality_comparable_with<base_type*, ConcretePtr<derived_type>>, true));
                }
            });
        };

        //============================================================
        // Common Reference
        //============================================================

        "basic_common_reference preserves concrete pointer type with cv-qualifications"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>() {
                expect(eq(std::common_reference_with<ConcretePtr<std::int32_t>, ConcretePtr<const std::int32_t>>, true));
                expect(eq(
                    std::same_as<
                        std::common_reference_t<ConcretePtr<std::int32_t>, ConcretePtr<const std::int32_t>>,
                        ConcretePtr<const std::int32_t>
                    >,
                    true
                ));

                expect(eq(std::common_reference_with<ConcretePtr<std::int32_t>, ConcretePtr<volatile std::int32_t>>, true));
                expect(eq(
                    std::same_as<
                        std::common_reference_t<ConcretePtr<std::int32_t>, ConcretePtr<volatile std::int32_t>>,
                        ConcretePtr<volatile std::int32_t>
                    >,
                    true
                ));

                expect(eq(std::common_reference_with<ConcretePtr<const std::int32_t>, ConcretePtr<const volatile std::int32_t>>, true));
                expect(eq(
                    std::same_as<
                        std::common_reference_t<ConcretePtr<const std::int32_t>, ConcretePtr<const volatile std::int32_t>>,
                        ConcretePtr<const volatile int>
                    >,
                    true
                ));

                expect(eq(std::common_reference_with<ConcretePtr<volatile std::int32_t>, ConcretePtr<const volatile std::int32_t>>, true));
                expect(eq(
                    std::same_as<
                        std::common_reference_t<ConcretePtr<volatile std::int32_t>, ConcretePtr<const volatile std::int32_t>>,
                        ConcretePtr<const volatile int>
                    >,
                    true
                ));

                expect(eq(std::common_reference_with<ConcretePtr<volatile std::int32_t>, ConcretePtr<const std::int32_t>>, true));
                expect(eq(
                    std::same_as<
                        std::common_reference_t<ConcretePtr<volatile std::int32_t>, ConcretePtr<const std::int32_t>>,
                        ConcretePtr<const volatile std::int32_t>
                    >,
                    true
                ));
            });
        };

        "basic_common_reference uses reference-to-pointer value category propagation"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>() {
                expect(eq(std::same_as<std::common_reference_t<int*&, const int*&>, const int*&>, true));
                expect(eq(std::same_as<std::common_reference_t<int*&, const int*&>, int*&>, true));
                expect(eq(std::same_as<std::common_reference_t<int*&, const int*&>, const int*>, true));
                expect(eq(std::same_as<std::common_reference_t<int*&, const int*&>, int*>, true));
                expect(eq(std::same_as<std::common_reference_t<int*&&, const int*&&>, const int*&&>, true));
                expect(eq(std::same_as<std::common_reference_t<int*&&, const int*&&>, int*&&>, true));
                expect(eq(std::same_as<std::common_reference_t<int*&&, const int*&&>, const int*>, true));
                expect(eq(std::same_as<std::common_reference_t<int*&&, const int*&&>, int*>, true));
                
                expect(eq(std::common_reference_with<ConcretePtr<std::int32_t>&, ConcretePtr<const std::int32_t>&>, true));
                expect(eq(
                    std::same_as<
                        std::common_reference_t<ConcretePtr<std::int32_t>&, ConcretePtr<const std::int32_t>&>,
                        ConcretePtr<const std::int32_t>&
                    >,
                    true
                ));

                expect(eq(std::common_reference_with<ConcretePtr<std::int32_t>&&, ConcretePtr<const std::int32_t>&&>, true));
                expect(eq(
                    std::same_as<
                        std::common_reference_t<ConcretePtr<std::int32_t>&&, ConcretePtr<const std::int32_t>&&>,
                        ConcretePtr<const std::int32_t>&&
                    >,
                    true
                ));
            });
        };

        "basic_common_reference matches raw pointer common_reference"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>() {
                expect(eq(std::common_reference_with<ConcretePtr<std::int32_t>, ConcretePtr<const volatile std::int32_t>>, true));
                expect(eq(
                    std::same_as<
                        std::common_reference_t<
                            ConcretePtr<std::int32_t>,
                            ConcretePtr<const volatile std::int32_t>
                        >,
                        ConcretePtr<
                            std::remove_pointer_t<
                                std::remove_cvref_t<
                                    std::common_reference_t<
                                        std::int32_t*,
                                        const volatile std::int32_t*
                                    >
                                >
                            >
                        >
                    >,
                    true
                ));
            });
        };

        "vocabulary pointer and raw pointer share raw pointer common reference"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>() {
                expect(eq(std::common_reference_with<ConcretePtr<const std::int32_t>, std::int32_t*>, true));
                expect(eq(
                    std::same_as<
                        std::common_reference_t<ConcretePtr<const std::int32_t>, std::int32_t*>,
                        const std::int32_t*
                    >,
                    true
                ));

                expect(eq(std::common_reference_with<std::int32_t*, ConcretePtr<const std::int32_t>>, true));
                expect(eq(
                    std::same_as<
                        std::common_reference_t<std::int32_t*, ConcretePtr<const std::int32_t>>,
                        const std::int32_t*
                    >,
                    true
                ));
            });
        };

        "common_reference supports covariance"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>() {
                expect(eq(std::common_reference_with<ConcretePtr<derived_type>, ConcretePtr<base_type>>, true));
                expect(eq(
                    std::same_as<
                        std::common_reference_t<ConcretePtr<derived_type>, ConcretePtr<base_type>>,
                        ConcretePtr<base_type>
                    >,
                    true
                ));

                expect(eq(std::common_reference_with<ConcretePtr<const derived_type>, ConcretePtr<base_type>>, true));
                expect(eq(
                    std::same_as<
                        std::common_reference_t<ConcretePtr<const derived_type>, ConcretePtr<base_type>>,
                        ConcretePtr<const base_type>
                    >,
                    true
                ));

                expect(eq(std::common_reference_with<ConcretePtr<derived_type>,ConcretePtr<const base_type>>, true));
                expect(eq(
                    std::same_as<
                        std::common_reference_t<ConcretePtr<derived_type>,ConcretePtr<const base_type>>,
                        ConcretePtr<const base_type>
                    >,
                    true
                ));

                expect(eq(std::common_reference_with<ConcretePtr<const derived_type>, ConcretePtr<volatile base_type>>, true));
                expect(eq(
                    std::same_as<
                        std::common_reference_t<ConcretePtr<const derived_type>, ConcretePtr<volatile base_type>>,
                        ConcretePtr<const volatile base_type>
                    >,
                    true
                ));

                expect(eq(std::common_reference_with<ConcretePtr<derived_type>, base_type*>, true));
                expect(eq(
                    std::same_as<
                        std::common_reference_t<ConcretePtr<derived_type>, base_type*>,
                        base_type*
                    >,
                    true
                ));

                expect(eq(std::common_reference_with<derived_type*, ConcretePtr<base_type>>, true));
                expect(eq(
                    std::same_as<
                        std::common_reference_t<derived_type*, ConcretePtr<base_type>>,
                        base_type*
                    >,
                    true
                ));

                expect(eq(std::common_reference_with<ConcretePtr<const derived_type>, volatile base_type*>, true));
                expect(eq(
                    std::same_as<
                        std::common_reference_t<ConcretePtr<const derived_type>, volatile base_type*>,
                        const volatile base_type*
                    >,
                    true
                ));
            });
        };

        //============================================================
        // Arithmetic operations
        //============================================================

        "pointer arithmetic operations according to policy"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                using T = ConcretePtr<std::int32_t>;

                expect(eq(HasAddition<T>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
                expect(eq(HasSubtraction<T>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
                expect(eq(HasDifference<T>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
                expect(eq(HasPreIncrement<T>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
                expect(eq(HasPostIncrement<T>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
                expect(eq(HasPreDecrement<T>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
                expect(eq(HasPostDecrement<T>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
            });
        };

        "ordering comparisons according to policy"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                expect(eq(std::three_way_comparable<ConcretePtr<std::int32_t>>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
                //expect(eq(std::three_way_comparable_with<ConcretePtr<std::int32_t>, std::int32_t*>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal)); //TODO: this needs common_type or basic_common_reference work
                expect(eq(std::three_way_comparable<ConcretePtr<base_type>>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
                expect(eq(std::three_way_comparable_with<ConcretePtr<base_type>, ConcretePtr<derived_type>>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
                expect(eq(std::three_way_comparable_with<ConcretePtr<base_type>, derived_type*>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
                expect(eq(std::three_way_comparable_with<base_type*, ConcretePtr<derived_type>>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
            });
        };

        "input_or _output_iterator according to policy"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                //note: all other iterator concepts subsume this one and thus are implicitly false when it is false
                expect(eq(std::input_or_output_iterator<ConcretePtr<std::int32_t>>, pointer_test_traits<ConcretePtr>::has_arithmetic_traversal));
            });
        };

        //============================================================
        // Traversal identity
        //============================================================

        "pointer arithmetic preserves native traversal semantics"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::has_arithmetic_traversal && pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    int values[] = {10, 20, 30, 40};

                    ConcretePtr<int> ptr{values[0]};

                    auto advanced = ptr + 2;

                    expect(eq(*advanced, values[2]));
                    expect(eq(advanced.get(), values + 2));
                }
            });
        };

        "difference matches raw pointer semantics"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::has_arithmetic_traversal && pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    int values[] = {10, 20, 30, 40};

                    ConcretePtr<int> first{values[0]};
                    ConcretePtr<int> last{values[3]};

                    expect(eq(last - first, 3L));
                }
            });
        };

        "increment and decrement traverse correctly"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::has_arithmetic_traversal && pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    int values[] = {1, 2, 3};

                    ConcretePtr<int> ptr{values[0]};

                    ++ptr;
                    expect(eq(*ptr, 2));

                    ptr++;
                    expect(eq(*ptr, 3));

                    --ptr;
                    expect(eq(*ptr, 2));

                    ptr--;
                    expect(eq(*ptr, 1));
                }
            });
        };

        "subscript matches raw pointer indexing"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::has_arithmetic_traversal && pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    int values[] = {5, 6, 7, 8};

                    ConcretePtr<int> ptr{values[0]};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
                    expect(eq(ptr[0], values[0]));
                    expect(eq(ptr[1], values[1]));
                    expect(eq(ptr[2], values[2]));
#pragma GCC diagnostic pop
                }
            });
        };

        "mixed raw and cursor arithmetic produce identical addresses"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::has_arithmetic_traversal && pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    int values[] = {1, 2, 3, 4};

                    ConcretePtr<int> ptr{values[0]};

                    expect(eq((ptr + 3).get(), values + 3));
                    expect(eq((3 + ptr).get(), values + 3));
                }
            });
        };

        //============================================================
        // CV-correctness propagation
        //============================================================

        "const element forbids mutation through dereference"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    const std::int32_t value{};
                    ConcretePtr<const std::int32_t> ptr{value};

                    // Compile-time: *ptr must NOT be assignable
                    constexpr bool can_assign = std::is_assignable_v<decltype(*ptr), std::int32_t>;

                    expect(eq(can_assign, false));
                }
            });
        };

        "non-const element allows mutation"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    std::int32_t value = 5;
                    ConcretePtr<std::int32_t> ptr{value};

                    *ptr = 10;

                    expect(eq(value, 10));
                }
            });
        };

        "`address_type` nested type preserves top-level const"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                using T = ConcretePtr<const std::int32_t>;

                expect(eq(std::same_as<typename T::address_type, const std::int32_t*>, true));
            });
        };

        "`reference` nested type preserves const"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                using T = ConcretePtr<const std::int32_t>;

                expect(eq(std::same_as<typename T::reference, const std::int32_t&>, true));
            });
        };

        "const pointer prevents rebinding but not mutation"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    std::int32_t value{};

                    const ConcretePtr<std::int32_t> ptr{value};

                    *ptr = 10; // allowed

                    constexpr bool can_rebind = std::is_assignable_v<const ConcretePtr<std::int32_t>&, std::int32_t&>;

                    expect(eq(value, 10));
                    expect(eq(can_rebind, false));
                }
            });
        };

        "qualification climbing construction and assignment"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    std::int32_t value{42};
                    std::int32_t other{};
                    ConcretePtr<std::int32_t> mutable_ptr{value};
                    ConcretePtr<const std::int32_t> const_ptr{other};

                    //Qualification climbing (Assignment)
                    const_ptr = mutable_ptr;
                    expect(eq(const_ptr.get() == mutable_ptr.get(), true));

                    //Qualification climbing (Construction)
                    ConcretePtr<const std::int32_t> const_copy{mutable_ptr};
                    expect(eq(const_copy.get() == mutable_ptr.get(), true));
                }
            });
        };

        "volatile qualifier preservation"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    volatile std::int32_t hardware_register = 0xAA;
                    ConcretePtr<volatile std::int32_t> ptr{hardware_register};

                    //Ensure the raw pointer retrieved is also volatile
                    expect(eq(std::same_as<decltype(ptr.get()), volatile std::int32_t*>, true));

                    //Ensure conversion to raw pointer preserves volatile
                    volatile int* raw = ptr;
                    expect(eq(raw, std::addressof(hardware_register)));

                    //Ensure dereference preserves volatile
                    decltype(auto) dereferenced = *ptr;
                    expect(eq(std::is_volatile_v<std::remove_reference_t<decltype(dereferenced)>>, true));
                    expect(eq(dereferenced, hardware_register));
                }
            });
        };

        "common_type preserves const qualification"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                using common_t = std::common_type_t<ConcretePtr<std::int32_t>, ConcretePtr<const std::int32_t>>;

                expect(eq(std::same_as<common_t, ConcretePtr<const std::int32_t>>, true));
            });
        };

        "common_reference preserves const qualification"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                using common_ref = std::common_reference_t<ConcretePtr<std::int32_t>, ConcretePtr<const std::int32_t>>;

                expect(eq(std::same_as<common_ref, ConcretePtr<const std::int32_t>>, true));
            });
        };

        //============================================================
        // Interoperability with raw pointer APIs
        //============================================================

        "implicit conversion works with raw pointer API"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    auto takes_ptr = [](std::int32_t* p) { return *p; };

                    std::int32_t value = 3;
                    ConcretePtr<std::int32_t> ptr{value};

                    expect(eq(takes_ptr(ptr), value));
                }
            });
        };

        "get() works with raw pointer API"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    auto takes_ptr = [](std::int32_t* p) { return *p; };

                    std::int32_t value = 4;
                    ConcretePtr<std::int32_t> ptr{value};

                    expect(eq(takes_ptr(ptr.get()), value));
                }
            });
        };

        "not constructible, convertible, nor assignable from C-array decay"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    std::int32_t array[3] = {0, 1, 2};

                    //ConcretePtr<std::int32_t> should_fail{array};

                    expect(eq(std::convertible_to<decltype(array), ConcretePtr<std::int32_t>>, false));
                    expect(eq(std::constructible_from<ConcretePtr<std::int32_t>, decltype(array)>, false));
                    expect(eq(std::is_assignable_v<ConcretePtr<std::int32_t>&, decltype(array)>, false));

                    expect(eq(std::constructible_from<ConcretePtr<std::int32_t>, decltype(array[0])>, true));
                    expect(eq(std::is_assignable_v<ConcretePtr<std::int32_t>&, decltype(array[0])>, true));

                    ConcretePtr<std::int32_t> ptr{array[1]};

                    //Ensure binding to the element is equivalent to expected array-to-pointer decay with pointer offset arithmetic
                    expect(eq(ptr.get(), array + 1));
                }
            });
        };

        "not constructible, convertible, nor assignable from C-array decay when pointing to an array"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    std::int32_t array[3][3] = {
                        {0, 1, 2},
                        {3, 4, 5},
                        {6, 7, 8}
                    };

                    //ConcretePtr<std::int32_t[3]> should_fail{array};

                    expect(eq(std::convertible_to<decltype(array), ConcretePtr<std::int32_t[3]>>, false));
                    expect(eq(std::constructible_from<ConcretePtr<std::int32_t[3]>, decltype(array)>, false));
                    expect(eq(std::is_assignable_v<ConcretePtr<std::int32_t[3]>&, decltype(array)>, false));

                    expect(eq(std::constructible_from<ConcretePtr<std::int32_t[3]>, decltype(array[0])>, true));
                    expect(eq(std::is_assignable_v<ConcretePtr<std::int32_t[3]>&, decltype(array[0])>, true));

                    ConcretePtr<std::int32_t[3]> ptr{array[1]};

                    //Ensure binding to the element is equivalent to expected array-to-pointer decay with pointer offset arithmetic
                    expect(eq(ptr.get(), array + 1));
                }
            });
        };

        //============================================================
        // Incomplete types
        //============================================================

        struct incomplete_type;

        "incomplete type support"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_pointer_binding) {
                    expect(eq(base::meta::concepts::InstantiableWith<ConcretePtr, incomplete_type>, true));

                    incomplete_type* raw = reinterpret_cast<incomplete_type*>(0x1234);

                    ConcretePtr<incomplete_type> ptr{raw};

                    expect(eq(ptr.get(), raw));
                }
            });
        };

        struct incomplete_type {
            int value;
        };

        "incomplete type becomes usable after completion"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    incomplete_type obj{42};

                    ConcretePtr<incomplete_type> ptr{obj};

                    expect(eq(ptr->value, obj.value));
                    expect(eq((*ptr).value, obj.value));
                }
            });
        };

        //============================================================
        // `void` support
        //============================================================

        "type aliases are correct for `void`"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
                    using T = ConcretePtr<void>;

                    constexpr bool element = std::same_as<typename T::element_type, void>;
                    constexpr bool value   = std::same_as<typename T::value_type, void>;
                    constexpr bool pointer = std::same_as<typename T::address_type, void*>;
                    constexpr bool ptrdiff = std::same_as<typename T::difference_type, std::ptrdiff_t>;

                    expect(eq(element, true));
                    expect(eq(value, true));
                    expect(eq(pointer, true));
                    expect(eq(ptrdiff, true));
                }
            });
        };

        "void specialization supports type erasure"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee && pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    std::int32_t x{};

                    ConcretePtr<std::int32_t> typed{x};
                    ConcretePtr<void> erased{typed};

                    expect(eq(erased.get(), static_cast<void*>(std::addressof(x))));
                }
            });
        };

        "void specialization disables dereference operators"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
                    expect(eq(Dereferenceable<ConcretePtr<std::int32_t>>, true));
                    expect(eq(ArrowAccessible<ConcretePtr<std::int32_t>>, true));

                    expect(eq(Dereferenceable<ConcretePtr<void>>, false));
                    expect(eq(ArrowAccessible<ConcretePtr<void>>, false));
                }
            });
        };

        "construction from void raw pointer is explicit"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
                    expect(eq(std::convertible_to<void*, ConcretePtr<void>>, false));
                    expect(eq(std::constructible_from<ConcretePtr<void>, void*>, true));
                }
            });
        };

        "construction from void smart pointer is explicit"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee) {
                    expect(eq(std::convertible_to<trivial_smart_ptr<void>&, ConcretePtr<void>>, false));
                    expect(eq(std::constructible_from<ConcretePtr<void>, trivial_smart_ptr<void>&>, true));
                }
            });
        };

        "void pointer constructs implicitly from typed pointer"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee && pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    expect(eq(std::convertible_to<ConcretePtr<std::int32_t>, ConcretePtr<void>>, true));
                    expect(eq(std::convertible_to<ConcretePtr<void>, ConcretePtr<std::int32_t>>, false));

                    const std::int32_t value{42};
                    ConcretePtr<const std::int32_t> typed_ptr{value};

                    // Should be implicit (convertible)
                    auto takes_void = [](ConcretePtr<const void> ptr) { return ptr.get(); };
                    expect(eq(takes_void(typed_ptr), static_cast<const void*>(std::addressof(value))));
                }
            });
        };

        "void pointer is equality comparable"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::permits_void_pointee && pointer_test_traits<ConcretePtr>::allows_pointer_binding && pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    const std::int32_t value{42};
                    ConcretePtr<const std::int32_t> typed_ptr{value};
                    const std::int32_t* typed_raw = std::addressof(value);
                    const void* erased_raw        = std::addressof(value);

                    ConcretePtr<const void> erased_ptr1{typed_raw};
                    ConcretePtr<const void> erased_ptr2{typed_ptr};

                    expect(eq(erased_ptr1 == erased_ptr2, true));
                    expect(eq(erased_ptr1 == typed_ptr, true));
                    expect(eq(erased_ptr1 == erased_raw, true));
                    expect(eq(erased_ptr1 == typed_raw, true));
                }
            });
        };

        //============================================================
        // CTAD Guide
        //============================================================

        "deduction guides work"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                constexpr std::int32_t value{};

                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    ConcretePtr ptr1{value};
                    expect(eq(ptr1.get(), std::addressof(value)));
                }

                if constexpr (pointer_test_traits<ConcretePtr>::allows_pointer_binding) {
                    ConcretePtr ptr2{std::addressof(value)};
                    expect(eq(ptr2.get(), std::addressof(value)));
                }
            });
        };

        //============================================================
        // Constant Expression Usage
        //============================================================

        "constexpr construction and dereference"_test = [] {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    static constexpr int value = 42;

                    constexpr ConcretePtr<const int> ptr{value};

                    expect(eq(*ptr, value));
                }
            });
        };

        "constexpr arithmetic"_test = [] {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding && pointer_test_traits<ConcretePtr>::has_arithmetic_traversal) {
                    static constexpr int values[] = {2, 4, 6};

                    constexpr ConcretePtr<const int> ptr{values[0]};

                    constexpr auto next = ptr + 1;
                    expect(eq(*next, values[1]));
                }
            });
        };

        "constexpr get and boolean conversion"_test = [] {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    static constexpr int value = 7;

                    constexpr ConcretePtr<const int> ptr{value};

                    expect(eq(ptr.get(), std::addressof(value)));
                    expect(eq(static_cast<bool>(ptr), true));
                }
            });
        };

        "constexpr equality"_test = [] {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    static constexpr int value = 11;

                    constexpr ConcretePtr<const int> a{value};
                    constexpr ConcretePtr<const int> b{value};

                    expect(eq(a == b, true));
                }
            });
        };

        "constexpr rebinding"_test = [] {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    static constexpr int a = 1;
                    static constexpr int b = 2;

                    constexpr auto rebound = std::invoke([] {
                        ConcretePtr<const int> ptr{a};
                        ptr = b;
                        return ptr;
                    });

                    expect(eq(*rebound, b));
                    expect(eq(rebound.get(), std::addressof(b)));
                }
            });
        };

        "constexpr swap"_test = [] {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    static constexpr int a = 1;
                    static constexpr int b = 2;

                    constexpr auto swapped = std::invoke([] {
                        ConcretePtr<const int> lhs{a};
                        ConcretePtr<const int> rhs{b};
        
                        using std::swap;
                        swap(lhs, rhs);
        
                        return std::pair{lhs, rhs};
                    });

                    expect(eq(*swapped.first, b));
                    expect(eq(*swapped.second, a));
                }
            });
        };

        //============================================================
        // Hash Support
        //============================================================

        "hash matches raw pointer hash"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    std::int32_t value{};

                    ConcretePtr<std::int32_t> ptr{value};

                    const auto ptr_hash = std::hash<ConcretePtr<std::int32_t>>{}(ptr);
                    const auto raw_hash = std::hash<std::int32_t*>{}(std::addressof(value));

                    expect(eq(ptr_hash, raw_hash));
                }
            });
        };

        "equal pointers produce equal hashes"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    std::int32_t value{};

                    ConcretePtr<std::int32_t> lhs{value};
                    ConcretePtr<const std::int32_t> rhs{value};

                    const auto lhs_hash = std::hash<ConcretePtr<std::int32_t>>{}(lhs);
                    const auto rhs_hash = std::hash<ConcretePtr<const std::int32_t>>{}(rhs);

                    expect(eq(lhs == rhs, true));
                    expect(eq(lhs_hash == rhs_hash, true));
                }
            });
        };

        //============================================================
        // Formatting and output stream support
        //============================================================

        "std::formatter formats as raw pointer"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    std::int32_t value{};

                    ConcretePtr<std::int32_t> ptr{value};

                    const auto formatted_ptr = std::format("{}", ptr);
                    const auto formatted_raw = std::format<void*>("{}", std::addressof(value));

                    expect(eq(formatted_ptr, formatted_raw));
                }
            });
        };

        "std::formatter formats null equivalently to raw pointer"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                    ConcretePtr<std::int32_t> ptr{nullptr};

                    const auto formatted_ptr = std::format("{}", ptr);
                    const auto formatted_raw = std::format<void*>("{}", nullptr);

                    expect(eq(formatted_ptr, formatted_raw));
                }
            });
        };

        "std::formatter supports cv-qualified element types"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    const std::int32_t value{};

                    ConcretePtr<const std::int32_t> ptr{value};

                    const auto formatted_ptr = std::format("{}", ptr);
                    const auto formatted_raw = std::format<const void*>("{}", std::addressof(value));

                    expect(eq(formatted_ptr, formatted_raw));
                }
            });
        };

        "ostream insertion outputs raw pointer representation"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    std::int32_t value{};

                    ConcretePtr<std::int32_t> ptr{value};

                    std::ostringstream ptr_stream;
                    std::ostringstream raw_stream;

                    ptr_stream << ptr;
                    raw_stream << std::addressof(value);

                    expect(eq(ptr_stream.str(), raw_stream.str()));
                }
            });
        };

        "ostream insertion outputs null equivalently to raw pointer"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::is_nullable) {
                    ConcretePtr<std::int32_t> ptr{nullptr};

                    std::ostringstream ptr_stream;
                    std::ostringstream raw_stream;

                    ptr_stream << ptr;
                    raw_stream << static_cast<std::int32_t*>(nullptr);

                    expect(eq(ptr_stream.str(), raw_stream.str()));
                }
            });
        };

        "ostream insertion supports cv-qualified element types"_test = [] mutable {
            test_each_pointer_type_with([]<template<typename> typename ConcretePtr>(){
                if constexpr (pointer_test_traits<ConcretePtr>::allows_reference_binding) {
                    volatile std::int32_t value{};

                    ConcretePtr<volatile std::int32_t> ptr{value};

                    std::ostringstream ptr_stream;
                    std::ostringstream raw_stream;

                    ptr_stream << ptr;
                    raw_stream << const_cast<std::add_pointer_t<std::remove_volatile_t<std::remove_pointer_t<decltype(std::addressof(value))>>>>(std::addressof(value));

                    expect(eq(ptr_stream.str(), raw_stream.str()));
                }
            });
        };
    };
} //namespace

int main() {}
