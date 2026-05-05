// SPDX-License-Identifier: Apache-2.0
// Unit tests for base.vocab.ptr:alias_ptr

import base.vocab.ptr;
import ut;
import std;

import base.meta.concepts;

using namespace ut;
using base::vocab::alias_ptr;

namespace {
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

    suite base_vocab_ptr_alias_ptr_unit = [] mutable {
        //============================================================
        // Template Constraint Validation
        //============================================================

        "template instantiation checks"_test = [] mutable {
            using base::meta::concepts::instantiable_with;
            expect(eq(instantiable_with<alias_ptr, std::int32_t>, true));
            expect(eq(instantiable_with<alias_ptr, std::int32_t*>, true));
            expect(eq(instantiable_with<alias_ptr, std::map<std::string, std::vector<std::int32_t>>>, true));

            expect(eq(instantiable_with<alias_ptr, void>, true));

            expect(eq(instantiable_with<alias_ptr, std::int32_t&>, false));
            expect(eq(instantiable_with<alias_ptr, std::int32_t&&>, false));
            expect(eq(instantiable_with<alias_ptr, void(int)>, false));
            expect(eq(instantiable_with<alias_ptr, void (&)(int)>, false));
            expect(eq(instantiable_with<alias_ptr, void (*)(int, float)>, false));
            expect(eq(instantiable_with<alias_ptr, void (**)(std::string, int)>, false));
            expect(eq(instantiable_with<alias_ptr, void (*******)(int)>, false));
        };

        //============================================================
        // Triviality & ABI properties
        //============================================================

        "triviality"_test = [] mutable {
            using simple_t = std::int32_t;

            expect(eq(std::is_standard_layout_v<alias_ptr<simple_t>>, true));
            expect(eq(std::is_trivially_copyable_v<alias_ptr<simple_t>>, true));
            expect(eq(std::is_trivially_destructible_v<alias_ptr<simple_t>>, true));
            expect(eq(std::is_trivially_copy_constructible_v<alias_ptr<simple_t>>, true));
            expect(eq(std::is_trivially_move_constructible_v<alias_ptr<simple_t>>, true));
            expect(eq(std::is_trivially_copy_assignable_v<alias_ptr<simple_t>>, true));
            expect(eq(std::is_trivially_move_assignable_v<alias_ptr<simple_t>>, true));
            expect(eq(std::is_nothrow_constructible_v<alias_ptr<simple_t>, simple_t&>, true));
            expect(eq(std::is_nothrow_swappable_v<alias_ptr<simple_t>>, true));

            using complex_t = std::map<std::string, std::vector<std::int32_t>>;

            expect(eq(std::is_standard_layout_v<alias_ptr<complex_t>>, true));
            expect(eq(std::is_trivially_copyable_v<alias_ptr<complex_t>>, true));
            expect(eq(std::is_trivially_destructible_v<alias_ptr<complex_t>>, true));
            expect(eq(std::is_trivially_copy_constructible_v<alias_ptr<complex_t>>, true));
            expect(eq(std::is_trivially_move_constructible_v<alias_ptr<complex_t>>, true));
            expect(eq(std::is_trivially_copy_assignable_v<alias_ptr<complex_t>>, true));
            expect(eq(std::is_trivially_move_assignable_v<alias_ptr<complex_t>>, true));
            expect(eq(std::is_nothrow_constructible_v<alias_ptr<complex_t>, complex_t&>, true));
            expect(eq(std::is_nothrow_swappable_v<alias_ptr<complex_t>>, true));
        };

        "size and alignment match raw pointers"_test = [] mutable {
            using simple_t = std::int32_t;

            expect(eq(sizeof(alias_ptr<simple_t>) == sizeof(simple_t*), true));
            expect(eq(alignof(alias_ptr<simple_t>) == alignof(simple_t*), true));

            using complex_t = std::map<std::string, std::vector<std::int32_t>>;

            expect(eq(sizeof(alias_ptr<complex_t>) == sizeof(complex_t*), true));
            expect(eq(alignof(alias_ptr<complex_t>) == alignof(complex_t*), true));
        };

        //============================================================
        // Type properties
        //============================================================

        "type aliases are correct"_test = [] mutable {
            using T = alias_ptr<const std::int32_t>;

            constexpr bool element = std::same_as<T::element_type, const std::int32_t>;
            constexpr bool value   = std::same_as<T::value_type, std::int32_t>;
            constexpr bool pointer = std::same_as<T::pointer, const std::int32_t*>;
            constexpr bool ref     = std::same_as<T::reference, const std::int32_t&>;
            constexpr bool rref    = std::same_as<T::rvalue_reference, const std::int32_t&&>;
            constexpr bool ptrdiff = std::same_as<T::difference_type, std::ptrdiff_t>;

            expect(eq(element, true));
            expect(eq(value, true));
            expect(eq(pointer, true));
            expect(eq(ref, true));
            expect(eq(rref, true));
            expect(eq(ptrdiff, true));
        };

        //============================================================
        // Construction
        //============================================================

        "constructible explicitly from lvalue reference"_test = [] mutable {
            using test_type = std::int32_t;

            //Explicitly constructible unless removing qualifier
            expect(eq(std::constructible_from<alias_ptr<test_type>, test_type&>, true));
            expect(eq(std::constructible_from<alias_ptr<test_type>, const test_type&>, false));
            expect(eq(std::constructible_from<alias_ptr<test_type>, volatile test_type&>, false));
            expect(eq(std::constructible_from<alias_ptr<test_type>, const volatile test_type&>, false));
            expect(eq(std::constructible_from<alias_ptr<const test_type>, test_type&>, true));
            expect(eq(std::constructible_from<alias_ptr<const test_type>, const test_type&>, true));
            expect(eq(std::constructible_from<alias_ptr<const test_type>, volatile test_type&>, false));
            expect(eq(std::constructible_from<alias_ptr<const test_type>, const volatile test_type&>, false));
            expect(eq(std::constructible_from<alias_ptr<volatile test_type>, test_type&>, true));
            expect(eq(std::constructible_from<alias_ptr<volatile test_type>, const test_type&>, false));
            expect(eq(std::constructible_from<alias_ptr<volatile test_type>, volatile test_type&>, true));
            expect(eq(std::constructible_from<alias_ptr<volatile test_type>, const volatile test_type&>, false));
            expect(eq(std::constructible_from<alias_ptr<const volatile test_type>, test_type&>, true));
            expect(eq(std::constructible_from<alias_ptr<const volatile test_type>, const test_type&>, true));
            expect(eq(std::constructible_from<alias_ptr<const volatile test_type>, volatile test_type&>, true));
            expect(eq(std::constructible_from<alias_ptr<const volatile test_type>, const volatile test_type&>, true));

            //Never implicitly convertible (constructor is explicit)
            expect(eq(std::convertible_to<test_type&, alias_ptr<test_type>>, false));
            expect(eq(std::convertible_to<const test_type&, alias_ptr<test_type>>, false));
            expect(eq(std::convertible_to<test_type&, alias_ptr<const test_type>>, false));
            expect(eq(std::convertible_to<const test_type&, alias_ptr<const test_type>>, false));
        };

        "default constructible"_test = [] mutable {
            expect(eq(std::default_initializable<alias_ptr<std::int32_t>>, true));
        };

        "constructible from nullptr"_test = [] mutable {
            expect(eq(std::constructible_from<alias_ptr<std::int32_t>, std::nullptr_t>, true));
        };

        "constructible from raw pointer"_test = [] mutable {
            using test_type = std::int32_t;

            //Explicitly constructible unless removing qualifier
            expect(eq(std::constructible_from<alias_ptr<test_type>, test_type*>, true));
            expect(eq(std::constructible_from<alias_ptr<test_type>, const test_type*>, false));
            expect(eq(std::constructible_from<alias_ptr<test_type>, volatile test_type*>, false));
            expect(eq(std::constructible_from<alias_ptr<test_type>, const volatile test_type*>, false));
            expect(eq(std::constructible_from<alias_ptr<const test_type>, test_type*>, true));
            expect(eq(std::constructible_from<alias_ptr<const test_type>, const test_type*>, true));
            expect(eq(std::constructible_from<alias_ptr<const test_type>, volatile test_type*>, false));
            expect(eq(std::constructible_from<alias_ptr<const test_type>, const volatile test_type*>, false));
            expect(eq(std::constructible_from<alias_ptr<volatile test_type>, test_type*>, true));
            expect(eq(std::constructible_from<alias_ptr<volatile test_type>, const test_type*>, false));
            expect(eq(std::constructible_from<alias_ptr<volatile test_type>, volatile test_type*>, true));
            expect(eq(std::constructible_from<alias_ptr<volatile test_type>, const volatile test_type*>, false));
            expect(eq(std::constructible_from<alias_ptr<const volatile test_type>, test_type*>, true));
            expect(eq(std::constructible_from<alias_ptr<const volatile test_type>, const test_type*>, true));
            expect(eq(std::constructible_from<alias_ptr<const volatile test_type>, volatile test_type*>, true));
            expect(eq(std::constructible_from<alias_ptr<const volatile test_type>, const volatile test_type*>, true));

            //Implicitly convertible unless removing qualifier
            expect(eq(std::convertible_to<test_type*, alias_ptr<test_type>>, true));
            expect(eq(std::convertible_to<const test_type*, alias_ptr<test_type>>, false));
            expect(eq(std::convertible_to<volatile test_type*, alias_ptr<test_type>>, false));
            expect(eq(std::convertible_to<const volatile test_type*, alias_ptr<test_type>>, false));
            expect(eq(std::convertible_to<test_type*, alias_ptr<const test_type>>, true));
            expect(eq(std::convertible_to<const test_type*, alias_ptr<const test_type>>, true));
            expect(eq(std::convertible_to<volatile test_type*, alias_ptr<const test_type>>, false));
            expect(eq(std::convertible_to<const volatile test_type*, alias_ptr<const test_type>>, false));
            expect(eq(std::convertible_to<test_type*, alias_ptr<volatile test_type>>, true));
            expect(eq(std::convertible_to<const test_type*, alias_ptr<volatile test_type>>, false));
            expect(eq(std::convertible_to<volatile test_type*, alias_ptr<volatile test_type>>, true));
            expect(eq(std::convertible_to<const volatile test_type*, alias_ptr<volatile test_type>>, false));
            expect(eq(std::convertible_to<test_type*, alias_ptr<const volatile test_type>>, true));
            expect(eq(std::convertible_to<const test_type*, alias_ptr<const volatile test_type>>, true));
            expect(eq(std::convertible_to<volatile test_type*, alias_ptr<const volatile test_type>>, true));
            expect(eq(std::convertible_to<const volatile test_type*, alias_ptr<const volatile test_type>>, true));
        };

        "constructible from smart pointer"_test = [] mutable {
            using test_type = std::int32_t;

            //Explicitly constructible unless removing qualifier
            expect(eq(std::constructible_from<alias_ptr<test_type>, trivial_smart_ptr<test_type>&>, true));
            expect(eq(std::constructible_from<alias_ptr<test_type>, trivial_smart_ptr<const test_type>&>, false));
            expect(eq(std::constructible_from<alias_ptr<test_type>, trivial_smart_ptr<volatile test_type>&>, false));
            expect(eq(std::constructible_from<alias_ptr<test_type>, trivial_smart_ptr<const volatile test_type>&>, false));
            expect(eq(std::constructible_from<alias_ptr<const test_type>, trivial_smart_ptr<test_type>&>, true));
            expect(eq(std::constructible_from<alias_ptr<const test_type>, trivial_smart_ptr<const test_type>&>, true));
            expect(eq(std::constructible_from<alias_ptr<const test_type>, trivial_smart_ptr<volatile test_type>&>, false));
            expect(
                eq(std::constructible_from<alias_ptr<const test_type>, trivial_smart_ptr<const volatile test_type>&>, false)
            );
            expect(eq(std::constructible_from<alias_ptr<volatile test_type>, trivial_smart_ptr<test_type>&>, true));
            expect(eq(std::constructible_from<alias_ptr<volatile test_type>, trivial_smart_ptr<const test_type>&>, false));
            expect(eq(std::constructible_from<alias_ptr<volatile test_type>, trivial_smart_ptr<volatile test_type>&>, true));
            expect(
                eq(std::constructible_from<alias_ptr<volatile test_type>, trivial_smart_ptr<const volatile test_type>&>,
                   false)
            );
            expect(eq(std::constructible_from<alias_ptr<const volatile test_type>, trivial_smart_ptr<test_type>&>, true));
            expect(
                eq(std::constructible_from<alias_ptr<const volatile test_type>, trivial_smart_ptr<const test_type>&>, true)
            );
            expect(
                eq(std::constructible_from<alias_ptr<const volatile test_type>, trivial_smart_ptr<volatile test_type>&>,
                   true)
            );
            expect(eq(
                std::constructible_from<alias_ptr<const volatile test_type>, trivial_smart_ptr<const volatile test_type>&>,
                true
            ));

            //Implicitly convertible unless removing qualifier
            expect(eq(std::convertible_to<trivial_smart_ptr<test_type>&, alias_ptr<test_type>>, true));
            expect(eq(std::convertible_to<trivial_smart_ptr<const test_type>&, alias_ptr<test_type>>, false));
            expect(eq(std::convertible_to<trivial_smart_ptr<volatile test_type>&, alias_ptr<test_type>>, false));
            expect(eq(std::convertible_to<trivial_smart_ptr<const volatile test_type>&, alias_ptr<test_type>>, false));
            expect(eq(std::convertible_to<trivial_smart_ptr<test_type>&, alias_ptr<const test_type>>, true));
            expect(eq(std::convertible_to<trivial_smart_ptr<const test_type>&, alias_ptr<const test_type>>, true));
            expect(eq(std::convertible_to<trivial_smart_ptr<volatile test_type>&, alias_ptr<const test_type>>, false));
            expect(eq(std::convertible_to<trivial_smart_ptr<const volatile test_type>&, alias_ptr<const test_type>>, false));
            expect(eq(std::convertible_to<trivial_smart_ptr<test_type>&, alias_ptr<volatile test_type>>, true));
            expect(eq(std::convertible_to<trivial_smart_ptr<const test_type>&, alias_ptr<volatile test_type>>, false));
            expect(eq(std::convertible_to<trivial_smart_ptr<volatile test_type>&, alias_ptr<volatile test_type>>, true));
            expect(
                eq(std::convertible_to<trivial_smart_ptr<const volatile test_type>&, alias_ptr<volatile test_type>>, false)
            );
            expect(eq(std::convertible_to<trivial_smart_ptr<test_type>&, alias_ptr<const volatile test_type>>, true));
            expect(eq(std::convertible_to<trivial_smart_ptr<const test_type>&, alias_ptr<const volatile test_type>>, true));
            expect(
                eq(std::convertible_to<trivial_smart_ptr<volatile test_type>&, alias_ptr<const volatile test_type>>, true)
            );
            expect(
                eq(std::convertible_to<trivial_smart_ptr<const volatile test_type>&, alias_ptr<const volatile test_type>>,
                   true)
            );
        };

        "not constructible from rvalue"_test = [] mutable {
            expect(eq(std::constructible_from<alias_ptr<std::int32_t>, std::int32_t>, false));
            expect(eq(std::constructible_from<alias_ptr<const std::int32_t>, std::int32_t>, false));
            expect(eq(std::constructible_from<alias_ptr<const std::int32_t>, const std::int32_t>, false));

            expect(eq(std::constructible_from<alias_ptr<std::int32_t>, std::int32_t&&>, false));
            expect(eq(std::constructible_from<alias_ptr<const std::int32_t>, std::int32_t&&>, false));
            expect(eq(std::constructible_from<alias_ptr<const std::int32_t>, const std::int32_t&&>, false));
        };

        //============================================================
        // Assignment
        //============================================================

        "assignable from lvalue reference"_test = [] mutable {
            using test_type = std::int32_t;

            expect(eq(std::is_assignable_v<alias_ptr<test_type>&, test_type&>, true));
            expect(eq(std::is_assignable_v<alias_ptr<test_type>&, const test_type&>, false));
            expect(eq(std::is_assignable_v<alias_ptr<test_type>&, volatile test_type&>, false));
            expect(eq(std::is_assignable_v<alias_ptr<test_type>&, const volatile test_type&>, false));
            expect(eq(std::is_assignable_v<alias_ptr<const test_type>&, test_type&>, true));
            expect(eq(std::is_assignable_v<alias_ptr<const test_type>&, const test_type&>, true));
            expect(eq(std::is_assignable_v<alias_ptr<const test_type>&, volatile test_type&>, false));
            expect(eq(std::is_assignable_v<alias_ptr<const test_type>&, const volatile test_type&>, false));
            expect(eq(std::is_assignable_v<alias_ptr<volatile test_type>&, test_type&>, true));
            expect(eq(std::is_assignable_v<alias_ptr<volatile test_type>&, const test_type&>, false));
            expect(eq(std::is_assignable_v<alias_ptr<volatile test_type>&, volatile test_type&>, true));
            expect(eq(std::is_assignable_v<alias_ptr<volatile test_type>&, const volatile test_type&>, false));
            expect(eq(std::is_assignable_v<alias_ptr<const volatile test_type>&, test_type&>, true));
            expect(eq(std::is_assignable_v<alias_ptr<const volatile test_type>&, const test_type&>, true));
            expect(eq(std::is_assignable_v<alias_ptr<const volatile test_type>&, volatile test_type&>, true));
            expect(eq(std::is_assignable_v<alias_ptr<const volatile test_type>&, const volatile test_type&>, true));
        };

        "assignable from nullptr"_test = [] mutable {
            expect(eq(std::is_assignable_v<alias_ptr<std::int32_t>&, std::nullptr_t>, true));
        };

        "assignable from raw pointer"_test = [] mutable {
            using test_type = std::int32_t;

            expect(eq(std::is_assignable_v<alias_ptr<test_type>&, test_type*>, true));
            expect(eq(std::is_assignable_v<alias_ptr<test_type>&, const test_type*>, false));
            expect(eq(std::is_assignable_v<alias_ptr<test_type>&, volatile test_type*>, false));
            expect(eq(std::is_assignable_v<alias_ptr<test_type>&, const volatile test_type*>, false));
            expect(eq(std::is_assignable_v<alias_ptr<const test_type>&, test_type*>, true));
            expect(eq(std::is_assignable_v<alias_ptr<const test_type>&, const test_type*>, true));
            expect(eq(std::is_assignable_v<alias_ptr<const test_type>&, volatile test_type*>, false));
            expect(eq(std::is_assignable_v<alias_ptr<const test_type>&, const volatile test_type*>, false));
            expect(eq(std::is_assignable_v<alias_ptr<volatile test_type>&, test_type*>, true));
            expect(eq(std::is_assignable_v<alias_ptr<volatile test_type>&, const test_type*>, false));
            expect(eq(std::is_assignable_v<alias_ptr<volatile test_type>&, volatile test_type*>, true));
            expect(eq(std::is_assignable_v<alias_ptr<volatile test_type>&, const volatile test_type*>, false));
            expect(eq(std::is_assignable_v<alias_ptr<const volatile test_type>&, test_type*>, true));
            expect(eq(std::is_assignable_v<alias_ptr<const volatile test_type>&, const test_type*>, true));
            expect(eq(std::is_assignable_v<alias_ptr<const volatile test_type>&, volatile test_type*>, true));
            expect(eq(std::is_assignable_v<alias_ptr<const volatile test_type>&, const volatile test_type*>, true));
        };

        "assignable from smart pointer"_test = [] mutable {
            using test_type = std::int32_t;

            expect(eq(std::is_assignable_v<alias_ptr<test_type>&, trivial_smart_ptr<test_type>&>, true));
            expect(eq(std::is_assignable_v<alias_ptr<test_type>&, trivial_smart_ptr<const test_type>&>, false));
            expect(eq(std::is_assignable_v<alias_ptr<test_type>&, trivial_smart_ptr<volatile test_type>&>, false));
            expect(eq(std::is_assignable_v<alias_ptr<test_type>&, trivial_smart_ptr<const volatile test_type>&>, false));
            expect(eq(std::is_assignable_v<alias_ptr<const test_type>&, trivial_smart_ptr<test_type>&>, true));
            expect(eq(std::is_assignable_v<alias_ptr<const test_type>&, trivial_smart_ptr<const test_type>&>, true));
            expect(eq(std::is_assignable_v<alias_ptr<const test_type>&, trivial_smart_ptr<volatile test_type>&>, false));
            expect(
                eq(std::is_assignable_v<alias_ptr<const test_type>&, trivial_smart_ptr<const volatile test_type>&>, false)
            );
            expect(eq(std::is_assignable_v<alias_ptr<volatile test_type>&, trivial_smart_ptr<test_type>&>, true));
            expect(eq(std::is_assignable_v<alias_ptr<volatile test_type>&, trivial_smart_ptr<const test_type>&>, false));
            expect(eq(std::is_assignable_v<alias_ptr<volatile test_type>&, trivial_smart_ptr<volatile test_type>&>, true));
            expect(
                eq(std::is_assignable_v<alias_ptr<volatile test_type>&, trivial_smart_ptr<const volatile test_type>&>, false)
            );
            expect(eq(std::is_assignable_v<alias_ptr<const volatile test_type>&, trivial_smart_ptr<test_type>&>, true));
            expect(
                eq(std::is_assignable_v<alias_ptr<const volatile test_type>&, trivial_smart_ptr<const test_type>&>, true)
            );
            expect(
                eq(std::is_assignable_v<alias_ptr<const volatile test_type>&, trivial_smart_ptr<volatile test_type>&>, true)
            );
            expect(
                eq(std::is_assignable_v<alias_ptr<const volatile test_type>&, trivial_smart_ptr<const volatile test_type>&>,
                   true)
            );
        };

        "not assignable from rvalue"_test = [] mutable {
            expect(eq(std::is_assignable_v<alias_ptr<std::int32_t>&, std::int32_t&&>, false));
            expect(eq(std::is_assignable_v<alias_ptr<const std::int32_t>&, std::int32_t&&>, false));
            expect(eq(std::is_assignable_v<alias_ptr<const std::int32_t>&, const std::int32_t&&>, false));
        };

        //============================================================
        // Pointer semantics
        //============================================================

        "operator* dereferences correctly"_test = [] mutable {
            int x = 55;
            alias_ptr<std::int32_t> ptr{x};

            expect(eq(*ptr, 55));
        };

        "operator-> provides member access"_test = [] mutable {
            base_type obj{123};
            alias_ptr<base_type> ptr{obj};

            expect(eq(ptr->value, 123));
        };

        "implicit conversion to raw pointer"_test = [] mutable {
            using test_type = std::int32_t;

            expect(eq(std::convertible_to<alias_ptr<test_type>, test_type*>, true));
            expect(eq(std::convertible_to<alias_ptr<test_type>, const test_type*>, true));
            expect(eq(std::convertible_to<alias_ptr<test_type>, volatile test_type*>, true));
            expect(eq(std::convertible_to<alias_ptr<test_type>, const volatile test_type*>, true));
            expect(eq(std::convertible_to<alias_ptr<const test_type>, test_type*>, false));
            expect(eq(std::convertible_to<alias_ptr<const test_type>, const test_type*>, true));
            expect(eq(std::convertible_to<alias_ptr<const test_type>, volatile test_type*>, false));
            expect(eq(std::convertible_to<alias_ptr<const test_type>, const volatile test_type*>, true));
            expect(eq(std::convertible_to<alias_ptr<volatile test_type>, test_type*>, false));
            expect(eq(std::convertible_to<alias_ptr<volatile test_type>, const test_type*>, false));
            expect(eq(std::convertible_to<alias_ptr<volatile test_type>, volatile test_type*>, true));
            expect(eq(std::convertible_to<alias_ptr<volatile test_type>, const volatile test_type*>, true));
            expect(eq(std::convertible_to<alias_ptr<const volatile test_type>, test_type*>, false));
            expect(eq(std::convertible_to<alias_ptr<const volatile test_type>, const test_type*>, false));
            expect(eq(std::convertible_to<alias_ptr<const volatile test_type>, volatile test_type*>, false));
            expect(eq(std::convertible_to<alias_ptr<const volatile test_type>, const volatile test_type*>, true));
        };

        "conversion to raw pointer preserves address"_test = [] mutable {
            const int x{};
            alias_ptr<const std::int32_t> ptr{x};

            const std::int32_t* raw = ptr;

            expect(eq(raw, std::addressof(x)));
        };

        "get returns raw pointer"_test = [] mutable {
            const int x{};
            alias_ptr<const std::int32_t> ptr{x};

            expect(eq(ptr.get(), std::addressof(x)));
        };

        "contextual boolean conversion is supported"_test = [] mutable {
            expect(eq(std::constructible_from<bool, alias_ptr<std::int32_t>>, true));

            const int x{};
            alias_ptr<const std::int32_t> ptr{x};

            bool converted{false};
            if (ptr)
                converted = true;

            expect(eq(static_cast<bool>(ptr), true));
            expect(eq(!ptr, false));
            expect(eq(converted, true));
        };

        //============================================================
        // Rebinding
        //============================================================

        "rebind via assignment from reference"_test = [] mutable {
            const int a{};
            const int b = 2;

            alias_ptr<const std::int32_t> ptr{a};
            ptr = b;

            expect(eq(*ptr, 2));
            expect(eq(ptr.get(), std::addressof(b)));
        };

        //============================================================
        // Nullable, so no exception throwing on null
        //============================================================

        "constructing from null raw pointer does not throw"_test = [] mutable {
            const std::int32_t value{42};
            const std::int32_t* const bound_source{std::addressof(value)};
            const std::int32_t* const null_source{};

            bool threw_when_bound = false;
            try {
                const alias_ptr<const std::int32_t> ptr1{bound_source};

                expect(eq(*ptr1, value));
                expect(eq(ptr1.get(), bound_source));
            } catch (...) {
                threw_when_bound = true;
            }

            expect(eq(threw_when_bound, false));

            bool threw_when_null = false;
            try {
                alias_ptr<const std::int32_t> ptr2{null_source};
                expect(eq(ptr2.get(), null_source));
            } catch (...) {
                threw_when_null = true;
            }

            expect(eq(threw_when_null, false));
        };

        "constructing from null smart pointer does not throw"_test = [] mutable {
            const std::int32_t value{42};
            const trivial_smart_ptr<const std::int32_t> bound_source{std::addressof(value)};
            const trivial_smart_ptr<const std::int32_t> null_source{};

            bool threw_when_bound = false;
            try {
                const alias_ptr<const std::int32_t> ptr1{bound_source};

                expect(eq(*ptr1, value));
                expect(eq(ptr1.get(), bound_source.get()));
            } catch (...) {
                threw_when_bound = true;
            }

            expect(eq(threw_when_bound, false));

            bool threw_when_null = false;
            try {
                alias_ptr<const std::int32_t> ptr2{null_source};
                expect(eq(ptr2.get(), null_source.get()));
            } catch (...) {
                threw_when_null = true;
            }

            expect(eq(threw_when_null, false));
        };

        "assigning from null raw pointer does not throw"_test = [] mutable {
            const std::int32_t value{42};
            const std::int32_t other{};

            const std::int32_t* const bound_source{std::addressof(value)};
            const std::int32_t* const null_source{nullptr};

            alias_ptr<const std::int32_t> ptr{other};

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
            try {
                ptr = null_source;
            } catch (...) {
                threw_when_null = true;
            }

            expect(eq(threw_when_null, false));
            expect(eq(ptr.get(), null_source));
        };

        "assigning from null smart pointer does not throw"_test = [] mutable {
            const std::int32_t value{42};
            const std::int32_t other{};

            const trivial_smart_ptr<const std::int32_t> bound_source{std::addressof(value)};
            const trivial_smart_ptr<std::int32_t> null_source{};

            alias_ptr<const std::int32_t> ptr{other};

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
            try {
                ptr = null_source;
            } catch (...) {
                threw_when_null = true;
            }

            expect(eq(threw_when_null, false));
            expect(eq(ptr.get() == null_source.get(), true));
        };

        //============================================================
        // Equality semantics
        //============================================================

        "equality compares pointer identity"_test = [] mutable {
            expect(eq(std::equality_comparable<alias_ptr<std::int32_t>>, true));

            std::int32_t x = 1;
            std::int32_t y = 1;

            alias_ptr<const std::int32_t> a{x};
            alias_ptr<std::int32_t> b{x};
            alias_ptr<std::int32_t> c{y};

            expect(eq(a == b, true));
            expect(eq(a == c, false));
            expect(eq(b == c, false));
        };

        //============================================================
        // Covariance
        //============================================================

        "construct base from derived alias_ptr"_test = [] mutable {
            derived_type d;
            alias_ptr<derived_type> dptr{d};

            alias_ptr<base_type> bptr{dptr};

            expect(eq(bptr.get(), static_cast<base_type*>(std::addressof(d))));
        };

        "construct base from derived reference"_test = [] mutable {
            derived_type d;

            alias_ptr<base_type> bptr{d};

            expect(eq(bptr.get(), static_cast<base_type*>(std::addressof(d))));
        };

        "assign base from derived alias_ptr"_test = [] mutable {
            derived_type d;
            alias_ptr<derived_type> dptr{d};

            base_type b;
            alias_ptr<base_type> bptr{b};

            bptr = dptr;

            expect(eq(bptr.get(), static_cast<base_type*>(std::addressof(d))));
        };

        "rebind base from derived reference"_test = [] mutable {
            derived_type d;

            base_type b;
            alias_ptr<base_type> bptr{b};

            bptr = d;

            expect(eq(bptr.get(), static_cast<base_type*>(std::addressof(d))));
        };

        "assign const base from derived alias_ptr"_test = [] mutable {
            derived_type d;
            alias_ptr<derived_type> dptr{d};

            base_type b;
            alias_ptr<const base_type> bptr{b};

            bptr = dptr;

            expect(eq(bptr.get(), static_cast<const base_type*>(std::addressof(d))));
        };

        "rebind const base from derived reference"_test = [] mutable {
            derived_type d;

            base_type b;
            alias_ptr<const base_type> bptr{b};

            bptr = d;

            expect(eq(bptr.get(), static_cast<const base_type*>(std::addressof(d))));
        };

        "covariant equality comparison"_test = [] mutable {
            expect(eq(std::equality_comparable_with<alias_ptr<base_type>, alias_ptr<derived_type>>, true));
            expect(eq(std::equality_comparable_with<alias_ptr<base_type>, derived_type*>, true));
            expect(eq(std::equality_comparable_with<base_type*, alias_ptr<derived_type>>, true));
        };

        //============================================================
        // Non-iterator / non-arithmetic guarantees
        //============================================================

        "no pointer arithmetic operations"_test = [] mutable {
            using T = alias_ptr<std::int32_t>;

            expect(eq(HasAddition<T>, false));
            expect(eq(HasSubtraction<T>, false));
            expect(eq(HasDifference<T>, false));
            expect(eq(HasPreIncrement<T>, false));
            expect(eq(HasPostIncrement<T>, false));
            expect(eq(HasPreDecrement<T>, false));
            expect(eq(HasPostDecrement<T>, false));
        };

        "no ordering comparisons"_test = [] mutable {
            expect(eq(std::three_way_comparable<alias_ptr<std::int32_t>>, false));
            expect(eq(std::three_way_comparable_with<alias_ptr<std::int32_t>, std::int32_t*>, false));
            expect(eq(std::three_way_comparable<alias_ptr<base_type>>, false));
            expect(eq(std::three_way_comparable_with<alias_ptr<base_type>, alias_ptr<derived_type>>, false));
            expect(eq(std::three_way_comparable_with<alias_ptr<base_type>, derived_type*>, false));
            expect(eq(std::three_way_comparable_with<base_type*, alias_ptr<derived_type>>, false));
        };

        //============================================================
        // CV-correctness propagation
        //============================================================

        "const element forbids mutation through dereference"_test = [] mutable {
            const std::int32_t x{};
            alias_ptr<const std::int32_t> ptr{x};

            // Compile-time: *ptr must NOT be assignable
            constexpr bool can_assign = std::is_assignable_v<decltype(*ptr), std::int32_t>;

            expect(eq(can_assign, false));
        };

        "non-const element allows mutation"_test = [] mutable {
            std::int32_t x = 5;
            alias_ptr<std::int32_t> ptr{x};

            *ptr = 10;

            expect(eq(x, 10));
        };

        "`pointer` nested type preserves top-level const"_test = [] mutable {
            using T = alias_ptr<const std::int32_t>;

            expect(eq(std::same_as<typename T::pointer, const std::int32_t*>, true));
        };

        "`reference` nested type preserves const"_test = [] mutable {
            using T = alias_ptr<const std::int32_t>;

            expect(eq(std::same_as<typename T::reference, const std::int32_t&>, true));
        };

        "const alias_ptr prevents rebinding but not mutation"_test = [] mutable {
            std::int32_t x{};

            const alias_ptr<std::int32_t> ptr{x};

            *ptr = 10; // allowed

            constexpr bool can_rebind = std::is_assignable_v<const alias_ptr<std::int32_t>&, std::int32_t&>;

            expect(eq(x, 10));
            expect(eq(can_rebind, false));
        };

        "qualification climbing construction and assignment"_test = [] mutable {
            std::int32_t value{42};
            std::int32_t other{};
            alias_ptr<std::int32_t> mutable_ptr{value};
            alias_ptr<const std::int32_t> const_ptr{other};

            //Qualification climbing (Assignment)
            const_ptr = mutable_ptr;
            expect(eq(const_ptr.get() == mutable_ptr.get(), true));

            //Qualification climbing (Construction)
            alias_ptr<const std::int32_t> const_copy{mutable_ptr};
            expect(eq(const_copy.get() == mutable_ptr.get(), true));
        };

        "volatile qualifier preservation"_test = [] mutable {
            volatile std::int32_t hardware_register = 0xAA;
            alias_ptr<volatile std::int32_t> ptr{hardware_register};

            //Ensure the raw pointer retrieved is also volatile
            expect(eq(std::same_as<decltype(ptr.get()), volatile std::int32_t*>, true));

            //Ensure conversion to raw pointer preserves volatile
            volatile int* raw = ptr;
            expect(eq(raw, std::addressof(hardware_register)));

            //Ensure dereference preserves volatile
            decltype(auto) dereferenced = *ptr;
            expect(eq(std::is_volatile_v<std::remove_reference_t<decltype(dereferenced)>>, true));
            expect(eq(dereferenced, hardware_register));
        };

        //============================================================
        // Interoperability with raw pointer APIs
        //============================================================

        "implicit conversion works with raw pointer API"_test = [] mutable {
            auto takes_ptr = [](std::int32_t* p) { return *p; };

            std::int32_t x = 3;
            alias_ptr<std::int32_t> ptr{x};

            expect(eq(takes_ptr(ptr), 3));
        };

        "get() works with raw pointer API"_test = [] mutable {
            auto takes_ptr = [](std::int32_t* p) { return *p; };

            std::int32_t x = 4;
            alias_ptr<std::int32_t> ptr{x};

            expect(eq(takes_ptr(ptr.get()), 4));
        };

        "not constructible, convertible, nor assignable from C-array decay"_test = [] mutable {
            std::int32_t array[3] = {0, 1, 2};

            //alias_ptr<std::int32_t> should_fail{array};

            expect(eq(std::convertible_to<decltype(array), alias_ptr<std::int32_t>>, false));
            expect(eq(std::constructible_from<alias_ptr<std::int32_t>, decltype(array)>, false));
            expect(eq(std::is_assignable_v<alias_ptr<std::int32_t>&, decltype(array)>, false));

            expect(eq(std::constructible_from<alias_ptr<std::int32_t>, decltype(array[0])>, true));
            expect(eq(std::is_assignable_v<alias_ptr<std::int32_t>&, decltype(array[0])>, true));

            alias_ptr<std::int32_t> ptr{array[1]};

            //Ensure binding to the element is equivalent to expected array-to-pointer decay with pointer offset arithmetic
            expect(eq(ptr.get(), array + 1));
        };

        //============================================================
        // `void` support
        //============================================================

        "type aliases are correct for `void`"_test = [] mutable {
            using T = alias_ptr<void>;

            constexpr bool element = std::same_as<T::element_type, void>;
            constexpr bool value   = std::same_as<T::value_type, void>;
            constexpr bool pointer = std::same_as<T::pointer, void*>;
            constexpr bool ref     = std::same_as<T::reference, std::monostate&>;
            constexpr bool rref    = std::same_as<T::rvalue_reference, std::monostate&&>;
            constexpr bool ptrdiff = std::same_as<T::difference_type, std::ptrdiff_t>;

            expect(eq(element, true));
            expect(eq(value, true));
            expect(eq(pointer, true));
            expect(eq(ref, true));
            expect(eq(rref, true));
            expect(eq(ptrdiff, true));
        };

        "void specialization supports type erasure"_test = [] mutable {
            std::int32_t x{};

            alias_ptr<std::int32_t> typed{x};
            alias_ptr<void> erased{typed};

            expect(eq(erased.get(), static_cast<void*>(std::addressof(x))));
        };

        "void specialization disables dereference operators"_test = [] mutable {
            expect(eq(Dereferenceable<alias_ptr<std::int32_t>>, true));
            expect(eq(ArrowAccessible<alias_ptr<std::int32_t>>, true));

            expect(eq(Dereferenceable<alias_ptr<void>>, false));
            expect(eq(ArrowAccessible<alias_ptr<void>>, false));
        };

        "void raw pointer construction is explicit"_test = [] mutable {
            expect(eq(std::convertible_to<void*, alias_ptr<void>>, false));
            expect(eq(std::constructible_from<alias_ptr<void>, void*>, true));
        };

        "void smart pointer construction is explicit"_test = [] mutable {
            expect(eq(std::convertible_to<trivial_smart_ptr<void>&, alias_ptr<void>>, false));
            expect(eq(std::constructible_from<alias_ptr<void>, trivial_smart_ptr<void>&>, true));
        };

        "void `alias_ptr` constructs implicitly from typed `alias_ptr`"_test = [] mutable {
            expect(eq(std::convertible_to<alias_ptr<std::int32_t>, alias_ptr<void>>, true));
            expect(eq(std::convertible_to<alias_ptr<void>, alias_ptr<std::int32_t>>, false));

            const std::int32_t value{42};
            alias_ptr<const std::int32_t> typed_ptr{value};

            // Should be implicit (convertible)
            auto takes_void = [](alias_ptr<const void> ptr) { return ptr.get(); };
            expect(eq(takes_void(typed_ptr), static_cast<const void*>(std::addressof(value))));
        };

        "void `alias_ptr` is equality comparable"_test = [] mutable {
            const std::int32_t value{42};
            alias_ptr<const std::int32_t> typed_ptr{value};
            const std::int32_t* typed_raw = std::addressof(value);
            const void* erased_raw        = std::addressof(value);

            alias_ptr<const void> erased_ptr1{typed_raw};
            alias_ptr<const void> erased_ptr2{typed_ptr};

            expect(eq(erased_ptr1 == erased_ptr2, true));
            expect(eq(erased_ptr1 == typed_ptr, true));
            expect(eq(erased_ptr1 == erased_raw, true));
            expect(eq(erased_ptr1 == typed_raw, true));
        };

        //============================================================
        // CTAD Guide
        //============================================================

        "deduction guides work"_test = [] mutable {
            constexpr std::int32_t x = {};
            alias_ptr ptr1{x};
            alias_ptr ptr2{std::addressof(x)};

            expect(eq(ptr1.get(), std::addressof(x)));
            expect(eq(ptr2.get(), std::addressof(x)));
        };

        //============================================================
        // Constant Expression Usage
        //============================================================

        "constexpr construction and dereference"_test = [] {
            static constexpr int x = 42;

            constexpr alias_ptr<const int> ptr{x};

            expect(eq(*ptr, 42));
        };
//--------------------------------------------------------------------------------
        //============================================================
        // Swap
        //============================================================

        "swap exchanges bindings"_test = [] mutable {
            const std::int32_t a = 1;
            const std::int32_t b = 2;

            alias_ptr<const std::int32_t> lhs{a};
            alias_ptr<const std::int32_t> rhs{b};

            using std::swap;
            swap(lhs, rhs);

            expect(eq(lhs.get(), std::addressof(b)));
            expect(eq(rhs.get(), std::addressof(a)));

            expect(eq(*lhs, b));
            expect(eq(*rhs, a));
        };

        //============================================================
        // Hash Support
        //============================================================

        "hash matches raw pointer hash"_test = [] mutable {
            std::int32_t value{};

            alias_ptr<std::int32_t> ptr{value};

            const auto ptr_hash = std::hash<alias_ptr<std::int32_t>>{}(ptr);
            const auto raw_hash = std::hash<std::int32_t*>{}(std::addressof(value));

            expect(eq(ptr_hash, raw_hash));
        };

        "equal pointers produce equal hashes"_test = [] mutable {
            std::int32_t value{};

            alias_ptr<std::int32_t> lhs{value};
            alias_ptr<const std::int32_t> rhs{value};

            const auto lhs_hash = std::hash<alias_ptr<std::int32_t>>{}(lhs);
            const auto rhs_hash = std::hash<alias_ptr<const std::int32_t>>{}(rhs);

            expect(eq(lhs == rhs, true));
            expect(eq(lhs_hash == rhs_hash, true));
        };

        //============================================================
        // Nullable Comparisons
        //============================================================

        "nullable comparisons with nullptr"_test = [] mutable {
            std::int32_t value{};

            alias_ptr<std::int32_t> bound{value};
            alias_ptr<std::int32_t> null{nullptr};

            expect(eq(bound == nullptr, false));
            expect(eq(nullptr == bound, false));

            expect(eq(bound != nullptr, true));
            expect(eq(nullptr != bound, true));

            expect(eq(null == nullptr, true));
            expect(eq(nullptr == null, true));

            expect(eq(null != nullptr, false));
            expect(eq(nullptr != null, false));
        };

        "null alias_ptr compares equal to null alias_ptr"_test = [] mutable {
            alias_ptr<std::int32_t> lhs{nullptr};
            alias_ptr<const std::int32_t> rhs{nullptr};

            expect(eq(lhs == rhs, true));
            expect(eq(lhs != rhs, false));
        };

        //============================================================
        // Common Reference / Common Type
        //============================================================

        "common_type works covariantly"_test = [] mutable {
            using common_t =
                std::common_type_t<
                    alias_ptr<derived_type>,
                    alias_ptr<base_type>
                >;

            expect(eq(std::same_as<common_t, alias_ptr<base_type>>, true));
        };

        "common_type preserves const qualification"_test = [] mutable {
            using common_t =
                std::common_type_t<
                    alias_ptr<std::int32_t>,
                    alias_ptr<const std::int32_t>
                >;

            expect(eq(std::same_as<common_t, alias_ptr<const std::int32_t>>, true));
        };

        "common_reference works covariantly"_test = [] mutable {
            using common_ref =
                std::common_reference_t<
                    alias_ptr<derived_type>,
                    alias_ptr<base_type>
                >;

            expect(eq(std::same_as<common_ref, alias_ptr<base_type>>, true));
        };

        "common_reference preserves const qualification"_test = [] mutable {
            using common_ref =
                std::common_reference_t<
                    alias_ptr<std::int32_t>,
                    alias_ptr<const std::int32_t>
                >;

            expect(eq(std::same_as<common_ref, alias_ptr<const std::int32_t>>, true));
        };

        //============================================================
        // Iterator Concept Exclusion
        //============================================================

        "not indirectly readable"_test = [] mutable {
            expect(eq(std::indirectly_readable<alias_ptr<std::int32_t>>, false));
        };

        "not weakly incrementable"_test = [] mutable {
            expect(eq(std::weakly_incrementable<alias_ptr<std::int32_t>>, false));
        };

        "not input iterator"_test = [] mutable {
            expect(eq(std::input_iterator<alias_ptr<std::int32_t>>, false));
        };

        "not forward iterator"_test = [] mutable {
            expect(eq(std::forward_iterator<alias_ptr<std::int32_t>>, false));
        };

        "not bidirectional iterator"_test = [] mutable {
            expect(eq(std::bidirectional_iterator<alias_ptr<std::int32_t>>, false));
        };

        "not random access iterator"_test = [] mutable {
            expect(eq(std::random_access_iterator<alias_ptr<std::int32_t>>, false));
        };

        "not contiguous iterator"_test = [] mutable {
            expect(eq(std::contiguous_iterator<alias_ptr<std::int32_t>>, false));
        };

    };
} //namespace

int main() {}
