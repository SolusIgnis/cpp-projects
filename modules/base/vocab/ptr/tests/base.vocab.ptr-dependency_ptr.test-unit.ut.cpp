// SPDX-License-Identifier: Apache-2.0
// Unit tests for base.vocab.ptr:dependency_ptr

import base.vocab.ptr;
import ut;
import std;

using namespace ut;
using base::vocab::dependency_ptr;

namespace {
    template <template <typename...> typename Template, typename... Args>
    concept instantiatable_with = requires {
        typename Template<Args...>;
    };

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
    
    suite dependency_ptr_tests = [] mutable {
        //============================================================
        // Template Constraint Validation
        //============================================================

        "template instantiation checks"_test = [] mutable {
            expect(eq(instantiatable_with<dependency_ptr, std::int32_t>, true));
            expect(eq(instantiatable_with<dependency_ptr, std::int32_t*>, true));
            expect(eq(instantiatable_with<dependency_ptr, std::map<std::string, std::vector<std::int32_t>>, true));

            expect(eq(instantiatable_with<dependency_ptr, void>, false));
            expect(eq(instantiatable_with<dependency_ptr, std::int32_t&>, false));
            expect(eq(instantiatable_with<dependency_ptr, std::int32_t&&>, false));
            expect(eq(instantiatable_with<dependency_ptr, void(int)>, false));
            expect(eq(instantiatable_with<dependency_ptr, void(&)(int)>, false));
            expect(eq(instantiatable_with<dependency_ptr, void(*)(int, float)>, false));
            expect(eq(instantiatable_with<dependency_ptr, void(**)(std::string, int)>, false));
            expect(eq(instantiatable_with<dependency_ptr, void(*******)(int)>, false));
        };
    
        //============================================================
        // Triviality & ABI properties
        //============================================================
        
        "triviality"_test = [] mutable {
            using simple_t = std::int32_t;

            expect(eq(std::is_standard_layout_v<dependency_ptr<simple_t>>, true));
            expect(eq(std::is_trivially_copyable_v<dependency_ptr<simple_t>>, true));
            expect(eq(std::is_trivially_destructible_v<dependency_ptr<simple_t>>, true));
            expect(eq(std::is_nothrow_constructible_v<dependency_ptr<simple_t>, simple_t&>, true));
            
            using complex_t = std::map<std::string, std::vector<std::int32_t>>;
            
            expect(eq(std::is_standard_layout_v<dependency_ptr<complex_t>>, true));
            expect(eq(std::is_trivially_copyable_v<dependency_ptr<complex_t>>, true));
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
            expect(eq(std::constructible_from<dependency_ptr<int>, int>, false));
            expect(eq(std::constructible_from<dependency_ptr<const int>, int>, false));
            expect(eq(std::constructible_from<dependency_ptr<const int>, const int>, false));

            expect(eq(std::constructible_from<dependency_ptr<int>, int&&>, false));
            expect(eq(std::constructible_from<dependency_ptr<const int>, int&&>, false));
            expect(eq(std::constructible_from<dependency_ptr<const int>, const int&&>, false));
        };

        //============================================================
        // Assignment
        //============================================================

        "assignable from lvalue reference"_test = [] mutable {
            expect(eq(std::is_assignable_v<dependency_ptr<int>&, int&>, true));
            expect(eq(std::is_assignable_v<dependency_ptr<int>&, const int&>, false));
            expect(eq(std::is_assignable_v<dependency_ptr<int>&, volatile int&>, false));
            expect(eq(std::is_assignable_v<dependency_ptr<int>&, const volatile int&>, false));
            expect(eq(std::is_assignable_v<dependency_ptr<const int>&, int&>, true));
            expect(eq(std::is_assignable_v<dependency_ptr<const int>&, const int&>, true));
            expect(eq(std::is_assignable_v<dependency_ptr<const int>&, volatile int&>, false));
            expect(eq(std::is_assignable_v<dependency_ptr<const int>&, const volatile int&>, false));
            expect(eq(std::is_assignable_v<dependency_ptr<volatile int>&, int&>, true));
            expect(eq(std::is_assignable_v<dependency_ptr<volatile int>&, const int&>, false));
            expect(eq(std::is_assignable_v<dependency_ptr<volatile int>&, volatile int&>, true));
            expect(eq(std::is_assignable_v<dependency_ptr<volatile int>&, const volatile int&>, false));
            expect(eq(std::is_assignable_v<dependency_ptr<const volatile int>&, int&>, true)); 
            expect(eq(std::is_assignable_v<dependency_ptr<const volatile int>&, const int&>, true));
            expect(eq(std::is_assignable_v<dependency_ptr<const volatile int>&, volatile int&>, true)); 
            expect(eq(std::is_assignable_v<dependency_ptr<const volatile int>&, const volatile int&>, true)); 
        };

        "not assignable from nullptr"_test = [] mutable {
            expect(eq(std::is_assignable_v<dependency_ptr<int>&, std::nullptr_t>, false));
        };
    
        "not assignable from raw pointer"_test = [] mutable {
            expect(eq(std::is_assignable_v<dependency_ptr<int>&, int*>, false));
            expect(eq(std::is_assignable_v<dependency_ptr<const int>&, int*>, false));
            expect(eq(std::is_assignable_v<dependency_ptr<const int>&, const int*>, false));
        };
    
        "not assignable from rvalue"_test = [] mutable {
            expect(eq(std::is_assignable_v<dependency_ptr<int>&, int&&>, false));
            expect(eq(std::is_assignable_v<dependency_ptr<const int>&, int&&>, false));
            expect(eq(std::is_assignable_v<dependency_ptr<const int>&, const int&&>, false));
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
    
            expect(eq(raw, std::addressof(x)));
        };
    
        "get returns raw pointer"_test = [] mutable {
            const int x{};
            dependency_ptr<const int> ptr{x};
    
            expect(eq(ptr.get(), std::addressof(x)));
        };
    
        "boolean conversion"_test = [] mutable {
            expect(eq(std::constructible_from<bool, dependency_ptr<int>>, true));
            
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
            expect(eq(ptr.get(), std::addressof(b)));
        };

        //============================================================
        // Equality semantics
        //============================================================
    
        "equality compares pointer identity"_test = [] mutable {
            expect(eq(std::equality_comparable<dependency_ptr<int>>, true));

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
    
            expect(eq(bptr.get(), static_cast<Base*>(std::addressof(d))));
        };
        
        "construct base from derived reference"_test = [] mutable {
            Derived d;
    
            dependency_ptr<Base> bptr{d};
    
            expect(eq(bptr.get(), static_cast<Base*>(std::addressof(d))));
        };
    
        "assign base from derived dependency_ptr"_test = [] mutable {
            Derived d;
            dependency_ptr<Derived> dptr{d};
    
            Base b;
            dependency_ptr<Base> bptr{b};
    
            bptr = dptr;
    
            expect(eq(bptr.get(), static_cast<Base*>(std::addressof(d))));
        };
        
        "rebind base from derived reference"_test = [] mutable {
            Derived d;

            Base b;
            dependency_ptr<Base> bptr{b};
    
            bptr = d;
    
            expect(eq(bptr.get(), static_cast<Base*>(std::addressof(d))));
        };
        
        "assign const base from derived dependency_ptr"_test = [] mutable {
            Derived d;
            dependency_ptr<Derived> dptr{d};
    
            Base b;
            dependency_ptr<const Base> bptr{b};
    
            bptr = dptr;
    
            expect(eq(bptr.get(), static_cast<const Base*>(std::addressof(d))));
        };

        "rebind const base from derived reference"_test = [] mutable {
            Derived d;

            Base b;
            dependency_ptr<const Base> bptr{b};
    
            bptr = d;
    
            expect(eq(bptr.get(), static_cast<const Base*>(std::addressof(d))));
        };

        "covariant equality comparison"_test = [] mutable {
            expect(eq(std::equality_comparable_with<dependency_ptr<Base>, dependency_ptr<Derived>>, true));
            expect(eq(std::equality_comparable_with<dependency_ptr<Base>, Derived*>, true));
            expect(eq(std::equality_comparable_with<Base*, dependency_ptr<Derived>>, true));
        };
    
        //============================================================
        // Non-iterator / non-arithmetic guarantees
        //============================================================
    
        "no pointer arithmetic operations"_test = [] mutable {
            using T = dependency_ptr<int>;
    
            expect(eq(HasAddition<T>, false));
            expect(eq(HasSubtraction<T>, false));
            expect(eq(HasDifference<T>, false));
            expect(eq(HasPreIncrement<T>, false));
            expect(eq(HasPostIncrement<T>, false));
            expect(eq(HasPreDecrement<T>, false));
            expect(eq(HasPostDecrement<T>, false));
        };
        /*
        "no ordering comparisons"_test = [] mutable {
            expect(eq(std::three_way_comparable<dependency_ptr<int>>, false));
            expect(eq(std::three_way_comparable_with<dependency_ptr<int>, int*>, false));
            expect(eq(std::three_way_comparable<dependency_ptr<Base>>, false));
            expect(eq(std::three_way_comparable_with<dependency_ptr<Base>, dependency_ptr<Derived>>, false));
            expect(eq(std::three_way_comparable_with<dependency_ptr<Base>, Derived*>, false));
            expect(eq(std::three_way_comparable_with<Base*, dependency_ptr<Derived>>, false));
        };*/
    
        //============================================================
        // Const-correctness propagation
        //============================================================
        
        "const element forbids mutation through dereference"_test = [] mutable {
            const int x{};
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
            int x{};
        
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
    
            expect(eq(ptr.get(), std::addressof(x)));
        };
    };
} //namespace

int main() {}
