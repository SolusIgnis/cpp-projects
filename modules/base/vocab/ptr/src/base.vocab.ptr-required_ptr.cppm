// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-required_ptr.cppm
 * @version 0.3.0
 * @date April 27, 2026
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
 * @brief `required_ptr`: A non-owning, non-nullable, non-arithmetic, void-permitting pointer type.
 *
 * @details `required_ptr` is a vocabulary pointer type representing
 * a non-owning, structurally non-null object alias. It is intended to
 * replace raw pointers as non-owning object aliases and should be
 * preferred over `alias_ptr` in contexts where the alias is required
 * to be non-null as a precondition or invariant.
 *
 * Unlike raw pointers, `required_ptr` cannot be default-constructed,
 * assigned `nullptr`, or participate in pointer arithmetic. These
 * restrictions intentionally model "required object alias" semantics
 * rather than general-purpose address manipulation.
 *
 * The type preserves pointer-like ergonomics through dereference,
 * member access, raw-pointer interoperability, and covariant conversion
 * between compatible pointee types while enforcing a permanent engaged
 * invariant.
 *
 * Construction and rebinding from raw or pointer-like sources perform
 * runtime null validation and throw `std::invalid_argument` when a null
 * value is supplied.
 *
 * `required_ptr` does not own the referenced object and performs no
 * lifetime management. Users are responsible for ensuring the referenced
 * object outlives all aliases.
 *
 * Ordering comparisons and pointer arithmetic are intentionally deleted
 * to discourage misuse as an iterator or contiguous traversal type.
 *
 * `required_ptr<void>` is supported through the primary template to
 * enable type-erased non-null aliasing without introducing a dedicated
 * specialization.
 *
 * @todo Future Development: Use `= delete("reason")` instead of the C-style comments once the C++26 feature becomes available.
 */

//Module partition interface unit
export module base.vocab.ptr:required_ptr;

import std;

import base.meta.traits;

import :forward_declarations;

export namespace base::vocab::inline ptr {
    /**
     * @brief Pointer type representing a required object alias.
     *
     * @tparam T The pointed-to type (must not be a reference type per "C++ standard [dcl.ptr]" nor a function pointer).
     *
     * @details `required_ptr` models a non-owning, non-nullable, non-arithmetic, void-permitting
     * pointer abstraction.
     *
     * @note Standard Layout type with size and alignment of a raw pointer.
     * @invariant Always engaged: a `required_ptr` always refers to a valid object; there is no null/disengaged representation. Attempts to construct/assign from another pointer throw on null.
     * @remark This type does not own the referenced object and does not participate in lifetime management.
     * @remark Copying and assignment rebind the pointer without affecting the lifetime of the underlying object.
     * @note Construction or assignment from `nullptr` is ill-formed.
     * @note Construction or assignment from raw pointer values performs runtime validation and throws `std::invalid_argument` on null. All operations provide the Strong Exception Safety Guarantee.
     * @note Non-arithmetic Pointer: Pointer arithmetic and ordering comparisons are intentionally disabled to prevent misuse as an iterator.
     * @note `required_ptr<void>` reuses the primary template rather than introducing a specialization. Placeholder reference aliases based on `std::monostate` are used solely to keep deleted overload declarations well-formed.
     * @remark Explicit equality comparison overloads are provided only where built-in pointer comparison cannot be reached through the implicit raw-pointer conversion operator alone.
     *
     * @warning The referenced object MUST outlive the `required_ptr`. Violating this results in undefined behavior.
     *
     * @see `alias_ptr` for nullable aliasing, `dependency_ptr` for dependency injection structural non-nullability, `cursor_ptr` for non-null iteration/traversal, `std::unique_ptr` and `std::shared_ptr` for ownership.
     */
    template<typename T>
        requires (!std::is_reference_v<T> && !std::is_function_v<base::meta::traits::remove_all_indirections_t<T>>)
    class required_ptr {
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

        ///@brief Constructs a `required_ptr` bound to an existing object.
        constexpr explicit required_ptr(reference source) noexcept
            requires (!std::is_void_v<T>)
            : address_(std::addressof(source))
        {}

        ///@brief (Conversion) Implicitly converts from another `required_ptr` according to underlying pointer conversions.
        template<typename U>
            requires (!std::same_as<U, T>) && std::convertible_to<std::add_pointer_t<U>, pointer>
        constexpr explicit(false) required_ptr(const required_ptr<U>& source) noexcept : address_(source.get())
        {}

        ///@brief Implicitly converts from a raw `pointer`. Explicit when `T` is void to avoid implicit conversion chaining.
        template<typename P>
            requires std::is_pointer_v<std::remove_cvref_t<P>> && std::convertible_to<std::decay_t<P>, pointer>
        constexpr explicit(std::is_void_v<T>) required_ptr(P&& source) : address_(check_for_null(source))
        {}

        ///@brief Implicitly converts from another wrapped/smart pointer type. Explicit when `T` is void to avoid implicit conversion chaining.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, required_ptr>)
                  && requires(Pointer<Element, Args...> ptr) {
                         { std::as_const(ptr).get() } -> std::convertible_to<pointer>;
                     }
        constexpr explicit(std::is_void_v<T>) required_ptr(const Pointer<Element, Args...>& source)
            : address_(check_for_null(source.get()))
        {}

        ///@brief Rebinds the `required_ptr` to another object.
        required_ptr& operator=(reference source) noexcept
            requires (!std::is_void_v<T>)
        {
            address_ = std::addressof(source);
            return *this;
        }

        ///@brief (Conversion) Assigns from another `required_ptr` according to underlying pointer conversions.
        template<typename U>
            requires (!std::same_as<U, T>) && std::convertible_to<std::add_pointer_t<U>, pointer>
        constexpr required_ptr& operator=(const required_ptr<U>& source) noexcept
        {
            address_ = source.get();
            return *this;
        }

        ///@brief Assigns from a raw `pointer`.
        template<typename P>
            requires std::is_pointer_v<std::remove_cvref_t<P>> && std::convertible_to<std::decay_t<P>, pointer>
        constexpr required_ptr& operator=(P&& source)
        {
            address_ = check_for_null(source);
            return *this;
        }

        ///@brief Assigns from another wrapped/smart pointer type.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, required_ptr>)
                  && requires(Pointer<Element, Args...> ptr) {
                         { std::as_const(ptr).get() } -> std::convertible_to<pointer>;
                     }
        constexpr required_ptr& operator=(const Pointer<Element, Args...>& source)
        {
            address_ = check_for_null(source.get());
            return *this;
        }

        ///@brief Swaps addresses.
        friend constexpr void swap(required_ptr& lhs, required_ptr& rhs) noexcept
        {
            using std::swap;
            swap(lhs.address_, rhs.address_);
        }

        //================================================================================
        // Deleted Constructors and Assignment Operators: Non-Null Structural Invariant
        //================================================================================

        ///@brief Deleted default constructor to prevent sources of null initialization.
        required_ptr() =
            delete /*("Default constructor deleted to prevent null initialization. Use `std::optional<required_ptr<T>>` for default-constructible optional dependencies.")*/
            ;

        ///@brief Deleted constructor from `nullptr` to prevent sources of null initialization.
        required_ptr(std::nullptr_t) =
            delete /*("Constructor from `nullptr` deleted to prevent null initialization. Use `std::optional<required_ptr<T>>` for optional dependencies.")*/
            ;

        ///@brief Deleted assignment from `nullptr` to prevent sources of invalid null rebinding.
        required_ptr& operator=(std::nullptr_t) =
            delete /*("Assignment from `nullptr` deleted to prevent null rebinding. Use `std::optional<required_ptr<T>>` for optional dependencies.")*/
            ;

        //================================================================================
        // Deleted Constructors and Assignment Operators: No Aliasing Temporaries
        //================================================================================

        ///@brief Deleted constructor from `rvalue_reference` to discourage dangling by rejecting direct binding to temporaries.
        required_ptr(rvalue_reference) =
            delete /*("Constructor from `rvalue_reference` deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        ///@brief Deleted constructor from pointer-like object rvalue to discourage dangling by rejecting direct binding to temporaries.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, required_ptr>)
                      && requires(Pointer<Element, Args...> ptr) {
                             { std::as_const(ptr).get() } -> std::convertible_to<pointer>;
                         }
        required_ptr(const Pointer<Element, Args...>&&) =
            delete /*("Constructor from pointer-like object rvalue deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        ///@brief Deleted assignment from `rvalue_reference` to discourage dangling by rejecting direct binding to temporaries.
        required_ptr& operator=(rvalue_reference) =
            delete /*("Assignment from `rvalue_reference` deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        ///@brief Deleted assignment from pointer-like object rvalue to discourage dangling by rejecting direct binding to temporaries.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, required_ptr>)
                      && requires(Pointer<Element, Args...> ptr) {
                             { std::as_const(ptr).get() } -> std::convertible_to<pointer>;
                         }
        required_ptr& operator=(const Pointer<Element, Args...>&&) =
            delete /*("Assignment from pointer-like object rvalue deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        //================================================================================
        // Deleted Constructors and Assignment Operators: No Array-to-Pointer Decay
        //================================================================================

        ///@brief Deleted constructor from C-array to prevent array-to-pointer decay.
        template<typename AnyCArray>
            requires std::is_array_v<AnyCArray>
        required_ptr(AnyCArray&) =
            delete /*("Constructor from C-array deleted to prevent array-to-pointer decay. To point to the first element, alias it explicitly.")*/
            ;

        ///@brief Deleted assignment from C-array to prevent array-to-pointer decay.
        template<typename AnyCArray>
            requires std::is_array_v<AnyCArray>
        required_ptr& operator=(AnyCArray&) =
            delete /*("Assignment from C-array deleted to prevent array-to-pointer decay. To point to the first element, alias it explicitly.")*/
            ;

        //================================================================================
        // Comparison Operators (Equality Allowed, Others Deleted)
        //================================================================================

        ///@brief Compares equality in terms of pointer identity.
        [[nodiscard]] friend constexpr bool operator==(const required_ptr& lhs, const required_ptr& rhs) noexcept = default;

        ///@brief Covariantly compares equality in terms of pointer identity.
        template<std::derived_from<T> DerivedT>
            requires (!std::same_as<DerivedT, T>)
        [[nodiscard]] friend constexpr bool operator==(const required_ptr& lhs, const required_ptr<DerivedT>& rhs) noexcept
        {
            return (lhs.get() == rhs.get());
        }

        ///@brief Covariantly compares equality of a `required_ptr`-to-base with a raw pointer-to-derived in terms of pointer identity.
        template<std::derived_from<T> DerivedT>
        [[nodiscard]] friend constexpr bool operator==(const required_ptr& lhs, const std::add_pointer_t<DerivedT> rhs) noexcept
        {
            return (lhs.get() == rhs);
        }

        ///@brief Covariantly compares equality of a raw pointer-to-base with a `required_ptr`-to-derived in terms of pointer identity.
        template<std::derived_from<T> DerivedT>
        [[nodiscard]] friend constexpr bool operator==(const pointer lhs, const required_ptr<DerivedT>& rhs) noexcept
        {
            return (lhs == rhs.get());
        }

        ///@brief Deleted comparison operators to prevent misuse as an iterator or ordered value type.
        auto operator<=>(required_ptr) const =
            delete /*("Comparison operators deleted to prevent address comparisons. `required_ptr` is not an iterator.")*/;

        ///@brief Deleted comparison operators to prevent misuse as an iterator or ordered value type.
        auto operator<=>(const pointer) const =
            delete /*("Comparison operators deleted to prevent address comparisons. `required_ptr` is not an iterator.")*/;

        //================================================================================
        // Pointer Operations
        //================================================================================

        ///@brief Provides pointer-like member access to the referenced object.
        [[nodiscard]] constexpr pointer operator->() const noexcept
            requires (!std::is_void_v<T>)
        {
            return address_;
        }

        ///@brief Dereferences the pointer to access the referenced object.
        [[nodiscard]] constexpr reference operator*() const noexcept
            requires (!std::is_void_v<T>)
        {
            return *address_;
        }

        ///@brief Returns the underlying raw pointer.
        [[nodiscard]] constexpr pointer get() const noexcept { return address_; }

        ///@brief Implicitly converts to the underlying raw pointer type.
        [[nodiscard]] constexpr explicit(false) operator pointer() const noexcept { return this->get(); }

        ///@brief Contextually converts to `bool` to test if the pointer is engaged. Always returns true to confirm structural invariant.
        [[nodiscard]] constexpr explicit operator bool() const noexcept { return true; }

        //================================================================================
        // Deleted Pointer Arithmetic Operators: Not an Iterator
        //================================================================================

        ///@brief Deleted prefix increment to prevent misuse as an iterator.
        required_ptr& operator++() =
            delete /*("Prefix increment deleted to prevent pointer arithmetic. `required_ptr` is not an iterator.")*/;

        ///@brief Deleted prefix decrement to prevent misuse as an iterator.
        required_ptr& operator--() =
            delete /*("Prefix decrement deleted to prevent pointer arithmetic. `required_ptr` is not an iterator.")*/;

        ///@brief Deleted postfix increment to prevent misuse as an iterator.
        required_ptr operator++(int) =
            delete /*("Postfix increment deleted to prevent pointer arithmetic. `required_ptr` is not an iterator.")*/;

        ///@brief Deleted postfix decrement to prevent misuse as an iterator.
        required_ptr operator--(int) =
            delete /*("Postfix decrement deleted to prevent pointer arithmetic. `required_ptr` is not an iterator.")*/;

        ///@brief Deleted addition assignment to prevent misuse as an iterator.
        required_ptr& operator+=(difference_type) =
            delete /*("Addition assignment deleted to prevent pointer arithmetic. `required_ptr` is not an iterator.")*/;

        ///@brief Deleted subtraction assignment to prevent misuse as an iterator.
        required_ptr& operator-=(difference_type) =
            delete /*("Subtraction assignment deleted to prevent pointer arithmetic. `required_ptr` is not an iterator.")*/;

        ///@brief Deleted pointer addition to prevent misuse as an iterator.
        friend required_ptr operator+(required_ptr, difference_type) =
            delete /*("Pointer addition deleted to prevent pointer arithmetic. `required_ptr` is not an iterator.")*/;

        ///@brief Deleted pointer addition to prevent misuse as an iterator.
        friend required_ptr operator+(difference_type, required_ptr) =
            delete /*("Pointer addition deleted to prevent pointer arithmetic. `required_ptr` is not an iterator.")*/;

        ///@brief Deleted pointer subtraction to prevent misuse as an iterator.
        friend difference_type operator-(required_ptr, required_ptr) =
            delete /*("Pointer subtraction deleted to prevent pointer arithmetic. `required_ptr` is not an iterator.")*/;

        ///@brief Deleted pointer subtraction to prevent misuse as an iterator.
        friend required_ptr operator-(required_ptr, difference_type) =
            delete /*("Pointer subtraction deleted to prevent pointer arithmetic. `required_ptr` is not an iterator.")*/;

    private:
        [[nodiscard]] constexpr static pointer check_for_null(pointer source)
        {
            if (source == nullptr) [[unlikely]]
                throw std::invalid_argument("`required_ptr` cannot be constructed or assigned from a null pointer.");
            return source;
        }
    }; //class required_ptr

    /**
     * @fn explicit required_ptr::required_ptr(reference source) noexcept
     *
     * @param source The object to reference.
     *
     * @pre `source` must refer to a valid object that outlives the constructed `required_ptr`.
     * @post `get() == std::addressof(source)`.
     *
     * @remark Establishes the non-null invariant at construction.
     * @remark Prevents binding to temporaries via deleted rvalue overload.
     */
    /**
     * @overload required_ptr::required_ptr(const required_ptr<U>& source) noexcept
     *
     * @tparam U The element type, with its pointer convertible to `pointer`, of the source `required_ptr`.
     *
     * @param source The `required_ptr` being converted.
     *
     * @pre `source` must point to a valid object that outlives the resulting `required_ptr`.
     * @post `get() == source.get()`.
     *
     * @details This single constructor handles:
     * 1. Derived-to-Base conversion (e.g., `ptr<Derived>` to `ptr<Base>`).
     * 2. Qualification conversion (e.g., `ptr<T>` to `ptr<const T>`).
     * 3. Type erasure (e.g., `ptr<T>` to `ptr<void>`).
     *
     * @remark Preserves covariance. Converts from required_ptr-to-derived to required_ptr-to-base implicitly.
     * @remark Enables type erasure by converting `required_ptr<U>` to `required_ptr<void>` when applicable.
     * @remark Preserves the non-null invariant.
     */
    /**
     * @overload explicit required_ptr::required_ptr(P&& source)
     *
     * @tparam P The raw pointer type which must decay to a type convertible to `pointer`.
     *
     * @param source The raw pointer to bind.
     *
     * @pre `source` must point to a valid object that outlives the constructed `required_ptr`.
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
     * @remark Explicit when `T` is `void` to prevent unintended implicit erasure chains.
     */
    /**
     * @overload explicit required_ptr::required_ptr(const Pointer<Element, Args...>& source)
     *
     * @tparam Pointer A class template modeling a pointer-like type.
     * @tparam Element The element type of the source pointer.
     * @tparam Args Additional template parameters of the pointer type.
     *
     * @param source The pointer-like object providing access to a pointer-compatible value via `get()`.
     *
     * @pre `source.get()` must be a valid expression convertible to `pointer`.
     * @pre `source.get()` must point to an object whose lifetime strictly exceeds that of the constructed `required_ptr`.
     * @post `get() == static_cast<pointer>(source.get())`.
     *
     * @throws std::invalid_argument if `source.get() == nullptr`. Provides the Strong Exception Safety Guarantee.
     *
     * @remark Enables interoperation with pointer-like types (e.g., smart pointers) that expose a `get()` member.
     * @remark The source type must not be a specialization of `required_ptr` (to avoid ambiguity with existing overloads).
     * @remark This constructor does not transfer ownership and does not affect the lifetime of the underlying object.
     */
    /**
     * @fn required_ptr& required_ptr::operator=(reference source) noexcept
     *
     * @param source The object to reference.
     * @return Reference to `*this`.
     *
     * @pre `source` must refer to a valid object that outlives the `required_ptr`.
     *
     * @remark Rebinds the dependency without affecting ownership or lifetime.
     */
    /**
     * @overload required_ptr& required_ptr::operator=(const required_ptr<U>& source) noexcept
     *
     * @tparam U The element type, with its pointer convertible to `pointer`, of the source `required_ptr`.
     *
     * @param source The `required_ptr` being converted.
     * @return Reference to `*this`.
     *
     * @pre `source` must point to a valid object that outlives the resulting `required_ptr`.
     *
     * @details This single assignment operator handles:
     * 1. Derived-to-Base conversion (e.g., `ptr<Derived>` to `ptr<Base>`).
     * 2. Qualification conversion (e.g., `ptr<T>` to `ptr<const T>`).
     * 3. Type erasure (e.g., `ptr<T>` to `ptr<void>`).
     *
     * @remark Preserves covariance. Converts from required_ptr-to-derived to required_ptr-to-base implicitly.
     * @remark Enables type erasure by converting `required_ptr<U>` to `required_ptr<void>` when applicable.
     * @remark Preserves the non-null invariant.
     */
    /**
     * @overload required_ptr& required_ptr::operator=(P&& source)
     *
     * @tparam P The raw pointer type which must decay to a type convertible to `pointer`.
     *
     * @param source The raw pointer to rebind to.
     * @return Reference to `*this`.
     *
     * @pre `source` must point to a valid object that outlives the `required_ptr`.
     * @post `get() == source`.
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
     * @overload required_ptr& required_ptr::operator=(const Pointer<Element, Args...>& source)
     *
     * @tparam Pointer A class template modeling a pointer-like type.
     * @tparam Element The element type of the source pointer.
     * @tparam Args Additional template parameters of the pointer type.
     *
     * @param source The pointer-like object providing access to a raw pointer via `get()`.
     * @return Reference to `*this`.
     *
     * @pre `source.get()` must be a valid expression convertible to `pointer`.
     * @pre `source.get()` must point to an object whose lifetime strictly exceeds that of the `required_ptr`.
     * @post `get() == static_cast<pointer>(source.get())`.
     *
     * @throws std::invalid_argument if `source.get() == nullptr`. Provides the Strong Exception Safety Guarantee.
     *
     * @remark Rebinds the `required_ptr` from a pointer-like source while preserving the non-null invariant.
     * @remark Does not transfer ownership and does not affect the lifetime of the underlying object.
     */
    /**
     * @fn constexpr void swap(required_ptr& lhs, required_ptr& rhs) noexcept
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
     * @fn constexpr bool operator==(const required_ptr& lhs, const required_ptr& rhs) noexcept
     *
     * @param lhs The left-hand side `required_ptr`.
     * @param rhs The right-hand side `required_ptr`.
     *
     * @return `true` if both pointers refer to the same object; otherwise `false`.
     *
     * @remark Compares the pointed-to addresses (aliasing), not object values.
     */
    /**
     * @overload constexpr bool operator==(const required_ptr& lhs, const required_ptr<DerivedT>& rhs) noexcept
     *
     * @tparam DerivedT The element type, derived from `T`, of the right-hand side `required_ptr`.
     *
     * @param lhs The left-hand side `required_ptr`.
     * @param rhs The right-hand side `required_ptr` to compare.
     *
     * @return `true` if both pointers refer to the same object; otherwise `false`.
     *
     * @remark Enables equality comparison between `required_ptr` instances of related types when that can't be synthesized by implicit conversion to raw pointers.
     * @remark Compares the pointed-to addresses (aliasing), not object values.
     */
    /**
     * @overload constexpr bool operator==(const required_ptr& lhs, const std::add_pointer_t<DerivedT> rhs) noexcept
     *
     * @tparam DerivedT The element type, derived from `T`, of the raw pointer.
     *
     * @param lhs The `required_ptr` being compared.
     * @param rhs The raw pointer to compare against.
     *
     * @return `true` if the wrapped pointer in `lhs` equals `rhs`; otherwise `false`.
     *
     * @remark Enables comparison with raw pointers-to-derived for when that can't be synthesized by implicit conversion to raw pointers.
     * @remark Compares the pointed-to addresses (aliasing), not object values.
     */
    /**
     * @overload constexpr bool operator==(const pointer lhs, const required_ptr<DerivedT>& rhs) noexcept
     *
     * @tparam DerivedT The element type, derived from `T`, of the right-hand side `required_ptr`.
     *
     * @param lhs The raw pointer to compare.
     * @param rhs The `required_ptr` being compared.
     *
     * @return `true` if `lhs` equals the wrapped pointer in `rhs`; otherwise `false`.
     *
     * @remark Enables comparison with raw pointers-to-base when that can't be synthesized by implicit conversion to raw pointers.
     * @remark Comparison is performed on the underlying addresses.
     */
    /**
     * @fn constexpr reference required_ptr::operator*() const noexcept
     *
     * @return Reference to the referenced object.
     *
     * @pre The stored pointer must remain valid (non-dangling).
     *
     * @remark Equivalent to dereferencing `get()`.
     */
    /**
     * @fn constexpr pointer required_ptr::get() const noexcept
     *
     * @return The underlying raw pointer.
     *
     * @post The returned pointer is non-null.
     *
     * @remark Provided for interoperability with pointer-based APIs.
     */
    /**
     * @fn constexpr required_ptr::operator pointer() const noexcept
     *
     * @return The underlying raw pointer.
     *
     * @post The returned pointer is non-null.
     *
     * @remark Enables seamless interoperability with legacy interfaces expecting raw pointers.
     * @warning Implicit conversion may obscure the non-owning, non-null semantics; prefer `get()` when clarity is important.
     */
    /**
     * @fn constexpr required_ptr::operator bool() const noexcept
     *
     * @return `true` (The pointer is structurally guaranteed to always be engaged.)
     *
     * @post Always evaluates to `true`.
     *
     * @remark Satisfies boolean-testable requirements in generic templates and logical contexts.
     * @remark Models pointer interface by providing contextual conversion to `bool` albeit redundantly.
     * @note Because this always returns `true`, the compiler may elide checks in generic code when `required_ptr` is the concrete type.
     * @note This does not indicate engagement/optionality as `required_ptr` has no disengaged state.
     */
    /**
     * @fn constexpr static pointer required_ptr::check_for_null(pointer source)
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
     * @brief Deduction guide for `required_ptr`.
     *
     * @tparam T The type of the referenced object.
     *
     * @remark Deduces `T` from the referenced object, preserving cv-qualification.
     */
    template<typename T>
    required_ptr(T&) -> required_ptr<T>;

    /**
     * @brief Deduction guide for `required_ptr`.
     *
     * @tparam T The type of the pointee.
     *
     * @remark Deduces `T` from the pointee, preserving cv-qualification.
     */
    template<typename T>
    required_ptr(T*) -> required_ptr<T>;
} //namespace base::vocab::inline ptr

/**
 * @brief Partial specialization of `std::hash` for `required_ptr`.
 *
 * @tparam T The element type of the `required_ptr`.
 *
 * @remark Hashes the underlying stored address rather than pointee object state or values.
 * @remark Consistent with `required_ptr` equality semantics.
 */
template<class T>
struct std::hash<base::vocab::required_ptr<T>> {
    ///@brief Hashes the `required_ptr` based on the underlying address.
    [[nodiscard]] constexpr std::size_t operator()(const base::vocab::required_ptr<T>& ptr) const noexcept
    {
        return std::hash<T*>{}(ptr.get());
    }
};
