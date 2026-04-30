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

    struct Base {
        int value{0};
    };

    struct Derived : Base {
        int extra{42};
    };

    suite base_vocab_ptr_required_ptr_unit = [] mutable {
        //============================================================
        // Template Constraint Validation
        //============================================================

        "template instantiation checks"_test = [] mutable {
            using base::meta::concepts::instantiable_with;
            expect(eq(instantiable_with<required_ptr, std::int32_t>, true));
            expect(eq(instantiable_with<required_ptr, std::int32_t*>, true));
            expect(eq(instantiable_with<required_ptr, std::map<std::string, std::vector<std::int32_t>>>, true));

            expect(eq(instantiable_with<required_ptr, void>, false));
            expect(eq(instantiable_with<required_ptr, std::int32_t&>, false));
            expect(eq(instantiable_with<required_ptr, std::int32_t&&>, false));
            expect(eq(instantiable_with<required_ptr, void(int)>, false));
            expect(eq(instantiable_with<required_ptr, void (&)(int)>, false));
            expect(eq(instantiable_with<required_ptr, void (*)(int, float)>, false));
            expect(eq(instantiable_with<required_ptr, void (**)(std::string, int)>, false));
            expect(eq(instantiable_with<required_ptr, void (*******)(int)>, false));
        };

        //============================================================
        // Triviality & ABI properties
        //============================================================

        "triviality"_test = [] mutable {
            using simple_t = std::int32_t;

            expect(eq(std::is_standard_layout_v<required_ptr<simple_t>>, true));
            expect(eq(std::is_trivially_copyable_v<required_ptr<simple_t>>, true));
            expect(eq(std::is_trivially_destructible_v<required_ptr<simple_t>>, true));
            expect(eq(std::is_nothrow_constructible_v<required_ptr<simple_t>, simple_t&>, true));

            using complex_t = std::map<std::string, std::vector<std::int32_t>>;

            expect(eq(std::is_standard_layout_v<required_ptr<complex_t>>, true));
            expect(eq(std::is_trivially_copyable_v<required_ptr<complex_t>>, true));
            expect(eq(std::is_trivially_destructible_v<required_ptr<complex_t>>, true));
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
            //Explicitly constructible unless removing qualifier
            expect(eq(std::constructible_from<required_ptr<std::int32_t>, std::int32_t&>, true));
            expect(eq(std::constructible_from<required_ptr<std::int32_t>, const std::int32_t&>, false));
            expect(eq(std::constructible_from<required_ptr<std::int32_t>, volatile std::int32_t&>, false));
            expect(eq(std::constructible_from<required_ptr<std::int32_t>, const volatile std::int32_t&>, false));
            expect(eq(std::constructible_from<required_ptr<const std::int32_t>, std::int32_t&>, true));
            expect(eq(std::constructible_from<required_ptr<const std::int32_t>, const std::int32_t&>, true));
            expect(eq(std::constructible_from<required_ptr<const std::int32_t>, volatile std::int32_t&>, false));
            expect(eq(std::constructible_from<required_ptr<const std::int32_t>, const volatile std::int32_t&>, false));
            expect(eq(std::constructible_from<required_ptr<volatile std::int32_t>, std::int32_t&>, true));
            expect(eq(std::constructible_from<required_ptr<volatile std::int32_t>, const std::int32_t&>, false));
            expect(eq(std::constructible_from<required_ptr<volatile std::int32_t>, volatile std::int32_t&>, true));
            expect(eq(std::constructible_from<required_ptr<volatile std::int32_t>, const volatile std::int32_t&>, false));
            expect(eq(std::constructible_from<required_ptr<const volatile std::int32_t>, std::int32_t&>, true));
            expect(eq(std::constructible_from<required_ptr<const volatile std::int32_t>, const std::int32_t&>, true));
            expect(eq(std::constructible_from<required_ptr<const volatile std::int32_t>, volatile std::int32_t&>, true));
            expect(
                eq(std::constructible_from<required_ptr<const volatile std::int32_t>, const volatile std::int32_t&>, true)
            );

            //Never implicitly convertible (constructor is explicit)
            expect(eq(std::convertible_to<std::int32_t&, required_ptr<std::int32_t>>, false));
            expect(eq(std::convertible_to<const std::int32_t&, required_ptr<std::int32_t>>, false));
            expect(eq(std::convertible_to<std::int32_t&, required_ptr<const std::int32_t>>, false));
            expect(eq(std::convertible_to<const std::int32_t&, required_ptr<const std::int32_t>>, false));
        };

        "not default constructible"_test = [] mutable {
            expect(eq(std::default_initializable<required_ptr<std::int32_t>>, false));
        };

        "not constructible from nullptr"_test = [] mutable {
            expect(eq(std::constructible_from<required_ptr<std::int32_t>, std::nullptr_t>, false));
        };

        "not constructible from raw pointer"_test = [] mutable {
            expect(eq(std::constructible_from<required_ptr<std::int32_t>, std::int32_t*>, false));
            expect(eq(std::constructible_from<required_ptr<const std::int32_t>, std::int32_t*>, false));
            expect(eq(std::constructible_from<required_ptr<const std::int32_t>, const std::int32_t*>, false));
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
            expect(eq(std::is_assignable_v<required_ptr<std::int32_t>&, std::int32_t&>, true));
            expect(eq(std::is_assignable_v<required_ptr<std::int32_t>&, const std::int32_t&>, false));
            expect(eq(std::is_assignable_v<required_ptr<std::int32_t>&, volatile std::int32_t&>, false));
            expect(eq(std::is_assignable_v<required_ptr<std::int32_t>&, const volatile std::int32_t&>, false));
            expect(eq(std::is_assignable_v<required_ptr<const std::int32_t>&, std::int32_t&>, true));
            expect(eq(std::is_assignable_v<required_ptr<const std::int32_t>&, const std::int32_t&>, true));
            expect(eq(std::is_assignable_v<required_ptr<const std::int32_t>&, volatile std::int32_t&>, false));
            expect(eq(std::is_assignable_v<required_ptr<const std::int32_t>&, const volatile std::int32_t&>, false));
            expect(eq(std::is_assignable_v<required_ptr<volatile std::int32_t>&, std::int32_t&>, true));
            expect(eq(std::is_assignable_v<required_ptr<volatile std::int32_t>&, const std::int32_t&>, false));
            expect(eq(std::is_assignable_v<required_ptr<volatile std::int32_t>&, volatile std::int32_t&>, true));
            expect(eq(std::is_assignable_v<required_ptr<volatile std::int32_t>&, const volatile std::int32_t&>, false));
            expect(eq(std::is_assignable_v<required_ptr<const volatile std::int32_t>&, std::int32_t&>, true));
            expect(eq(std::is_assignable_v<required_ptr<const volatile std::int32_t>&, const std::int32_t&>, true));
            expect(eq(std::is_assignable_v<required_ptr<const volatile std::int32_t>&, volatile std::int32_t&>, true));
            expect(eq(std::is_assignable_v<required_ptr<const volatile std::int32_t>&, const volatile std::int32_t&>, true));
        };

        "not assignable from nullptr"_test = [] mutable {
            expect(eq(std::is_assignable_v<required_ptr<std::int32_t>&, std::nullptr_t>, false));
        };

        "not assignable from raw pointer"_test = [] mutable {
            expect(eq(std::is_assignable_v<required_ptr<std::int32_t>&, std::int32_t*>, false));
            expect(eq(std::is_assignable_v<required_ptr<const std::int32_t>&, std::int32_t*>, false));
            expect(eq(std::is_assignable_v<required_ptr<const std::int32_t>&, const std::int32_t*>, false));
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
            Base obj{123};
            required_ptr<Base> ptr{obj};

            expect(eq(ptr->value, 123));
        };

        "implicit conversion to raw pointer"_test = [] mutable {
            expect(eq(std::convertible_to<required_ptr<std::int32_t>, std::int32_t*>, true));
            expect(eq(std::convertible_to<required_ptr<std::int32_t>, const std::int32_t*>, true));
            expect(eq(std::convertible_to<required_ptr<std::int32_t>, volatile std::int32_t*>, true));
            expect(eq(std::convertible_to<required_ptr<std::int32_t>, const volatile std::int32_t*>, true));
            expect(eq(std::convertible_to<required_ptr<const std::int32_t>, std::int32_t*>, false));
            expect(eq(std::convertible_to<required_ptr<const std::int32_t>, const std::int32_t*>, true));
            expect(eq(std::convertible_to<required_ptr<const std::int32_t>, volatile std::int32_t*>, false));
            expect(eq(std::convertible_to<required_ptr<const std::int32_t>, const volatile std::int32_t*>, true));
            expect(eq(std::convertible_to<required_ptr<volatile std::int32_t>, std::int32_t*>, false));
            expect(eq(std::convertible_to<required_ptr<volatile std::int32_t>, const std::int32_t*>, false));
            expect(eq(std::convertible_to<required_ptr<volatile std::int32_t>, volatile std::int32_t*>, true));
            expect(eq(std::convertible_to<required_ptr<volatile std::int32_t>, const volatile std::int32_t*>, true));
            expect(eq(std::convertible_to<required_ptr<const volatile std::int32_t>, std::int32_t*>, false));
            expect(eq(std::convertible_to<required_ptr<const volatile std::int32_t>, const std::int32_t*>, false));
            expect(eq(std::convertible_to<required_ptr<const volatile std::int32_t>, volatile std::int32_t*>, false));
            expect(eq(std::convertible_to<required_ptr<const volatile std::int32_t>, const volatile std::int32_t*>, true));
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

        "boolean conversion"_test = [] mutable {
            expect(eq(std::constructible_from<bool, required_ptr<std::int32_t>>, true));

            const int x{};
            required_ptr<const std::int32_t> ptr{x};

            expect(eq(static_cast<bool>(ptr), true));
            expect(eq(!ptr, false));
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
        // Equality semantics
        //============================================================

        "equality compares pointer identity"_test = [] mutable {
            expect(eq(std::equality_comparable<required_ptr<std::int32_t>>, true));

            const int x = 1;
            const int y = 1;

            required_ptr<const std::int32_t> a{x};
            required_ptr<const std::int32_t> b{x};
            required_ptr<const std::int32_t> c{y};

            expect(eq(a == b, true));
            expect(eq(a == c, false));
        };

        //============================================================
        // Covariance
        //============================================================

        "construct base from derived required_ptr"_test = [] mutable {
            Derived d;
            required_ptr<Derived> dptr{d};

            required_ptr<Base> bptr{dptr};

            expect(eq(bptr.get(), static_cast<Base*>(std::addressof(d))));
        };

        "construct base from derived reference"_test = [] mutable {
            Derived d;

            required_ptr<Base> bptr{d};

            expect(eq(bptr.get(), static_cast<Base*>(std::addressof(d))));
        };

        "assign base from derived required_ptr"_test = [] mutable {
            Derived d;
            required_ptr<Derived> dptr{d};

            Base b;
            required_ptr<Base> bptr{b};

            bptr = dptr;

            expect(eq(bptr.get(), static_cast<Base*>(std::addressof(d))));
        };

        "rebind base from derived reference"_test = [] mutable {
            Derived d;

            Base b;
            required_ptr<Base> bptr{b};

            bptr = d;

            expect(eq(bptr.get(), static_cast<Base*>(std::addressof(d))));
        };

        "assign const base from derived required_ptr"_test = [] mutable {
            Derived d;
            required_ptr<Derived> dptr{d};

            Base b;
            required_ptr<const Base> bptr{b};

            bptr = dptr;

            expect(eq(bptr.get(), static_cast<const Base*>(std::addressof(d))));
        };

        "rebind const base from derived reference"_test = [] mutable {
            Derived d;

            Base b;
            required_ptr<const Base> bptr{b};

            bptr = d;

            expect(eq(bptr.get(), static_cast<const Base*>(std::addressof(d))));
        };

        "covariant equality comparison"_test = [] mutable {
            expect(eq(std::equality_comparable_with<required_ptr<Base>, required_ptr<Derived>>, true));
            expect(eq(std::equality_comparable_with<required_ptr<Base>, Derived*>, true));
            expect(eq(std::equality_comparable_with<Base*, required_ptr<Derived>>, true));
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
            expect(eq(std::three_way_comparable<required_ptr<Base>>, false));
            expect(eq(std::three_way_comparable_with<required_ptr<Base>, required_ptr<Derived>>, false));
            expect(eq(std::three_way_comparable_with<required_ptr<Base>, Derived*>, false));
            expect(eq(std::three_way_comparable_with<Base*, required_ptr<Derived>>, false));
        };

        //============================================================
        // Const-correctness propagation
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

        "`pointer` nested type preserves top-level const"_test = [] mutable {
            using T = required_ptr<const std::int32_t>;

            expect(eq(std::same_as<typename T::pointer, const std::int32_t*>, true));
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

        //============================================================
        // CTAD Guide
        //============================================================

        "deduction guide works"_test = [] mutable {
            constexpr std::int32_t x = {};
            required_ptr ptr{x};

            expect(eq(ptr.get(), std::addressof(x)));
        };
    };
} //namespace

int main() {}
