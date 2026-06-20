// SPDX-License-Identifier: Apache-2.0
// Unit tests for base.vocab.ptr:required_ptr

import base.vocab.ptr;
import ut;
import std;

import base.meta.concepts;

using namespace ut;
using base::vocab::required_ptr;

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

    suite base_vocab_ptr_required_ptr_unit = [] mutable {
        //============================================================
        // Template Constraint Validation
        //============================================================

        "template instantiation checks"_test = [] mutable {
            using base::meta::concepts::InstantiableWith;
            expect(eq(InstantiableWith<required_ptr, std::int32_t>, true));
            expect(eq(InstantiableWith<required_ptr, std::int32_t*>, true));
            expect(eq(InstantiableWith<required_ptr, std::map<std::string, std::vector<std::int32_t>>>, true));

            expect(eq(InstantiableWith<required_ptr, void>, true));

            expect(eq(InstantiableWith<required_ptr, std::int32_t&>, false));
            expect(eq(InstantiableWith<required_ptr, std::int32_t&&>, false));
            expect(eq(InstantiableWith<required_ptr, void(int)>, false));
            expect(eq(InstantiableWith<required_ptr, void (&)(int)>, false));
            expect(eq(InstantiableWith<required_ptr, void (*)(int, float)>, false));
            expect(eq(InstantiableWith<required_ptr, void (**)(std::string, int)>, false));
            expect(eq(InstantiableWith<required_ptr, void (*******)(int)>, false));
        };

        //============================================================
        // Triviality & ABI properties
        //============================================================

        "triviality"_test = [] mutable {
            using simple_t = std::int32_t;

            expect(eq(std::is_standard_layout_v<required_ptr<simple_t>>, true));
            expect(eq(std::is_trivially_copyable_v<required_ptr<simple_t>>, true));
            expect(eq(std::is_trivially_destructible_v<required_ptr<simple_t>>, true));
            expect(eq(std::is_trivially_copy_constructible_v<required_ptr<simple_t>>, true));
            expect(eq(std::is_trivially_move_constructible_v<required_ptr<simple_t>>, true));
            expect(eq(std::is_trivially_copy_assignable_v<required_ptr<simple_t>>, true));
            expect(eq(std::is_trivially_move_assignable_v<required_ptr<simple_t>>, true));
            expect(eq(std::is_nothrow_constructible_v<required_ptr<simple_t>, simple_t&>, true));

            using complex_t = std::map<std::string, std::vector<std::int32_t>>;

            expect(eq(std::is_standard_layout_v<required_ptr<complex_t>>, true));
            expect(eq(std::is_trivially_copyable_v<required_ptr<complex_t>>, true));
            expect(eq(std::is_trivially_destructible_v<required_ptr<complex_t>>, true));
            expect(eq(std::is_trivially_copy_constructible_v<required_ptr<complex_t>>, true));
            expect(eq(std::is_trivially_move_constructible_v<required_ptr<complex_t>>, true));
            expect(eq(std::is_trivially_copy_assignable_v<required_ptr<complex_t>>, true));
            expect(eq(std::is_trivially_move_assignable_v<required_ptr<complex_t>>, true));
            expect(eq(std::is_nothrow_constructible_v<required_ptr<complex_t>, complex_t&>, true));
        };

        "size and alignment match raw pointers"_test = [] mutable {
            using simple_t = std::int32_t;

            expect(eq(sizeof(required_ptr<simple_t>) == sizeof(simple_t*), true));
            expect(eq(alignof(required_ptr<simple_t>) == alignof(simple_t*), true));

            using complex_t = std::map<std::string, std::vector<std::int32_t>>;

            expect(eq(sizeof(required_ptr<complex_t>) == sizeof(complex_t*), true));
            expect(eq(alignof(required_ptr<complex_t>) == alignof(complex_t*), true));
        };

        //============================================================
        // Type properties
        //============================================================

        "type aliases are correct"_test = [] mutable {
            using T = required_ptr<const std::int32_t>;

            constexpr bool element = std::same_as<T::element_type, const std::int32_t>;
            constexpr bool value   = std::same_as<T::value_type, std::int32_t>;
            constexpr bool pointer = std::same_as<T::address_type, const std::int32_t*>;
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
            expect(eq(std::constructible_from<required_ptr<test_type>, test_type&>, true));
            expect(eq(std::constructible_from<required_ptr<test_type>, const test_type&>, false));
            expect(eq(std::constructible_from<required_ptr<test_type>, volatile test_type&>, false));
            expect(eq(std::constructible_from<required_ptr<test_type>, const volatile test_type&>, false));
            expect(eq(std::constructible_from<required_ptr<const test_type>, test_type&>, true));
            expect(eq(std::constructible_from<required_ptr<const test_type>, const test_type&>, true));
            expect(eq(std::constructible_from<required_ptr<const test_type>, volatile test_type&>, false));
            expect(eq(std::constructible_from<required_ptr<const test_type>, const volatile test_type&>, false));
            expect(eq(std::constructible_from<required_ptr<volatile test_type>, test_type&>, true));
            expect(eq(std::constructible_from<required_ptr<volatile test_type>, const test_type&>, false));
            expect(eq(std::constructible_from<required_ptr<volatile test_type>, volatile test_type&>, true));
            expect(eq(std::constructible_from<required_ptr<volatile test_type>, const volatile test_type&>, false));
            expect(eq(std::constructible_from<required_ptr<const volatile test_type>, test_type&>, true));
            expect(eq(std::constructible_from<required_ptr<const volatile test_type>, const test_type&>, true));
            expect(eq(std::constructible_from<required_ptr<const volatile test_type>, volatile test_type&>, true));
            expect(eq(std::constructible_from<required_ptr<const volatile test_type>, const volatile test_type&>, true));

            //Never implicitly convertible (constructor is explicit)
            expect(eq(std::convertible_to<test_type&, required_ptr<test_type>>, false));
            expect(eq(std::convertible_to<const test_type&, required_ptr<test_type>>, false));
            expect(eq(std::convertible_to<test_type&, required_ptr<const test_type>>, false));
            expect(eq(std::convertible_to<const test_type&, required_ptr<const test_type>>, false));
        };

        "not default constructible"_test = [] mutable {
            expect(eq(std::default_initializable<required_ptr<std::int32_t>>, false));
        };

        "not constructible from nullptr"_test = [] mutable {
            expect(eq(std::constructible_from<required_ptr<std::int32_t>, std::nullptr_t>, false));
        };

        "constructible from raw pointer"_test = [] mutable {
            using test_type = std::int32_t;

            //Explicitly constructible unless removing qualifier
            expect(eq(std::constructible_from<required_ptr<test_type>, test_type*>, true));
            expect(eq(std::constructible_from<required_ptr<test_type>, const test_type*>, false));
            expect(eq(std::constructible_from<required_ptr<test_type>, volatile test_type*>, false));
            expect(eq(std::constructible_from<required_ptr<test_type>, const volatile test_type*>, false));
            expect(eq(std::constructible_from<required_ptr<const test_type>, test_type*>, true));
            expect(eq(std::constructible_from<required_ptr<const test_type>, const test_type*>, true));
            expect(eq(std::constructible_from<required_ptr<const test_type>, volatile test_type*>, false));
            expect(eq(std::constructible_from<required_ptr<const test_type>, const volatile test_type*>, false));
            expect(eq(std::constructible_from<required_ptr<volatile test_type>, test_type*>, true));
            expect(eq(std::constructible_from<required_ptr<volatile test_type>, const test_type*>, false));
            expect(eq(std::constructible_from<required_ptr<volatile test_type>, volatile test_type*>, true));
            expect(eq(std::constructible_from<required_ptr<volatile test_type>, const volatile test_type*>, false));
            expect(eq(std::constructible_from<required_ptr<const volatile test_type>, test_type*>, true));
            expect(eq(std::constructible_from<required_ptr<const volatile test_type>, const test_type*>, true));
            expect(eq(std::constructible_from<required_ptr<const volatile test_type>, volatile test_type*>, true));
            expect(eq(std::constructible_from<required_ptr<const volatile test_type>, const volatile test_type*>, true));

            //Implicitly convertible unless removing qualifier
            expect(eq(std::convertible_to<test_type*, required_ptr<test_type>>, true));
            expect(eq(std::convertible_to<const test_type*, required_ptr<test_type>>, false));
            expect(eq(std::convertible_to<volatile test_type*, required_ptr<test_type>>, false));
            expect(eq(std::convertible_to<const volatile test_type*, required_ptr<test_type>>, false));
            expect(eq(std::convertible_to<test_type*, required_ptr<const test_type>>, true));
            expect(eq(std::convertible_to<const test_type*, required_ptr<const test_type>>, true));
            expect(eq(std::convertible_to<volatile test_type*, required_ptr<const test_type>>, false));
            expect(eq(std::convertible_to<const volatile test_type*, required_ptr<const test_type>>, false));
            expect(eq(std::convertible_to<test_type*, required_ptr<volatile test_type>>, true));
            expect(eq(std::convertible_to<const test_type*, required_ptr<volatile test_type>>, false));
            expect(eq(std::convertible_to<volatile test_type*, required_ptr<volatile test_type>>, true));
            expect(eq(std::convertible_to<const volatile test_type*, required_ptr<volatile test_type>>, false));
            expect(eq(std::convertible_to<test_type*, required_ptr<const volatile test_type>>, true));
            expect(eq(std::convertible_to<const test_type*, required_ptr<const volatile test_type>>, true));
            expect(eq(std::convertible_to<volatile test_type*, required_ptr<const volatile test_type>>, true));
            expect(eq(std::convertible_to<const volatile test_type*, required_ptr<const volatile test_type>>, true));
        };

        "constructible from smart pointer"_test = [] mutable {
            using test_type = std::int32_t;

            //Explicitly constructible unless removing qualifier
            expect(eq(std::constructible_from<required_ptr<test_type>, trivial_smart_ptr<test_type>&>, true));
            expect(eq(std::constructible_from<required_ptr<test_type>, trivial_smart_ptr<const test_type>&>, false));
            expect(eq(std::constructible_from<required_ptr<test_type>, trivial_smart_ptr<volatile test_type>&>, false));
            expect(eq(std::constructible_from<required_ptr<test_type>, trivial_smart_ptr<const volatile test_type>&>, false));
            expect(eq(std::constructible_from<required_ptr<const test_type>, trivial_smart_ptr<test_type>&>, true));
            expect(eq(std::constructible_from<required_ptr<const test_type>, trivial_smart_ptr<const test_type>&>, true));
            expect(eq(std::constructible_from<required_ptr<const test_type>, trivial_smart_ptr<volatile test_type>&>, false));
            expect(
                eq(std::constructible_from<required_ptr<const test_type>, trivial_smart_ptr<const volatile test_type>&>, false)
            );
            expect(eq(std::constructible_from<required_ptr<volatile test_type>, trivial_smart_ptr<test_type>&>, true));
            expect(eq(std::constructible_from<required_ptr<volatile test_type>, trivial_smart_ptr<const test_type>&>, false));
            expect(eq(std::constructible_from<required_ptr<volatile test_type>, trivial_smart_ptr<volatile test_type>&>, true));
            expect(
                eq(std::constructible_from<required_ptr<volatile test_type>, trivial_smart_ptr<const volatile test_type>&>,
                   false)
            );
            expect(eq(std::constructible_from<required_ptr<const volatile test_type>, trivial_smart_ptr<test_type>&>, true));
            expect(
                eq(std::constructible_from<required_ptr<const volatile test_type>, trivial_smart_ptr<const test_type>&>, true)
            );
            expect(
                eq(std::constructible_from<required_ptr<const volatile test_type>, trivial_smart_ptr<volatile test_type>&>,
                   true)
            );
            expect(eq(
                std::constructible_from<required_ptr<const volatile test_type>, trivial_smart_ptr<const volatile test_type>&>,
                true
            ));

            //Implicitly convertible unless removing qualifier
            expect(eq(std::convertible_to<trivial_smart_ptr<test_type>&, required_ptr<test_type>>, true));
            expect(eq(std::convertible_to<trivial_smart_ptr<const test_type>&, required_ptr<test_type>>, false));
            expect(eq(std::convertible_to<trivial_smart_ptr<volatile test_type>&, required_ptr<test_type>>, false));
            expect(eq(std::convertible_to<trivial_smart_ptr<const volatile test_type>&, required_ptr<test_type>>, false));
            expect(eq(std::convertible_to<trivial_smart_ptr<test_type>&, required_ptr<const test_type>>, true));
            expect(eq(std::convertible_to<trivial_smart_ptr<const test_type>&, required_ptr<const test_type>>, true));
            expect(eq(std::convertible_to<trivial_smart_ptr<volatile test_type>&, required_ptr<const test_type>>, false));
            expect(eq(std::convertible_to<trivial_smart_ptr<const volatile test_type>&, required_ptr<const test_type>>, false));
            expect(eq(std::convertible_to<trivial_smart_ptr<test_type>&, required_ptr<volatile test_type>>, true));
            expect(eq(std::convertible_to<trivial_smart_ptr<const test_type>&, required_ptr<volatile test_type>>, false));
            expect(eq(std::convertible_to<trivial_smart_ptr<volatile test_type>&, required_ptr<volatile test_type>>, true));
            expect(
                eq(std::convertible_to<trivial_smart_ptr<const volatile test_type>&, required_ptr<volatile test_type>>, false)
            );
            expect(eq(std::convertible_to<trivial_smart_ptr<test_type>&, required_ptr<const volatile test_type>>, true));
            expect(eq(std::convertible_to<trivial_smart_ptr<const test_type>&, required_ptr<const volatile test_type>>, true));
            expect(
                eq(std::convertible_to<trivial_smart_ptr<volatile test_type>&, required_ptr<const volatile test_type>>, true)
            );
            expect(
                eq(std::convertible_to<trivial_smart_ptr<const volatile test_type>&, required_ptr<const volatile test_type>>,
                   true)
            );
        };

        "not constructible from rvalue"_test = [] mutable {
            expect(eq(std::constructible_from<required_ptr<std::int32_t>, std::int32_t>, false));
            expect(eq(std::constructible_from<required_ptr<const std::int32_t>, std::int32_t>, false));
            expect(eq(std::constructible_from<required_ptr<const std::int32_t>, const std::int32_t>, false));

            expect(eq(std::constructible_from<required_ptr<std::int32_t>, std::int32_t&&>, false));
            expect(eq(std::constructible_from<required_ptr<const std::int32_t>, std::int32_t&&>, false));
            expect(eq(std::constructible_from<required_ptr<const std::int32_t>, const std::int32_t&&>, false));
        };

        //============================================================
        // Assignment
        //============================================================

        "assignable from lvalue reference"_test = [] mutable {
            using test_type = std::int32_t;

            expect(eq(std::is_assignable_v<required_ptr<test_type>&, test_type&>, true));
            expect(eq(std::is_assignable_v<required_ptr<test_type>&, const test_type&>, false));
            expect(eq(std::is_assignable_v<required_ptr<test_type>&, volatile test_type&>, false));
            expect(eq(std::is_assignable_v<required_ptr<test_type>&, const volatile test_type&>, false));
            expect(eq(std::is_assignable_v<required_ptr<const test_type>&, test_type&>, true));
            expect(eq(std::is_assignable_v<required_ptr<const test_type>&, const test_type&>, true));
            expect(eq(std::is_assignable_v<required_ptr<const test_type>&, volatile test_type&>, false));
            expect(eq(std::is_assignable_v<required_ptr<const test_type>&, const volatile test_type&>, false));
            expect(eq(std::is_assignable_v<required_ptr<volatile test_type>&, test_type&>, true));
            expect(eq(std::is_assignable_v<required_ptr<volatile test_type>&, const test_type&>, false));
            expect(eq(std::is_assignable_v<required_ptr<volatile test_type>&, volatile test_type&>, true));
            expect(eq(std::is_assignable_v<required_ptr<volatile test_type>&, const volatile test_type&>, false));
            expect(eq(std::is_assignable_v<required_ptr<const volatile test_type>&, test_type&>, true));
            expect(eq(std::is_assignable_v<required_ptr<const volatile test_type>&, const test_type&>, true));
            expect(eq(std::is_assignable_v<required_ptr<const volatile test_type>&, volatile test_type&>, true));
            expect(eq(std::is_assignable_v<required_ptr<const volatile test_type>&, const volatile test_type&>, true));
        };

        "not assignable from nullptr"_test = [] mutable {
            expect(eq(std::is_assignable_v<required_ptr<std::int32_t>&, std::nullptr_t>, false));
        };

        "assignable from raw pointer"_test = [] mutable {
            using test_type = std::int32_t;

            expect(eq(std::is_assignable_v<required_ptr<test_type>&, test_type*>, true));
            expect(eq(std::is_assignable_v<required_ptr<test_type>&, const test_type*>, false));
            expect(eq(std::is_assignable_v<required_ptr<test_type>&, volatile test_type*>, false));
            expect(eq(std::is_assignable_v<required_ptr<test_type>&, const volatile test_type*>, false));
            expect(eq(std::is_assignable_v<required_ptr<const test_type>&, test_type*>, true));
            expect(eq(std::is_assignable_v<required_ptr<const test_type>&, const test_type*>, true));
            expect(eq(std::is_assignable_v<required_ptr<const test_type>&, volatile test_type*>, false));
            expect(eq(std::is_assignable_v<required_ptr<const test_type>&, const volatile test_type*>, false));
            expect(eq(std::is_assignable_v<required_ptr<volatile test_type>&, test_type*>, true));
            expect(eq(std::is_assignable_v<required_ptr<volatile test_type>&, const test_type*>, false));
            expect(eq(std::is_assignable_v<required_ptr<volatile test_type>&, volatile test_type*>, true));
            expect(eq(std::is_assignable_v<required_ptr<volatile test_type>&, const volatile test_type*>, false));
            expect(eq(std::is_assignable_v<required_ptr<const volatile test_type>&, test_type*>, true));
            expect(eq(std::is_assignable_v<required_ptr<const volatile test_type>&, const test_type*>, true));
            expect(eq(std::is_assignable_v<required_ptr<const volatile test_type>&, volatile test_type*>, true));
            expect(eq(std::is_assignable_v<required_ptr<const volatile test_type>&, const volatile test_type*>, true));
        };

        "assignable from smart pointer"_test = [] mutable {
            using test_type = std::int32_t;

            expect(eq(std::is_assignable_v<required_ptr<test_type>&, trivial_smart_ptr<test_type>&>, true));
            expect(eq(std::is_assignable_v<required_ptr<test_type>&, trivial_smart_ptr<const test_type>&>, false));
            expect(eq(std::is_assignable_v<required_ptr<test_type>&, trivial_smart_ptr<volatile test_type>&>, false));
            expect(eq(std::is_assignable_v<required_ptr<test_type>&, trivial_smart_ptr<const volatile test_type>&>, false));
            expect(eq(std::is_assignable_v<required_ptr<const test_type>&, trivial_smart_ptr<test_type>&>, true));
            expect(eq(std::is_assignable_v<required_ptr<const test_type>&, trivial_smart_ptr<const test_type>&>, true));
            expect(eq(std::is_assignable_v<required_ptr<const test_type>&, trivial_smart_ptr<volatile test_type>&>, false));
            expect(
                eq(std::is_assignable_v<required_ptr<const test_type>&, trivial_smart_ptr<const volatile test_type>&>, false)
            );
            expect(eq(std::is_assignable_v<required_ptr<volatile test_type>&, trivial_smart_ptr<test_type>&>, true));
            expect(eq(std::is_assignable_v<required_ptr<volatile test_type>&, trivial_smart_ptr<const test_type>&>, false));
            expect(eq(std::is_assignable_v<required_ptr<volatile test_type>&, trivial_smart_ptr<volatile test_type>&>, true));
            expect(
                eq(std::is_assignable_v<required_ptr<volatile test_type>&, trivial_smart_ptr<const volatile test_type>&>, false)
            );
            expect(eq(std::is_assignable_v<required_ptr<const volatile test_type>&, trivial_smart_ptr<test_type>&>, true));
            expect(
                eq(std::is_assignable_v<required_ptr<const volatile test_type>&, trivial_smart_ptr<const test_type>&>, true)
            );
            expect(
                eq(std::is_assignable_v<required_ptr<const volatile test_type>&, trivial_smart_ptr<volatile test_type>&>, true)
            );
            expect(
                eq(std::is_assignable_v<required_ptr<const volatile test_type>&, trivial_smart_ptr<const volatile test_type>&>,
                   true)
            );
        };

        "not assignable from rvalue"_test = [] mutable {
            expect(eq(std::is_assignable_v<required_ptr<std::int32_t>&, std::int32_t&&>, false));
            expect(eq(std::is_assignable_v<required_ptr<const std::int32_t>&, std::int32_t&&>, false));
            expect(eq(std::is_assignable_v<required_ptr<const std::int32_t>&, const std::int32_t&&>, false));
        };

        //============================================================
        // Pointer semantics
        //============================================================

        "operator* dereferences correctly"_test = [] mutable {
            int x = 55;
            required_ptr<std::int32_t> ptr{x};

            expect(eq(*ptr, 55));
        };

        "operator-> provides member access"_test = [] mutable {
            base_type obj{123};
            required_ptr<base_type> ptr{obj};

            expect(eq(ptr->value, 123));
        };

        "implicit conversion to raw pointer"_test = [] mutable {
            using test_type = std::int32_t;

            expect(eq(std::convertible_to<required_ptr<test_type>, test_type*>, true));
            expect(eq(std::convertible_to<required_ptr<test_type>, const test_type*>, true));
            expect(eq(std::convertible_to<required_ptr<test_type>, volatile test_type*>, true));
            expect(eq(std::convertible_to<required_ptr<test_type>, const volatile test_type*>, true));
            expect(eq(std::convertible_to<required_ptr<const test_type>, test_type*>, false));
            expect(eq(std::convertible_to<required_ptr<const test_type>, const test_type*>, true));
            expect(eq(std::convertible_to<required_ptr<const test_type>, volatile test_type*>, false));
            expect(eq(std::convertible_to<required_ptr<const test_type>, const volatile test_type*>, true));
            expect(eq(std::convertible_to<required_ptr<volatile test_type>, test_type*>, false));
            expect(eq(std::convertible_to<required_ptr<volatile test_type>, const test_type*>, false));
            expect(eq(std::convertible_to<required_ptr<volatile test_type>, volatile test_type*>, true));
            expect(eq(std::convertible_to<required_ptr<volatile test_type>, const volatile test_type*>, true));
            expect(eq(std::convertible_to<required_ptr<const volatile test_type>, test_type*>, false));
            expect(eq(std::convertible_to<required_ptr<const volatile test_type>, const test_type*>, false));
            expect(eq(std::convertible_to<required_ptr<const volatile test_type>, volatile test_type*>, false));
            expect(eq(std::convertible_to<required_ptr<const volatile test_type>, const volatile test_type*>, true));
        };

        "conversion to raw pointer preserves address"_test = [] mutable {
            const int x{};
            required_ptr<const std::int32_t> ptr{x};

            const std::int32_t* raw = ptr;

            expect(eq(raw, std::addressof(x)));
        };

        "get returns raw pointer"_test = [] mutable {
            const int x{};
            required_ptr<const std::int32_t> ptr{x};

            expect(eq(ptr.get(), std::addressof(x)));
        };

        "contextual boolean conversion is supported"_test = [] mutable {
            expect(eq(std::constructible_from<bool, required_ptr<std::int32_t>>, true));

            const int x{};
            required_ptr<const std::int32_t> ptr{x};

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

            required_ptr<const std::int32_t> ptr{a};
            ptr = b;

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
                const required_ptr<const std::int32_t> ptr{bound_source};

                expect(eq(*ptr, value));
                expect(eq(ptr.get(), bound_source));
            } catch (...) {
                threw_when_bound = true;
            }

            expect(eq(threw_when_bound, false));

            bool threw_when_null = false;
            bool wrong_exception = false;
            try {
                [[maybe_unused]] required_ptr<const std::int32_t> dummy_ptr{null_source};
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
                const required_ptr<const std::int32_t> ptr{bound_source};

                expect(eq(*ptr, value));
                expect(eq(ptr.get(), bound_source.get()));
            } catch (...) {
                threw_when_bound = true;
            }

            expect(eq(threw_when_bound, false));

            bool threw_when_null = false;
            bool wrong_exception = false;
            try {
                [[maybe_unused]] required_ptr<const std::int32_t> dummy_ptr{null_source};
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

            required_ptr<const std::int32_t> ptr{other};

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

            required_ptr<const std::int32_t> ptr{other};

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
        // Equality semantics
        //============================================================

        "equality compares pointer identity"_test = [] mutable {
            expect(eq(std::equality_comparable<required_ptr<std::int32_t>>, true));

            std::int32_t x = 1;
            std::int32_t y = 1;

            required_ptr<const std::int32_t> a{x};
            required_ptr<std::int32_t> b{x};
            required_ptr<std::int32_t> c{y};

            expect(eq(a == b, true));
            expect(eq(a == c, false));
            expect(eq(b == c, false));
        };

        //============================================================
        // Covariance
        //============================================================

        "construct base from derived required_ptr"_test = [] mutable {
            derived_type d;
            required_ptr<derived_type> dptr{d};

            required_ptr<base_type> bptr{dptr};

            expect(eq(bptr.get(), static_cast<base_type*>(std::addressof(d))));
        };

        "construct base from derived reference"_test = [] mutable {
            derived_type d;

            required_ptr<base_type> bptr{d};

            expect(eq(bptr.get(), static_cast<base_type*>(std::addressof(d))));
        };

        "assign base from derived required_ptr"_test = [] mutable {
            derived_type d;
            required_ptr<derived_type> dptr{d};

            base_type b;
            required_ptr<base_type> bptr{b};

            bptr = dptr;

            expect(eq(bptr.get(), static_cast<base_type*>(std::addressof(d))));
        };

        "rebind base from derived reference"_test = [] mutable {
            derived_type d;

            base_type b;
            required_ptr<base_type> bptr{b};

            bptr = d;

            expect(eq(bptr.get(), static_cast<base_type*>(std::addressof(d))));
        };

        "assign const base from derived required_ptr"_test = [] mutable {
            derived_type d;
            required_ptr<derived_type> dptr{d};

            base_type b;
            required_ptr<const base_type> bptr{b};

            bptr = dptr;

            expect(eq(bptr.get(), static_cast<const base_type*>(std::addressof(d))));
        };

        "rebind const base from derived reference"_test = [] mutable {
            derived_type d;

            base_type b;
            required_ptr<const base_type> bptr{b};

            bptr = d;

            expect(eq(bptr.get(), static_cast<const base_type*>(std::addressof(d))));
        };

        "covariant equality comparison"_test = [] mutable {
            expect(eq(std::equality_comparable_with<required_ptr<base_type>, required_ptr<derived_type>>, true));
            expect(eq(std::equality_comparable_with<required_ptr<base_type>, derived_type*>, true));
            expect(eq(std::equality_comparable_with<base_type*, required_ptr<derived_type>>, true));
        };

        //============================================================
        // Non-iterator / non-arithmetic guarantees
        //============================================================

        "no pointer arithmetic operations"_test = [] mutable {
            using T = required_ptr<std::int32_t>;

            expect(eq(HasAddition<T>, false));
            expect(eq(HasSubtraction<T>, false));
            expect(eq(HasDifference<T>, false));
            expect(eq(HasPreIncrement<T>, false));
            expect(eq(HasPostIncrement<T>, false));
            expect(eq(HasPreDecrement<T>, false));
            expect(eq(HasPostDecrement<T>, false));
        };

        "no ordering comparisons"_test = [] mutable {
            expect(eq(std::three_way_comparable<required_ptr<std::int32_t>>, false));
            expect(eq(std::three_way_comparable_with<required_ptr<std::int32_t>, std::int32_t*>, false));
            expect(eq(std::three_way_comparable<required_ptr<base_type>>, false));
            expect(eq(std::three_way_comparable_with<required_ptr<base_type>, required_ptr<derived_type>>, false));
            expect(eq(std::three_way_comparable_with<required_ptr<base_type>, derived_type*>, false));
            expect(eq(std::three_way_comparable_with<base_type*, required_ptr<derived_type>>, false));
        };

        //============================================================
        // CV-correctness propagation
        //============================================================

        "const element forbids mutation through dereference"_test = [] mutable {
            const std::int32_t x{};
            required_ptr<const std::int32_t> ptr{x};

            // Compile-time: *ptr must NOT be assignable
            constexpr bool can_assign = std::is_assignable_v<decltype(*ptr), std::int32_t>;

            expect(eq(can_assign, false));
        };

        "non-const element allows mutation"_test = [] mutable {
            std::int32_t x = 5;
            required_ptr<std::int32_t> ptr{x};

            *ptr = 10;

            expect(eq(x, 10));
        };

        "`address_type` nested type preserves top-level const"_test = [] mutable {
            using T = required_ptr<const std::int32_t>;

            expect(eq(std::same_as<typename T::address_type, const std::int32_t*>, true));
        };

        "`reference` nested type preserves const"_test = [] mutable {
            using T = required_ptr<const std::int32_t>;

            expect(eq(std::same_as<typename T::reference, const std::int32_t&>, true));
        };

        "const required_ptr prevents rebinding but not mutation"_test = [] mutable {
            std::int32_t x{};

            const required_ptr<std::int32_t> ptr{x};

            *ptr = 10; // allowed

            constexpr bool can_rebind = std::is_assignable_v<const required_ptr<std::int32_t>&, std::int32_t&>;

            expect(eq(x, 10));
            expect(eq(can_rebind, false));
        };

        "qualification climbing construction and assignment"_test = [] mutable {
            std::int32_t value{42};
            std::int32_t other{};
            required_ptr<std::int32_t> mutable_ptr{value};
            required_ptr<const std::int32_t> const_ptr{other};

            //Qualification climbing (Assignment)
            const_ptr = mutable_ptr;
            expect(eq(const_ptr.get() == mutable_ptr.get(), true));

            //Qualification climbing (Construction)
            required_ptr<const std::int32_t> const_copy{mutable_ptr};
            expect(eq(const_copy.get() == mutable_ptr.get(), true));
        };

        "volatile qualifier preservation"_test = [] mutable {
            volatile std::int32_t hardware_register = 0xAA;
            required_ptr<volatile std::int32_t> ptr{hardware_register};

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
            required_ptr<std::int32_t> ptr{x};

            expect(eq(takes_ptr(ptr), 3));
        };

        "get() works with raw pointer API"_test = [] mutable {
            auto takes_ptr = [](std::int32_t* p) { return *p; };

            std::int32_t x = 4;
            required_ptr<std::int32_t> ptr{x};

            expect(eq(takes_ptr(ptr.get()), 4));
        };

        "not constructible, convertible, nor assignable from C-array decay"_test = [] mutable {
            std::int32_t array[3] = {0, 1, 2};

            //required_ptr<std::int32_t> should_fail{array};

            expect(eq(std::convertible_to<decltype(array), required_ptr<std::int32_t>>, false));
            expect(eq(std::constructible_from<required_ptr<std::int32_t>, decltype(array)>, false));
            expect(eq(std::is_assignable_v<required_ptr<std::int32_t>&, decltype(array)>, false));

            expect(eq(std::constructible_from<required_ptr<std::int32_t>, decltype(array[0])>, true));
            expect(eq(std::is_assignable_v<required_ptr<std::int32_t>&, decltype(array[0])>, true));

            required_ptr<std::int32_t> ptr{array[1]};

            //Ensure binding to the element is equivalent to expected array-to-pointer decay with pointer offset arithmetic
            expect(eq(ptr.get(), array + 1));
        };

        //============================================================
        // `void` support
        //============================================================

        "type aliases are correct for `void`"_test = [] mutable {
            using T = required_ptr<void>;

            constexpr bool element = std::same_as<T::element_type, void>;
            constexpr bool value   = std::same_as<T::value_type, void>;
            constexpr bool pointer = std::same_as<T::address_type, void*>;
            constexpr bool ptrdiff = std::same_as<T::difference_type, std::ptrdiff_t>;

            expect(eq(element, true));
            expect(eq(value, true));
            expect(eq(pointer, true));
            expect(eq(ptrdiff, true));
        };

        "void specialization supports type erasure"_test = [] mutable {
            std::int32_t x{};

            required_ptr<std::int32_t> typed{x};
            required_ptr<void> erased{typed};

            expect(eq(erased.get(), static_cast<void*>(std::addressof(x))));
        };

        "void specialization disables dereference operators"_test = [] mutable {
            expect(eq(Dereferenceable<required_ptr<std::int32_t>>, true));
            expect(eq(ArrowAccessible<required_ptr<std::int32_t>>, true));

            expect(eq(Dereferenceable<required_ptr<void>>, false));
            expect(eq(ArrowAccessible<required_ptr<void>>, false));
        };

        "void raw pointer construction is explicit"_test = [] mutable {
            expect(eq(std::convertible_to<void*, required_ptr<void>>, false));
            expect(eq(std::constructible_from<required_ptr<void>, void*>, true));
        };

        "void smart pointer construction is explicit"_test = [] mutable {
            expect(eq(std::convertible_to<trivial_smart_ptr<void>&, required_ptr<void>>, false));
            expect(eq(std::constructible_from<required_ptr<void>, trivial_smart_ptr<void>&>, true));
        };

        "void `required_ptr` constructs implicitly from typed `required_ptr`"_test = [] mutable {
            expect(eq(std::convertible_to<required_ptr<std::int32_t>, required_ptr<void>>, true));
            expect(eq(std::convertible_to<required_ptr<void>, required_ptr<std::int32_t>>, false));

            const std::int32_t value{42};
            required_ptr<const std::int32_t> typed_ptr{value};

            // Should be implicit (convertible)
            auto takes_void = [](required_ptr<const void> ptr) { return ptr.get(); };
            expect(eq(takes_void(typed_ptr), static_cast<const void*>(std::addressof(value))));
        };

        "void `required_ptr` is equality comparable"_test = [] mutable {
            const std::int32_t value{42};
            required_ptr<const std::int32_t> typed_ptr{value};
            const std::int32_t* typed_raw = std::addressof(value);
            const void* erased_raw        = std::addressof(value);

            required_ptr<const void> erased_ptr1{typed_raw};
            required_ptr<const void> erased_ptr2{typed_ptr};

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
            required_ptr ptr1{x};
            required_ptr ptr2{std::addressof(x)};

            expect(eq(ptr1.get(), std::addressof(x)));
            expect(eq(ptr2.get(), std::addressof(x)));
        };

        //============================================================
        // Constant Expression Usage
        //============================================================

        "constexpr construction and dereference"_test = [] {
            static constexpr int x = 42;

            constexpr required_ptr<const int> ptr{x};

            expect(eq(*ptr, 42));
        };
    };
} //namespace

int main() {}
