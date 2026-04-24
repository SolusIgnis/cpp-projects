// SPDX-License-Identifier: Apache-2.0
// Unit tests for base.vocab.ptr:dependency_ptr

import base.vocab.ptr;
import ut;
import std;

using namespace ut;
using base::vocab::dependency_ptr;

namespace {
    struct Base {
        int value{0};
    };
    
    struct Derived : Base {
        int extra{42};
    };
    
    suite dependency_ptr_tests = [] mutable {
    
        //============================================================
        // Construction
        //============================================================
    
        "constructible explicitly from lvalue reference"_test = [] mutable {
            //Explicitly constructible unless removing qualifier
            expect(eq(std::constructible_from<dependency_ptr<int>, int&>, true));
            expect(eq(std::constructible_from<dependency_ptr<int>, const int&>, false));
            expect(eq(std::constructible_from<dependency_ptr<int>, volatile int&>, false));
            expect(eq(std::constructible_from<dependency_ptr<int>, const volatile int&>, false));
            expect(eq(std::constructible_from<dependency_ptr<const int>, int&>, true));
            expect(eq(std::constructible_from<dependency_ptr<const int>, const int&>, true));
            expect(eq(std::constructible_from<dependency_ptr<const int>, volatile int&>, false));
            expect(eq(std::constructible_from<dependency_ptr<const int>, const volatile int&>, false));
            expect(eq(std::constructible_from<dependency_ptr<volatile int>, int&>, true));
            expect(eq(std::constructible_from<dependency_ptr<volatile int>, const int&>, false));
            expect(eq(std::constructible_from<dependency_ptr<volatile int>, volatile int&>, true));
            expect(eq(std::constructible_from<dependency_ptr<volatile int>, const volatile int&>, false));
            expect(eq(std::constructible_from<dependency_ptr<const volatile int>, int&>, true)); 
            expect(eq(std::constructible_from<dependency_ptr<const volatile int>, const int&>, true));
            expect(eq(std::constructible_from<dependency_ptr<const volatile int>, volatile int&>, true)); 
            expect(eq(std::constructible_from<dependency_ptr<const volatile int>, const volatile int&>, true)); 
            
            //Never implicitly convertible (constructor is explicit)
            expect(eq(std::convertible_to<int&, dependency_ptr<int>>, false));
            expect(eq(std::convertible_to<const int&, dependency_ptr<int>>, false));
            expect(eq(std::convertible_to<int&, dependency_ptr<const int>>, false));
            expect(eq(std::convertible_to<const int&, dependency_ptr<const int>>, false));
        };

        "not default constructible"_test = [] mutable {
            expect(eq(std::default_initializable<dependency_ptr<int>>, false));
        };
    
        "not constructible from nullptr"_test = [] mutable {
            expect(eq(std::constructible_from<dependency_ptr<int>, std::nullptr_t>, false));
        };
    
        "not constructible from raw pointer"_test = [] mutable {
            expect(eq(std::constructible_from<dependency_ptr<int>, int*>, false));
            expect(eq(std::constructible_from<dependency_ptr<const int>, int*>, false));
            expect(eq(std::constructible_from<dependency_ptr<const int>, const int*>, false));
        };
    
        "not constructible from rvalue"_test = [] mutable {
            expect(eq(std::constructible_from<dependency_ptr<int>, int&&>, false));
            expect(eq(std::constructible_from<dependency_ptr<const int>, int&&>, false));
            expect(eq(std::constructible_from<dependency_ptr<const int>, const int&&>, false));
        };

        //============================================================
        // Assignment
        //============================================================

        "assignable from lvalue reference"_test = [] mutable {
            expect(eq(std::assignable_from<dependency_ptr<int>&, int&>, true));
            expect(eq(std::assignable_from<dependency_ptr<int>&, const int&>, false));
            expect(eq(std::assignable_from<dependency_ptr<int>&, volatile int&>, false));
            expect(eq(std::assignable_from<dependency_ptr<int>&, const volatile int&>, false));
            expect(eq(std::assignable_from<dependency_ptr<const int>&, int&>, true));
            expect(eq(std::assignable_from<dependency_ptr<const int>&, const int&>, true));
            expect(eq(std::assignable_from<dependency_ptr<const int>&, volatile int&>, false));
            expect(eq(std::assignable_from<dependency_ptr<const int>&, const volatile int&>, false));
            expect(eq(std::assignable_from<dependency_ptr<volatile int>&, int&>, true));
            expect(eq(std::assignable_from<dependency_ptr<volatile int>&, const int&>, false));
            expect(eq(std::assignable_from<dependency_ptr<volatile int>&, volatile int&>, true));
            expect(eq(std::assignable_from<dependency_ptr<volatile int>&, const volatile int&>, false));
            expect(eq(std::assignable_from<dependency_ptr<const volatile int>&, int&>, true)); 
            expect(eq(std::assignable_from<dependency_ptr<const volatile int>&, const int&>, true));
            expect(eq(std::assignable_from<dependency_ptr<const volatile int>&, volatile int&>, true)); 
            expect(eq(std::assignable_from<dependency_ptr<const volatile int>&, const volatile int&>, true)); 
        };

        "not assignable from nullptr"_test = [] mutable {
            expect(eq(std::assignable_from<dependency_ptr<int>&, std::nullptr_t>, false));
        };
    
        "not assignable from raw pointer"_test = [] mutable {
            expect(eq(std::assignable_from<dependency_ptr<int>&, int*>, false));
            expect(eq(std::assignable_from<dependency_ptr<const int>&, int*>, false));
            expect(eq(std::assignable_from<dependency_ptr<const int>&, const int*>, false));
        };
    
        "not assignable from rvalue"_test = [] mutable {
            expect(eq(std::assignable_from<dependency_ptr<int>&, int&&>, false));
            expect(eq(std::assignable_from<dependency_ptr<const int>&, int&&>, false));
            expect(eq(std::assignable_from<dependency_ptr<const int>&, const int&&>, false));
        };

        //============================================================
        // Pointer semantics
        //============================================================

        "operator* dereferences correctly"_test = [] mutable {
            int x = 55;
            dependency_ptr<int> ptr{x};
    
            expect(eq(*ptr, 55));
        };

        "operator-> provides member access"_test = [] mutable {
            Base obj{123};
            dependency_ptr<Base> ptr{obj};
    
            expect(eq(ptr->value, 123));
        };
    
        "implicit conversion to raw pointer"_test = [] mutable {
            expect(eq(std::convertible_to<dependency_ptr<int>, int*>, true));
            expect(eq(std::convertible_to<dependency_ptr<int>, const int*>, true));
            expect(eq(std::convertible_to<dependency_ptr<int>, volatile int*>, true));
            expect(eq(std::convertible_to<dependency_ptr<int>, const volatile int*>, true));
            expect(eq(std::convertible_to<dependency_ptr<const int>, int*>, false));
            expect(eq(std::convertible_to<dependency_ptr<const int>, const int*>, true));
            expect(eq(std::convertible_to<dependency_ptr<const int>, volatile int*>, false));
            expect(eq(std::convertible_to<dependency_ptr<const int>, const volatile int*>, true));
            expect(eq(std::convertible_to<dependency_ptr<volatile int>, int*>, false));
            expect(eq(std::convertible_to<dependency_ptr<volatile int>, const int*>, false));
            expect(eq(std::convertible_to<dependency_ptr<volatile int>, volatile int*>, true));
            expect(eq(std::convertible_to<dependency_ptr<volatile int>, const volatile int*>, true));
            expect(eq(std::convertible_to<dependency_ptr<const volatile int>, int*>, false)); 
            expect(eq(std::convertible_to<dependency_ptr<const volatile int>, const int*>, false));
            expect(eq(std::convertible_to<dependency_ptr<const volatile int>, volatile int*>, false)); 
            expect(eq(std::convertible_to<dependency_ptr<const volatile int>, const volatile int*>, true)); 
        };

        "conversion to raw pointer preserves address"_test = [] mutable {
            const int x{};
            dependency_ptr<const int> ptr{x};
    
            const int* raw = ptr;
    
            expect(eq(raw, &x));
        };
    
        "get returns raw pointer"_test = [] mutable {
            const int x{};
            dependency_ptr<const int> ptr{x};
    
            expect(eq(ptr.get(), &x));
        };
    
        "boolean conversion"_test = [] mutable {
            expect(eq(std::convertible_to<dependency_ptr<int>, bool>));
            
            const int x{};
            dependency_ptr<const int> ptr{x};
            
            expect(eq(static_cast<bool>(ptr), true));
            expect(eq(!ptr, false));
        };

        //============================================================
        // Rebinding
        //============================================================
    
        "rebind via assignment from reference"_test = [] mutable {
            const int a{};
            const int b = 2;
    
            dependency_ptr<const int> ptr{a};
            ptr = b;
    
            expect(eq(*ptr, 2));
            expect(eq(ptr.get(), &b));
        };

        //============================================================
        // Equality semantics
        //============================================================
    
        "equality compares pointer identity"_test = [] mutable {
            const int x = 1;
            const int y = 1;
    
            dependency_ptr<const int> a{x};
            dependency_ptr<const int> b{x};
            dependency_ptr<const int> c{y};
    
            expect(eq(a == b, true));
            expect(eq(a == c, false));
        };
    
        //============================================================
        // Covariance
        //============================================================
    
        "construct base from derived dependency_ptr"_test = [] mutable {
            Derived d;
            dependency_ptr<Derived> dptr{d};
    
            dependency_ptr<Base> bptr{dptr};
    
            expect(eq(bptr.get(), static_cast<Base*>(&d)));
        };
    
        "assign base from derived dependency_ptr"_test = [] mutable {
            Derived d;
            dependency_ptr<Derived> dptr{d};
    
            Base b;
            dependency_ptr<Base> bptr{b};
    
            bptr = dptr;
    
            expect(eq(bptr.get(), static_cast<Base*>(&d)));
        };
    
        //============================================================
        // Non-iterator guarantees
        //============================================================
    
        "no pointer arithmetic operations"_test = [] mutable {
            using T = dependency_ptr<int>;
    
            constexpr bool add = requires(T t) { t + 1; } || requires(T t) { 1 + t; };
            constexpr bool sub = requires(T t) { t - 1; };
            constexpr bool dif = requires(T t) { t - t; };
            constexpr bool preinc = requires(T t) { ++t; };
            constexpr bool pstinc = requires(T t) { t++; };
            constexpr bool predec = requires(T t) { --t; };
            constexpr bool pstdec = requires(T t) { t--; };
    
            expect(eq(add, false));
            expect(eq(sub, false));
            expect(eq(dif, false));
            expect(eq(preinc, false));
            expect(eq(pstinc, false));
            expect(eq(predec, false));
            expect(eq(pstdec, false));
        };
    
        //============================================================
        // Type properties
        //============================================================
    
        "type aliases are correct"_test = [] mutable {
            using T = dependency_ptr<const int>;
    
            constexpr bool element = std::same_as<T::element_type, const int>;
            constexpr bool value   = std::same_as<T::value_type, int>;
            constexpr bool pointer = std::same_as<T::pointer, const int*>;
            constexpr bool ref     = std::same_as<T::reference, const int&>;
            constexpr bool rref    = std::same_as<T::rvalue_reference, const int&&>;
            constexpr bool ptrdiff = std::same_as<T::difference_type, std::ptrdiff_t>;
    
            expect(eq(element, true));
            expect(eq(value, true));
            expect(eq(pointer, true));
            expect(eq(ref, true));
            expect(eq(rref, true));
            expect(eq(ptrdiff, true));
        };

        //============================================================
        // Triviality & ABI properties
        //============================================================
        
        "triviality"_test = [] mutable {
            using simple_t = std::int32_t;

            expect(eq(std::is_trivially_copyable_v<dependency_ptr<simple_t>>, true));
            expect(eq(std::is_standard_layout_v<dependency_ptr<simple_t>>, true));
            expect(eq(std::is_trivially_destructible_v<dependency_ptr<simple_t>>, true));
            expect(eq(std::is_nothrow_constructible_v<dependency_ptr<simple_t>, simple_t&>, true));
            
            using complex_t = std::map<std::string, std::vector<std::int32_t>>;
            
            expect(eq(std::is_trivially_copyable_v<dependency_ptr<complex_t>>, true));
            expect(eq(std::is_standard_layout_v<dependency_ptr<complex_t>>, true));
            expect(eq(std::is_trivially_destructible_v<dependency_ptr<complex_t>>, true));
            expect(eq(std::is_nothrow_constructible_v<dependency_ptr<complex_t>, complex_t&>, true));
        };

        "size and alignment match raw pointers"_test = [] mutable {
            using simple_t = std::int32_t;
            
            expect(eq(sizeof(dependency_ptr<simple_t>) == sizeof(simple_t*), true));
            expect(eq(alignof(dependency_ptr<simple_t>) == alignof(simple_t*), true));

            using complex_t = std::map<std::string, std::vector<std::int32_t>>;

            expect(eq(sizeof(dependency_ptr<complex_t>) == sizeof(complex_t*), true));
            expect(eq(alignof(dependency_ptr<complex_t>) == alignof(complex_t*), true));
        };
        
        //============================================================
        // Const-correctness propagation
        //============================================================
        
        "const element forbids mutation through dereference"_test = [] mutable {
            const int x = 5;
            dependency_ptr<const int> ptr{x};
        
            // Compile-time: *ptr must NOT be assignable
            constexpr bool can_assign =
                std::is_assignable_v<decltype(*ptr), int>;
        
            expect(eq(can_assign, false));
        };
        
        "non-const element allows mutation"_test = [] mutable {
            int x = 5;
            dependency_ptr<int> ptr{x};
        
            *ptr = 10;
        
            expect(eq(x, 10));
        };
        
        "`pointer` nested type preserves top-level const"_test = [] mutable {
            using T = dependency_ptr<const int>;
        
            expect(eq(std::same_as<typename T::pointer, const int*>, true));
        };
        
        "`reference` nested type preserves const"_test = [] mutable {
            using T = dependency_ptr<const int>;

            expect(eq(std::same_as<typename T::reference, const int&>, true));
        };
        
        "const dependency_ptr prevents rebinding but not mutation"_test = [] mutable {
            int x = 1;
            int y = 2;
        
            const dependency_ptr<int> ptr{x};
        
            *ptr = 10;  // allowed
        
            constexpr bool can_rebind =
                std::is_assignable_v<const dependency_ptr<int>&, int&>;
        
            expect(eq(x, 10));
            expect(eq(can_rebind, false));
        };
        
        //============================================================
        // Interoperability with raw pointer APIs
        //============================================================
        
        "implicit conversion works with raw pointer API"_test = [] mutable {
            auto takes_ptr = [](int* p) { return *p; };
        
            int x = 3;
            dependency_ptr<int> ptr{x};
        
            expect(eq(takes_ptr(ptr), 3));
        };
        
        "get() works with raw pointer API"_test = [] mutable {
            auto takes_ptr = [](int* p) { return *p; };
        
            int x = 4;
            dependency_ptr<int> ptr{x};
        
            expect(eq(takes_ptr(ptr.get()), 4));
        };

        //============================================================
        // CTAD Guide
        //============================================================

        "deduction guide works"_test = [] mutable {
            constexpr int x = {};
            dependency_ptr ptr{x};
    
            expect(eq(ptr.get(), &x));
        };
    };
} //namespace

int main() {}
