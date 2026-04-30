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
 * @details
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
     * @details `required_ptr` models a non-owning, non-nullable, non-arithmetic, non-void-permitting
     * pointer abstraction.
     *
     * @note Standard Layout type with size and alignment of a raw pointer.
     * @invariant Always engaged: a `required_ptr` always refers to a valid object; there is no null/disengaged representation. Attempts to construct/assign from another pointer throw on null.
     * @remark This type does not own the referenced object and does not participate in lifetime management.
     * @remark Copying and assignment rebind the pointer without affecting the lifetime of the underlying object.
     *
     * @see `alias_ptr` for nullable aliasing, `dependency_ptr` for dependency injection structural non-nullability, `cursor_ptr` for non-null iteration/traversal, `std::unique_ptr` and `std::shared_ptr` for ownership.
     */
    template<typename T>
        requires (!std::is_reference_v<T> && !std::is_function_v<base::meta::traits::remove_all_indirections_t<T>>)
    class required_ptr {
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
        constexpr explicit required_ptr(reference source) noexcept : address_(std::addressof(source)) {}

        ///@brief (Covariance) Implicitly converts from `required_ptr<U>` to `required_ptr<T>` when `U` is publicly derived from `T`.
        template<std::derived_from<T> U>
            requires (!std::same_as<U, T>)
        constexpr explicit(false) required_ptr(const required_ptr<U>& source) noexcept : address_(source.get())
        {}

        ///@brief (Erasure) Implicitly converts from `required_ptr<U>` to `required_ptr<void>`.
        template<typename U>
            requires (!std::same_as<U, T>)
        constexpr explicit(false) required_ptr(const required_ptr<U>& source) noexcept requires std::is_void_v<T> : address_(source.get())
        {}

        ///@brief Implicitly converts from a raw `pointer`. Explicit when `T` is void to avoid implicit conversion chaining.
        constexpr explicit(std::is_void_v<T>) required_ptr(pointer source) : address_(source)
        {
            if (source == nullptr) [[unlikely]]
                throw std::invalid_argument("required_ptr cannot be constructed from a null pointer.");
        }

        ///@brief Rebinds the `required_ptr` to another object.
        required_ptr& operator=(reference source) noexcept
        {
            address_ = std::addressof(source);
            return *this;
        }

        ///@brief (Covariance) Assigns from `required_ptr<U>` to `required_ptr<T>` when `U` is publicly derived from `T`.
        template<std::derived_from<T> U>
            requires (!std::same_as<U, T>)
        required_ptr& operator=(const required_ptr<U>& source) noexcept
        {
            address_ = source.get();
            return *this;
        }

        ///@brief (Erasure) Assigns from `required_ptr<U>` to `required_ptr<void>`.
        template<typename U>
            requires (!std::same_as<U, T>)
        required_ptr& operator=(const required_ptr<U>& source) noexcept requires std::is_void_v<T>
        {
            address_ = source.get();
            return *this;
        }

        ///@brief Assigns from a raw `pointer`.
        required_ptr& operator=(pointer source)
        {
            if (source == nullptr) [[unlikely]]
                throw std::invalid_argument("required_ptr cannot be assigned from a null pointer.");
            address_ = source;
            return *this;
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

        ///@brief Deleted assignment from `rvalue_reference` to discourage dangling by rejecting direct binding to temporaries.
        required_ptr& operator=(rvalue_reference) =
            delete /*("Assignment from `rvalue_reference` deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        //================================================================================
        // Comparison Operators (Equality Allowed, Others Deleted)
        //================================================================================

        ///@brief Compares equality in terms of pointer identity.
        [[nodiscard]] friend bool operator==(const required_ptr& lhs, const required_ptr& rhs) noexcept = default;

        ///@brief Covariantly compares equality in terms of pointer identity.
        template<std::derived_from<T> U>
            requires (!std::same_as<U, T>)
        [[nodiscard]] friend bool operator==(const required_ptr& lhs, const required_ptr<U>& rhs) noexcept
        {
            return (lhs.get() == rhs.get());
        }

        ///@brief Covariantly compares equality of a `required_ptr`-to-base with a raw pointer-to-derived in terms of pointer identity.
        template<std::derived_from<T> U>
        [[nodiscard]] friend bool operator==(const required_ptr& lhs, const U* rhs) noexcept
        {
            return (lhs.get() == rhs);
        }

        ///@brief Covariantly compares equality of a raw pointer-to-base with a `required_ptr`-to-derived in terms of pointer identity.
        template<std::derived_from<T> U>
        [[nodiscard]] friend bool operator==(const pointer lhs, const required_ptr<U>& rhs) noexcept
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
        [[nodiscard]] constexpr pointer operator->() const noexcept requires (!is_void_v<T>) { return address_; }

        ///@brief Dereferences the pointer to access the referenced object.
        [[nodiscard]] constexpr reference operator*() const noexcept requires (!is_void_v<T>) { return *address_; }

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
    }; //class required_ptr
    /**
     * @fn explicit required_ptr::required_ptr(reference source) noexcept
     *
     * @param source The object to reference.
     *
     * @pre `source` must refer to a valid object that outlives the constructed `required_ptr`.
     *
     * @remark Establishes the non-null invariant at construction.
     * @remark Prevents binding to temporaries via deleted rvalue overload.
     */
    /**
     * @fn required_ptr::required_ptr(const required_ptr<U>& source) noexcept
     *
     * @tparam U The element type, derived from T, of the source `required_ptr`.
     *
     * @param source The pointer-to-derived being converted.
     *
     * @pre `*source` must refer to a valid object that outlives the resulting `required_ptr`.
     *
     * @remark Preserves covariance. Converts from required_ptr-to-derived to required_ptr-to-base implicitly.
     */
    /**
     * @fn required_ptr::required_ptr(const required_ptr<U>& source) noexcept
     *
     * @tparam U The element type of the source `required_ptr`.
     *
     * @param source The `required_ptr` being converted.
     *
     * @pre `*source` must refer to a valid object that outlives the resulting `required_ptr`.
     *
     * @post `get() == source.get()`.
     *
     * @remark Enables type erasure by converting `required_ptr<U>` to `required_ptr<void>`.
     * @remark Preserves the non-null invariant.
     */
    /**
     * @fn explicit required_ptr::required_ptr(pointer source)
     *
     * @param source The raw pointer to bind.
     *
     * @pre `source` must be non-null and must point to a valid object that outlives the constructed `required_ptr`.
     *
     * @post `get() == source`.
     *
     * @throws std::invalid_argument if `source == nullptr`.
     *
     * @remark Establishes the non-null invariant at runtime when constructed from raw pointers.
     * @remark Explicit when `T` is `void` to prevent unintended implicit erasure chains.
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
     * @fn required_ptr& required_ptr::operator=(const required_ptr<U>& source) noexcept
     *
     * @tparam U The element type, derived from T, of the source `required_ptr`.
     *
     * @param source The pointer-to-derived being converted.
     * @return Reference to `*this`.
     *
     * @pre `*source` must refer to a valid object that outlives the resulting `required_ptr`.
     *
     * @remark Preserves covariance. Converts from required_ptr-to-derived to required_ptr-to-base implicitly.
     */
    /**
     * @fn required_ptr& required_ptr::operator=(const required_ptr<U>& source) noexcept
     *
     * @tparam U The element type of the source `required_ptr`.
     *
     * @param source The `required_ptr` being assigned from.
     * @return Reference to `*this`.
     *
     * @pre `*source` must refer to a valid object that outlives the resulting `required_ptr`.
     *
     * @post `get() == source.get()`.
     *
     * @remark Enables type erasure assignment to `required_ptr<void>`.
     * @remark Preserves the non-null invariant.
     */
    /**
     * @fn required_ptr& required_ptr::operator=(pointer source)
     *
     * @param source The raw pointer to rebind to.
     * @return Reference to `*this`.
     *
     * @pre `source` must be non-null and must point to a valid object that outlives the `required_ptr`.
     *
     * @post `get() == source`.
     *
     * @throws std::invalid_argument if `source == nullptr`.
     *
     * @remark Rebinds the pointer while preserving the non-null invariant.
     * @remark Does not affect the lifetime of the referenced object.
     */
    /**
     * @fn bool operator==(const required_ptr& lhs, const required_ptr& rhs) noexcept
     *
     * @param lhs The left-hand side `required_ptr`.
     * @param rhs The right-hand side `required_ptr`.
     *
     * @return `true` if both pointers refer to the same object; otherwise `false`.
     *
     * @remark Compares the pointed-to addresses (aliasing), not object values.
     */
    /**
     * @overload bool operator==(const required_ptr& lhs, const required_ptr<U>& rhs) noexcept
     *
     * @tparam U The element type, derived from `T`, of the right-hand side `required_ptr`.
     *
     * @param lhs The left-hand side `required_ptr`.
     * @param rhs The right-hand side `required_ptr` to compare.
     *
     * @return `true` if both pointers refer to the same object; otherwise `false`.
     *
     * @remark Enables equality comparison between `required_ptr` instances of related types.
     * @remark Compares the pointed-to addresses (aliasing), not object values.
     */
    /**
     * @overload bool operator==(const required_ptr& lhs, const U* rhs) noexcept
     *
     * @tparam U The element type, derived from `T`, of the raw pointer.
     *
     * @param lhs The `required_ptr` being compared.
     * @param rhs The raw pointer to compare against.
     *
     * @return `true` if the wrapped pointer in `lhs` equals `rhs`; otherwise `false`.
     *
     * @remark Enables comparison with raw pointers-to-derived for interoperability with legacy APIs.
     * @remark Compares the pointed-to addresses (aliasing), not object values.
     */
    /**
     * @overload bool operator==(const pointer lhs, const required_ptr<U>& rhs) noexcept
     *
     * @tparam U The element type, derived from `T`, of the right-hand side `required_ptr`.
     *
     * @param lhs The raw pointer to compare.
     * @param rhs The `required_ptr` being compared.
     *
     * @return `true` if `lhs` equals the wrapped pointer in `rhs`; otherwise `false`.
     *
     * @remark Enables comparison with raw pointers-to-base for interoperability with legacy APIs.
     * @remark Comparison is performed on the underlying addresses.
     */
    /**
     * @fn constexpr pointer required_ptr::operator->() const noexcept
     *
     * @return Pointer to the referenced object.
     *
     * @pre The stored pointer must remain valid (non-dangling).
     * @post The returned pointer is non-null.
     *
     * @remark Operator syntax equivalent to `get()->`. Function syntax equivalent to calling `get()`.
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
     * @fn required_ptr::operator pointer() const noexcept
     *
     * @return The underlying raw pointer.
     *
     * @post The returned pointer is non-null.
     *
     * @remark Enables seamless interoperability with legacy interfaces expecting raw pointers.
     * @warning Implicit conversion may obscure the non-owning, non-null semantics; prefer `get()` when clarity is important.
     */
    /**
     * @fn required_ptr::operator bool() const noexcept
     *
     * @return `true` (The pointer is structurally guaranteed to always be engaged.)
     *
     * @post Always evaluates to `true`.
     *
     * @remark Satisfies boolean-testable requirements in generic templates and logical contexts.
     * @remark Models pointer interface by providing contextual conversion to `bool` albeit redundantly.
     * @note Because this always returns `true`, the compiler can elide checks in generic code when `required_ptr` is the concrete type.
     * @note This does not indicate engagement/optionality as `required_ptr` has no disengaged state.
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
