// SPDX-License-Identifier: Apache-2.0
// Unit tests for base.vocab.ptr:cursor_ptr

import base.vocab.ptr;
import ut;
import std;

import base.meta.concepts;

using namespace ut;
using base::vocab::cursor_ptr;

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

    suite base_vocab_ptr_cursor_ptr_unit = [] mutable {
        //============================================================
        // Template Constraint Validation
        //============================================================

        "template instantiation checks"_test = [] mutable {
            using base::meta::concepts::instantiable_with;
            expect(eq(instantiable_with<cursor_ptr, std::int32_t>, true));
            expect(eq(instantiable_with<cursor_ptr, std::int32_t*>, true));
            expect(eq(instantiable_with<cursor_ptr, std::map<std::string, std::vector<std::int32_t>>>, true));

            expect(eq(instantiable_with<cursor_ptr, void>, true));

            expect(eq(instantiable_with<cursor_ptr, std::int32_t&>, false));
            expect(eq(instantiable_with<cursor_ptr, std::int32_t&&>, false));
            expect(eq(instantiable_with<cursor_ptr, void(int)>, false));
            expect(eq(instantiable_with<cursor_ptr, void (&)(int)>, false));
            expect(eq(instantiable_with<cursor_ptr, void (*)(int, float)>, false));
            expect(eq(instantiable_with<cursor_ptr, void (**)(std::string, int)>, false));
            expect(eq(instantiable_with<cursor_ptr, void (*******)(int)>, false));
        };

        //============================================================
        // Triviality & ABI properties
        //============================================================

        "triviality"_test = [] mutable {
            using simple_t = std::int32_t;

            expect(eq(std::is_standard_layout_v<cursor_ptr<simple_t>>, true));
            expect(eq(std::is_trivially_copyable_v<cursor_ptr<simple_t>>, true));
            expect(eq(std::is_trivially_destructible_v<cursor_ptr<simple_t>>, true));
            expect(eq(std::is_trivially_copy_constructible_v<cursor_ptr<simple_t>>, true));
            expect(eq(std::is_trivially_move_constructible_v<cursor_ptr<simple_t>>, true));
            expect(eq(std::is_trivially_copy_assignable_v<cursor_ptr<simple_t>>, true));
            expect(eq(std::is_trivially_move_assignable_v<cursor_ptr<simple_t>>, true));
            expect(eq(std::is_nothrow_constructible_v<cursor_ptr<simple_t>, simple_t&>, true));

            using complex_t = std::map<std::string, std::vector<std::int32_t>>;

            expect(eq(std::is_standard_layout_v<cursor_ptr<complex_t>>, true));
            expect(eq(std::is_trivially_copyable_v<cursor_ptr<complex_t>>, true));
            expect(eq(std::is_trivially_destructible_v<cursor_ptr<complex_t>>, true));
            expect(eq(std::is_trivially_copy_constructible_v<cursor_ptr<complex_t>>, true));
            expect(eq(std::is_trivially_move_constructible_v<cursor_ptr<complex_t>>, true));
            expect(eq(std::is_trivially_copy_assignable_v<cursor_ptr<complex_t>>, true));
            expect(eq(std::is_trivially_move_assignable_v<cursor_ptr<complex_t>>, true));
            expect(eq(std::is_nothrow_constructible_v<cursor_ptr<complex_t>, complex_t&>, true));
        };

        "size and alignment match raw pointers"_test = [] mutable {
            using simple_t = std::int32_t;

            expect(eq(sizeof(cursor_ptr<simple_t>) == sizeof(simple_t*), true));
            expect(eq(alignof(cursor_ptr<simple_t>) == alignof(simple_t*), true));

            using complex_t = std::map<std::string, std::vector<std::int32_t>>;

            expect(eq(sizeof(cursor_ptr<complex_t>) == sizeof(complex_t*), true));
            expect(eq(alignof(cursor_ptr<complex_t>) == alignof(complex_t*), true));
        };

        //============================================================
        // Type properties
        //============================================================

        "type aliases are correct"_test = [] mutable {
            using T = cursor_ptr<const std::int32_t>;

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
            expect(eq(std::constructible_from<cursor_ptr<test_type>, test_type&>, true));
            expect(eq(std::constructible_from<cursor_ptr<test_type>, const test_type&>, false));
            expect(eq(std::constructible_from<cursor_ptr<test_type>, volatile test_type&>, false));
            expect(eq(std::constructible_from<cursor_ptr<test_type>, const volatile test_type&>, false));
            expect(eq(std::constructible_from<cursor_ptr<const test_type>, test_type&>, true));
            expect(eq(std::constructible_from<cursor_ptr<const test_type>, const test_type&>, true));
            expect(eq(std::constructible_from<cursor_ptr<const test_type>, volatile test_type&>, false));
            expect(eq(std::constructible_from<cursor_ptr<const test_type>, const volatile test_type&>, false));
            expect(eq(std::constructible_from<cursor_ptr<volatile test_type>, test_type&>, true));
            expect(eq(std::constructible_from<cursor_ptr<volatile test_type>, const test_type&>, false));
            expect(eq(std::constructible_from<cursor_ptr<volatile test_type>, volatile test_type&>, true));
            expect(eq(std::constructible_from<cursor_ptr<volatile test_type>, const volatile test_type&>, false));
            expect(eq(std::constructible_from<cursor_ptr<const volatile test_type>, test_type&>, true));
            expect(eq(std::constructible_from<cursor_ptr<const volatile test_type>, const test_type&>, true));
            expect(eq(std::constructible_from<cursor_ptr<const volatile test_type>, volatile test_type&>, true));
            expect(eq(std::constructible_from<cursor_ptr<const volatile test_type>, const volatile test_type&>, true));

            //Never implicitly convertible (constructor is explicit)
            expect(eq(std::convertible_to<test_type&, cursor_ptr<test_type>>, false));
            expect(eq(std::convertible_to<const test_type&, cursor_ptr<test_type>>, false));
            expect(eq(std::convertible_to<test_type&, cursor_ptr<const test_type>>, false));
            expect(eq(std::convertible_to<const test_type&, cursor_ptr<const test_type>>, false));
        };

        "not default constructible"_test = [] mutable {
            expect(eq(std::default_initializable<cursor_ptr<std::int32_t>>, false));
        };

        "not constructible from nullptr"_test = [] mutable {
            expect(eq(std::constructible_from<cursor_ptr<std::int32_t>, std::nullptr_t>, false));
        };

        "constructible from raw pointer"_test = [] mutable {
            using test_type = std::int32_t;

            //Explicitly constructible unless removing qualifier
            expect(eq(std::constructible_from<cursor_ptr<test_type>, test_type*>, true));
            expect(eq(std::constructible_from<cursor_ptr<test_type>, const test_type*>, false));
            expect(eq(std::constructible_from<cursor_ptr<test_type>, volatile test_type*>, false));
            expect(eq(std::constructible_from<cursor_ptr<test_type>, const volatile test_type*>, false));
            expect(eq(std::constructible_from<cursor_ptr<const test_type>, test_type*>, true));
            expect(eq(std::constructible_from<cursor_ptr<const test_type>, const test_type*>, true));
            expect(eq(std::constructible_from<cursor_ptr<const test_type>, volatile test_type*>, false));
            expect(eq(std::constructible_from<cursor_ptr<const test_type>, const volatile test_type*>, false));
            expect(eq(std::constructible_from<cursor_ptr<volatile test_type>, test_type*>, true));
            expect(eq(std::constructible_from<cursor_ptr<volatile test_type>, const test_type*>, false));
            expect(eq(std::constructible_from<cursor_ptr<volatile test_type>, volatile test_type*>, true));
            expect(eq(std::constructible_from<cursor_ptr<volatile test_type>, const volatile test_type*>, false));
            expect(eq(std::constructible_from<cursor_ptr<const volatile test_type>, test_type*>, true));
            expect(eq(std::constructible_from<cursor_ptr<const volatile test_type>, const test_type*>, true));
            expect(eq(std::constructible_from<cursor_ptr<const volatile test_type>, volatile test_type*>, true));
            expect(eq(std::constructible_from<cursor_ptr<const volatile test_type>, const volatile test_type*>, true));

            //Implicitly convertible unless removing qualifier
            expect(eq(std::convertible_to<test_type*, cursor_ptr<test_type>>, true));
            expect(eq(std::convertible_to<const test_type*, cursor_ptr<test_type>>, false));
            expect(eq(std::convertible_to<volatile test_type*, cursor_ptr<test_type>>, false));
            expect(eq(std::convertible_to<const volatile test_type*, cursor_ptr<test_type>>, false));
            expect(eq(std::convertible_to<test_type*, cursor_ptr<const test_type>>, true));
            expect(eq(std::convertible_to<const test_type*, cursor_ptr<const test_type>>, true));
            expect(eq(std::convertible_to<volatile test_type*, cursor_ptr<const test_type>>, false));
            expect(eq(std::convertible_to<const volatile test_type*, cursor_ptr<const test_type>>, false));
            expect(eq(std::convertible_to<test_type*, cursor_ptr<volatile test_type>>, true));
            expect(eq(std::convertible_to<const test_type*, cursor_ptr<volatile test_type>>, false));
            expect(eq(std::convertible_to<volatile test_type*, cursor_ptr<volatile test_type>>, true));
            expect(eq(std::convertible_to<const volatile test_type*, cursor_ptr<volatile test_type>>, false));
            expect(eq(std::convertible_to<test_type*, cursor_ptr<const volatile test_type>>, true));
            expect(eq(std::convertible_to<const test_type*, cursor_ptr<const volatile test_type>>, true));
            expect(eq(std::convertible_to<volatile test_type*, cursor_ptr<const volatile test_type>>, true));
            expect(eq(std::convertible_to<const volatile test_type*, cursor_ptr<const volatile test_type>>, true));
        };

        "constructible from smart pointer"_test = [] mutable {
            using test_type = std::int32_t;

            //Explicitly constructible unless removing qualifier
            expect(eq(std::constructible_from<cursor_ptr<test_type>, trivial_smart_ptr<test_type>&>, true));
            expect(eq(std::constructible_from<cursor_ptr<test_type>, trivial_smart_ptr<const test_type>&>, false));
            expect(eq(std::constructible_from<cursor_ptr<test_type>, trivial_smart_ptr<volatile test_type>&>, false));
            expect(eq(std::constructible_from<cursor_ptr<test_type>, trivial_smart_ptr<const volatile test_type>&>, false));
            expect(eq(std::constructible_from<cursor_ptr<const test_type>, trivial_smart_ptr<test_type>&>, true));
            expect(eq(std::constructible_from<cursor_ptr<const test_type>, trivial_smart_ptr<const test_type>&>, true));
            expect(eq(std::constructible_from<cursor_ptr<const test_type>, trivial_smart_ptr<volatile test_type>&>, false));
            expect(
                eq(std::constructible_from<cursor_ptr<const test_type>, trivial_smart_ptr<const volatile test_type>&>, false)
            );
            expect(eq(std::constructible_from<cursor_ptr<volatile test_type>, trivial_smart_ptr<test_type>&>, true));
            expect(eq(std::constructible_from<cursor_ptr<volatile test_type>, trivial_smart_ptr<const test_type>&>, false));
            expect(eq(std::constructible_from<cursor_ptr<volatile test_type>, trivial_smart_ptr<volatile test_type>&>, true));
            expect(
                eq(std::constructible_from<cursor_ptr<volatile test_type>, trivial_smart_ptr<const volatile test_type>&>,
                   false)
            );
            expect(eq(std::constructible_from<cursor_ptr<const volatile test_type>, trivial_smart_ptr<test_type>&>, true));
            expect(
                eq(std::constructible_from<cursor_ptr<const volatile test_type>, trivial_smart_ptr<const test_type>&>, true)
            );
            expect(
                eq(std::constructible_from<cursor_ptr<const volatile test_type>, trivial_smart_ptr<volatile test_type>&>,
                   true)
            );
            expect(eq(
                std::constructible_from<cursor_ptr<const volatile test_type>, trivial_smart_ptr<const volatile test_type>&>,
                true
            ));

            //Implicitly convertible unless removing qualifier
            expect(eq(std::convertible_to<trivial_smart_ptr<test_type>&, cursor_ptr<test_type>>, true));
            expect(eq(std::convertible_to<trivial_smart_ptr<const test_type>&, cursor_ptr<test_type>>, false));
            expect(eq(std::convertible_to<trivial_smart_ptr<volatile test_type>&, cursor_ptr<test_type>>, false));
            expect(eq(std::convertible_to<trivial_smart_ptr<const volatile test_type>&, cursor_ptr<test_type>>, false));
            expect(eq(std::convertible_to<trivial_smart_ptr<test_type>&, cursor_ptr<const test_type>>, true));
            expect(eq(std::convertible_to<trivial_smart_ptr<const test_type>&, cursor_ptr<const test_type>>, true));
            expect(eq(std::convertible_to<trivial_smart_ptr<volatile test_type>&, cursor_ptr<const test_type>>, false));
            expect(eq(std::convertible_to<trivial_smart_ptr<const volatile test_type>&, cursor_ptr<const test_type>>, false));
            expect(eq(std::convertible_to<trivial_smart_ptr<test_type>&, cursor_ptr<volatile test_type>>, true));
            expect(eq(std::convertible_to<trivial_smart_ptr<const test_type>&, cursor_ptr<volatile test_type>>, false));
            expect(eq(std::convertible_to<trivial_smart_ptr<volatile test_type>&, cursor_ptr<volatile test_type>>, true));
            expect(
                eq(std::convertible_to<trivial_smart_ptr<const volatile test_type>&, cursor_ptr<volatile test_type>>, false)
            );
            expect(eq(std::convertible_to<trivial_smart_ptr<test_type>&, cursor_ptr<const volatile test_type>>, true));
            expect(eq(std::convertible_to<trivial_smart_ptr<const test_type>&, cursor_ptr<const volatile test_type>>, true));
            expect(
                eq(std::convertible_to<trivial_smart_ptr<volatile test_type>&, cursor_ptr<const volatile test_type>>, true)
            );
            expect(
                eq(std::convertible_to<trivial_smart_ptr<const volatile test_type>&, cursor_ptr<const volatile test_type>>,
                   true)
            );
        };

        "not constructible from rvalue"_test = [] mutable {
            expect(eq(std::constructible_from<cursor_ptr<std::int32_t>, std::int32_t>, false));
            expect(eq(std::constructible_from<cursor_ptr<const std::int32_t>, std::int32_t>, false));
            expect(eq(std::constructible_from<cursor_ptr<const std::int32_t>, const std::int32_t>, false));

            expect(eq(std::constructible_from<cursor_ptr<std::int32_t>, std::int32_t&&>, false));
            expect(eq(std::constructible_from<cursor_ptr<const std::int32_t>, std::int32_t&&>, false));
            expect(eq(std::constructible_from<cursor_ptr<const std::int32_t>, const std::int32_t&&>, false));
        };

        //============================================================
        // Assignment
        //============================================================

        "assignable from lvalue reference"_test = [] mutable {
            using test_type = std::int32_t;

            expect(eq(std::is_assignable_v<cursor_ptr<test_type>&, test_type&>, true));
            expect(eq(std::is_assignable_v<cursor_ptr<test_type>&, const test_type&>, false));
            expect(eq(std::is_assignable_v<cursor_ptr<test_type>&, volatile test_type&>, false));
            expect(eq(std::is_assignable_v<cursor_ptr<test_type>&, const volatile test_type&>, false));
            expect(eq(std::is_assignable_v<cursor_ptr<const test_type>&, test_type&>, true));
            expect(eq(std::is_assignable_v<cursor_ptr<const test_type>&, const test_type&>, true));
            expect(eq(std::is_assignable_v<cursor_ptr<const test_type>&, volatile test_type&>, false));
            expect(eq(std::is_assignable_v<cursor_ptr<const test_type>&, const volatile test_type&>, false));
            expect(eq(std::is_assignable_v<cursor_ptr<volatile test_type>&, test_type&>, true));
            expect(eq(std::is_assignable_v<cursor_ptr<volatile test_type>&, const test_type&>, false));
            expect(eq(std::is_assignable_v<cursor_ptr<volatile test_type>&, volatile test_type&>, true));
            expect(eq(std::is_assignable_v<cursor_ptr<volatile test_type>&, const volatile test_type&>, false));
            expect(eq(std::is_assignable_v<cursor_ptr<const volatile test_type>&, test_type&>, true));
            expect(eq(std::is_assignable_v<cursor_ptr<const volatile test_type>&, const test_type&>, true));
            expect(eq(std::is_assignable_v<cursor_ptr<const volatile test_type>&, volatile test_type&>, true));
            expect(eq(std::is_assignable_v<cursor_ptr<const volatile test_type>&, const volatile test_type&>, true));
        };

        "not assignable from nullptr"_test = [] mutable {
            expect(eq(std::is_assignable_v<cursor_ptr<std::int32_t>&, std::nullptr_t>, false));
        };

        "assignable from raw pointer"_test = [] mutable {
            using test_type = std::int32_t;

            expect(eq(std::is_assignable_v<cursor_ptr<test_type>&, test_type*>, true));
            expect(eq(std::is_assignable_v<cursor_ptr<test_type>&, const test_type*>, false));
            expect(eq(std::is_assignable_v<cursor_ptr<test_type>&, volatile test_type*>, false));
            expect(eq(std::is_assignable_v<cursor_ptr<test_type>&, const volatile test_type*>, false));
            expect(eq(std::is_assignable_v<cursor_ptr<const test_type>&, test_type*>, true));
            expect(eq(std::is_assignable_v<cursor_ptr<const test_type>&, const test_type*>, true));
            expect(eq(std::is_assignable_v<cursor_ptr<const test_type>&, volatile test_type*>, false));
            expect(eq(std::is_assignable_v<cursor_ptr<const test_type>&, const volatile test_type*>, false));
            expect(eq(std::is_assignable_v<cursor_ptr<volatile test_type>&, test_type*>, true));
            expect(eq(std::is_assignable_v<cursor_ptr<volatile test_type>&, const test_type*>, false));
            expect(eq(std::is_assignable_v<cursor_ptr<volatile test_type>&, volatile test_type*>, true));
            expect(eq(std::is_assignable_v<cursor_ptr<volatile test_type>&, const volatile test_type*>, false));
            expect(eq(std::is_assignable_v<cursor_ptr<const volatile test_type>&, test_type*>, true));
            expect(eq(std::is_assignable_v<cursor_ptr<const volatile test_type>&, const test_type*>, true));
            expect(eq(std::is_assignable_v<cursor_ptr<const volatile test_type>&, volatile test_type*>, true));
            expect(eq(std::is_assignable_v<cursor_ptr<const volatile test_type>&, const volatile test_type*>, true));
        };

        "assignable from smart pointer"_test = [] mutable {
            using test_type = std::int32_t;

            expect(eq(std::is_assignable_v<cursor_ptr<test_type>&, trivial_smart_ptr<test_type>&>, true));
            expect(eq(std::is_assignable_v<cursor_ptr<test_type>&, trivial_smart_ptr<const test_type>&>, false));
            expect(eq(std::is_assignable_v<cursor_ptr<test_type>&, trivial_smart_ptr<volatile test_type>&>, false));
            expect(eq(std::is_assignable_v<cursor_ptr<test_type>&, trivial_smart_ptr<const volatile test_type>&>, false));
            expect(eq(std::is_assignable_v<cursor_ptr<const test_type>&, trivial_smart_ptr<test_type>&>, true));
            expect(eq(std::is_assignable_v<cursor_ptr<const test_type>&, trivial_smart_ptr<const test_type>&>, true));
            expect(eq(std::is_assignable_v<cursor_ptr<const test_type>&, trivial_smart_ptr<volatile test_type>&>, false));
            expect(
                eq(std::is_assignable_v<cursor_ptr<const test_type>&, trivial_smart_ptr<const volatile test_type>&>, false)
            );
            expect(eq(std::is_assignable_v<cursor_ptr<volatile test_type>&, trivial_smart_ptr<test_type>&>, true));
            expect(eq(std::is_assignable_v<cursor_ptr<volatile test_type>&, trivial_smart_ptr<const test_type>&>, false));
            expect(eq(std::is_assignable_v<cursor_ptr<volatile test_type>&, trivial_smart_ptr<volatile test_type>&>, true));
            expect(
                eq(std::is_assignable_v<cursor_ptr<volatile test_type>&, trivial_smart_ptr<const volatile test_type>&>, false)
            );
            expect(eq(std::is_assignable_v<cursor_ptr<const volatile test_type>&, trivial_smart_ptr<test_type>&>, true));
            expect(
                eq(std::is_assignable_v<cursor_ptr<const volatile test_type>&, trivial_smart_ptr<const test_type>&>, true)
            );
            expect(
                eq(std::is_assignable_v<cursor_ptr<const volatile test_type>&, trivial_smart_ptr<volatile test_type>&>, true)
            );
            expect(
                eq(std::is_assignable_v<cursor_ptr<const volatile test_type>&, trivial_smart_ptr<const volatile test_type>&>,
                   true)
            );
        };

        "not assignable from rvalue"_test = [] mutable {
            expect(eq(std::is_assignable_v<cursor_ptr<std::int32_t>&, std::int32_t&&>, false));
            expect(eq(std::is_assignable_v<cursor_ptr<const std::int32_t>&, std::int32_t&&>, false));
            expect(eq(std::is_assignable_v<cursor_ptr<const std::int32_t>&, const std::int32_t&&>, false));
        };

        //============================================================
        // Pointer semantics
        //============================================================

        "operator* dereferences correctly"_test = [] mutable {
            int x = 55;
            cursor_ptr<std::int32_t> ptr{x};

            expect(eq(*ptr, 55));
        };

        "operator-> provides member access"_test = [] mutable {
            base_type obj{123};
            cursor_ptr<base_type> ptr{obj};

            expect(eq(ptr->value, 123));
        };

        "implicit conversion to raw pointer"_test = [] mutable {
            using test_type = std::int32_t;

            expect(eq(std::convertible_to<cursor_ptr<test_type>, test_type*>, true));
            expect(eq(std::convertible_to<cursor_ptr<test_type>, const test_type*>, true));
            expect(eq(std::convertible_to<cursor_ptr<test_type>, volatile test_type*>, true));
            expect(eq(std::convertible_to<cursor_ptr<test_type>, const volatile test_type*>, true));
            expect(eq(std::convertible_to<cursor_ptr<const test_type>, test_type*>, false));
            expect(eq(std::convertible_to<cursor_ptr<const test_type>, const test_type*>, true));
            expect(eq(std::convertible_to<cursor_ptr<const test_type>, volatile test_type*>, false));
            expect(eq(std::convertible_to<cursor_ptr<const test_type>, const volatile test_type*>, true));
            expect(eq(std::convertible_to<cursor_ptr<volatile test_type>, test_type*>, false));
            expect(eq(std::convertible_to<cursor_ptr<volatile test_type>, const test_type*>, false));
            expect(eq(std::convertible_to<cursor_ptr<volatile test_type>, volatile test_type*>, true));
            expect(eq(std::convertible_to<cursor_ptr<volatile test_type>, const volatile test_type*>, true));
            expect(eq(std::convertible_to<cursor_ptr<const volatile test_type>, test_type*>, false));
            expect(eq(std::convertible_to<cursor_ptr<const volatile test_type>, const test_type*>, false));
            expect(eq(std::convertible_to<cursor_ptr<const volatile test_type>, volatile test_type*>, false));
            expect(eq(std::convertible_to<cursor_ptr<const volatile test_type>, const volatile test_type*>, true));
        };

        "conversion to raw pointer preserves address"_test = [] mutable {
            const int x{};
            cursor_ptr<const std::int32_t> ptr{x};

            const std::int32_t* raw = ptr;

            expect(eq(raw, std::addressof(x)));
        };

        "get returns raw pointer"_test = [] mutable {
            const int x{};
            cursor_ptr<const std::int32_t> ptr{x};

            expect(eq(ptr.get(), std::addressof(x)));
        };

        "contextual boolean conversion is supported"_test = [] mutable {
            expect(eq(std::constructible_from<bool, cursor_ptr<std::int32_t>>, true));

            const int x{};
            cursor_ptr<const std::int32_t> ptr{x};

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

            cursor_ptr<const std::int32_t> ptr{a};
            ptr = b;

            expect(eq(*ptr, 2));
            expect(eq(ptr.get(), std::addressof(b)));
        };

        "rebind via `rebind` call with reference argument"_test = [] mutable {
            const int a{};
            const int b = 2;

            alias_ptr<const std::int32_t> ptr{a};
            ptr.rebind(b);

            expect(eq(*ptr, 2));
            expect(eq(ptr.get(), std::addressof(b)));
        };

        "rebind via `reset` call with reference argument"_test = [] mutable {
            const int a{};
            const int b = 2;

            alias_ptr<const std::int32_t> ptr{a};
            ptr.reset(b);

            expect(eq(*ptr, 2));
            expect(eq(ptr.get(), std::addressof(b)));
        };

        //============================================================
        // Non-nullable exception throwing
        //============================================================

        "constructing from null raw pointer throws"_test = [] mutable {
            const std::int32_t value{42};
            const std::int32_t* const bound_source{std::addressof(value)};
            const std::int32_t* const null_source{};

            bool threw_when_bound = false;
            try {
                const cursor_ptr<const std::int32_t> ptr{bound_source};

                expect(eq(*ptr, value));
                expect(eq(ptr.get(), bound_source));
            } catch (...) {
                threw_when_bound = true;
            }

            expect(eq(threw_when_bound, false));

            bool threw_when_null = false;
            bool wrong_exception = false;
            try {
                [[maybe_unused]] cursor_ptr<const std::int32_t> dummy_ptr{null_source};
            } catch (const std::invalid_argument&) {
                threw_when_null = true;
            } catch (...) {
                wrong_exception = true;
            }

            expect(eq(threw_when_null, true));
            expect(eq(wrong_exception, false));
        };

        "constructing from null smart pointer throws"_test = [] mutable {
            const std::int32_t value{42};
            const trivial_smart_ptr<const std::int32_t> bound_source{std::addressof(value)};
            const trivial_smart_ptr<const std::int32_t> null_source{};

            bool threw_when_bound = false;
            try {
                const cursor_ptr<const std::int32_t> ptr{bound_source};

                expect(eq(*ptr, value));
                expect(eq(ptr.get(), bound_source.get()));
            } catch (...) {
                threw_when_bound = true;
            }

            expect(eq(threw_when_bound, false));

            bool threw_when_null = false;
            bool wrong_exception = false;
            try {
                [[maybe_unused]] cursor_ptr<const std::int32_t> dummy_ptr{null_source};
            } catch (const std::invalid_argument&) {
                threw_when_null = true;
            } catch (...) {
                wrong_exception = true;
            }

            expect(eq(threw_when_null, true));
            expect(eq(wrong_exception, false));
        };

        "assigning from null raw pointer throws"_test = [] mutable {
            const std::int32_t value{42};
            const std::int32_t other{};

            const std::int32_t* const bound_source{std::addressof(value)};
            const std::int32_t* const null_source{nullptr};

            cursor_ptr<const std::int32_t> ptr{other};

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

            expect(eq(threw_when_null, true));
            expect(eq(wrong_exception, false));

            //Invariant preserved after failed assignment
            expect(eq(*ptr, *bound_source));
            expect(eq(ptr.get(), bound_source));
        };

        "assigning from null smart pointer throws"_test = [] mutable {
            const std::int32_t value{42};
            const std::int32_t other{};

            const trivial_smart_ptr<const std::int32_t> bound_source{std::addressof(value)};
            const trivial_smart_ptr<std::int32_t> null_source{};

            cursor_ptr<const std::int32_t> ptr{other};

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

            expect(eq(threw_when_null, true));
            expect(eq(wrong_exception, false));

            //Invariant preserved after failed assignment
            expect(eq(*ptr, value));
            expect(eq(ptr.get(), bound_source.get()));
        };

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
        // Equality semantics
        //============================================================

        "equality compares pointer identity"_test = [] mutable {
            expect(eq(std::equality_comparable<cursor_ptr<std::int32_t>>, true));

            std::int32_t x = 1;
            std::int32_t y = 1;

            cursor_ptr<const std::int32_t> a{x};
            cursor_ptr<std::int32_t> b{x};
            cursor_ptr<std::int32_t> c{y};

            expect(eq(a == b, true));
            expect(eq(a == c, false));
            expect(eq(b == c, false));
        };

        //============================================================
        // Covariance
        //============================================================

        "construct base from derived cursor_ptr"_test = [] mutable {
            derived_type d;
            cursor_ptr<derived_type> dptr{d};

            cursor_ptr<base_type> bptr{dptr};

            expect(eq(bptr.get(), static_cast<base_type*>(std::addressof(d))));
        };

        "construct base from derived reference"_test = [] mutable {
            derived_type d;

            cursor_ptr<base_type> bptr{d};

            expect(eq(bptr.get(), static_cast<base_type*>(std::addressof(d))));
        };

        "assign base from derived cursor_ptr"_test = [] mutable {
            derived_type d;
            cursor_ptr<derived_type> dptr{d};

            base_type b;
            cursor_ptr<base_type> bptr{b};

            bptr = dptr;

            expect(eq(bptr.get(), static_cast<base_type*>(std::addressof(d))));
        };

        "rebind base from derived reference"_test = [] mutable {
            derived_type d;

            base_type b;
            cursor_ptr<base_type> bptr{b};

            bptr = d;

            expect(eq(bptr.get(), static_cast<base_type*>(std::addressof(d))));
        };

        "assign const base from derived cursor_ptr"_test = [] mutable {
            derived_type d;
            cursor_ptr<derived_type> dptr{d};

            base_type b;
            cursor_ptr<const base_type> bptr{b};

            bptr = dptr;

            expect(eq(bptr.get(), static_cast<const base_type*>(std::addressof(d))));
        };

        "rebind const base from derived reference"_test = [] mutable {
            derived_type d;

            base_type b;
            cursor_ptr<const base_type> bptr{b};

            bptr = d;

            expect(eq(bptr.get(), static_cast<const base_type*>(std::addressof(d))));
        };

        "covariant equality comparison"_test = [] mutable {
            expect(eq(std::equality_comparable_with<cursor_ptr<base_type>, cursor_ptr<derived_type>>, true));
            expect(eq(std::equality_comparable_with<cursor_ptr<base_type>, derived_type*>, true));
            expect(eq(std::equality_comparable_with<base_type*, cursor_ptr<derived_type>>, true));
        };

        //============================================================
        // Non-iterator / non-arithmetic guarantees
        //============================================================

        "no pointer arithmetic operations"_test = [] mutable {
            using T = cursor_ptr<std::int32_t>;

            expect(eq(HasAddition<T>, false));
            expect(eq(HasSubtraction<T>, false));
            expect(eq(HasDifference<T>, false));
            expect(eq(HasPreIncrement<T>, false));
            expect(eq(HasPostIncrement<T>, false));
            expect(eq(HasPreDecrement<T>, false));
            expect(eq(HasPostDecrement<T>, false));
        };

        "no ordering comparisons"_test = [] mutable {
            expect(eq(std::three_way_comparable<cursor_ptr<std::int32_t>>, false));
            expect(eq(std::three_way_comparable_with<cursor_ptr<std::int32_t>, std::int32_t*>, false));
            expect(eq(std::three_way_comparable<cursor_ptr<base_type>>, false));
            expect(eq(std::three_way_comparable_with<cursor_ptr<base_type>, cursor_ptr<derived_type>>, false));
            expect(eq(std::three_way_comparable_with<cursor_ptr<base_type>, derived_type*>, false));
            expect(eq(std::three_way_comparable_with<base_type*, cursor_ptr<derived_type>>, false));
        };

        //============================================================
        // CV-correctness propagation
        //============================================================

        "const element forbids mutation through dereference"_test = [] mutable {
            const std::int32_t x{};
            cursor_ptr<const std::int32_t> ptr{x};

            // Compile-time: *ptr must NOT be assignable
            constexpr bool can_assign = std::is_assignable_v<decltype(*ptr), std::int32_t>;

            expect(eq(can_assign, false));
        };

        "non-const element allows mutation"_test = [] mutable {
            std::int32_t x = 5;
            cursor_ptr<std::int32_t> ptr{x};

            *ptr = 10;

            expect(eq(x, 10));
        };

        "`pointer` nested type preserves top-level const"_test = [] mutable {
            using T = cursor_ptr<const std::int32_t>;

            expect(eq(std::same_as<typename T::pointer, const std::int32_t*>, true));
        };

        "`reference` nested type preserves const"_test = [] mutable {
            using T = cursor_ptr<const std::int32_t>;

            expect(eq(std::same_as<typename T::reference, const std::int32_t&>, true));
        };

        "const cursor_ptr prevents rebinding but not mutation"_test = [] mutable {
            std::int32_t x{};

            const cursor_ptr<std::int32_t> ptr{x};

            *ptr = 10; // allowed

            constexpr bool can_rebind = std::is_assignable_v<const cursor_ptr<std::int32_t>&, std::int32_t&>;

            expect(eq(x, 10));
            expect(eq(can_rebind, false));
        };

        "qualification climbing construction and assignment"_test = [] mutable {
            std::int32_t value{42};
            std::int32_t other{};
            cursor_ptr<std::int32_t> mutable_ptr{value};
            cursor_ptr<const std::int32_t> const_ptr{other};

            //Qualification climbing (Assignment)
            const_ptr = mutable_ptr;
            expect(eq(const_ptr.get() == mutable_ptr.get(), true));

            //Qualification climbing (Construction)
            cursor_ptr<const std::int32_t> const_copy{mutable_ptr};
            expect(eq(const_copy.get() == mutable_ptr.get(), true));
        };

        "volatile qualifier preservation"_test = [] mutable {
            volatile std::int32_t hardware_register = 0xAA;
            cursor_ptr<volatile std::int32_t> ptr{hardware_register};

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
            cursor_ptr<std::int32_t> ptr{x};

            expect(eq(takes_ptr(ptr), 3));
        };

        "get() works with raw pointer API"_test = [] mutable {
            auto takes_ptr = [](std::int32_t* p) { return *p; };

            std::int32_t x = 4;
            cursor_ptr<std::int32_t> ptr{x};

            expect(eq(takes_ptr(ptr.get()), 4));
        };

        "not constructible, convertible, nor assignable from C-array decay"_test = [] mutable {
            std::int32_t array[3] = {0, 1, 2};

            //cursor_ptr<std::int32_t> should_fail{array};

            expect(eq(std::convertible_to<decltype(array), cursor_ptr<std::int32_t>>, false));
            expect(eq(std::constructible_from<cursor_ptr<std::int32_t>, decltype(array)>, false));
            expect(eq(std::is_assignable_v<cursor_ptr<std::int32_t>&, decltype(array)>, false));

            expect(eq(std::constructible_from<cursor_ptr<std::int32_t>, decltype(array[0])>, true));
            expect(eq(std::is_assignable_v<cursor_ptr<std::int32_t>&, decltype(array[0])>, true));

            cursor_ptr<std::int32_t> ptr{array[1]};

            //Ensure binding to the element is equivalent to expected array-to-pointer decay with pointer offset arithmetic
            expect(eq(ptr.get(), array + 1));
        };

        //============================================================
        // CTAD Guide
        //============================================================

        "deduction guides work"_test = [] mutable {
            constexpr std::int32_t x = {};
            cursor_ptr ptr1{x};
            cursor_ptr ptr2{std::addressof(x)};

            expect(eq(ptr1.get(), std::addressof(x)));
            expect(eq(ptr2.get(), std::addressof(x)));
        };

        //============================================================
        // Constant Expression Usage
        //============================================================

        "constexpr construction and dereference"_test = [] {
            static constexpr int x = 42;

            constexpr cursor_ptr<const int> ptr{x};

            expect(eq(*ptr, 42));
        };

    };
} //namespace

int main() {}
