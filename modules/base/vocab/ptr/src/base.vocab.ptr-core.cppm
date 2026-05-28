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

export import :policies;

namespace base::vocab::inline ptr {
    template<typename T>
    concept VocabPtr = requires { typename T::derived_from_ptr_core; };

    template<typename Pointee>
    inline constexpr bool valid_pointee_v = (!std::is_reference_v<Pointee> && !std::is_function_v<base::meta::traits::remove_all_indirections_t<Pointee>>);
} //namespace base::vocab::inline ptr

export namespace base::vocab::inline ptr {
    template<template<typename> typename ConcretePtr, typename Pointee, ptr_policies::PtrPolicyList PolicySet>
        requires valid_pointee_v<Pointee>
    class ptr_core : public pointer_metadata<Pointee> {
    public:
        struct derived_from_ptr_core;

    private:
        using policy_set = PolicySet;
        using metadata   = pointer_metadata<Pointee>;

        metadata::pointer address_; ///<@brief The stored address used by all concrete pointer types.

    public:
        using iterator_concept = std::conditional_t<ptr_policies::arithmetic_traversal_v<policy_set>, std::contiguous_iterator_tag, void>;
        using iterator_category = std::conditional_t<ptr_policies::arithmetic_traversal_v<policy_set>, std::random_access_iterator_tag, void>;

        //================================================================================
        // Construction, Assignment, and Swap
        //================================================================================

        //===== Universal Core =====

        ///@brief (Conversion) Implicitly converts from another pointer according to nested `pointer` type conversions.
        template<typename OtherPointee>
            requires (!std::same_as<OtherPointee, Pointee>) && std::convertible_to<std::add_pointer_t<OtherPointee>, typename metadata::pointer>
        constexpr explicit(false) ptr_core(const ConcretePtr<OtherPointee>& source) noexcept : address_(source.get())
        {}

        ///@brief (Conversion) Assigns from another pointer according to nested `pointer` type conversions.
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
            : address_(std::addressof(source))
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
            : address_(apply_nullability_policy(std::forward<P>(source)))
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
                  && requires(Pointer<Element, Args...> ptr) {
                         { std::as_const(ptr).get() } -> std::convertible_to<typename metadata::pointer>;
                     }
        constexpr explicit(std::is_void_v<Pointee>) ptr_core(const Pointer<Element, Args...>& source) noexcept(noexcept(apply_nullability_policy(source.get())))
            requires ptr_policies::allowed_pointer_binding_v<policy_set>
            : address_(apply_nullability_policy(source.get()))
        {}

        ///@brief Assigns from another pointer-like type.
        template<typename Self, template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                  && (!std::is_const_v<Self>)
                  && requires(Pointer<Element, Args...> ptr) {
                         { std::as_const(ptr).get() } -> std::convertible_to<typename metadata::pointer>;
                     }
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
            : address_(null) {}

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
