// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-core.cppm
 * @version 0.6.0
 * @date May 28, 2026
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
 * @brief Core pointer vocabulary infrastructure.
 *
 * @note Deleted overloads intentionally use forwarding explicit object parameters to ensure policy diagnostics dominate value-category diagnostics during overload resolution.
 */

//Module partition interface unit
export module base.vocab.ptr:core;

import std;

import base.meta.sequences;
import base.meta.traits;
import base.meta.concepts;

import :metadata;

#ifdef CONTENTS_OF_METADATA_PARTITION_FOR_EXPOSITION_ONLY
namespace base::vocab::inline ptr {
    template<typename Pointee>
    struct pointer_metadata {
    private:
        struct void_reference;

    public:
        /**
         * @typedef element_type
         * @brief The stored element type.
         */
        using element_type = Pointee;

        /**
         * @typedef value_type
         * @brief The unqualified element type (`std::remove_cv_t<Pointee>`).
         */
        using value_type = std::remove_cv_t<Pointee>;

        /**
         * @typedef pointer
         * @brief The raw pointer type of the stored address (`Pointee*`).
         */
        using pointer = std::add_pointer_t<Pointee>;

        /**
         * @typedef reference
         * @brief The reference type (`Pointee&`).
         * @remark When `Pointee` is `void`, uses `void_reference&` because `void` as a function parameter is ill-formed.
         */
        using reference = std::conditional_t<
            std::is_void_v<Pointee>,
            std::add_lvalue_reference_t<void_reference>,
            std::add_lvalue_reference_t<Pointee>
        >;

        /**
         * @typedef rvalue_reference
         * @brief The rvalue reference type (`Pointee&&`).
         *
         * @remark When `Pointee` is `void`, uses `void_reference&&` because `void` as a function parameter is ill-formed.
         * @note Used only for deletion of invalid overloads to prevent binding to temporaries.
         */
        using rvalue_reference = std::conditional_t<
            std::is_void_v<Pointee>,
            std::add_rvalue_reference_t<void_reference>,
            std::add_rvalue_reference_t<Pointee>
        >;

        /**
         * @typedef difference_type
         * @brief Pointer difference type (`std::ptrdiff_t`).
         *
         * @note Provided to model pointer interface even when arithmetic is disabled.
         */
        using difference_type = std::ptrdiff_t;
    }; //struct pointer_metadata
} //namespace base::vocab::inline ptr
#endif

export import :policies;

namespace base::vocab::inline ptr {
    template<typename T>
    concept VocabPtr = requires { typename T::derived_from_ptr_core; };

    template<typename Pointee>
    inline constexpr bool is_valid_pointee_v = (!std::is_reference_v<Pointee> && !std::is_function_v<base::meta::traits::remove_all_indirections_t<Pointee>>);

    template<typename Source, typename Target>
    inline constexpr bool is_smart_ptr_convertible_to_v = requires(Source ptr) {
                                                              { std::as_const(ptr).get() } -> std::convertible_to<Target>;
                                                          };

    // Primary template: Null is NOT allowed. No default initializer provided.
    template<typename AddressType, bool IsNullable>
        requires std::is_pointer_v<AddressType>
    class address_storage {
    private:
        AddressType stored_address_; ///< @note Intentionally uninitialized.
    public:
        constexpr explicit(false) address_storage(AddressType source) noexcept : stored_address_{source} {}
        [[nodiscard]] constexpr explicit(false) operator AddressType() const noexcept { return stored_address_; }

        template<typename Self>
            requires (!std::is_const_v<Self>)
        constexpr Self& operator=(this Self& self, AddressType source) noexcept
        {
            self.stored_address_ = source;
            return self;
        }
        
        ///@brief Prefix increment: increments the stored address.
        template<typename Self>
        constexpr decltype(auto) operator++(this Self&& self) noexcept
        {
            ++self.stored_address_;
            return std::forward<Self>(self);
        }

        ///@brief Prefix decrement: decrements the stored address.
        template<typename Self>
        constexpr decltype(auto) operator--(this Self&& self) noexcept
        {
            --self.stored_address_;
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
        constexpr decltype(auto) operator+=(this Self&& self, std::ptrdiff_t diff) noexcept
        {
            self.stored_address_ += diff;
            return std::forward<Self>(self);
        }

        ///@brief Subtraction assignment: decrements the stored address by a given distance.
        template<typename Self>
        constexpr decltype(auto) operator-=(this Self&& self, std::ptrdiff_t diff) noexcept
        {
            self.stored_address_ -= diff;
            return std::forward<Self>(self);
        }
#if 0
        ///@brief Pointer addition: gets a pointer to an address a given distance after the stored address.
        friend constexpr address_storage operator+(address_storage ptr, std::ptrdiff_t diff) noexcept
        {
            return ptr += diff;
        }

        ///@brief Pointer addition (commutative): gets a pointer to an address a given distance after the stored address.
        friend constexpr address_storage operator+(std::ptrdiff_t diff, address_storage ptr) noexcept
        {
            return ptr += diff;
        }

        ///@brief Pointer subtraction: gets a pointer to an address a given distance before the stored address.
        friend constexpr address_storage operator-(address_storage ptr, std::ptrdiff_t diff) noexcept
        {
            return ptr -= diff;
        }

        ///@brief Pointer subtraction (difference): computes the distance between the addresses stored in two pointers.
        friend constexpr std::ptrdiff_t operator-(address_storage lhs, address_storage rhs) noexcept
        {
            return lhs.get() - rhs.get();
        }
#endif
    };

    // Specialization: Null IS allowed. Default initializer provided.
    template<typename AddressType>
        requires std::is_pointer_v<AddressType>
    class address_storage<AddressType, true> {
    private:
        AddressType stored_address_{}; ///< @note Value initialized to null.
    public:
        constexpr address_storage() noexcept = default;
        constexpr explicit(false) address_storage(AddressType source) noexcept : stored_address_{source} {}
        [[nodiscard]] constexpr explicit(false) operator AddressType() const noexcept { return stored_address_; }

        template<typename Self>
            requires (!std::is_const_v<Self>)
        constexpr Self& operator=(this Self& self, AddressType source) noexcept
        {
            self.stored_address_ = source;
            return self;
        }
        
        ///@brief Prefix increment: increments the stored address.
        template<typename Self>
        constexpr decltype(auto) operator++(this Self&& self) noexcept
        {
            ++self.stored_address_;
            return std::forward<Self>(self);
        }

        ///@brief Prefix decrement: decrements the stored address.
        template<typename Self>
        constexpr decltype(auto) operator--(this Self&& self) noexcept
        {
            --self.stored_address_;
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
        constexpr decltype(auto) operator+=(this Self&& self, std::ptrdiff_t diff) noexcept
        {
            self.stored_address_ += diff;
            return std::forward<Self>(self);
        }

        ///@brief Subtraction assignment: decrements the stored address by a given distance.
        template<typename Self>
        constexpr decltype(auto) operator-=(this Self&& self, std::ptrdiff_t diff) noexcept
        {
            self.stored_address_ -= diff;
            return std::forward<Self>(self);
        }
    };
} //namespace base::vocab::inline ptr

export namespace base::vocab::inline ptr {
    /**
     * @class ptr_core
     * @brief Policy-driven foundational layer for the vocabulary pointer suite.
     *
     * @tparam ConcretePtr The derived template pointer specialization (CRTP-like template-template pattern).
     * @tparam Pointee The type of the object being pointed to.
     * @tparam PolicySet A type-list configuration enforcing rules across traversability, binding, and nullability axes.
     *
     * ### Architecture & Design Intent
     * `ptr_core` centralizes universal raw pointer management and structural type-traits to prevent API and implementation
     * drift among specialized types (`alias_ptr`, `required_ptr`, `dependency_ptr`, and `cursor_ptr`). Rather than using 
     * classic polymorphic inheritance or verbose mixins, it leverages a **Nybble-driven Policy Mapping System** to selectively
     * expose or `= delete` fundamental operations at compile time based on the structural requirements of the concrete type.
     *
     * ### Structural Policy Propagation
     * Behavior is customized statically through boolean flags embedded within the `PolicySet`. This approach yields zero
     * runtime overhead via Empty Base Optimization (EBO) and compiler optimization of constrained overloads:
     *
     * Policy Group        | Policy State                   | Invariant / Interface Changes
     * ------------------- | ------------------------------ | -----------------------------------------------------------------------
     * `traversal`         | `traversal::rebinding`         | Exposes purely invariant identity comparison (`operator==`). Deletes
     *                     |                                | all pointer arithmetic and ordering operators (`operator<=>`,
     *                     |                                | `operator++`, etc.).
     *                     | `traversal::arithmetic`        | Models `std::contiguous_iterator`. Exposes full relational operators,
     *                     |                                | displacement operators (`operator+=`, `operator+`), and defines valid
     *                     |                                | standard iterator tags.
     * `pointer_binding`   | `pointer_binding::allowed`     | Synthesizes constructors and assignment operators from matching raw
     *                     |                                | pointers and compatible smart pointers.
     *                     | `pointer_binding::forbidden`   | Explicitly deletes raw pointer constructors to enforce alternate
     *                     |                                | initialization sequences (e.g., forcing reference-only binding).
     * `reference_binding` | `reference_binding::allowed`   | Synthesizes constructors and assignment operators from matching
     *                     |                                | lvalue references while deleting them from rvalue references to
     *                     |                                | eliminate a source of reference dangling (binding to a temporary).
     *                     | `reference_binding::forbidden` | Deletes consteuction and assignment from both lvalue and rvalue
     *                     |                                | references.
     * `nullability`       | `nullability::yes`             | Provides `nullptr` constructors/assignments, `release()`, a `nullptr`
     *                     |                                | sensitive `reset()`, and contextual conversion to `bool` checking for
     *                     |                                | engagement.
     *                     | `nullability::no`              | Deletes `nullptr_t` overloads, deletes the default constructor, and
     *                     |                                | forces a contextual conversion to `bool` that unconditionally returns
     *                     |                                | `true` to optimize validation paths.
     *
     * ### Explicit Object Parameters ("Deducing This")
     * This facade utilizes C++23 explicit object parameters across its interface. By abstracting the value category and 
     * cv-qualification of the calling instance via `this auto&& self`, the class eliminates the traditional explosion of 
     * four-way cv/ref qualifiers for accessors (`get()`, `operator*()`, `operator->()`). 
     * * @note Deletion signatures deliberately use forwarding explicit object parameters (`this Self&&`). This architectural choice 
     * ensures that during overload resolution, explicit policy-driven diagnostic errors cleanly dominate over casual 
     * value-category mismatches, providing readable compiler errors.
     *
     * ### Memory Safety & Invariants
     * - **Array Decay Prevention:** Constructors taking raw C-arrays are explicitly intercepted and deleted for binding-enabled
     * configurations to block inadvertent pointer-decay errors when targeting blocks of contiguous memory.
     * - **Lifetime Sanitization:** Direct binding from temporary variables (`rvalue_references` or pointer-like temporaries) 
     * is structurally blocked using deleted sinks, eliminating a primary vector for dangling pointers at the library boundary.
     * - **Layout Guarantee:** Concrete pointers derived from `ptr_core` maintain a strict `static_assert(std::is_standard_layout_v)`
     * footprint, preserving the visual and physical properties of a scalar raw pointer.
     */
    template<template<typename> typename ConcretePtr, typename Pointee, ptr_policies::PtrPolicyList PolicySet>
        requires is_valid_pointee_v<Pointee>
    class ptr_core : public pointer_metadata<Pointee> {
    public:
        struct derived_from_ptr_core;

    private:
        using policy_set = PolicySet;
        using metadata   = pointer_metadata<Pointee>;
        using address_t  = address_storage<typename metadata::pointer, ptr_policies::nullable_v<policy_set>>;

        address_t address_; ///<@brief The stored address used by all concrete pointer types.

    public:
        using iterator_concept = std::conditional_t<ptr_policies::arithmetic_traversal_v<policy_set>, std::contiguous_iterator_tag, void>;
        using iterator_category = std::conditional_t<ptr_policies::arithmetic_traversal_v<policy_set>, std::random_access_iterator_tag, void>;

        //================================================================================
        // Construction, Assignment, and Swap
        //================================================================================

        //===== Universal Core =====

        ///@brief (Conversion) Implicitly converts from another `ConcretePtr` specialization according to nested `metadata::pointer` type conversions.
        template<typename OtherPointee>
            requires (!std::same_as<OtherPointee, Pointee>) && std::convertible_to<std::add_pointer_t<OtherPointee>, typename metadata::pointer>
        constexpr explicit(false) ptr_core(const ConcretePtr<OtherPointee>& source) noexcept : address_{source.get()}
        {}

        ///@brief (Conversion) Assigns from another `ConcretePtr` specialization according to nested `metadata::pointer` type conversions.
        template<typename Self, typename OtherPointee>
            requires (!std::is_const_v<Self>) && (!std::same_as<OtherPointee, Pointee>) && std::convertible_to<std::add_pointer_t<OtherPointee>, typename metadata::pointer>
        constexpr Self& operator=(this Self& self, const ConcretePtr<OtherPointee>& source) noexcept
        {
            self.address_ = source.get();
            return self;
        }

        ///@brief Swaps addresses.
        friend constexpr void swap(ptr_core& lhs, ptr_core& rhs) noexcept
        {
            using std::swap;
            swap(lhs.address_, rhs.address_);
        }

        //===== Reference Binding (Allowed) =====

        ///@brief Constructs a pointer bound to an existing object.
        constexpr explicit ptr_core(metadata::reference source) noexcept
            requires ptr_policies::allowed_reference_binding_v<policy_set>
            : address_{std::addressof(source)}
        {}

        ///@brief Rebinds the pointer to another object.
        template<typename Self>
            requires (!std::is_const_v<Self>)
        constexpr Self& operator=(this Self& self, metadata::reference source) noexcept
            requires ptr_policies::allowed_reference_binding_v<policy_set>
        {
            self.address_ = std::addressof(source);
            return self;
        }

        //===== Pointer Binding (Allowed)  =====

        ///@brief Implicitly converts from a raw `pointer`. Explicit when `Pointee` is void to avoid implicit conversion chaining.
        template<typename P>
            requires std::is_pointer_v<std::remove_cvref_t<P>>
                  && std::convertible_to<std::decay_t<P>, typename metadata::pointer>
        constexpr explicit(std::is_void_v<Pointee>) ptr_core(P&& source) noexcept(noexcept(apply_nullability_policy(std::forward<P>(source))))
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
            : address_{apply_nullability_policy(std::forward<P>(source))}
        {}

        ///@brief Assigns from a raw `pointer`.
        template<typename Self, typename P>
            requires (!std::is_const_v<Self>) && std::is_pointer_v<std::remove_cvref_t<P>>
                  && std::convertible_to<std::decay_t<P>, typename metadata::pointer>
        constexpr Self& operator=(this Self& self, P&& source) noexcept(noexcept(apply_nullability_policy(std::forward<P>(source))))
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
        {
            self.address_ = apply_nullability_policy(std::forward<P>(source));
            return self;
        }

        ///@brief Implicitly converts from another pointer-like type. Explicit when `Pointee` is void to avoid implicit conversion chaining.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                  && is_smart_ptr_convertible_to_v<Pointer<Element, Args...>, typename metadata::pointer>
        constexpr explicit(std::is_void_v<Pointee>) ptr_core(const Pointer<Element, Args...>& source) noexcept(noexcept(apply_nullability_policy(source.get())))
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
            : address_{apply_nullability_policy(source.get())}
        {}

        ///@brief Assigns from another pointer-like type.
        template<typename Self, template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                  && (!std::is_const_v<Self>)
                  && is_smart_ptr_convertible_to_v<Pointer<Element, Args...>, typename metadata::pointer>
        constexpr Self& operator=(this Self& self, const Pointer<Element, Args...>& source) noexcept(noexcept(apply_nullability_policy(source.get())))
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
        {
            self.address_ = apply_nullability_policy(source.get());
            return self;
        }

        //===== Nullability (Yes) =====

        ///@brief Default constructor initializes to null.
        ptr_core() noexcept
            requires ptr_policies::nullable_v<policy_set>
        = default;

        ///@brief Constructor from `nullptr` initializes to null.
        ptr_core(std::nullptr_t null) noexcept
            requires ptr_policies::nullable_v<policy_set>
            : address_{null} {}

        ///@brief Assignment from `nullptr` rebinds to null.
        template<typename Self>
            requires (!std::is_const_v<Self>)
        Self& operator=(this Self& self, std::nullptr_t null) noexcept
            requires ptr_policies::nullable_v<policy_set>
        {
            self.address_ = null;
            return self;
        }

        //================================================================================
        // Deleted Constructors and Assignment Operators: Non-Null Structural Invariant
        //================================================================================

        //===== Nullability (No) =====

        ///@brief Deleted default constructor to prevent sources of null initialization.
        ptr_core()
            requires ptr_policies::nonnullable_v<policy_set>
        = delete /*("Default constructor deleted by policy `nullability::no` to prevent null initialization. Use `std::optional<ptr_type<T>>` for default-constructible optional pointers.")*/
            ;

        ///@brief Deleted constructor from `nullptr` to prevent sources of null initialization.
        ptr_core(std::nullptr_t)
            requires ptr_policies::nonnullable_v<policy_set>
        = delete /*("Constructor from `nullptr` deleted to prevent null initialization. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        ///@brief Deleted assignment from `nullptr` to prevent sources of invalid null rebinding.
        template<typename Self>
        Self& operator=(this Self&&, std::nullptr_t)
            requires ptr_policies::nonnullable_v<policy_set>
        = delete /*("Assignment from `nullptr` deleted by policy `nullability::no` to prevent null rebinding. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        //===== Pointer Binding (Forbidden) =====

        ///@brief Deleted constructor from `pointer` to structurally guarantee non-null initialization.
        ptr_core(metadata::pointer)
            requires ptr_policies::forbidden_pointer_binding_v<policy_set>
        = delete /*("Constructor from `pointer` deleted by policy `pointer_binding::forbidden`. Dereference first to guarantee non-null initialization. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        ///@brief Deleted assignment from `pointer` to structurally guarantee non-null rebinding.
        template<typename Self>
        Self& operator=(this Self&&, metadata::pointer)
            requires ptr_policies::forbidden_pointer_binding_v<policy_set>
        = delete /*("Assignment from `pointer` deleted by policy `pointer_binding::forbidden`. Dereference first to guarantee non-null initialization. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        ///@brief Deleted constructor from another pointer-like type to structurally guarantee non-null initialization.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                  && requires(Pointer<Element, Args...> ptr) {
                         { std::as_const(ptr).get() } -> std::convertible_to<typename metadata::pointer>;
                     }
        ptr_core(const Pointer<Element, Args...>&)
            requires ptr_policies::forbidden_pointer_binding_v<policy_set>
        = delete /*("Constructor from pointer-like types deleted by policy `pointer_binding::forbidden`. Dereference first to guarantee non-null initialization. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        ///@brief Deleted assignment from another pointer-like type to structurally guarantee non-null rebinding.
        template<typename Self, template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                  && requires(Pointer<Element, Args...> ptr) {
                         { std::as_const(ptr).get() } -> std::convertible_to<typename metadata::pointer>;
                     }
        Self& operator=(this Self&&, const Pointer<Element, Args...>&)
            requires ptr_policies::forbidden_pointer_binding_v<policy_set>
        = delete /*("Assignment from pointer-like types deleted by policy `pointer_binding::forbidden`. Dereference first to guarantee non-null initialization. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        //================================================================================
        // Deleted Constructors and Assignment Operators: No Array-to-Pointer Decay
        //================================================================================

        //===== Pointer Binding (Allowed) =====

        ///@brief Deleted constructor from C-array to prevent array-to-pointer decay.
        template<typename AnyCArray>
            requires std::is_array_v<AnyCArray>
        ptr_core(AnyCArray&)
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
        = delete /*("Constructor from C-array deleted to prevent array-to-pointer decay. To point to the first element, alias it explicitly.")*/
            ;

        ///@brief Deleted assignment from C-array to prevent array-to-pointer decay.
        template<typename Self, typename AnyCArray>
            requires std::is_array_v<AnyCArray>
        Self& operator=(this Self&&, AnyCArray&)
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
        = delete /*("Assignment from C-array deleted to prevent array-to-pointer decay. To point to the first element, alias it explicitly.")*/
            ;

        //================================================================================
        // Deleted Constructors and Assignment Operators: No Aliasing Temporaries
        //================================================================================

        //===== Reference Binding (Allowed) =====

        ///@brief Deleted constructor from `rvalue_reference` to discourage dangling by rejecting direct binding to temporaries.
        ptr_core(metadata::rvalue_reference)
            requires ptr_policies::allowed_reference_binding_v<policy_set>
        = delete /*("Constructor from `rvalue_reference` deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        ///@brief Deleted assignment from `rvalue_reference` to discourage dangling by rejecting direct binding to temporaries.
        template<typename Self>
        Self& operator=(this Self&&, metadata::rvalue_reference)
            requires ptr_policies::allowed_reference_binding_v<policy_set>
        = delete /*("Assignment from `rvalue_reference` deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        //===== Pointer Binding (Allowed) =====

        ///@brief Deleted constructor from pointer-like object rvalue to discourage dangling by rejecting direct binding to temporaries.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                  && requires(Pointer<Element, Args...> ptr) {
                         { std::as_const(ptr).get() } -> std::convertible_to<typename metadata::pointer>;
                     }
        ptr_core(std::add_rvalue_reference_t<Pointer<Element, Args...>>)
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
        = delete /*("Constructor from pointer-like object rvalue deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        ///@brief Deleted assignment from pointer-like object rvalue to discourage dangling by rejecting direct binding to temporaries.
        template<typename Self, template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                  && requires(Pointer<Element, Args...> ptr) {
                         { std::as_const(ptr).get() } -> std::convertible_to<typename metadata::pointer>;
                     }
        Self& operator=(this Self&&, std::add_rvalue_reference_t<Pointer<Element, Args...>>)
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
        = delete /*("Assignment from pointer-like object rvalue deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        //===== Reference Binding (Forbidden) =====

        ///@brief Deleted constructor from `const reference` to forbid binding to lvalue or rvalue references.
        ptr_core(const metadata::reference)
            requires ptr_policies::forbidden_reference_binding_v<policy_set>
        = delete /*("Constructor from references deleted by policy `reference_binding::forbidden`. Try constructing from the address directly.")*/
            ;

        ///@brief Deleted assignment from `const references` to forbid binding to lvalue or rvalue references.
        template<typename Self>
        Self& operator=(this Self&&, const metadata::reference)
            requires ptr_policies::forbidden_reference_binding_v<policy_set>
        = delete /*("Assignment from references deleted by policy `reference_binding::forbidden`. Try assigning from the address directly.")*/
            ;

        //================================================================================
        // Pointer Operations
        //================================================================================

        //===== Universal Core =====

        ///@brief Provides member access to the pointee object.
        [[nodiscard]] constexpr metadata::pointer operator->(this auto&& self) noexcept
            requires base::meta::concepts::CompletePointee<Pointee>
        {
            return self.address_;
        }

        ///@brief Provides a reference to the pointee object.
        [[nodiscard]] constexpr auto& operator*(this auto&& self) noexcept
            requires base::meta::concepts::CompletePointee<Pointee>
        {
            return *self.address_;
        }

        ///@brief Returns a raw pointer to the stored address.
        [[nodiscard]] constexpr metadata::pointer get(this auto&& self) noexcept { return self.address_; }

        ///@brief Implicitly converts to the nested `pointer` type.
        [[nodiscard]] constexpr explicit(false) operator typename metadata::pointer() const noexcept { return this->get(); }

        ///@brief Rebinding passes through to assignment.
        template<typename Self, typename P>
        Self& rebind(this Self& self, P&& source) noexcept(std::is_nothrow_assignable_v<Self&, P>)
            requires std::is_assignable_v<Self&, P>
        {
            return self = std::forward<P>(source);
        }

        ///@brief Resets the pointer to a new address.
        template<typename Self, typename P>
            requires (!std::same_as<P, std::nullptr_t>)
        constexpr void reset(this Self& self, P&& source) noexcept(std::is_nothrow_assignable_v<Self&, P>)
            requires std::is_assignable_v<Self&, P>
        {
            self = std::forward<P>(source);
        }

        //===== Nullability (Yes) =====

        ///@brief Reset the pointer to `nullptr`.
        constexpr void reset(this auto& self, std::nullptr_t null = nullptr) noexcept
            requires ptr_policies::nullable_v<policy_set>
        { self = null; }

        ///@brief Returns a raw pointer to the stored address while disengaging the pointer.
        [[nodiscard]] constexpr metadata::pointer release(this auto&& self) noexcept
            requires ptr_policies::nullable_v<policy_set>
        {
            return std::exchange(self.address_, nullptr);
        }

        ///@brief Compares equality against `nullptr`.
        [[nodiscard]] friend constexpr bool operator==(const ConcretePtr<Pointee>& ptr, std::nullptr_t null) noexcept
            requires ptr_policies::nullable_v<policy_set>
        {
            return (ptr.get() == null);
        }

        ///@brief Contextually converts to `bool` to test if the pointer is engaged.
        [[nodiscard]] constexpr explicit operator bool(this auto&& self) noexcept
            requires ptr_policies::nullable_v<policy_set>
        { return (self.get() != nullptr); }

        //===== Nullability (No) =====

        ///@brief Contextually converts to `bool` to "test" if the pointer is engaged. Always returns `true` to confirm invariant.
        [[nodiscard]] constexpr explicit operator bool(this auto&&) noexcept
            requires ptr_policies::nonnullable_v<policy_set>
        { return true; }

        //================================================================================
        // Arithmetic Operators: Implemented for Iteration, Deleted Otherwise
        //================================================================================

        //===== Traversal (Arithmetic) =====

        ///@brief Subscript operator provided solely to comply with random-access iterator requirements.
        [[nodiscard]] [[deprecated(
            "Subscript operator conflates pointers with arrays. Use `*(ptr + offset)` for explicit traversal or consider subscripting the container instead."
        )]]
        constexpr auto& operator[](this auto self, metadata::difference_type offset) noexcept
            requires base::meta::concepts::CompletePointee<Pointee>
            && ptr_policies::arithmetic_traversal_v<policy_set>
        {
            return *(self + offset);
        }

        ///@brief Prefix increment: increments the stored address.
        template<typename Self>
        constexpr decltype(auto) operator++(this Self&& self) noexcept
            requires base::meta::concepts::CompletePointee<Pointee>
            && ptr_policies::arithmetic_traversal_v<policy_set>
        {
            ++self.address_;
            return std::forward<Self>(self);
        }

        ///@brief Prefix decrement: decrements the stored address.
        template<typename Self>
        constexpr decltype(auto) operator--(this Self&& self) noexcept
            requires base::meta::concepts::CompletePointee<Pointee>
            && ptr_policies::arithmetic_traversal_v<policy_set>
        {
            --self.address_;
            return std::forward<Self>(self);
        }

        ///@brief Postfix increment: increments the stored address but return a pointer to the prior stored address.
        template<typename Self>
        constexpr auto operator++(this Self&& self, int) noexcept
            requires base::meta::concepts::CompletePointee<Pointee>
            && ptr_policies::arithmetic_traversal_v<policy_set>
        {
            std::decay_t<Self> old{self};
            ++self;
            return old;
        }

        ///@brief Postfix decrement: decrements the stored address but return a pointer to the prior stored address.
        template<typename Self>
        constexpr auto operator--(this Self&& self, int) noexcept
            requires base::meta::concepts::CompletePointee<Pointee>
            && ptr_policies::arithmetic_traversal_v<policy_set>
        {
            std::decay_t<Self> old{self};
            --self;
            return old;
        }

        ///@brief Addition assignment: increments the stored address by a given distance.
        template<typename Self>
        constexpr decltype(auto) operator+=(this Self&& self, metadata::difference_type diff) noexcept
            requires base::meta::concepts::CompletePointee<Pointee>
            && ptr_policies::arithmetic_traversal_v<policy_set>
        {
            self.address_ += diff;
            return std::forward<Self>(self);
        }

        ///@brief Subtraction assignment: decrements the stored address by a given distance.
        template<typename Self>
        constexpr decltype(auto) operator-=(this Self&& self, metadata::difference_type diff) noexcept
            requires base::meta::concepts::CompletePointee<Pointee>
            && ptr_policies::arithmetic_traversal_v<policy_set>
        {
            self.address_ -= diff;
            return std::forward<Self>(self);
        }

        ///@brief Pointer addition: gets a pointer to an address a given distance after the stored address.
        friend constexpr ConcretePtr<Pointee> operator+(ConcretePtr<Pointee> ptr, metadata::difference_type diff) noexcept
            requires base::meta::concepts::CompletePointee<Pointee>
            && ptr_policies::arithmetic_traversal_v<policy_set>
        {
            return ptr += diff;
        }

        ///@brief Pointer addition (commutative): gets a pointer to an address a given distance after the stored address.
        friend constexpr ConcretePtr<Pointee> operator+(metadata::difference_type diff, ConcretePtr<Pointee> ptr) noexcept
            requires base::meta::concepts::CompletePointee<Pointee>
            && ptr_policies::arithmetic_traversal_v<policy_set>
        {
            return ptr += diff;
        }

        ///@brief Pointer subtraction: gets a pointer to an address a given distance before the stored address.
        friend constexpr ConcretePtr<Pointee> operator-(ConcretePtr<Pointee> ptr, metadata::difference_type diff) noexcept
            requires base::meta::concepts::CompletePointee<Pointee>
            && ptr_policies::arithmetic_traversal_v<policy_set>
        {
            return ptr -= diff;
        }

        ///@brief Pointer subtraction (difference): computes the distance between the addresses stored in two pointers.
        friend constexpr metadata::difference_type operator-(ConcretePtr<Pointee> lhs, ConcretePtr<Pointee> rhs) noexcept
            requires base::meta::concepts::CompletePointee<Pointee>
            && ptr_policies::arithmetic_traversal_v<policy_set>
        {
            return lhs.get() - rhs.get();
        }

        //===== Traversal (Rebinding) =====

        ///@brief Deleted prefix increment to prevent misuse as an iterator.
        template<typename Self>
        Self& operator++(this Self&&)
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Prefix increment deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/
            ;

        ///@brief Deleted prefix decrement to prevent misuse as an iterator.
        template<typename Self>
        Self& operator--(this Self&&)
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Prefix decrement deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/
            ;

        ///@brief Deleted postfix increment to prevent misuse as an iterator.
        template<typename Self>
        Self operator++(this Self&&, int)
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Postfix increment deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/
            ;

        ///@brief Deleted postfix decrement to prevent misuse as an iterator.
        template<typename Self>
        Self operator--(this Self&&, int)
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Postfix decrement deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/
            ;

        ///@brief Deleted addition assignment to prevent misuse as an iterator.
        template<typename Self>
        Self& operator+=(this Self&&, metadata::difference_type)
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Addition assignment deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/
            ;

        ///@brief Deleted subtraction assignment to prevent misuse as an iterator.
        template<typename Self>
        Self& operator-=(this Self&&, metadata::difference_type)
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Subtraction assignment deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/
            ;

        ///@brief Deleted pointer addition to prevent misuse as an iterator.
        friend ConcretePtr<Pointee> operator+(ConcretePtr<Pointee>, metadata::difference_type)
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Pointer addition deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/
            ;

        ///@brief Deleted pointer addition to prevent misuse as an iterator.
        friend ConcretePtr<Pointee> operator+(metadata::difference_type, ConcretePtr<Pointee>)
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Pointer addition deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/
            ;

        ///@brief Deleted pointer subtraction to prevent misuse as an iterator.
        friend metadata::difference_type operator-(ConcretePtr<Pointee>, ConcretePtr<Pointee>)
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Pointer subtraction deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/
            ;

        ///@brief Deleted pointer subtraction to prevent misuse as an iterator.
        friend ConcretePtr<Pointee> operator-(ConcretePtr<Pointee>, metadata::difference_type)
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Pointer subtraction deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/
            ;

        //================================================================================
        // Comparison Operators (Three-way for Arithmetic Traversal,  Equality for Rebinding)
        //================================================================================

        //===== Traversal (Arithmetic) =====

        ///@brief Compares in terms of pointer identity.
        [[nodiscard]] friend constexpr auto
            operator<=>(const ConcretePtr<Pointee>& lhs, const ConcretePtr<Pointee>& rhs) noexcept
            requires ptr_policies::arithmetic_traversal_v<policy_set>
        {
            return (lhs.get() <=> rhs.get());
        }

        ///@brief Covariantly compares in terms of pointer identity.
        template<std::derived_from<Pointee> DerivedT>
            requires (!std::same_as<DerivedT, Pointee>)
        [[nodiscard]] friend constexpr auto
            operator<=>(const ConcretePtr<Pointee>& lhs, const ConcretePtr<DerivedT>& rhs) noexcept
            requires ptr_policies::arithmetic_traversal_v<policy_set>
        {
            return (lhs.get() <=> rhs.get());
        }

        ///@brief Covariantly compares a `ConcretePtr`-to-base with a raw pointer-to-derived in terms of pointer identity.
        template<std::derived_from<Pointee> DerivedT>
        [[nodiscard]] friend constexpr auto
            operator<=>(const ConcretePtr<Pointee>& lhs, const std::add_pointer_t<DerivedT> rhs) noexcept
            requires ptr_policies::arithmetic_traversal_v<policy_set>
        {
            return (lhs.get() <=> rhs);
        }

        ///@brief Covariantly compares a raw pointer-to-base with a `ConcretePtr`-to-derived in terms of pointer identity.
        template<std::derived_from<Pointee> DerivedT>
        [[nodiscard]] friend constexpr auto operator<=>(const metadata::pointer lhs, const ConcretePtr<DerivedT>& rhs) noexcept
            requires ptr_policies::arithmetic_traversal_v<policy_set>
        {
            return (lhs <=> rhs.get());
        }

        ///@brief Deleted comparison against `nullptr`.
        [[nodiscard]] friend constexpr bool operator==(const ConcretePtr<Pointee>&, std::nullptr_t) noexcept
            requires ptr_policies::arithmetic_traversal_v<policy_set>
        = delete
            ;

        //===== Traversal (Rebinding) =====

        ///@brief Compares equality in terms of pointer identity.
        [[nodiscard]] friend constexpr bool
            operator==(const ConcretePtr<Pointee>& lhs, const ConcretePtr<Pointee>& rhs) noexcept
            requires ptr_policies::rebinding_traversal_v<policy_set>
        {
            return (lhs.get() == rhs.get());
        }

        ///@brief Covariantly compares equality in terms of pointer identity.
        template<std::derived_from<Pointee> DerivedT>
            requires (!std::same_as<DerivedT, Pointee>)
        [[nodiscard]] friend constexpr bool
            operator==(const ConcretePtr<Pointee>& lhs, const ConcretePtr<DerivedT>& rhs) noexcept
            requires ptr_policies::rebinding_traversal_v<policy_set>
        {
            return (lhs.get() == rhs.get());
        }

        ///@brief Covariantly compares equality of an `ConcretePtr`-to-base with a raw pointer-to-derived in terms of pointer identity.
        template<std::derived_from<Pointee> DerivedT>
        [[nodiscard]] friend constexpr bool
            operator==(const ConcretePtr<Pointee>& lhs, const std::add_pointer_t<DerivedT> rhs) noexcept
            requires ptr_policies::rebinding_traversal_v<policy_set>
        {
            return (lhs.get() == rhs);
        }

        ///@brief Covariantly compares equality of a raw pointer-to-base with an `ConcretePtr`-to-derived in terms of pointer identity.
        template<std::derived_from<Pointee> DerivedT>
        [[nodiscard]] friend constexpr bool operator==(const metadata::pointer lhs, const ConcretePtr<DerivedT>& rhs) noexcept
            requires ptr_policies::rebinding_traversal_v<policy_set>
        {
            return (lhs == rhs.get());
        }

        ///@brief Deleted comparison operators to prevent misuse as an iterator or ordered value type.
        template<typename Self>
        auto operator<=>(this Self&&, Self)
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Comparison operators deleted by policy `traversal::rebinding` to prevent address comparisons. ConcretePtrUse `traversal::arithmetic` pointers for iterators.")*/
            ;

        ///@brief Deleted comparison operators to prevent misuse as an iterator or ordered value type.
        auto operator<=>(const metadata::pointer) const
            requires ptr_policies::rebinding_traversal_v<policy_set>
        = delete /*("Comparison operators deleted by policy `traversal::rebinding` to prevent address comparisons. ConcretePtrUse `traversal::arithmetic` pointers for iterators.")*/
            ;

        //================================================================================
        // Stream Output
        //================================================================================

        ///@brief Outputs a `ptr_core` address to a `std::basic_ostream`.
        template<typename CharT, typename Traits>
        friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& stream, const ptr_core& ptr)
        {
            // In order to support pointers to arbitrarily cv-qualified objects:
            // 1. `static_cast` to `const volatile void*` to preserve all qualifiers while converting the pointer to `void*`.
            // 2. `const_cast` to `const void*` to satisfy the inserter's interface which lacks `volatile void*` overloads.
            // This is safe because formatting is a read-only numerical operation on the address.
            return stream << const_cast<const void*>(static_cast<const volatile void*>(ptr.get()));
        }

    private:
        ///@brief Enforces the non-null invariant for `nullability::no` pointers by only passing the address through when it is not null but allows unchecked pass-through otherwise.  
        [[nodiscard]] static constexpr metadata::pointer apply_nullability_policy(metadata::pointer source)
            noexcept(!ptr_policies::nonnullable_v<policy_set>)
        {  
            if constexpr (ptr_policies::nonnullable_v<policy_set>) {  
                if (source == nullptr) [[unlikely]] {  
                    throw std::invalid_argument(  
                        "`nullability::no` pointers cannot be constructed or assigned from a null pointer."  
                    );  
                }  
            }  
            return source;  
        }
    }; //class ptr_core

    /**
     * Post-class method documentation goes here.
     */
     
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
     * @remark This constructor does not transfer ownership nor affect the lifetime of the pointee object.
     */
    /**
     * @fn alias_ptr& alias_ptr::operator=(reference source) noexcept
     *
     * @param source The object to reference.
     * @return Reference to `*this`.
     *
     * @pre `source` must refer to a valid object that outlives the `alias_ptr`.
     *
     * @remark Rebinds the stored address without affecting ownership or pointee lifetime.
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
     * @param source The pointer-like object providing access to a pointer-compatible value via `get()`.
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
     * @param lhs The first pointer object.
     * @param rhs The second pointer object.
     *
     * @post `lhs.get()` equals the value of `rhs.get()` prior to the call.
     * @post `rhs.get()` equals the value of `lhs.get()` prior to the call.
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
     * @remark Compares pointer identity (stored addresses), not pointee object values.
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
     * @remark Compares pointer identity (stored addresses), not pointee object values.
     */
    /**
     * @overload constexpr bool operator==(const alias_ptr& lhs, const std::add_pointer_t<DerivedT> rhs) noexcept
     *
     * @tparam DerivedT The element type, derived from `T`, of the raw pointer.
     *
     * @param lhs The `alias_ptr` being compared.
     * @param rhs The raw pointer to compare against.
     *
     * @return `true` if the stored address in `lhs` equals `rhs`; otherwise `false`.
     *
     * @remark Enables equality comparison with raw pointers-to-derived when that can't be synthesized by implicit conversion to raw pointers.
     * @remark Compares pointer identity (stored addresses), not pointee object values.
     */
    /**
     * @overload constexpr bool operator==(const pointer lhs, const alias_ptr<DerivedT>& rhs) noexcept
     *
     * @tparam DerivedT The element type, derived from `T`, of the right-hand side `alias_ptr`.
     *
     * @param lhs The raw pointer to compare.
     * @param rhs The `alias_ptr` being compared.
     *
     * @return `true` if `lhs` equals the stored address in `rhs`; otherwise `false`.
     *
     * @remark Enables comparison with raw pointers-to-base when that can't be synthesized by implicit conversion to raw pointers.
     * @remark Compares pointer identity (stored addresses), not pointee object values.
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
     * @fn constexpr pointer alias_ptr::operator->() const noexcept
     *
     * @return A raw pointer to the stored address.
     *
     * @pre `get() != nullptr`
     * @pre The pointee object must remain valid (non-dangling).
     *
     * @remark Provides pointee member access semantics.
     */
    /**
     * @fn constexpr reference alias_ptr::operator*() const noexcept
     *
     * @return Reference to the pointee object.
     *
     * @pre `get() != nullptr`
     * @pre The pointee object must remain valid (non-dangling).
     *
     * @remark Equivalent to dereferencing `get()`.
     */
    /**
     * @fn constexpr pointer alias_ptr::get() const noexcept
     *
     * @return A raw pointer to the stored address.
     *
     * @remark Provided for interoperability with pointer-based APIs.
     */
    /**
     * @fn constexpr alias_ptr::operator pointer() const noexcept
     *
     * @return A raw pointer to the stored address.
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
     * @fn constexpr pointer alias_ptr::release() noexcept
     *
     * @return The previously stored address.
     *
     * @post `get() == nullptr`.
     *
     * @remark Returns the stored address and disengages the pointer without affecting ownership or pointee lifetime.
     * @remark Provided for interoperability with generic pointer-like interfaces.
     */
    /**
     * @fn constexpr Self& alias_ptr::rebind(P&& source)
     *
     * @tparam P A pointer-compatible source type assignable to `alias_ptr`.
     *
     * @param source The source used to replace the stored address.
     *
     * @return Reference to `*this`.
     *
     * @post Equivalent to assignment from `std::forward<P>(source)`.
     *
     * @remark Convenience wrapper over assignment for generic pointer-like interoperability.
     */
    /**
     * @fn constexpr void alias_ptr::reset(std::nullptr_t) noexcept
     *
     * @post `get() == nullptr`.
     *
     * @remark Disengages the pointer by resetting the stored address to `nullptr`.
     */
    /**
     * @fn constexpr void alias_ptr::reset(P&& source)
     *
     * @tparam P A pointer-compatible source type assignable to `alias_ptr`.
     *
     * @param source The source used to replace the stored address.
     *
     * @post Equivalent to assignment from `std::forward<P>(source)`.
     *
     * @remark Replaces the stored address without affecting ownership or pointee lifetime.
     */


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

} //namespace base::vocab::inline ptr

/**
 * @brief Partial specialization of `std::hash` for `VocabPtr`s.
 *
 * @tparam T The concrete pointer type.
 *
 * @remark Hashes the underlying stored address rather than pointee object state or values.
 * @remark Consistent with `ptr_core` equality semantics.
 */
export template<base::vocab::ptr::VocabPtr T>
struct std::hash<T> {
    ///@brief Hashes the pointer based on the underlying address.
    [[nodiscard]] constexpr std::size_t operator()(const T& ptr) const noexcept
    {
        return std::hash<typename T::pointer>{}(ptr.get());
    }
};

/**
 * @brief Partial specialization of `std::formatter` for `VocabPtr`s.
 *
 * @tparam T The concrete pointer type.
 * @tparam CharT The character type used by the format string.
 *
 * @remark Formats the stored address according to the rules for its nested `pointer` type.
 */
export template<base::vocab::ptr::VocabPtr T, typename CharT>
struct std::formatter<T, CharT> : std::formatter<const void*, CharT> {
    ///@brief Formats as a raw pointer to the stored address.
    auto format(const T& ptr, auto& ctx) const
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
