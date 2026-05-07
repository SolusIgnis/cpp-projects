// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-alias_ptr.cppm
 * @version 0.4.0
 * @date May 3, 2026
 *
 * @copyright © 2026 Jeremy Murphy and any Contributors
 * @par License: @parblock
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. @endparblock
 *
 * @brief `alias_ptr`: A non-owning, nullable, non-arithmetic, void-permitting pointer type.
 *
 * @details
 *
 * @todo Future Development: Use `= delete("reason")` once the C++26 feature becomes available.
 */

//Module partition interface unit
export module base.vocab.ptr:alias_ptr;

import std;

import base.meta.traits;

import :forward_declarations;

export namespace base::vocab::inline ptr {
    /**
     * @brief Pointer type representing a potentially null object alias.
     *
     * @tparam T The pointed-to type (must not be a reference type per "C++ standard [dcl.ptr]" nor a function pointer).
     *
     * @details `alias_ptr` models a non-owning, nullable, non-arithmetic, void-permitting
     * pointer abstraction.
     *
     * @note Standard Layout type with size and alignment of a raw pointer.
     * @remark This type does not own the referenced object and does not participate in lifetime management.
     * @remark Copying and assignment rebind the pointer without affecting the lifetime of the underlying object.
     * @note Non-arithmetic Pointer: Pointer arithmetic and ordering comparisons are intentionally disabled to prevent misuse as an iterator.
     * @note `alias_ptr<void>` reuses the primary template rather than introducing a specialization. Placeholder reference aliases based on `std::monostate` are used solely to keep deleted overload declarations well-formed.
     * @remark Explicit equality comparison overloads are provided only where built-in pointer comparison cannot be reached through the implicit raw-pointer conversion operator alone.
     *
     * @warning The referenced object MUST outlive the `alias_ptr`. Violating this results in undefined behavior.
     *
     * @see `required_ptr` for non-null aliasing, `dependency_ptr` for dependency injection structural non-nullability, `cursor_ptr` for non-null iteration/traversal, `std::unique_ptr` and `std::shared_ptr` for ownership.
     */
    template<typename T>
        requires (!std::is_reference_v<T> && !std::is_function_v<base::meta::traits::remove_all_indirections_t<T>>)
    class alias_ptr {
    public:
        /**
         * @typedef element_type
         * @brief The stored element type.
         */
        using element_type = T;

        /**
         * @typedef value_type
         * @brief The unqualified element type (`std::remove_cv_t<T>`).
         */
        using value_type = std::remove_cv_t<T>;

        /**
         * @typedef pointer
         * @brief The underlying pointer type (`T*`).
         */
        using pointer = std::add_pointer_t<T>;

        /**
         * @typedef reference
         * @brief The reference type (`T&`).
         * @remark When `T` is `void`, uses `std::monostate&` because `void` as a function parameter is ill-formed.
         */
        using reference =
            std::conditional_t<std::is_void_v<T>, std::add_lvalue_reference_t<std::monostate>, std::add_lvalue_reference_t<T>>;

        /**
         * @typedef rvalue_reference
         * @brief The rvalue reference type (`T&&`).
         *
         * @remark When `T` is `void`, uses `std::monostate&&` because `void` as a function parameter is ill-formed.
         * @note Used only for deletion of invalid overloads to prevent binding to temporaries.
         */
        using rvalue_reference =
            std::conditional_t<std::is_void_v<T>, std::add_rvalue_reference_t<std::monostate>, std::add_rvalue_reference_t<T>>;

        /**
         * @typedef difference_type
         * @brief Pointer difference type (`std::ptrdiff_t`).
         *
         * @note Provided to model pointer interface, though arithmetic is disabled.
         */
        using difference_type = std::ptrdiff_t;

    private:
        pointer address_;

    public:
        //================================================================================
        // Constructors and Assignment Operators
        //================================================================================

        ///@brief Constructs an `alias_ptr` bound to an existing object.
        constexpr explicit alias_ptr(reference source) noexcept
            requires (!std::is_void_v<T>)
            : address_(std::addressof(source))
        {}

        ///@brief (Conversion) Implicitly converts from another `alias_ptr` according to underlying pointer conversions.
        template<typename U>
            requires (!std::same_as<U, T>) && std::convertible_to<std::add_pointer_t<U>, pointer>
        constexpr explicit(false) alias_ptr(const alias_ptr<U>& source) noexcept : address_(source.get())
        {}

        ///@brief Implicitly converts from a raw `pointer`. Explicit when `T` is void to avoid implicit conversion chaining.
        template<typename P>
            requires std::is_pointer_v<std::remove_cvref_t<P>> && std::convertible_to<std::decay_t<P>, pointer>
        constexpr explicit(std::is_void_v<T>) alias_ptr(P&& source) : address_(source)
        {}

        ///@brief Implicitly converts from another wrapped/smart pointer type. Explicit when `T` is void to avoid implicit conversion chaining.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, alias_ptr>)
                  && requires(Pointer<Element, Args...> ptr) {
                         { std::as_const(ptr).get() } -> std::convertible_to<pointer>;
                     }
        constexpr explicit(std::is_void_v<T>) alias_ptr(const Pointer<Element, Args...>& source)
            : address_(source.get())
        {}
        
        ///@brief Rebinds the `alias_ptr` to another object.
        template<typename Self>
            requires (!std::is_const_v<Self>)
        constexpr Self& operator=(this Self& self, reference source) noexcept
            requires (!std::is_void_v<T>)
        {
            self.address_ = std::addressof(source);
            return self;
        }

        ///@brief (Conversion) Assigns from another `alias_ptr` according to underlying pointer conversions.
        template<typename Self, typename U>
            requires (!std::is_const_v<Self>) && (!std::same_as<U, T>) && std::convertible_to<std::add_pointer_t<U>, pointer>
        constexpr Self& operator=(this Self& self, const alias_ptr<U>& source) noexcept
        {
            self.address_ = source.get();
            return self;
        }

        ///@brief Assigns from a raw `pointer`.
        template<typename Self, typename P>
            requires (!std::is_const_v<Self>) && std::is_pointer_v<std::remove_cvref_t<P>> && std::convertible_to<std::decay_t<P>, pointer>
        constexpr Self& operator=(this Self& self, P&& source)
        {
            self.address_ = source;
            return self;
        }

        ///@brief Assigns from another wrapped/smart pointer type.
        template<typename Self, template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, alias_ptr>)
                  && (!std::is_const_v<Self>)
                  && requires(Pointer<Element, Args...> ptr) {
                         { std::as_const(ptr).get() } -> std::convertible_to<pointer>;
                     }
        constexpr Self& operator=(this Self& self, const Pointer<Element, Args...>& source)
        {
            self.address_ = source.get();
            return self;
        }

        ///@brief Swaps addresses.
        friend constexpr void swap(alias_ptr& lhs, alias_ptr& rhs) noexcept
        {
            using std::swap;
            swap(lhs.address_, rhs.address_);
        }
        
        //================================================================================
        // Constructors and Assignment Operators: Nullable Implementation
        //================================================================================

        ///@brief Default constructor initializes to null.
        alias_ptr() = default;

        ///@brief Constructor from `nullptr` initializes to null.
        alias_ptr(std::nullptr_t null) :  address_(null) {}

        ///@brief Assignment from `nullptr` rebinds to null.
        template<typename Self>
            requires (!std::is_const_v<Self>)
        Self& operator=(this Self& self, std::nullptr_t null) { self.address_ = null; return self; }

        //================================================================================
        // Deleted Constructors and Assignment Operators: No Aliasing Temporaries
        //================================================================================

        ///@brief Deleted constructor from `rvalue_reference` to discourage dangling by rejecting direct binding to temporaries.
        alias_ptr(rvalue_reference) =
            delete /*("Constructor from `rvalue_reference` deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        ///@brief Deleted constructor from pointer-like object rvalue to discourage dangling by rejecting direct binding to temporaries.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, alias_ptr>)
                      && requires(Pointer<Element, Args...> ptr) {
                             { std::as_const(ptr).get() } -> std::convertible_to<pointer>;
                         }
        alias_ptr(const Pointer<Element, Args...>&&) =
            delete /*("Constructor from pointer-like object rvalue deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        ///@brief Deleted assignment from `rvalue_reference` to discourage dangling by rejecting direct binding to temporaries.
        template<typename Self>
        Self& operator=(this Self&&, rvalue_reference) =
            delete /*("Assignment from `rvalue_reference` deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        ///@brief Deleted assignment from pointer-like object rvalue to discourage dangling by rejecting direct binding to temporaries.
        template<typename Self, template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, alias_ptr>)
                      && requires(Pointer<Element, Args...> ptr) {
                             { std::as_const(ptr).get() } -> std::convertible_to<pointer>;
                         }
        Self& operator=(this Self&&, const Pointer<Element, Args...>&&) =
            delete /*("Assignment from pointer-like object rvalue deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        //================================================================================
        // Deleted Constructors and Assignment Operators: No Array-to-Pointer Decay
        //================================================================================

        ///@brief Deleted constructor from C-array to prevent array-to-pointer decay.
        template<typename AnyCArray>
            requires std::is_array_v<AnyCArray>
        alias_ptr(AnyCArray&) =
            delete /*("Constructor from C-array deleted to prevent array-to-pointer decay. To point to the first element, alias it explicitly.")*/
            ;

        ///@brief Deleted assignment from C-array to prevent array-to-pointer decay.
        template<typename Self, typename AnyCArray>
            requires std::is_array_v<AnyCArray>
        Self& operator=(this Self&&, AnyCArray&) =
            delete /*("Assignment from C-array deleted to prevent array-to-pointer decay. To point to the first element, alias it explicitly.")*/
            ;

        //================================================================================
        // Comparison Operators (Equality Allowed, Others Deleted)
        //================================================================================

        ///@brief Compares equality in terms of pointer identity.
        [[nodiscard]] friend constexpr bool operator==(const alias_ptr& lhs, const alias_ptr& rhs) noexcept = default;

        ///@brief Covariantly compares equality in terms of pointer identity.
        template<std::derived_from<T> DerivedT>
            requires (!std::same_as<DerivedT, T>)
        [[nodiscard]] friend constexpr bool operator==(const alias_ptr& lhs, const alias_ptr<DerivedT>& rhs) noexcept
        {
            return (lhs.get() == rhs.get());
        }

        ///@brief Covariantly compares equality of an `alias_ptr`-to-base with a raw pointer-to-derived in terms of pointer identity.
        template<std::derived_from<T> DerivedT>
        [[nodiscard]] friend constexpr bool operator==(const alias_ptr& lhs, const std::add_pointer_t<DerivedT> rhs) noexcept
        {
            return (lhs.get() == rhs);
        }

        ///@brief Covariantly compares equality of a raw pointer-to-base with an `alias_ptr`-to-derived in terms of pointer identity.
        template<std::derived_from<T> DerivedT>
        [[nodiscard]] friend constexpr bool operator==(const pointer lhs, const alias_ptr<DerivedT>& rhs) noexcept
        {
            return (lhs == rhs.get());
        }
        
        ///@brief Compares equality against `nullptr`.
        [[nodiscard]] friend constexpr bool operator==(const alias_ptr& ptr, std::nullptr_t null) noexcept { return (ptr.get() == null); }

        ///@brief Deleted comparison operators to prevent misuse as an iterator or ordered value type.
        auto operator<=>(alias_ptr) const =
            delete /*("Comparison operators deleted to prevent address comparisons. `alias_ptr` is not an iterator.")*/;

        ///@brief Deleted comparison operators to prevent misuse as an iterator or ordered value type.
        auto operator<=>(const pointer) const =
            delete /*("Comparison operators deleted to prevent address comparisons. `alias_ptr` is not an iterator.")*/;

        //================================================================================
        // Pointer Operations
        //================================================================================

        ///@brief Provides pointer-like member access to the referenced object.
        [[nodiscard]] constexpr pointer operator->(this auto && self) noexcept
            requires (!std::is_void_v<T>)
        {
            return self.address_;
        }

        ///@brief Dereferences the pointer to access the referenced object.
        [[nodiscard]] constexpr reference operator*(this auto && self) noexcept
            requires (!std::is_void_v<T>)
        {
            return *self.address_;
        }

        ///@brief Returns the underlying raw pointer.
        [[nodiscard]] constexpr pointer get(this auto && self) noexcept { return self.address_; }

        ///@brief Implicitly converts to the underlying raw pointer type.
        [[nodiscard]] constexpr explicit(false) operator pointer() noexcept { return this->get(); }

        ///@brief Contextually converts to `bool` to test if the pointer is engaged.
        [[nodiscard]] constexpr explicit operator bool(this auto && self) noexcept { return (self.address_ != nullptr); }
        
        ///@brief Returns the underlying raw pointer while resetting to `nullptr`.
        [[nodiscard]] constexpr pointer release(this auto && self) noexcept { return std::exchange(self.address_, nullptr); }

        ///@brief Rebinding passes through to assignment.
        template<typename Self, typename P>
        Self& rebind(this Self& self, P&& source)
            noexcept(std::is_nothrow_assignable_v<Self&, P>)
            requires std::is_assignable_v<Self&, P>
        {
            return self = std::forward<P>(source);
        }

        ///@brief Reset the pointer to `nullptr`.
        constexpr void reset(this auto& self, std::nullptr_t null = nullptr) noexcept { self = null; }

        ///@brief Reset the pointer to a new address.
        template<typename Self, typename P>
            requires (!std::same_as<P, std::nullptr_t>)
        constexpr void reset(this Self& self, P&& source)
            noexcept(std::is_nothrow_assignable_v<Self&, P>)
            requires std::is_assignable_v<Self&, P>
        {
            self = std::forward<P>(source);
        }

        //================================================================================
        // Deleted Pointer Arithmetic Operators: Not an Iterator
        //================================================================================

        ///@brief Deleted prefix increment to prevent misuse as an iterator.
        template<typename Self>
        Self& operator++(this Self&&) =
            delete /*("Prefix increment deleted to prevent pointer arithmetic. `alias_ptr` is not an iterator.")*/;

        ///@brief Deleted prefix decrement to prevent misuse as an iterator.
        template<typename Self>
        Self& operator--(this Self&&) =
            delete /*("Prefix decrement deleted to prevent pointer arithmetic. `alias_ptr` is not an iterator.")*/;

        ///@brief Deleted postfix increment to prevent misuse as an iterator.
        template<typename Self>
        Self operator++(this Self&&, int) =
            delete /*("Postfix increment deleted to prevent pointer arithmetic. `alias_ptr` is not an iterator.")*/;

        ///@brief Deleted postfix decrement to prevent misuse as an iterator.
        template<typename Self>
        Self operator--(this Self&&, int) =
            delete /*("Postfix decrement deleted to prevent pointer arithmetic. `alias_ptr` is not an iterator.")*/;

        ///@brief Deleted addition assignment to prevent misuse as an iterator.
        template<typename Self>
        Self& operator+=(this Self&&, difference_type) =
            delete /*("Addition assignment deleted to prevent pointer arithmetic. `alias_ptr` is not an iterator.")*/;

        ///@brief Deleted subtraction assignment to prevent misuse as an iterator.
        template<typename Self>
        Self& operator-=(this Self&&, difference_type) =
            delete /*("Subtraction assignment deleted to prevent pointer arithmetic. `alias_ptr` is not an iterator.")*/;

        ///@brief Deleted pointer addition to prevent misuse as an iterator.
        friend alias_ptr operator+(alias_ptr, difference_type) =
            delete /*("Pointer addition deleted to prevent pointer arithmetic. `alias_ptr` is not an iterator.")*/;

        ///@brief Deleted pointer addition to prevent misuse as an iterator.
        friend alias_ptr operator+(difference_type, alias_ptr) =
            delete /*("Pointer addition deleted to prevent pointer arithmetic. `alias_ptr` is not an iterator.")*/;

        ///@brief Deleted pointer subtraction to prevent misuse as an iterator.
        friend difference_type operator-(alias_ptr, alias_ptr) =
            delete /*("Pointer subtraction deleted to prevent pointer arithmetic. `alias_ptr` is not an iterator.")*/;

        ///@brief Deleted pointer subtraction to prevent misuse as an iterator.
        friend alias_ptr operator-(alias_ptr, difference_type) =
            delete /*("Pointer subtraction deleted to prevent pointer arithmetic. `alias_ptr` is not an iterator.")*/;

        //================================================================================
        // Stream Output
        //================================================================================

        ///@brief Output an `alias_ptr` address to a `std::basic_ostream`.
        template <typename CharT, typename Traits>
        friend std::basic_ostream<CharT, Traits>& operator<<(
            std::basic_ostream<CharT, Traits>& stream, 
            const alias_ptr& ptr) 
        {
            // In order to support pointers to arbitrarily cv-qualified objects:
            // 1. `static_cast` to `const volatile void*` to preserve all qualifiers while converting the pointer to `void*`.
            // 2. `const_cast` to `const void*` to satisfy the inserter's interface which lacks `volatile void*` overloads.
            // This is safe because formatting is a read-only numerical operation on the address.
            return stream << const_cast<const void*>(static_cast<const volatile void*>(ptr.get()));
        }
    }; //class alias_ptr
    
    /**
     * @fn explicit alias_ptr::alias_ptr(reference source) noexcept
     *
     * @param source The object to reference.
     *
     * @pre `source` must refer to a valid object that outlives the constructed `alias_ptr`.
     * @post `get() == std::addressof(source)`.
     *
     * @remark Prevents binding to temporaries via deleted rvalue overload.
     */
    /**
     * @overload alias_ptr::alias_ptr(const alias_ptr<U>& source) noexcept
     *
     * @tparam U The element type, with its pointer convertible to `pointer`, of the source `alias_ptr`.
     *
     * @param source The `alias_ptr` being converted.
     *
     * @pre `source` must be null or point to a valid object that outlives the resulting `alias_ptr`.
     * @post `get() == source.get()`.
     *
     * @details This single constructor handles:
     * 1. Derived-to-Base conversion (e.g., `ptr<Derived>` to `ptr<Base>`).
     * 2. Qualification conversion (e.g., `ptr<T>` to `ptr<const T>`).
     * 3. Type erasure (e.g., `ptr<T>` to `ptr<void>`).
     *
     * @remark Preserves covariance. Converts from alias_ptr-to-derived to alias_ptr-to-base implicitly.
     * @remark Enables type erasure by converting `alias_ptr<U>` to `alias_ptr<void>` when applicable.
     */
    /**
     * @overload explicit alias_ptr::alias_ptr(P&& source)
     *
     * @tparam P The raw pointer type which must decay to a type convertible to `pointer`.
     *
     * @param source The raw pointer to bind.
     *
     * @pre `source` must be null or point to a valid object that outlives the constructed `alias_ptr`.
     * @post `get() == source`.
     *
     * @details Captures the original argument type prior to decay via
     * forwarding reference so that C-array arguments can be diagnosed
     * explicitly instead of silently decaying to element pointers.
     *
     * @note This constructor is constrained to raw pointers to prevent hijacking copy/move operations or accepting arrays.
     * @remark Explicit when `T` is `void` to prevent unintended implicit erasure chains.
     */
    /**
     * @overload explicit alias_ptr::alias_ptr(const Pointer<Element, Args...>& source)
     *
     * @tparam Pointer A class template modeling a pointer-like type.
     * @tparam Element The element type of the source pointer.
     * @tparam Args Additional template parameters of the pointer type.
     *
     * @param source The pointer-like object providing access to a pointer-compatible value via `get()`.
     *
     * @pre `source.get()` must be a valid expression convertible to `pointer`.
     * @pre `source.get()` must be null or point to an object whose lifetime strictly exceeds that of the constructed `alias_ptr`.
     * @post `get() == static_cast<pointer>(source.get())`.
     *
     * @remark Enables interoperation with pointer-like types (e.g., smart pointers) that expose a `get()` member.
     * @remark The source type must not be a specialization of `alias_ptr` (to avoid ambiguity with existing overloads).
     * @remark This constructor does not transfer ownership and does not affect the lifetime of the underlying object.
     */
    /**
     * @fn alias_ptr& alias_ptr::operator=(reference source) noexcept
     *
     * @param source The object to reference.
     * @return Reference to `*this`.
     *
     * @pre `source` must refer to a valid object that outlives the `alias_ptr`.
     *
     * @remark Rebinds the pointer without affecting ownership or lifetime.
     */
    /**
     * @overload alias_ptr& alias_ptr::operator=(const alias_ptr<U>& source) noexcept
     *
     * @tparam U The element type, with its pointer convertible to `pointer`, of the source `alias_ptr`.
     *
     * @param source The `alias_ptr` being converted.
     * @return Reference to `*this`.
     *
     * @pre `source` must be null or point to a valid object that outlives the resulting `alias_ptr`.
     *
     * @details This single assignment operator handles:
     * 1. Derived-to-Base conversion (e.g., `ptr<Derived>` to `ptr<Base>`).
     * 2. Qualification conversion (e.g., `ptr<T>` to `ptr<const T>`).
     * 3. Type erasure (e.g., `ptr<T>` to `ptr<void>`).
     *
     * @remark Preserves covariance. Converts from alias_ptr-to-derived to alias_ptr-to-base implicitly.
     * @remark Enables type erasure by converting `alias_ptr<U>` to `alias_ptr<void>` when applicable.
     */
    /**
     * @overload alias_ptr& alias_ptr::operator=(P&& source)
     *
     * @tparam P The raw pointer type which must decay to a type convertible to `pointer`.
     *
     * @param source The raw pointer to rebind to.
     * @return Reference to `*this`.
     *
     * @pre `source` must be null or point to a valid object that outlives the `alias_ptr`.
     * @post `get() == source`.
     *
     * @details Captures the original argument type prior to decay via
     * forwarding reference so that C-array arguments can be diagnosed
     * explicitly instead of silently decaying to element pointers.
     *
     * @note This assignment operator is constrained to raw pointers to prevent hijacking copy/move operations or accepting arrays.
     * @remark Does not affect the lifetime of the referenced object.
     */
    /**
     * @overload alias_ptr& alias_ptr::operator=(const Pointer<Element, Args...>& source)
     *
     * @tparam Pointer A class template modeling a pointer-like type.
     * @tparam Element The element type of the source pointer.
     * @tparam Args Additional template parameters of the pointer type.
     *
     * @param source The pointer-like object providing access to a raw pointer via `get()`.
     * @return Reference to `*this`.
     *
     * @pre `source.get()` must be a valid expression convertible to `pointer`.
     * @pre `source.get()` must be null or point to an object whose lifetime strictly exceeds that of the `alias_ptr`.
     * @post `get() == static_cast<pointer>(source.get())`.
     *
     * @remark Does not transfer ownership and does not affect the lifetime of the underlying object.
     */
    /**
     * @fn constexpr void swap(alias_ptr& lhs, alias_ptr& rhs) noexcept
     *
     * @param lhs The first pointer wrapper.
     * @param rhs The second pointer wrapper.
     *
     * @post `lhs` refers to the object previously referenced by `rhs`.
     * @post `rhs` refers to the object previously referenced by `lhs`.
     *
     * @remark Swaps only the stored addresses.
     * @remark Does not affect ownership or pointee lifetime.
     * @remark Provided as a hidden friend for ADL interoperability.
     */
    /**
     * @fn constexpr bool operator==(const alias_ptr& lhs, const alias_ptr& rhs) noexcept
     *
     * @param lhs The left-hand side `alias_ptr`.
     * @param rhs The right-hand side `alias_ptr`.
     *
     * @return `true` if both pointers refer to the same object; otherwise `false`.
     *
     * @remark Compares the pointed-to addresses (aliasing), not object values.
     */
    /**
     * @overload constexpr bool operator==(const alias_ptr& lhs, const alias_ptr<DerivedT>& rhs) noexcept
     *
     * @tparam DerivedT The element type, derived from `T`, of the right-hand side `alias_ptr`.
     *
     * @param lhs The left-hand side `alias_ptr`.
     * @param rhs The right-hand side `alias_ptr` to compare.
     *
     * @return `true` if both pointers refer to the same object; otherwise `false`.
     *
     * @remark Enables equality comparison between `alias_ptr` instances of related types when that can't be synthesized by implicit conversion to raw pointers.
     * @remark Compares the pointed-to addresses (aliasing), not object values.
     */
    /**
     * @overload constexpr bool operator==(const alias_ptr& lhs, const std::add_pointer_t<DerivedT> rhs) noexcept
     *
     * @tparam DerivedT The element type, derived from `T`, of the raw pointer.
     *
     * @param lhs The `alias_ptr` being compared.
     * @param rhs The raw pointer to compare against.
     *
     * @return `true` if the wrapped pointer in `lhs` equals `rhs`; otherwise `false`.
     *
     * @remark Enables comparison with raw pointers-to-derived for when that can't be synthesized by implicit conversion to raw pointers.
     * @remark Compares the pointed-to addresses (aliasing), not object values.
     */
    /**
     * @overload constexpr bool operator==(const pointer lhs, const alias_ptr<DerivedT>& rhs) noexcept
     *
     * @tparam DerivedT The element type, derived from `T`, of the right-hand side `alias_ptr`.
     *
     * @param lhs The raw pointer to compare.
     * @param rhs The `alias_ptr` being compared.
     *
     * @return `true` if `lhs` equals the wrapped pointer in `rhs`; otherwise `false`.
     *
     * @remark Enables comparison with raw pointers-to-base when that can't be synthesized by implicit conversion to raw pointers.
     * @remark Comparison is performed on the underlying addresses.
     */
    /**
     * @overload constexpr bool operator==(const alias_ptr& ptr, std::nullptr_t null) noexcept
     *
     * @param ptr The pointer being compared.
     * @param null The `nullptr_t` parameter.
     *
     * @return `true` if `ptr` is disengaged; otherwise `false`.
     */
    /**
     * @fn constexpr reference alias_ptr::operator*() const noexcept
     *
     * @return Reference to the referenced object.
     *
     * @pre The stored pointer must remain valid (non-dangling).
     *
     * @remark Equivalent to dereferencing `get()`.
     */
    /**
     * @fn constexpr pointer alias_ptr::get() const noexcept
     *
     * @return The underlying raw pointer.
     *
     * @remark Provided for interoperability with pointer-based APIs.
     */
    /**
     * @fn constexpr alias_ptr::operator pointer() const noexcept
     *
     * @return The underlying raw pointer.
     *
     * @remark Enables seamless interoperability with legacy interfaces expecting raw pointers.
     * @warning Implicit conversion may obscure the non-owning semantics; prefer `get()` when clarity is important.
     */
    /**
     * @fn constexpr alias_ptr::operator bool() const noexcept
     *
     * @return `true` if the pointer is engaged (i.e., the stored address is not `nullptr`); otherwise `false`.
     *
     * @remark Satisfies boolean-testable requirements in generic templates and logical contexts.
     * @remark Models pointer interface by providing contextual conversion to `bool`.
     */

     /**
     * @brief Deduction guide for `alias_ptr`.
     *
     * @tparam T The type of the referenced object.
     *
     * @remark Deduces `T` from the referenced object, preserving cv-qualification.
     */
    template<typename T>
    alias_ptr(T&) -> alias_ptr<T>;

    /**
     * @brief Deduction guide for `alias_ptr`.
     *
     * @tparam T The type of the pointee.
     *
     * @remark Deduces `T` from the pointee, preserving cv-qualification.
     */
    template<typename T>
    alias_ptr(T*) -> alias_ptr<T>;
} //namespace base::vocab::inline ptr

/**
 * @brief Partial specialization of `std::hash` for `alias_ptr`.
 *
 * @tparam T The element type of the `alias_ptr`.
 *
 * @remark Hashes the underlying stored address rather than pointee object state or values.
 * @remark Consistent with `alias_ptr` equality semantics.
 */
template<class T>
struct std::hash<base::vocab::ptr::alias_ptr<T>> {
    ///@brief Hashes the `alias_ptr` based on the underlying address.
    [[nodiscard]] constexpr std::size_t operator()(const base::vocab::ptr::alias_ptr<T>& ptr) const noexcept
    {
        return std::hash<T*>{}(ptr.get());
    }
};

/**
 * @brief Partial specialization of `std::formatter` for `alias_ptr`.
 *
 * @tparam T The element type of the `alias_ptr`.
 * @tparam CharT The character type used by the format string.
 *
 * @remark Formats an `alias_ptr` as its underlying raw pointer representation.
 */
template<typename T, typename CharT>
struct std::formatter<base::vocab::ptr::alias_ptr<T>, CharT> 
    : std::formatter<const void*, CharT> {
    
    ///@brief Format as the underlying raw pointer address.
    auto format(const base::vocab::ptr::alias_ptr<T>& ptr, auto& ctx) const {
        // In order to support pointers to arbitrarily cv-qualified objects:
        // 1. `static_cast` to `const volatile void*` to preserve all qualifiers while converting the pointer to `void*`.
        // 2. `const_cast` to `const void*` to satisfy the formatter's interface which lacks `volatile void*` specializations.
        // This is safe because formatting is a read-only numerical operation on the address.
        return std::formatter<const void*, CharT>::format(const_cast<const void*>(static_cast<const volatile void*>(ptr.get())), ctx);
    }
};
