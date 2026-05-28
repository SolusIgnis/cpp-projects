// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-cursor_ptr.cppm
 * @version 0.6.0
 * @date May 7, 2026
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
 * @brief `cursor_ptr`: non-owning, non-nullable, arithmetic, non-void-permitting
 *
 * @details
 *
 * `cursor_ptr` provides a vocabulary pointer type representing a non-owning,
 * non-null cursor into contiguous memory. Unlike stable aliasing pointer
 * abstractions, `cursor_ptr` intentionally exposes traversal semantics through
 * pointer arithmetic, ordering, and iterator interoperability.
 *
 * This type models a lightweight random-access and contiguous iterator over
 * existing storage while preserving explicit non-ownership semantics. It is
 * intended for APIs where nullable states are invalid by construction and where
 * pointer traversal is semantically meaningful.
 *
 * The abstraction preserves the operational behavior of raw pointers while
 * constraining several historically error-prone language behaviors:
 * - Null construction and rebinding are prohibited structurally.
 * - Direct binding to temporaries is rejected to discourage dangling.
 * - C-array decay is rejected to prevent implicit loss of extent information.
 * - Ownership transfer semantics are intentionally absent.
 *
 * `cursor_ptr` is suitable for:
 * - Traversing contiguous object sequences.
 * - Expressing non-null iterator-like API contracts.
 * - Interoperating with low-level and legacy pointer-based interfaces.
 * - Generic code requiring contiguous or random-access iterator semantics.
 *
 * `cursor_ptr` is not intended to model:
 * - Ownership or lifetime management.
 * - Optional/disengaged pointer states.
 * - Stable aliases to a single object independent of surrounding storage.
 *
 * As with raw pointers and iterators, validity depends entirely on external
 * lifetime and storage guarantees. Operations that invalidate the underlying
 * contiguous storage also invalidate all associated `cursor_ptr` instances.
 *
 * @todo Future Development: Use `= delete("reason")` once the C++26 feature becomes available.
 */

//Module partition interface unit
export module base.vocab.ptr:cursor_ptr;

import std;

import base.meta.traits;
import base.meta.concepts;

#ifndef EXPERIMENTAL_CORE_PARTITION
import :core;

export namespace base::vocab::inline ptr {
    /**
     * @brief Pointer type representing a non-null cursor/iterator.
     *
     * @tparam T The pointed-to type (must not be a reference type per "C++ standard [dcl.ptr]", `void`, nor a function pointer).
     *
     * @details `cursor_ptr` models a non-owning, non-nullable, arithmetic, non-void-permitting
     * pointer abstraction. It is designed for random-access and contiguous iteration over objects
     * stored in contiguous memory. Because it exposes memory traversal semantics (pointer
     * arithmetic and ordering), it is not a stable alias to a single object.
     *
     * @note Standard Layout type with size and alignment of a raw pointer.
     * @invariant Always engaged: a `cursor_ptr` always stores a valid memory address; there is no null/disengaged representation. Attempts to construct/assign from another pointer throw on null.
     * @remark This type does not own the pointee object and does not participate in lifetime management.
     * @remark Copying and assignment rebind the stored address without affecting pointee lifetime.
     * @note Construction or assignment from `nullptr` is ill-formed.
     * @note Construction or assignment from pointers and pointer-like values performs runtime validation and throws `std::invalid_argument` on null. All operations provide the Strong Exception Safety Guarantee.
     * @remark Explicit comparison overloads are provided only where implicit conversion to the nested `pointer` type is insufficient to enable the desired comparison.
     *
     * @warning The referenced object MUST outlive the `cursor_ptr`. Violating this results in undefined behavior.
     * @warning Any operation that invalidates the underlying contiguous storage also invalidates associated `cursor_ptr` instances.
     *
     * @see `alias_ptr` for nullable aliasing, `required_ptr` for non-null aliasing, `dependency_ptr` for dependency injection structural non-nullability, `std::unique_ptr` and `std::shared_ptr` for ownership.
     */
    template<typename Pointee>
        requires valid_pointee_v<Pointee> && (!std::is_void_v<Pointee>)
    class cursor_ptr : public ptr_core<
        cursor_ptr,
        Pointee,
        ptr_policies::type_list<
            ptr_policies::nullability::no,
            ptr_policies::pointer_binding::allowed,
            ptr_policies::reference_binding::allowed,
            ptr_policies::traversal::arithmetic
        >
    > {
    private:
        using base_type = cursor_ptr::ptr_core;
    public:
        using base_type::base_type;
        using base_type::operator=;
    };

    /**
     * @brief Deduction guide for `cursor_ptr`.
     *
     * @tparam T The type of the referenced object.
     *
     * @remark Deduces `T` from the referenced object, preserving cv-qualification.
     */
    template<typename T>
    cursor_ptr(T&) -> cursor_ptr<T>;

    /**
     * @brief Deduction guide for `cursor_ptr`.
     *
     * @tparam T The type of the pointee.
     *
     * @remark Deduces `T` from the pointee, preserving cv-qualification.
     */
    template<typename T>
    cursor_ptr(T*) -> cursor_ptr<T>;
} //namespace base::vocab::inline ptr
#else
export namespace base::vocab::inline ptr {
    /**
     * @brief Pointer type representing a non-null cursor/iterator.
     *
     * @tparam T The pointed-to type (must not be a reference type per "C++ standard [dcl.ptr]", `void`, nor a function pointer).
     *
     * @details `cursor_ptr` models a non-owning, non-nullable, arithmetic, non-void-permitting
     * pointer abstraction. It is designed for random-access and contiguous iteration over objects
     * stored in contiguous memory. Because it exposes memory traversal semantics (pointer
     * arithmetic and ordering), it is not a stable alias to a single object.
     *
     * @note Standard Layout type with size and alignment of a raw pointer.
     * @invariant Always engaged: a `cursor_ptr` always stores a valid memory address; there is no null/disengaged representation. Attempts to construct/assign from another pointer throw on null.
     * @remark This type does not own the pointee object and does not participate in lifetime management.
     * @remark Copying and assignment rebind the stored address without affecting pointee lifetime.
     * @note Construction or assignment from `nullptr` is ill-formed.
     * @note Construction or assignment from pointers and pointer-like values performs runtime validation and throws `std::invalid_argument` on null. All operations provide the Strong Exception Safety Guarantee.
     * @remark Explicit comparison overloads are provided only where implicit conversion to the nested `pointer` type is insufficient to enable the desired comparison.
     *
     * @warning The referenced object MUST outlive the `cursor_ptr`. Violating this results in undefined behavior.
     * @warning Any operation that invalidates the underlying contiguous storage also invalidates associated `cursor_ptr` instances.
     *
     * @see `alias_ptr` for nullable aliasing, `required_ptr` for non-null aliasing, `dependency_ptr` for dependency injection structural non-nullability, `std::unique_ptr` and `std::shared_ptr` for ownership.
     */
    template<typename T>
        requires (
            !std::is_reference_v<T> && !std::is_void_v<T>
            && !std::is_function_v<base::meta::traits::remove_all_indirections_t<T>>
        )
    class cursor_ptr {
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
         * @brief The raw pointer type of the stored address (`T*`).
         */
        using pointer = std::add_pointer_t<T>;

        /**
         * @typedef reference
         * @brief The reference type (`T&`).
         */
        using reference = std::add_lvalue_reference_t<T>;

        /**
         * @typedef rvalue_reference
         * @brief The rvalue reference type (`T&&`).
         *
         * @note Used only for deletion of invalid overloads to prevent binding to temporaries.
         */
        using rvalue_reference = std::add_rvalue_reference_t<T>;

        /**
         * @typedef difference_type
         * @brief Pointer difference type (`std::ptrdiff_t`).
         */
        using difference_type = std::ptrdiff_t;

        using iterator_concept = std::contiguous_iterator_tag;

        using iterator_category = std::random_access_iterator_tag;

    private:
        pointer address_;

    public:
        //================================================================================
        // Constructors and Assignment Operators
        //================================================================================

        ///@brief Constructs a `cursor_ptr` bound to an existing object.
        constexpr explicit cursor_ptr(reference source) noexcept : address_(std::addressof(source)) {}

        ///@brief (Conversion) Implicitly converts from another `cursor_ptr` according to nested `pointer` type conversions.
        template<typename U>
            requires (!std::same_as<U, T>) && std::convertible_to<std::add_pointer_t<U>, pointer>
        constexpr explicit(false) cursor_ptr(const cursor_ptr<U>& source) noexcept : address_(source.get())
        {}

        ///@brief Implicitly converts from a raw `pointer`.
        template<typename P>
            requires std::is_pointer_v<std::remove_cvref_t<P>> && std::convertible_to<std::decay_t<P>, pointer>
        constexpr explicit(false) cursor_ptr(P&& source) : address_(check_for_null(source))
        {}

        ///@brief Implicitly converts from another pointer-like type.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, cursor_ptr>)
                  && requires(Pointer<Element, Args...> ptr) {
                         { std::as_const(ptr).get() } -> std::convertible_to<pointer>;
                     }
        constexpr explicit(false) cursor_ptr(const Pointer<Element, Args...>& source) : address_(check_for_null(source.get()))
        {}

        ///@brief Rebinds the `cursor_ptr` to another object.
        template<typename Self>
            requires (!std::is_const_v<Self>)
        constexpr Self& operator=(this Self& self, reference source) noexcept
        {
            self.address_ = std::addressof(source);
            return self;
        }

        ///@brief (Conversion) Assigns from another `cursor_ptr` according to nested `pointer` type conversions.
        template<typename Self, typename U>
            requires (!std::is_const_v<Self>) && (!std::same_as<U, T>) && std::convertible_to<std::add_pointer_t<U>, pointer>
        constexpr Self& operator=(this Self& self, const cursor_ptr<U>& source) noexcept
        {
            self.address_ = source.get();
            return self;
        }

        ///@brief Assigns from a raw `pointer`.
        template<typename Self, typename P>
            requires (!std::is_const_v<Self>)
                  && std::is_pointer_v<std::remove_cvref_t<P>> && std::convertible_to<std::decay_t<P>, pointer>
        constexpr Self& operator=(this Self& self, P&& source)
        {
            self.address_ = check_for_null(source);
            return self;
        }

        ///@brief Assigns from another pointer-like type.
        template<typename Self, template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, cursor_ptr>)
                  && (!std::is_const_v<Self>) && requires(Pointer<Element, Args...> ptr) {
                                                     { std::as_const(ptr).get() } -> std::convertible_to<pointer>;
                                                 }
        constexpr Self& operator=(this Self& self, const Pointer<Element, Args...>& source)
        {
            self.address_ = check_for_null(source.get());
            return self;
        }

        ///@brief Swaps addresses.
        friend constexpr void swap(cursor_ptr& lhs, cursor_ptr& rhs) noexcept
        {
            using std::swap;
            swap(lhs.address_, rhs.address_);
        }

        //================================================================================
        // Deleted Constructors and Assignment Operators: Non-Null Structural Invariant
        //================================================================================

        ///@brief Deleted default constructor to prevent sources of null initialization.
        cursor_ptr() =
            delete /*("Default constructor deleted to prevent null initialization. Use `std::optional<cursor_ptr<T>>` for default-constructible optional dependencies.")*/
            ;

        ///@brief Deleted constructor from `nullptr` to prevent sources of null initialization.
        cursor_ptr(std::nullptr_t) =
            delete /*("Constructor from `nullptr` deleted to prevent null initialization. Use `std::optional<cursor_ptr<T>>` for optional dependencies.")*/
            ;

        ///@brief Deleted assignment from `nullptr` to prevent sources of invalid null rebinding.
        cursor_ptr& operator=(std::nullptr_t) =
            delete /*("Assignment from `nullptr` deleted to prevent null rebinding. Use `std::optional<cursor_ptr<T>>` for optional dependencies.")*/
            ;

        //================================================================================
        // Deleted Constructors and Assignment Operators: No Aliasing Temporaries
        //================================================================================

        ///@brief Deleted constructor from `rvalue_reference` to discourage dangling by rejecting direct binding to temporaries.
        cursor_ptr(rvalue_reference) =
            delete /*("Constructor from `rvalue_reference` deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        ///@brief Deleted constructor from pointer-like object rvalue to discourage dangling by rejecting direct binding to temporaries.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, cursor_ptr>)
                      && requires(Pointer<Element, Args...> ptr) {
                             { std::as_const(ptr).get() } -> std::convertible_to<pointer>;
                         }
        cursor_ptr(const Pointer<Element, Args...>&&) =
            delete /*("Constructor from pointer-like object rvalue deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        ///@brief Deleted assignment from `rvalue_reference` to discourage dangling by rejecting direct binding to temporaries.
        template<typename Self>
        Self& operator=(this Self&&, rvalue_reference) =
            delete /*("Assignment from `rvalue_reference` deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        ///@brief Deleted assignment from pointer-like object rvalue to discourage dangling by rejecting direct binding to temporaries.
        template<typename Self, template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, cursor_ptr>)
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
        cursor_ptr(AnyCArray&) =
            delete /*("Constructor from C-array deleted to prevent array-to-pointer decay. To point to the first element, alias it explicitly.")*/
            ;

        ///@brief Deleted assignment from C-array to prevent array-to-pointer decay.
        template<typename Self, typename AnyCArray>
            requires std::is_array_v<AnyCArray>
        Self& operator=(this Self&&, AnyCArray&) =
            delete /*("Assignment from C-array deleted to prevent array-to-pointer decay. To point to the first element, alias it explicitly.")*/
            ;

        //================================================================================
        // Comparison Operators (Three-way)
        //================================================================================

        ///@brief Compares in terms of pointer identity.
        [[nodiscard]] friend constexpr auto operator<=>(const cursor_ptr& lhs, const cursor_ptr& rhs) noexcept = default;

        ///@brief Covariantly compares in terms of pointer identity.
        template<std::derived_from<T> DerivedT>
            requires (!std::same_as<DerivedT, T>)
        [[nodiscard]] friend constexpr auto operator<=>(const cursor_ptr& lhs, const cursor_ptr<DerivedT>& rhs) noexcept
        {
            return (lhs.get() <=> rhs.get());
        }

        ///@brief Covariantly compares a `cursor_ptr`-to-base with a raw pointer-to-derived in terms of pointer identity.
        template<std::derived_from<T> DerivedT>
        [[nodiscard]] friend constexpr auto operator<=>(const cursor_ptr& lhs, const std::add_pointer_t<DerivedT> rhs) noexcept
        {
            return (lhs.get() <=> rhs);
        }

        ///@brief Covariantly compares a raw pointer-to-base with a `cursor_ptr`-to-derived in terms of pointer identity.
        template<std::derived_from<T> DerivedT>
        [[nodiscard]] friend constexpr auto operator<=>(const pointer lhs, const cursor_ptr<DerivedT>& rhs) noexcept
        {
            return (lhs <=> rhs.get());
        }

        ///@brief Deleted comparison against `nullptr`.
        [[nodiscard]] friend constexpr bool operator==(const cursor_ptr&, std::nullptr_t) noexcept = delete;

        //================================================================================
        // Pointer Operations
        //================================================================================

        ///@brief Provides member access to the pointee object.
        [[nodiscard]] constexpr pointer operator->(this auto&& self) noexcept
            requires base::meta::concepts::CompletePointee<T>
        {
            return self.address_;
        }

        ///@brief Provides a reference to the pointee object.
        [[nodiscard]] constexpr reference operator*(this auto&& self) noexcept
            requires base::meta::concepts::CompletePointee<T>
        {
            return *self.address_;
        }

        ///@brief Returns a raw pointer to the stored address.
        [[nodiscard]] constexpr pointer get(this auto&& self) noexcept { return self.address_; }

        ///@brief Implicitly converts to the nested `pointer` type.
        [[nodiscard]] constexpr explicit(false) operator pointer() const noexcept { return this->get(); }

        ///@brief Contextually converts to `bool` to test if the pointer is engaged. Always returns true to confirm structural invariant.
        [[nodiscard]] constexpr explicit operator bool() const noexcept { return true; }

        ///@brief Deleted `release` to prevent sources of invalid null rebinding.
        [[nodiscard]] constexpr pointer release() =
            delete /*("`release` deleted to prevent null rebinding. Use `std::optional<cursor_ptr<T>>` for optionality.")*/;

        ///@brief Rebinding passes through to assignment.
        template<typename Self, typename P>
        Self& rebind(this Self& self, P&& source) noexcept(std::is_nothrow_assignable_v<Self&, P>)
            requires std::is_assignable_v<Self&, P>
        {
            return self = std::forward<P>(source);
        }

        ///@brief Deleted `reset` from `nullptr` to prevent sources of invalid null rebinding.
        constexpr void reset(this auto& self, std::nullptr_t null = nullptr) =
            delete /*("`reset` to `nullptr` deleted to prevent null rebinding. Use `std::optional<cursor_ptr<T>>` for optionality.")*/
            ;

        ///@brief Resets the pointer to a new address.
        template<typename Self, typename P>
            requires (!std::same_as<P, std::nullptr_t>)
        constexpr void reset(this Self& self, P&& source) noexcept(std::is_nothrow_assignable_v<Self&, P>)
            requires std::is_assignable_v<Self&, P>
        {
            self = std::forward<P>(source);
        }

        //================================================================================
        // Arithmetic Operators: Implemented for Iteration
        //================================================================================

        ///@brief Subscript operator provided solely to comply with random-access iterator requirements.
        [[nodiscard]] [[deprecated(
            "Subscript operator conflates pointers with arrays. Use `*(ptr + offset)` for explicit traversal or consider subscripting the container instead."
        )]]
        constexpr element_type& operator[](this auto self, difference_type offset) noexcept
        {
            return *(self + offset);
        }

        ///@brief Prefix increment: increments the stored address.
        template<typename Self>
        constexpr decltype(auto) operator++(this Self&& self) noexcept
        {
            ++self.address_;
            return std::forward<Self>(self);
        }

        ///@brief Prefix decrement: decrements the stored address.
        template<typename Self>
        constexpr decltype(auto) operator--(this Self&& self) noexcept
        {
            --self.address_;
            return std::forward<Self>(self);
        }

        ///@brief Postfix increment: increments the stored address but return a pointer to the prior stored address.
        template<typename Self>
        constexpr auto operator++(this Self&& self, int) noexcept
        {
            std::decay_t<Self> old{self};
            ++self;
            return old;
        }

        ///@brief Postfix decrement: decrements the stored address but return a pointer to the prior stored address.
        template<typename Self>
        constexpr auto operator--(this Self&& self, int) noexcept
        {
            std::decay_t<Self> old{self};
            --self;
            return old;
        }

        ///@brief Addition assignment: increments the stored address by a given distance.
        template<typename Self>
        constexpr decltype(auto) operator+=(this Self&& self, difference_type diff) noexcept
        {
            self.address_ += diff;
            return std::forward<Self>(self);
        }

        ///@brief Subtraction assignment: decrements the stored address by a given distance.
        template<typename Self>
        constexpr decltype(auto) operator-=(this Self&& self, difference_type diff) noexcept
        {
            self.address_ -= diff;
            return std::forward<Self>(self);
        }

        ///@brief Pointer addition: gets a pointer to an address a given distance after the stored address.
        friend constexpr cursor_ptr operator+(cursor_ptr ptr, difference_type diff) noexcept { return ptr += diff; }

        ///@brief Pointer addition (commutative): gets a pointer to an address a given distance after the stored address.
        friend constexpr cursor_ptr operator+(difference_type diff, cursor_ptr ptr) noexcept { return ptr += diff; }

        ///@brief Pointer subtraction: gets a pointer to an address a given distance before the stored address.
        friend constexpr cursor_ptr operator-(cursor_ptr ptr, difference_type diff) noexcept { return ptr -= diff; }

        ///@brief Pointer subtraction (difference): computes the distance between the addresses stored in two pointers.
        friend constexpr difference_type operator-(cursor_ptr lhs, cursor_ptr rhs) noexcept { return lhs.get() - rhs.get(); }

        //================================================================================
        // Stream Output
        //================================================================================

        ///@brief Outputs a `cursor_ptr` address to a `std::basic_ostream`.
        template<typename CharT, typename Traits>
        friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& stream, const cursor_ptr& ptr)
        {
            // In order to support pointers to arbitrarily cv-qualified objects:
            // 1. `static_cast` to `const volatile void*` to preserve all qualifiers while converting the pointer to `void*`.
            // 2. `const_cast` to `const void*` to satisfy the inserter's interface which lacks `volatile void*` overloads.
            // This is safe because formatting is a read-only numerical operation on the address.
            return stream << const_cast<const void*>(static_cast<const volatile void*>(ptr.get()));
        }

    private:
        [[nodiscard]] constexpr static pointer check_for_null(pointer source)
        {
            if (source == nullptr) [[unlikely]]
                throw std::invalid_argument("`cursor_ptr` cannot be constructed or assigned from a null pointer.");
            return source;
        }
    }; //class cursor_ptr

    /**
     * @fn explicit cursor_ptr::cursor_ptr(reference source) noexcept
     *
     * @param source The object to reference.
     *
     * @pre `source` must refer to a valid object that outlives the constructed `cursor_ptr`.
     * @post `get() == std::addressof(source)`.
     *
     * @remark Establishes the non-null invariant at construction.
     * @remark Prevents binding to temporaries via deleted rvalue overload.
     */
    /**
     * @overload cursor_ptr::cursor_ptr(const cursor_ptr<U>& source) noexcept
     *
     * @tparam U The element type, with its pointer convertible to `pointer`, of the source `cursor_ptr`.
     *
     * @param source The `cursor_ptr` being converted.
     *
     * @pre `source` must point to a valid object that outlives the resulting `cursor_ptr`.
     * @post `get() == source.get()`.
     *
     * @details This single constructor handles:
     * 1. Derived-to-Base conversion (e.g., `ptr<Derived>` to `ptr<Base>`).
     * 2. Qualification conversion (e.g., `ptr<T>` to `ptr<const T>`).
     *
     * @remark Preserves covariance. Converts from cursor_ptr-to-derived to cursor_ptr-to-base implicitly.
     * @remark Preserves the non-null invariant.
     */
    /**
     * @overload explicit cursor_ptr::cursor_ptr(P&& source)
     *
     * @tparam P The raw pointer type which must decay to a type convertible to `pointer`.
     *
     * @param source The raw pointer to bind.
     *
     * @pre `source` must point to a valid object that outlives the constructed `cursor_ptr`.
     * @post `get() == source`.
     *
     * @throws std::invalid_argument if `source == nullptr`. Provides the Strong Exception Safety Guarantee.
     *
     * @details Captures the original argument type prior to decay via
     * forwarding reference so that C-array arguments can be diagnosed
     * explicitly instead of silently decaying to element pointers.
     *
     * @note This constructor is constrained to raw pointers to prevent hijacking copy/move operations or accepting arrays.
     * @remark Establishes the non-null invariant at runtime when constructed from raw pointers.
     */
    /**
     * @overload explicit cursor_ptr::cursor_ptr(const Pointer<Element, Args...>& source)
     *
     * @tparam Pointer A class template modeling a pointer-like type.
     * @tparam Element The element type of the source pointer.
     * @tparam Args Additional template parameters of the pointer type.
     *
     * @param source The pointer-like object providing access to a pointer-compatible value via `get()`.
     *
     * @pre `source.get()` must be a valid expression convertible to `pointer`.
     * @pre `source.get()` must point to an object whose lifetime strictly exceeds that of the constructed `cursor_ptr`.
     * @post `get() == static_cast<pointer>(source.get())`.
     *
     * @throws std::invalid_argument if `source.get() == nullptr`. Provides the Strong Exception Safety Guarantee.
     *
     * @remark Enables interoperation with pointer-like types (e.g., smart pointers) that expose a `get()` member.
     * @remark The source type must not be a specialization of `cursor_ptr` (to avoid ambiguity with existing overloads).
     * @remark This constructor does not transfer ownership and does not affect the lifetime of the underlying object.
     */
    /**
     * @fn constexpr Self& cursor_ptr::operator=(this Self& self, reference source) noexcept
     *
     * @tparam Self The non-const `cursor_ptr` type deduced from the call site.
     *
     * @param self The target `cursor_ptr` being rebound.
     * @param source The object to reference.
     *
     * @return Reference to `self`.
     *
     * @pre `source` must refer to a valid object that outlives the `cursor_ptr`.
     *
     * @remark Rebinds the stored address without affecting ownership or pointee lifetime.
     */
    /**
     * @overload constexpr Self& cursor_ptr::operator=(this Self& self, const cursor_ptr<U>& source) noexcept
     *
     * @tparam Self The non-const `cursor_ptr` type deduced from the call site.
     * @tparam U The element type, with its pointer convertible to `pointer`, of the source `cursor_ptr`.
     *
     * @param self The target `cursor_ptr` being rebound.
     * @param source The `cursor_ptr` being converted.
     *
     * @return Reference to `self`.
     *
     * @pre `source` must point to a valid object that outlives the resulting `cursor_ptr`.
     *
     * @details This single assignment operator handles:
     * 1. Derived-to-Base conversion (e.g., `ptr<Derived>` to `ptr<Base>`).
     * 2. Qualification conversion (e.g., `ptr<T>` to `ptr<const T>`).
     *
     * @remark Preserves covariance. Converts from cursor_ptr-to-derived to cursor_ptr-to-base implicitly.
     * @remark Preserves the non-null invariant.
     */
    /**
     * @overload constexpr Self& cursor_ptr::operator=(this Self& self, P&& source)
     *
     * @tparam Self The non-const `cursor_ptr` type deduced from the call site.
     * @tparam P The raw pointer type which must decay to a type convertible to `pointer`.
     *
     * @param self The target `cursor_ptr` being rebound.
     * @param source The raw pointer to rebind to.
     *
     * @return Reference to `self`.
     *
     * @pre `source` must point to a valid object that outlives the `cursor_ptr`.
     * @post `self.get() == source`.
     *
     * @throws std::invalid_argument if `source == nullptr`. Provides the Strong Exception Safety Guarantee.
     *
     * @details Captures the original argument type prior to decay via
     * forwarding reference so that C-array arguments can be diagnosed
     * explicitly instead of silently decaying to element pointers.
     *
     * @note This assignment operator is constrained to raw pointers to prevent hijacking copy/move operations or accepting arrays.
     * @remark Rebinds the pointer while preserving the non-null invariant.
     * @remark Does not affect the lifetime of the referenced object.
     */
    /**
     * @overload constexpr Self& cursor_ptr::operator=(this Self& self, const Pointer<Element, Args...>& source)
     *
     * @tparam Self The cv-qualified `cursor_ptr` type deduced from the call site.
     * @tparam Pointer A class template modeling a pointer-like type.
     * @tparam Element The element type of the source pointer.
     * @tparam Args Additional template parameters of the pointer type.
     *
     * @param self The target `cursor_ptr` being rebound.
     * @param source The pointer-like object providing access to a raw pointer via `get()`.
     *
     * @return Reference to `self`.
     *
     * @pre `source.get()` must be a valid expression convertible to `pointer`.
     * @pre `source.get()` must point to an object whose lifetime strictly exceeds that of the `cursor_ptr`.
     * @post `self.get() == static_cast<pointer>(source.get())`.
     *
     * @throws std::invalid_argument if `source.get() == nullptr`. Provides the Strong Exception Safety Guarantee.
     *
     * @remark Rebinds the `cursor_ptr` from a pointer-like source while preserving the non-null invariant.
     * @remark Does not transfer ownership and does not affect the lifetime of the underlying object.
     */
    /**
     * @fn constexpr void swap(cursor_ptr& lhs, cursor_ptr& rhs) noexcept
     *
     * @param lhs The first pointer object.
     * @param rhs The second pointer object.
     *
     * @post `lhs` refers to the object previously referenced by `rhs`.
     * @post `rhs` refers to the object previously referenced by `lhs`.
     *
     * @remark Swaps only the stored addresses.
     * @remark Does not affect ownership or pointee lifetime.
     * @remark Provided as a hidden friend for ADL interoperability.
     */
    /**
     * @fn constexpr auto operator<=>(const cursor_ptr& lhs, const cursor_ptr& rhs) noexcept
     *
     * @param lhs The left-hand side `cursor_ptr`.
     * @param rhs The right-hand side `cursor_ptr`.
     *
     * @return The three-way comparison result of the stored addresses.
     *
     * @remark Compares pointer identity (stored addresses), not pointee object values.
     * @remark Defaulted comparison preserving raw pointer ordering semantics.
     */
    /**
     * @overload constexpr auto operator<=>(const cursor_ptr& lhs, const cursor_ptr<DerivedT>& rhs) noexcept
     *
     * @tparam DerivedT The element type, derived from `T`, of the right-hand side `cursor_ptr`.
     *
     * @param lhs The left-hand side `cursor_ptr`.
     * @param rhs The right-hand side `cursor_ptr` to compare.
     *
     * @return The three-way comparison result of the stored addresses.
     *
     * @remark Enables comparison between `cursor_ptr` instances of related types when that can't be synthesized by implicit conversion to raw pointers.
     * @remark Compares pointer identity (stored addresses), not pointee object values.
     */
    /**
     * @overload constexpr auto operator<=>(const cursor_ptr& lhs, const std::add_pointer_t<DerivedT> rhs) noexcept
     *
     * @tparam DerivedT The element type, derived from `T`, of the raw pointer.
     *
     * @param lhs The `cursor_ptr` being compared.
     * @param rhs The raw pointer to compare against.
     *
     * @return The three-way comparison result of the stored addresses.
     *
     * @remark Enables comparison with raw pointers-to-derived when that can't be synthesized by implicit conversion to raw pointers.
     * @remark Compares pointer identity (stored addresses), not pointee object values.
     */
    /**
     * @overload constexpr auto operator<=>(const pointer lhs, const cursor_ptr<DerivedT>& rhs) noexcept
     *
     * @tparam DerivedT The element type, derived from `T`, of the right-hand side `cursor_ptr`.
     *
     * @param lhs The raw pointer to compare.
     * @param rhs The `cursor_ptr` being compared.
     *
     * @return The three-way comparison result of the stored addresses.
     *
     * @remark Enables comparison with raw pointers-to-base when that can't be synthesized by implicit conversion to raw pointers.
     * @remark Compares pointer identity (stored addresses), not pointee object values.
     */
    /**
     * @fn constexpr pointer cursor_ptr::operator->(this auto&& self) noexcept
     *
     * @param self The `cursor_ptr` providing member access.
     *
     * @return A raw pointer to the stored address.
     *
     * @pre `self.get() != nullptr`
     * @pre The pointee object must remain valid (non-dangling).
     *
     * @remark Provides pointee member access semantics.
     */
    /**
     * @fn constexpr reference cursor_ptr::operator*(this auto&& self) noexcept
     *
     * @param self The `cursor_ptr` being dereferenced.
     *
     * @return Reference to the pointee object.
     *
     * @pre The pointee object must remain valid (non-dangling).
     *
     * @remark Equivalent to dereferencing `self.get()`.
     */
    /**
     * @fn constexpr pointer cursor_ptr::get(this auto&& self) noexcept
     *
     * @param self The `cursor_ptr` exposing its stored address.
     *
     * @return The underlying raw pointer.
     *
     * @post The returned pointer is non-null.
     *
     * @remark Provided for interoperability with pointer-based APIs.
     */
    /**
     * @fn constexpr cursor_ptr::operator pointer() const noexcept
     *
     * @return The underlying raw pointer.
     *
     * @post The returned pointer is non-null.
     *
     * @remark Enables seamless interoperability with legacy interfaces expecting raw pointers.
     * @warning Implicit conversion may obscure the non-owning, non-null semantics; prefer `get()` when clarity is important.
     */
    /**
     * @fn constexpr cursor_ptr::operator bool() const noexcept
     *
     * @return `true` (The pointer is structurally guaranteed to always be engaged.)
     *
     * @post Always evaluates to `true`.
     *
     * @remark Satisfies boolean-testable requirements in generic templates and logical contexts.
     * @remark Models pointer interface by providing contextual conversion to `bool` albeit redundantly.
     * @note Because this always returns `true`, the compiler may elide checks in generic code when `cursor_ptr` is the concrete type.
     * @note This does not indicate engagement/optionality as `cursor_ptr` has no disengaged state.
     */
    /**
     * @fn constexpr Self& cursor_ptr::rebind(this Self& self, P&& source)
     *
     * @tparam Self The non-const `cursor_ptr` type deduced from the call site.
     * @tparam P A pointer-compatible source type assignable to `cursor_ptr`.
     *
     * @param self The `cursor_ptr` being rebound.
     * @param source The source used to replace the stored address.
     *
     * @return Reference to `self`.
     *
     * @post Equivalent to `self = std::forward<P>(source)`.
     *
     * @remark Convenience wrapper over assignment for generic pointer-like interoperability.
     */
    /**
     * @fn constexpr void cursor_ptr::reset(this Self& self, P&& source)
     *
     * @tparam Self The non-const `cursor_ptr` type deduced from the call site.
     * @tparam P A pointer-compatible source type assignable to `cursor_ptr`.
     *
     * @param self The `cursor_ptr` being rebound.
     * @param source The source used to replace the stored address.
     *
     * @post Equivalent to `self = std::forward<P>(source)`.
     *
     * @remark Replaces the stored address without affecting ownership or pointee lifetime.
     */
    /**
     * @fn constexpr element_type& cursor_ptr::operator[](this auto self, difference_type offset) noexcept
     *
     * @param self The `cursor_ptr` whose stored address is the base of the offset address.
     * @param offset The offset to add to the base address to compute the address to dereference.
     *
     * @return Reference to the pointee object located at `self.get() + offset`.
     *
     * @note This operator is provided solely to fulfill the requirements of `std::random_access_iterator`.
     * @deprecated Applying the subscript operator to a pointer conflates pointers with arrays. Use `*(ptr + offset)` for explicit traversal or consider subscripting the container instead.
     * @warning Indexing beyond the bounds of the referenced contiguous sequence results in undefined behavior.
     */
    /**
     * @fn constexpr Self& cursor_ptr::operator++(this Self&& self) noexcept
     *
     * @tparam Self The non-const `cursor_ptr` type deduced from the call site.
     *
     * @param self The `cursor_ptr` whose stored address is incremented.
     *
     * @return Reference to `self`.
     *
     * @post `self.get()` refers to the next contiguous element.
     *
     * @remark Advances the stored address using built-in pointer increment semantics.
     * @warning Advancing beyond the bounds of the referenced contiguous sequence results in undefined behavior.
     */
    /**
     * @overload constexpr Self cursor_ptr::operator++(this Self&& self, int) noexcept
     *
     * @tparam Self The non-const `cursor_ptr` type deduced from the call site.
     *
     * @param self The `cursor_ptr` whose stored address is incremented.
     *
     * @return A copy of the `cursor_ptr` prior to increment.
     *
     * @post `self.get()` refers to the next contiguous element.
     *
     * @remark Advances the stored address using built-in pointer increment semantics.
     * @warning Advancing beyond the bounds of the referenced contiguous sequence results in undefined behavior.
     */
    /**
     * @fn constexpr Self& cursor_ptr::operator--(this Self&& self) noexcept
     *
     * @tparam Self The non-const `cursor_ptr` type deduced from the call site.
     *
     * @param self The `cursor_ptr` whose stored address is decremented.
     *
     * @return Reference to `self`.
     *
     * @post `self.get()` refers to the previous contiguous element.
     *
     * @remark Retreats the stored address using built-in pointer decrement semantics.
     * @warning Decrementing before the bounds of the referenced contiguous sequence results in undefined behavior.
     */
    /**
     * @overload constexpr Self cursor_ptr::operator--(this Self&& self, int) noexcept
     *
     * @tparam Self The non-const `cursor_ptr` type deduced from the call site.
     *
     * @param self The `cursor_ptr` whose stored address is decremented.
     *
     * @return A copy of the `cursor_ptr` prior to decrement.
     *
     * @post `self.get()` refers to the previous contiguous element.
     *
     * @remark Retreats the stored address using built-in pointer decrement semantics.
     * @warning Decrementing before the bounds of the referenced contiguous sequence results in undefined behavior.
     */
    /**
     * @fn constexpr Self& cursor_ptr::operator+=(this Self&& self, difference_type diff) noexcept
     *
     * @tparam Self The non-const `cursor_ptr` type deduced from the call site.
     *
     * @param self The `cursor_ptr` whose stored address is advanced.
     * @param diff The signed offset, in elements, to add to the stored address.
     *
     * @return Reference to `self`.
     *
     * @post `self.get() == std::next(old(self.get()), diff)`.
     *
     * @remark Advances the stored address by `diff` elements using built-in pointer arithmetic semantics.
     * @warning Advancing outside the bounds of the referenced contiguous sequence results in undefined behavior.
     */
    /**
     * @fn constexpr Self& cursor_ptr::operator-=(this Self&& self, difference_type diff) noexcept
     *
     * @tparam Self The non-const `cursor_ptr` type deduced from the call site.
     *
     * @param self The `cursor_ptr` whose stored address is retreated.
     * @param diff The signed offset, in elements, to subtract from the stored address.
     *
     * @return Reference to `self`.
     *
     * @post `self.get() == std::prev(old(self.get()), diff)`.
     *
     * @remark Retreats the stored address by `diff` elements using built-in pointer arithmetic semantics.
     * @warning Retreating outside the bounds of the referenced contiguous sequence results in undefined behavior.
     */
    /**
     * @fn constexpr cursor_ptr operator+(cursor_ptr ptr, difference_type diff) noexcept
     *
     * @param ptr The base `cursor_ptr`.
     * @param diff The signed offset, in elements, to add to the stored address.
     *
     * @return A new `cursor_ptr` referring to the address `diff` elements after `ptr`.
     *
     * @remark Computes an offset pointer using built-in pointer arithmetic semantics.
     * @remark Does not modify the original `ptr`.
     * @warning Advancing outside the bounds of the referenced contiguous sequence results in undefined behavior.
     */
    /**
     * @overload constexpr cursor_ptr operator+(difference_type diff, cursor_ptr ptr) noexcept
     *
     * @param diff The signed offset, in elements, to add to the stored address.
     * @param ptr The base `cursor_ptr`.
     *
     * @return A new `cursor_ptr` referring to the address `diff` elements after `ptr`.
     *
     * @remark Provides commutative addition syntax for pointer arithmetic.
     * @remark Does not modify the original `ptr`.
     * @warning Advancing outside the bounds of the referenced contiguous sequence results in undefined behavior.
     */
    /**
     * @fn constexpr cursor_ptr operator-(cursor_ptr ptr, difference_type diff) noexcept
     *
     * @param ptr The base `cursor_ptr`.
     * @param diff The signed offset, in elements, to subtract from the stored address.
     *
     * @return A new `cursor_ptr` referring to the address `diff` elements before `ptr`.
     *
     * @remark Computes an offset pointer using built-in pointer arithmetic semantics.
     * @remark Does not modify the original `ptr`.
     * @warning Retreating outside the bounds of the referenced contiguous sequence results in undefined behavior.
     */
    /**
     * @fn constexpr difference_type operator-(cursor_ptr lhs, cursor_ptr rhs) noexcept
     *
     * @param lhs The left-hand side `cursor_ptr`.
     * @param rhs The right-hand side `cursor_ptr`.
     *
     * @return The distance, in elements, between the stored addresses.
     *
     * @remark Equivalent to `lhs.get() - rhs.get()`.
     * @remark The result is positive when `lhs` refers to a later element than `rhs`.
     * @warning Subtracting pointers that do not refer into the same contiguous sequence results in undefined behavior.
     */
    /**
     * @fn constexpr static pointer cursor_ptr::check_for_null(pointer source)
     *
     * @param source The raw pointer to validate.
     *
     * @return The same pointer value if non-null.
     *
     * @post The returned pointer is guaranteed to be non-null.
     *
     * @throws std::invalid_argument if `source == nullptr`. Provides the Strong Exception Safety Guarantee.
     *
     * @remark Centralizes enforcement of the non-null invariant for all constructors and assignment operators accepting raw or pointer-like inputs.
     * @remark Marked `[[nodiscard]]` to discourage accidental ignoring of the validated result.
     * @remark Defined as a private static helper to avoid duplication and ensure consistent exception semantics.
     * @note This function does not perform lifetime validation; it assumes the caller ensures the pointee remains valid.
     */

    /**
     * @brief Deduction guide for `cursor_ptr`.
     *
     * @tparam T The type of the referenced object.
     *
     * @remark Deduces `T` from the referenced object, preserving cv-qualification.
     */
    template<typename T>
    cursor_ptr(T&) -> cursor_ptr<T>;

    /**
     * @brief Deduction guide for `cursor_ptr`.
     *
     * @tparam T The type of the pointee.
     *
     * @remark Deduces `T` from the pointee, preserving cv-qualification.
     */
    template<typename T>
    cursor_ptr(T*) -> cursor_ptr<T>;
} //namespace base::vocab::inline ptr

/**
 * @brief Partial specialization of `std::hash` for `cursor_ptr`.
 *
 * @tparam T The element type of the `cursor_ptr`.
 *
 * @remark Hashes the underlying stored address rather than pointee object state or values.
 * @remark Consistent with `cursor_ptr` equality semantics.
 */
export template<class T>
struct std::hash<base::vocab::ptr::cursor_ptr<T>> {
    ///@brief Hashes the `cursor_ptr` based on the underlying address.
    [[nodiscard]] constexpr std::size_t operator()(const base::vocab::ptr::cursor_ptr<T>& ptr) const noexcept
    {
        return std::hash<T*>{}(ptr.get());
    }
};

/**
 * @brief Partial specialization of `std::formatter` for `cursor_ptr`.
 *
 * @tparam T The element type of the `cursor_ptr`.
 * @tparam CharT The character type used by the format string.
 *
 * @remark Formats the stored address according to the rules for its nested `pointer` type.
 */
export template<typename T, typename CharT>
struct std::formatter<base::vocab::ptr::cursor_ptr<T>, CharT> : std::formatter<const void*, CharT> {
    ///@brief Formats as a raw pointer to the stored address.
    auto format(const base::vocab::ptr::cursor_ptr<T>& ptr, auto& ctx) const
    {
        // In order to support pointers to arbitrarily cv-qualified objects:
        // 1. `static_cast` to `const volatile void*` to preserve all qualifiers while converting the pointer to `void*`.
        // 2. `const_cast` to `const void*` to satisfy the formatter's interface which lacks `volatile void*` specializations.
        // This is safe because formatting is a read-only numerical operation on the address.
        return std::formatter<const void*, CharT>::format(
            const_cast<const void*>(static_cast<const volatile void*>(ptr.get())), ctx
        );
    }
};
#endif
