// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-dependency_ptr.cppm
 * @version 0.2.0
 * @date March 11, 2026
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
 * @brief `dependency_ptr`: A non-owning, never-null pointer type for a required dependency.
 *
 * @details `dependency_ptr` acts as a rebindable reference with pointer syntax. It is 
 * designed to express a mandatory dependency that must exist for the duration of the 
 * consumer's lifetime. Optional dependencies should be wrapped in `std::optional`
 * rather than using `nullptr` for a disengaged value.
 *
 * By utilizing structural constraints (deleted constructors for `nullptr_t` and raw 
 * `pointer`), it enforces a validity at the call site. It is intended to replace:
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

export namespace base::vocab::inline ptr {
    /**
     * @brief Non-owning, never-null pointer type representing a required dependency.
     *
     * @tparam T The pointed-to type (must not be a reference type per "C++ standard [dcl.ptr]").
     *
     * @details `dependency_ptr` is designed for dependency injection scenarios, where a dependency
     * is required to exist and outlive the consumer. It serves as a replacement for references as
     * nonstatic data members and in other cases where rebinding, pointer semantics, or interoperability
     * with pointer-based APIs is desirable.
     *
     * @note Semantically equivalent to a rebindable reference with a pointer interface.
     * @invariant The stored pointer is guaranteed to be non-null by construction; this invariant is enforced structurally.
     * @remark This type does not own the referenced object and does not participate in lifetime management.
     * @remark Copying and assignment rebind the pointer without affecting the lifetime of the underlying object.
     * @remark Marked `[[nodiscard]]` to prevent accidental construction of unused dependency objects. Intentional discards should use `[[maybe_unused]]` to document intent.
     * @remark Construction requires an lvalue reference, preventing null initialization and discouraging dangling by binding to temporaries.
     * @remark Pointer arithmetic and ordering comparisons are intentionally disabled to prevent misuse as an iterator.
     * @remark Implicit conversion to raw pointer is provided for interoperability with legacy or low-level APIs.
     *
     * @warning The referenced object MUST outlive the `dependency_ptr`. Violating this results in undefined behavior.
     *
     * @see `alias_ptr` for nullable aliasing, `required_ptr` for non-null aliasing, `std::unique_ptr` and `std::shared_ptr` for ownership.
     */
    template<typename T>
        requires (!std::is_reference_v<T>)
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
         * @note Provided for compatibility with pointer-like conventions, though arithmetic is disabled.
         */
        using difference_type = std::ptrdiff_t;

    private:
        pointer ptr_;

    public:
        //================================================================================
        // Constructors and Assignment Operators
        //================================================================================

        ///@brief Constructs a `dependency_ptr` bound to an existing object.
        constexpr explicit dependency_ptr(reference other) noexcept : ptr_(&other) {}

        ///@brief (Covariance) Constructs a `dependency_ptr<Base>` implicitly from a `dependency_ptr<Derived>`
        template<std::derived_from<T> U>
            requires (!std::is_same<U, T>)
        constexpr explicit(false) dependency_ptr(const dependency_ptr<U>& other) noexcept 
          : ptr_(other.get()) {}

        ///@brief Rebinds the `dependency_ptr` to another object.
        dependency_ptr& operator=(reference other) noexcept
        {
            ptr_ = &other;
            return *this;
        }

        ///@brief (Covariance) Assigns a `dependency_ptr<Derived>` to a `dependency_ptr<Base>`
        template<std::derived_from<T> U>
            requires (!std::is_same<U, T>)
        dependency_ptr& operator=(const dependency_ptr<U>& other) noexcept
        {
            ptr_ = other.get();
            return *this;
        }

        //================================================================================
        // Deleted Constructors and Assignment Operators: Non-Null Structural Invariant
        //================================================================================

        ///@brief Deleted default constructor to prevent null initialization.
        dependency_ptr() = delete/*("Default constructor deleted to prevent null initialization. Use `std::optional<dependency_ptr<T>>` for default-constructible optional dependencies.")*/;

        ///@brief Deleted constructor from `nullptr` to prevent null initialization.
        dependency_ptr(std::nullptr_t) = delete/*("Constructor from `nullptr` deleted to prevent null initialization. Use `std::optional<dependency_ptr<T>>` for optional dependencies.")*/;

        ///@brief Deleted constructor from `pointer` to structurally guarantee non-null initialization.
        dependency_ptr(pointer) = delete/*("Constructor from `pointer` deleted. Dereference first to guarantee non-null initialization. Use `std::optional<dependency_ptr<T>>` for optional dependencies.")*/;

        ///@brief Deleted assignment from `nullptr` to prevent invalid null reseating.
        dependency_ptr& operator=(std::nullptr_t) = delete/*("Assignment from `nullptr` deleted to prevent null reseating. Use `std::optional<dependency_ptr<T>>` for optional dependencies.")*/;

        ///@brief Deleted assignment from `pointer` to structurally guarantee non-null reseatng.
        dependency_ptr& operator=(pointer) = delete/*("Assignment from `pointer` deleted. Dereference first to guarantee non-null initialization. Use `std::optional<dependency_ptr<T>>` for default-constructible optional dependencies.")*/;

        //================================================================================
        // Deleted Constructors and Assignment Operators: No Aliasing Temporaries
        //================================================================================

        ///@brief Deleted constructor from `rvalue_reference` to discourage dangling by binding to temporaries.
        dependency_ptr(rvalue_reference) = delete/*("Constructor from `rvalue_reference` deleted to discourage dangling by binding to temporaries.")*/;

        ///@brief Deleted assignment from `rvalue_reference` to discourage dangling by binding to temporaries.
        dependency_ptr& operator=(rvalue_reference) = delete/*("Assignment from `rvalue_reference` deleted to discourage dangling by binding to temporaries.")*/;

        //================================================================================
        // Comparison Operators (Equality Allowed, Others Deleted)
        //================================================================================

        ///@brief Defaulted equality comparison operator returns true if both pointers alias each other.
        [[nodiscard]] bool operator==(const dependency_ptr&) const noexcept = default;

        ///@brief Deleted comparison operators to prevent misuse as an iterator.
        auto operator<=>(dependency_ptr) const = delete/*("Comparison operators deleted to prevent address comparisons. `dependency_ptr` is not an iterator.")*/;
        
        ///@brief Deleted comparison operators to prevent misuse as an iterator.
        auto operator<=>(const pointer other) const = delete/*("Comparison operators deleted to prevent address comparisons. `dependency_ptr` is not an iterator.")*/;

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

        //================================================================================
        // Deleted Pointer Arithmetic Operators: Not an Iterator
        //================================================================================

        ///@brief Deleted prefix increment to prevent misuse as an iterator.
        dependency_ptr& operator++() = delete/*("Prefix increment deleted to prevent pointer arithmetic. `dependency_ptr` is not an iterator.")*/;

        ///@brief Deleted prefix decrement to prevent misuse as an iterator.
        dependency_ptr& operator--() = delete/*("Prefix decrement deleted to prevent pointer arithmetic. `dependency_ptr` is not an iterator.")*/;

        ///@brief Deleted postfix increment to prevent misuse as an iterator.
        dependency_ptr operator++(int) = delete/*("Postfix increment deleted to prevent pointer arithmetic. `dependency_ptr` is not an iterator.")*/;

        ///@brief Deleted postfix decrement to prevent misuse as an iterator.
        dependency_ptr operator--(int) = delete/*("Postfix decrement deleted to prevent pointer arithmetic. `dependency_ptr` is not an iterator.")*/;

        ///@brief Deleted addition assignment to prevent misuse as an iterator.
        dependency_ptr& operator+=(difference_type) = delete/*("Addition assignment deleted to prevent pointer arithmetic. `dependency_ptr` is not an iterator.")*/;

        ///@brief Deleted subtraction assignment to prevent misuse as an iterator.
        dependency_ptr& operator-=(difference_type) = delete/*("Subtraction assignment deleted to prevent pointer arithmetic. `dependency_ptr` is not an iterator.")*/;

        ///@brief Deleted pointer addition to prevent misuse as an iterator.
        friend dependency_ptr operator+(dependency_ptr, difference_type) = delete/*("Pointer addition deleted to prevent pointer arithmetic. `dependency_ptr` is not an iterator.")*/;

        ///@brief Deleted pointer addition to prevent misuse as an iterator.
        friend dependency_ptr operator+(difference_type, dependency_ptr) = delete/*("Pointer addition deleted to prevent pointer arithmetic. `dependency_ptr` is not an iterator.")*/;

        ///@brief Deleted pointer subtraction to prevent misuse as an iterator.
        friend difference_type operator-(dependency_ptr, dependency_ptr) = delete/*("Pointer subtraction deleted to prevent pointer arithmetic. `dependency_ptr` is not an iterator.")*/;

        ///@brief Deleted pointer subtraction to prevent misuse as an iterator.
        friend dependency_ptr operator-(dependency_ptr, difference_type) = delete/*("Pointer subtraction deleted to prevent pointer arithmetic. `dependency_ptr` is not an iterator.")*/;
    }; //class dependency_ptr
    /**
     * @fn explicit dependency_ptr::dependency_ptr(reference other) noexcept
     *
     * @param other The object to reference.
     *
     * @pre `other` must refer to a valid object that outlives the constructed `dependency_ptr`.
     *
     * @remark Establishes the non-null invariant at construction.
     * @remark Prevents binding to temporaries via deleted rvalue overload.
     */
    /**
     * @fn dependency_ptr::dependency_ptr(const dependency_ptr<U>& other) noexcept
     *
     * @tparam U The element type [derived from T] of the `dependency_ptr` to convert from.
     *
     * @param other The pointer-to-derived being converted.
     *
     * @remark Preserves covariance. Converts from dependency_ptr-to-derived to dependency_ptr-to-base implicitly.
     */
    /**
     * @fn dependency_ptr& dependency_ptr::operator=(reference other) noexcept
     *
     * @param other The object to reference.
     * @return Reference to `*this`.
     *
     * @pre `other` must refer to a valid object that outlives the `dependency_ptr`.
     *
     * @remark Rebinds the dependency without affecting ownership or lifetime.
     */
    /**
     * @fn dependency_ptr& dependency_ptr::operator=(const dependency_ptr<U>& other) noexcept
     *
     * @tparam U The element type [derived from T] of the `dependency_ptr` to convert from.
     *
     * @param other The pointer-to-derived being converted.
     * @return Reference to `*this`.
     *
     * @remark Preserves covariance. Converts from dependency_ptr-to-derived to dependency_ptr-to-base implicitly.
     */
    /**
     * @fn constexpr pointer dependency_ptr::operator->() const noexcept
     *
     * @return Pointer to the referenced object.
     *
     * @pre The stored pointer must remain valid (non-dangling).
     */
    /**
     * @fn constexpr reference dependency_ptr::operator*() const noexcept
     *
     * @return Reference to the referenced object.
     *
     * @pre The stored pointer must remain valid (non-dangling).
     */
    /**
     * @fn constexpr pointer dependency_ptr::get() const noexcept
     *
     * @return The underlying raw pointer.
     *
     * @remark Provided for interoperability with pointer-based APIs.
     */
    /**
     * @fn dependency_ptr::operator pointer() const noexcept
     *
     * @return The underlying raw pointer.
     *
     * @remark Enables seamless interoperability with legacy interfaces expecting raw pointers.
     * @warning Implicit conversion may obscure the non-owning, non-null semantics; prefer `get()` when clarity is important.
     */

    /**
     * @brief Deduction guide for `dependency_ptr`.
     *
     * @param ref The referenced object.
     *
     * @remark Deduces `T` directly from `T&`, preserving cv-qualification.
     */
    template<typename T>
    dependency_ptr(T&) -> dependency_ptr<T>;
} //namespace base::vocab::inline ptr
