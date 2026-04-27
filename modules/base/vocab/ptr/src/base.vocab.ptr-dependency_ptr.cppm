// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-dependency_ptr.cppm
 * @version 0.3.0
 * @date April 26, 2026
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
 * @brief `dependency_ptr`: A non-owning, never-null, non-arithmetic pointer type for a required dependency.
 *
 * @details `dependency_ptr` acts like a rebindable reference with pointer syntax. It
 * expresses a mandatory dependency that must exist for the duration of the consumer's
 * lifetime. It naturally composes as `std::optional<dependency_ptr<T>>` to represent
 * a contextually optional dependency decoupled from the pointer itself.
 *
 * By utilizing structural constraints (deleted constructors for `nullptr_t` and raw
 * `pointer`), it enforces validity at the point of construction. It is intended to replace:
 * - Non-static reference data members (which make a class non-assignable).
 * - Raw pointers used as dependencies (which are semantically muddy regarding optionality and ownership).
 * - `std::reference_wrapper` (when pointer-like `->` access is preferred).
 * - `required_ptr` or `alias_ptr` when used for dependency injection.
 *
 * @todo Future Development: Use `= delete("reason")` instead of the C-style comments once the C++26 feature becomes available.
 */

//Module partition interface unit
export module base.vocab.ptr:dependency_ptr;

import std;

import base.meta.traits;

export namespace base::vocab::inline ptr {
    /**
     * @brief Pointer type representing a required dependency.
     *
     * @tparam T The pointed-to type (must not be a reference type per "C++ standard [dcl.ptr]", `void`, nor a function pointer).
     *
     * @details `dependency_ptr` models a non-owning, never-null, non-arithmetic pointer abstraction.
     * It is designed for dependency injection scenarios, where a dependency is required to exist and
     * outlive the consumer. It serves as a replacement for references as nonstatic data members and in
     * other cases where rebinding, pointer semantics, or interoperability with pointer-based APIs is
     * desirable. Optional dependencies naturally compose as `std::optional<dependency_ptr<T>>`.
     *
     * @note Semantically equivalent to a rebindable reference with a pointer interface.
     * @note Standard Layout type with size and alignment of a raw pointer.
     * @invariant Always engaged: a `dependency_ptr` always refers to a valid object; there is no null/disengaged representation. The wrapped pointer is guaranteed to be non-null by construction.
     * @remark This type does not own the referenced object and does not participate in lifetime management.
     * @remark Copying and assignment rebind the pointer without affecting the lifetime of the underlying object.
     * @remark Marked `[[nodiscard]]` to prevent accidental construction of unused dependency objects. Intentional discards should use `[[maybe_unused]]` to document intent.
     * @remark Construction requires an lvalue reference, preventing null initialization and discouraging dangling by rejecting direct binding to temporaries.
     * @note Non-arithmetic Pointer: Pointer arithmetic and ordering comparisons are intentionally disabled to prevent misuse as an iterator.
     * @remark Implicit conversion to raw pointer is provided for interoperability with legacy or low-level APIs.
     * @note All operations provide the no-throw guarantee; operations consist exclusively of non-throwing pointer manipulation.
     *
     * @warning The referenced object MUST outlive the `dependency_ptr`. Violating this results in undefined behavior.
     *
     * @see `alias_ptr` for nullable aliasing, `required_ptr` for non-null aliasing, `std::unique_ptr` and `std::shared_ptr` for ownership.
     */
    template<typename T>
        requires (
            !std::is_reference_v<T> && !std::is_void_v<T>
            && !std::is_function_v<base::meta::traits::remove_all_indirections_t<T>>
        )
    class [[nodiscard]] dependency_ptr {
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
        using pointer = T*;

        /**
         * @typedef reference
         * @brief The reference type (`T&`).
         */
        using reference = T&;

        /**
         * @typedef rvalue_reference
         * @brief The rvalue reference type (`T&&`).
         *
         * @note Used only for deletion of invalid overloads to prevent binding to temporaries.
         */
        using rvalue_reference = T&&;

        /**
         * @typedef difference_type
         * @brief Pointer difference type (`std::ptrdiff_t`).
         *
         * @note Provided to model pointer interface, though arithmetic is disabled.
         */
        using difference_type = std::ptrdiff_t;

    private:
        pointer ptr_;

    public:
        //================================================================================
        // Constructors and Assignment Operators
        //================================================================================

        ///@brief Constructs a `dependency_ptr` bound to an existing object.
        constexpr explicit dependency_ptr(reference source) noexcept : ptr_(&source) {}

        ///@brief (Covariance) Implicitly converts from `dependency_ptr<U>` to `dependency_ptr<T>` when `U` is publicly derived from `T`.
        template<std::derived_from<T> U>
            requires (!std::same_as<U, T>)
        constexpr explicit(false) dependency_ptr(const dependency_ptr<U>& source) noexcept : ptr_(source.get())
        {}

        ///@brief Rebinds the `dependency_ptr` to another object.
        dependency_ptr& operator=(reference source) noexcept
        {
            ptr_ = &source;
            return *this;
        }

        ///@brief (Covariance) Assigns from `dependency_ptr<U>` to `dependency_ptr<T>` when `U` is publicly derived from `T`.
        template<std::derived_from<T> U>
            requires (!std::same_as<U, T>)
        dependency_ptr& operator=(const dependency_ptr<U>& source) noexcept
        {
            ptr_ = source.get();
            return *this;
        }

        //================================================================================
        // Deleted Constructors and Assignment Operators: Non-Null Structural Invariant
        //================================================================================

        ///@brief Deleted default constructor to prevent sources of null initialization.
        dependency_ptr() =
            delete /*("Default constructor deleted to prevent null initialization. Use `std::optional<dependency_ptr<T>>` for default-constructible optional dependencies.")*/
            ;

        ///@brief Deleted constructor from `nullptr` to prevent sources of null initialization.
        dependency_ptr(std::nullptr_t) =
            delete /*("Constructor from `nullptr` deleted to prevent null initialization. Use `std::optional<dependency_ptr<T>>` for optional dependencies.")*/
            ;

        ///@brief Deleted constructor from `pointer` to structurally guarantee non-null initialization.
        dependency_ptr(pointer) =
            delete /*("Constructor from `pointer` deleted. Dereference first to guarantee non-null initialization. Use `std::optional<dependency_ptr<T>>` for optional dependencies.")*/
            ;

        ///@brief Deleted assignment from `nullptr` to prevent sources of invalid null rebinding.
        dependency_ptr& operator=(std::nullptr_t) =
            delete /*("Assignment from `nullptr` deleted to prevent null rebinding. Use `std::optional<dependency_ptr<T>>` for optional dependencies.")*/
            ;

        ///@brief Deleted assignment from `pointer` to structurally guarantee non-null rebinding.
        dependency_ptr& operator=(pointer) =
            delete /*("Assignment from `pointer` deleted. Dereference first to guarantee non-null initialization. Use `std::optional<dependency_ptr<T>>` for optional dependencies.")*/
            ;

        //================================================================================
        // Deleted Constructors and Assignment Operators: No Aliasing Temporaries
        //================================================================================

        ///@brief Deleted constructor from `rvalue_reference` to discourage dangling by rejecting direct binding to temporaries.
        dependency_ptr(rvalue_reference) =
            delete /*("Constructor from `rvalue_reference` deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        ///@brief Deleted assignment from `rvalue_reference` to discourage dangling by rejecting direct binding to temporaries.
        dependency_ptr& operator=(rvalue_reference) =
            delete /*("Assignment from `rvalue_reference` deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        //================================================================================
        // Comparison Operators (Equality Allowed, Others Deleted)
        //================================================================================

        ///@brief Compares equality in terms of pointer identity.
        [[nodiscard]] friend bool operator==(const dependency_ptr& lhs, const dependency_ptr& rhs) noexcept = default;

        ///@brief Covariantly compares equality in terms of pointer identity.
        template<std::derived_from<T> U>
            requires (!std::same_as<U, T>)
        [[nodiscard]] friend bool operator==(const dependency_ptr& lhs, const dependency_ptr<U>& rhs) noexcept
        {
            return (lhs.get() == rhs.get());
        }

        ///@brief Covariantly compares equality of a `dependency_ptr`-to-base with a raw pointer-to-derived in terms of pointer identity.
        template<std::derived_from<T> U>
        [[nodiscard]] friend bool operator==(const dependency_ptr& lhs, const U* rhs) noexcept
        {
            return (lhs.get() == rhs);
        }

        ///@brief Covariantly compares equality of a raw pointer-to-base with a `dependency_ptr`-to-derived in terms of pointer identity.
        template<std::derived_from<T> U>
        [[nodiscard]] friend bool operator==(const pointer lhs, const dependency_ptr<U>& rhs) noexcept
        {
            return (lhs == rhs.get());
        }

        ///@brief Deleted comparison operators to prevent misuse as an iterator or ordered value type.
        auto operator<=>(dependency_ptr) const =
            delete /*("Comparison operators deleted to prevent address comparisons. `dependency_ptr` is not an iterator.")*/;

        ///@brief Deleted comparison operators to prevent misuse as an iterator or ordered value type.
        auto operator<=>(const pointer) const =
            delete /*("Comparison operators deleted to prevent address comparisons. `dependency_ptr` is not an iterator.")*/;

        //================================================================================
        // Pointer Operations
        //================================================================================

        ///@brief Provides pointer-like member access to the referenced object.
        [[nodiscard]] constexpr pointer operator->() const noexcept { return ptr_; }

        ///@brief Dereferences the pointer to access the referenced object.
        [[nodiscard]] constexpr reference operator*() const noexcept { return *ptr_; }

        ///@brief Returns the underlying raw pointer.
        [[nodiscard]] constexpr pointer get() const noexcept { return ptr_; }

        ///@brief Implicitly converts to the underlying raw pointer type.
        [[nodiscard]] constexpr explicit(false) operator pointer() const noexcept { return this->get(); }

        ///@brief Contextually converts to `bool` to test if the pointer is engaged. Always returns true to confirm structural invariant.
        [[nodiscard]] constexpr explicit operator bool() const noexcept { return true; }

        //================================================================================
        // Deleted Pointer Arithmetic Operators: Not an Iterator
        //================================================================================

        ///@brief Deleted prefix increment to prevent misuse as an iterator.
        dependency_ptr& operator++() =
            delete /*("Prefix increment deleted to prevent pointer arithmetic. `dependency_ptr` is not an iterator.")*/;

        ///@brief Deleted prefix decrement to prevent misuse as an iterator.
        dependency_ptr& operator--() =
            delete /*("Prefix decrement deleted to prevent pointer arithmetic. `dependency_ptr` is not an iterator.")*/;

        ///@brief Deleted postfix increment to prevent misuse as an iterator.
        dependency_ptr operator++(int) =
            delete /*("Postfix increment deleted to prevent pointer arithmetic. `dependency_ptr` is not an iterator.")*/;

        ///@brief Deleted postfix decrement to prevent misuse as an iterator.
        dependency_ptr operator--(int) =
            delete /*("Postfix decrement deleted to prevent pointer arithmetic. `dependency_ptr` is not an iterator.")*/;

        ///@brief Deleted addition assignment to prevent misuse as an iterator.
        dependency_ptr& operator+=(difference_type) =
            delete /*("Addition assignment deleted to prevent pointer arithmetic. `dependency_ptr` is not an iterator.")*/;

        ///@brief Deleted subtraction assignment to prevent misuse as an iterator.
        dependency_ptr& operator-=(difference_type) =
            delete /*("Subtraction assignment deleted to prevent pointer arithmetic. `dependency_ptr` is not an iterator.")*/;

        ///@brief Deleted pointer addition to prevent misuse as an iterator.
        friend dependency_ptr operator+(dependency_ptr, difference_type) =
            delete /*("Pointer addition deleted to prevent pointer arithmetic. `dependency_ptr` is not an iterator.")*/;

        ///@brief Deleted pointer addition to prevent misuse as an iterator.
        friend dependency_ptr operator+(difference_type, dependency_ptr) =
            delete /*("Pointer addition deleted to prevent pointer arithmetic. `dependency_ptr` is not an iterator.")*/;

        ///@brief Deleted pointer subtraction to prevent misuse as an iterator.
        friend difference_type operator-(dependency_ptr, dependency_ptr) =
            delete /*("Pointer subtraction deleted to prevent pointer arithmetic. `dependency_ptr` is not an iterator.")*/;

        ///@brief Deleted pointer subtraction to prevent misuse as an iterator.
        friend dependency_ptr operator-(dependency_ptr, difference_type) =
            delete /*("Pointer subtraction deleted to prevent pointer arithmetic. `dependency_ptr` is not an iterator.")*/;
    }; //class dependency_ptr

    /**
     * @fn explicit dependency_ptr::dependency_ptr(reference source) noexcept
     *
     * @param source The object to reference.
     *
     * @pre `source` must refer to a valid object that outlives the constructed `dependency_ptr`.
     *
     * @remark Establishes the non-null invariant at construction.
     * @remark Prevents binding to temporaries via deleted rvalue overload.
     */
    /**
     * @fn dependency_ptr::dependency_ptr(const dependency_ptr<U>& source) noexcept
     *
     * @tparam U The element type, derived from T, of the source `dependency_ptr`.
     *
     * @param source The pointer-to-derived being converted.
     *
     * @pre `*source` must refer to a valid object that outlives the resulting `dependency_ptr`.
     *
     * @remark Preserves covariance. Converts from dependency_ptr-to-derived to dependency_ptr-to-base implicitly.
     */
    /**
     * @fn dependency_ptr& dependency_ptr::operator=(reference source) noexcept
     *
     * @param source The object to reference.
     * @return Reference to `*this`.
     *
     * @pre `source` must refer to a valid object that outlives the `dependency_ptr`.
     *
     * @remark Rebinds the dependency without affecting ownership or lifetime.
     */
    /**
     * @fn dependency_ptr& dependency_ptr::operator=(const dependency_ptr<U>& source) noexcept
     *
     * @tparam U The element type, derived from T, of the source `dependency_ptr`.
     *
     * @param source The pointer-to-derived being converted.
     * @return Reference to `*this`.
     *
     * @pre `*source` must refer to a valid object that outlives the resulting `dependency_ptr`.
     *
     * @remark Preserves covariance. Converts from dependency_ptr-to-derived to dependency_ptr-to-base implicitly.
     */
    /**
     * @fn constexpr pointer dependency_ptr::operator->() const noexcept
     *
     * @return Pointer to the referenced object.
     *
     * @pre The stored pointer must remain valid (non-dangling).
     * @post The returned pointer is non-null.
     *
     * @remark Operator syntax equivalent to `get()->`. Function syntax equivalent to calling `get()`.
     */
    /**
     * @fn constexpr reference dependency_ptr::operator*() const noexcept
     *
     * @return Reference to the referenced object.
     *
     * @pre The stored pointer must remain valid (non-dangling).
     *
     * @remark Equivalent to dereferencing `get()`.
     */
    /**
     * @fn constexpr pointer dependency_ptr::get() const noexcept
     *
     * @return The underlying raw pointer.
     *
     * @post The returned pointer is non-null.
     *
     * @remark Provided for interoperability with pointer-based APIs.
     */
    /**
     * @fn dependency_ptr::operator pointer() const noexcept
     *
     * @return The underlying raw pointer.
     *
     * @post The returned pointer is non-null.
     *
     * @remark Enables seamless interoperability with legacy interfaces expecting raw pointers.
     * @warning Implicit conversion may obscure the non-owning, non-null semantics; prefer `get()` when clarity is important.
     */
    /**
     * @fn dependency_ptr::operator bool() const noexcept
     *
     * @return `true` (The pointer is structurally guaranteed to always be engaged.)
     *
     * @post Always evaluates to `true`.
     *
     * @remark Satisfies boolean-testable requirements in generic templates and logical contexts.
     * @remark Models pointer interface by providing contextual conversion to `bool` albeit redundantly.
     * @note Because this always returns `true`, the compiler can elide checks in generic code when `dependency_ptr` is the concrete type.
     * @note This does not indicate engagement/optionality as `dependency_ptr` has no disengaged state.
     */
    /**
     * @fn bool operator==(const dependency_ptr& lhs, const dependency_ptr& rhs) noexcept
     *
     * @param lhs The left-hand side `dependency_ptr`.
     * @param rhs The right-hand side `dependency_ptr`.
     *
     * @return `true` if both pointers refer to the same object; otherwise `false`.
     *
     * @remark Compares the pointed-to addresses (aliasing), not object values.
     */
    /**
     * @overload bool operator==(const dependency_ptr& lhs, const dependency_ptr<U>& rhs) noexcept
     *
     * @tparam U The element type, derived from `T`, of the right-hand side `dependency_ptr`.
     *
     * @param lhs The left-hand side `dependency_ptr`.
     * @param rhs The right-hand side `dependency_ptr` to compare.
     *
     * @return `true` if both pointers refer to the same object; otherwise `false`.
     *
     * @remark Enables equality comparison between `dependency_ptr` instances of related types.
     * @remark Compares the pointed-to addresses (aliasing), not object values.
     */
    /**
     * @overload bool operator==(const dependency_ptr& lhs, const U* rhs) noexcept
     *
     * @tparam U The element type, derived from `T`, of the raw pointer.
     *
     * @param lhs The `dependency_ptr` being compared.
     * @param rhs The raw pointer to compare against.
     *
     * @return `true` if the wrapped pointer in `lhs` equals `rhs`; otherwise `false`.
     *
     * @remark Enables comparison with raw pointers-to-derived for interoperability with legacy APIs.
     * @remark Compares the pointed-to addresses (aliasing), not object values.
     */
    /**
     * @overload bool operator==(const pointer lhs, const dependency_ptr<U>& rhs) noexcept
     *
     * @tparam U The element type, derived from `T`, of the right-hand side `dependency_ptr`.
     *
     * @param lhs The raw pointer to compare.
     * @param rhs The `dependency_ptr` being compared.
     *
     * @return `true` if `lhs` equals the wrapped pointer in `rhs`; otherwise `false`.
     *
     * @remark Enables comparison with raw pointers-to-base for interoperability with legacy APIs.
     * @remark Comparison is performed on the underlying addresses.
     */

    /**
     * @brief Deduction guide for `dependency_ptr`.
     *
     * @tparam T The type of the referenced object.
     *
     * @remark Deduces `T` from the referenced object, preserving cv-qualification.
     */
    template<typename T>
    dependency_ptr(T&) -> dependency_ptr<T>;
} //namespace base::vocab::inline ptr
