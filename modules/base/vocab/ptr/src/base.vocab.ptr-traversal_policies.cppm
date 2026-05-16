// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-traversal_policies.cppm
 * @version 0.6.0
 * @date May 10, 2026
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
 * @brief Policies governing pointer traversal.
 */

//Module partition interface unit
export module base.vocab.ptr:traversal_policies;

import std;

import base.meta.traits;
import base.meta.concepts;

import :metadata;

namespace base::vocab::inline ptr::ptr_policies::traversal {
    struct policy_group_tag;

    template<template<typename> typename ConcretePtr, typename Pointee>
        requires (!std::is_void_v<Pointee>)
    class arithmetic {
        using metadata = pointer_metadata<Pointee>;
    public:
        using policy_group = policy_group_tag;

        //================================================================================
        // Arithmetic Operators: Implemented for Iteration
        //================================================================================

        ///@brief Subscript operator provided solely to comply with random-access iterator requirements.
        [[nodiscard]] [[deprecated(
            "Subscript operator conflates pointers with arrays. Use `*(ptr + offset)` for explicit traversal or consider subscripting the container instead."
        )]]
        constexpr auto& operator[](this auto self, metadata::difference_type offset) noexcept
            requires base::meta::concepts::CompletePointee<Pointee>
        {
            return *(self + offset);
        }

        ///@brief Prefix increment: increments the stored address.
        template<typename Self>
        constexpr decltype(auto) operator++(this Self&& self) noexcept
            requires base::meta::concepts::CompletePointee<Pointee>
        {
            ++self.address_;
            return std::forward<Self>(self);
        }

        ///@brief Prefix decrement: decrements the stored address.
        template<typename Self>
        constexpr decltype(auto) operator--(this Self&& self) noexcept
            requires base::meta::concepts::CompletePointee<Pointee>
        {
            --self.address_;
            return std::forward<Self>(self);
        }

        ///@brief Postfix increment: increments the stored address but return a pointer to the prior stored address.
        template<typename Self>
        constexpr auto operator++(this Self&& self, int) noexcept
            requires base::meta::concepts::CompletePointee<Pointee>
        {
            std::decay_t<Self> old{self};
            ++self;
            return old;
        }

        ///@brief Postfix decrement: decrements the stored address but return a pointer to the prior stored address.
        template<typename Self>
        constexpr auto operator--(this Self&& self, int) noexcept
            requires base::meta::concepts::CompletePointee<Pointee>
        {
            std::decay_t<Self> old{self};
            --self;
            return old;
        }

        ///@brief Addition assignment: increments the stored address by a given distance.
        template<typename Self>
        constexpr decltype(auto) operator+=(this Self&& self, metadata::difference_type diff) noexcept
            requires base::meta::concepts::CompletePointee<Pointee>
        {
            self.address_ += diff;
            return std::forward<Self>(self);
        }

        ///@brief Subtraction assignment: decrements the stored address by a given distance.
        template<typename Self>
        constexpr decltype(auto) operator-=(this Self&& self, metadata::difference_type diff) noexcept
            requires base::meta::concepts::CompletePointee<Pointee>
        {
            self.address_ -= diff;
            return std::forward<Self>(self);
        }

        ///@brief Pointer addition: gets a pointer to an address a given distance after the stored address.
        friend constexpr ConcretePtr<Pointee> operator+(ConcretePtr<Pointee> ptr, metadata::difference_type diff) noexcept requires base::meta::concepts::CompletePointee<Pointee> { return ptr += diff; }

        ///@brief Pointer addition (commutative): gets a pointer to an address a given distance after the stored address.
        friend constexpr ConcretePtr<Pointee> operator+(metadata::difference_type diff, ConcretePtr<Pointee> ptr) noexcept requires base::meta::concepts::CompletePointee<Pointee> { return ptr += diff; }

        ///@brief Pointer subtraction: gets a pointer to an address a given distance before the stored address.
        friend constexpr ConcretePtr<Pointee> operator-(ConcretePtr<Pointee> ptr, metadata::difference_type diff) noexcept requires base::meta::concepts::CompletePointee<Pointee> { return ptr -= diff; }

        ///@brief Pointer subtraction (difference): computes the distance between the addresses stored in two pointers.
        friend constexpr metadata::difference_type operator-(ConcretePtr<Pointee> lhs, ConcretePtr<Pointee> rhs) noexcept requires base::meta::concepts::CompletePointee<Pointee> { return lhs.get() - rhs.get(); }

        //================================================================================
        // Comparison Operators (Three-way)
        //================================================================================

        ///@brief Compares in terms of pointer identity.
        [[nodiscard]] friend constexpr auto operator<=>(const ConcretePtr<Pointee>& lhs, const ConcretePtr<Pointee>& rhs) noexcept = default;

        ///@brief Covariantly compares in terms of pointer identity.
        template<std::derived_from<Pointee> DerivedT>
            requires (!std::same_as<DerivedT, Pointee>)
        [[nodiscard]] friend constexpr auto operator<=>(const ConcretePtr<Pointee>& lhs, const ConcretePtr<DerivedT>& rhs) noexcept
        {
            return (lhs.get() <=> rhs.get());
        }

        ///@brief Covariantly compares a `ConcretePtr`-to-base with a raw pointer-to-derived in terms of pointer identity.
        template<std::derived_from<Pointee> DerivedT>
        [[nodiscard]] friend constexpr auto operator<=>(const ConcretePtr<Pointee>& lhs, const std::add_pointer_t<DerivedT> rhs) noexcept
        {
            return (lhs.get() <=> rhs);
        }

        ///@brief Covariantly compares a raw pointer-to-base with a `ConcretePtr`-to-derived in terms of pointer identity.
        template<std::derived_from<Pointee> DerivedT>
        [[nodiscard]] friend constexpr auto operator<=>(const metadata::pointer lhs, const ConcretePtr<DerivedT>& rhs) noexcept
        {
            return (lhs <=> rhs.get());
        }

        ///@brief Deleted comparison against `nullptr`.
        [[nodiscard]] friend constexpr bool operator==(const ConcretePtr<Pointee>&, std::nullptr_t) noexcept = delete;

    }; //class arithmetic

    template<template<typename> typename ConcretePtr, typename Pointee>
    class rebinding {
        using metadata = pointer_metadata<Pointee>;
    public:
        using policy_group = policy_group_tag;

        //================================================================================
        // Deleted Pointer Arithmetic Operators: Not an Iterator
        //================================================================================

        ///@brief Deleted prefix increment to prevent misuse as an iterator.
        template<typename Self>
        Self& operator++(this Self&&) =
            delete /*("Prefix increment deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/;

        ///@brief Deleted prefix decrement to prevent misuse as an iterator.
        template<typename Self>
        Self& operator--(this Self&&) =
            delete /*("Prefix decrement deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/;

        ///@brief Deleted postfix increment to prevent misuse as an iterator.
        template<typename Self>
        Self operator++(this Self&&, int) =
            delete /*("Postfix increment deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/;

        ///@brief Deleted postfix decrement to prevent misuse as an iterator.
        template<typename Self>
        Self operator--(this Self&&, int) =
            delete /*("Postfix decrement deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/;

        ///@brief Deleted addition assignment to prevent misuse as an iterator.
        template<typename Self>
        Self& operator+=(this Self&&, metadata::difference_type) =
            delete /*("Addition assignment deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/;

        ///@brief Deleted subtraction assignment to prevent misuse as an iterator.
        template<typename Self>
        Self& operator-=(this Self&&, metadata::difference_type) =
            delete /*("Subtraction assignment deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/;

        ///@brief Deleted pointer addition to prevent misuse as an iterator.
        friend ConcretePtr<Pointee> operator+(ConcretePtr<Pointee>, metadata::difference_type) =
            delete /*("Pointer addition deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/;

        ///@brief Deleted pointer addition to prevent misuse as an iterator.
        friend ConcretePtr<Pointee> operator+(metadata::difference_type, ConcretePtr<Pointee>) =
            delete /*("Pointer addition deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/;

        ///@brief Deleted pointer subtraction to prevent misuse as an iterator.
        friend metadata::difference_type operator-(ConcretePtr<Pointee>, ConcretePtr<Pointee>) =
            delete /*("Pointer subtraction deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/;

        ///@brief Deleted pointer subtraction to prevent misuse as an iterator.
        friend ConcretePtr<Pointee> operator-(ConcretePtr<Pointee>, metadata::difference_type) =
            delete /*("Pointer subtraction deleted by policy `traversal::rebinding` to prevent pointer arithmetic. Use `traversal::arithmetic` pointers for iterators.")*/;

        //================================================================================
        // Comparison Operators (Equality Allowed, Others Deleted)
        //================================================================================

        ///@brief Compares equality in terms of pointer identity.
        [[nodiscard]] friend constexpr bool operator==(const ConcretePtr<Pointee>& lhs, const ConcretePtr<Pointee>& rhs) noexcept = default;

        ///@brief Covariantly compares equality in terms of pointer identity.
        template<std::derived_from<Pointee> DerivedT>
            requires (!std::same_as<DerivedT, Pointee> )
        [[nodiscard]] friend constexpr bool operator==(const ConcretePtr<Pointee>& lhs, const ConcretePtr<DerivedT>& rhs) noexcept
        {
            return (lhs.get() == rhs.get());
        }

        ///@brief Covariantly compares equality of an `ConcretePtr`-to-base with a raw pointer-to-derived in terms of pointer identity.
        template<std::derived_from<Pointee> DerivedT>
        [[nodiscard]] friend constexpr bool operator==(const ConcretePtr<Pointee>& lhs, const std::add_pointer_t<DerivedT> rhs) noexcept
        {
            return (lhs.get() == rhs);
        }

        ///@brief Covariantly compares equality of a raw pointer-to-base with an `ConcretePtr`-to-derived in terms of pointer identity.
        template<std::derived_from<Pointee> DerivedT>
        [[nodiscard]] friend constexpr bool operator==(const metadata::pointer lhs, const ConcretePtr<DerivedT>& rhs) noexcept
        {
            return (lhs == rhs.get());
        }

        ///@brief Compares equality against `nullptr`.
        [[nodiscard]] friend constexpr bool operator==(const ConcretePtr<Pointee>& ptr, std::nullptr_t null) noexcept
        {
            return (ptr.get() == null);
        }

        ///@brief Deleted comparison operators to prevent misuse as an iterator or ordered value type.
        template<typename Self>
        auto operator<=>(this Self&&, Self) =
            delete /*("Comparison operators deleted by policy `traversal::rebinding` to prevent address comparisons. ConcretePtrUse `traversal::arithmetic` pointers for iterators.")*/;

        ///@brief Deleted comparison operators to prevent misuse as an iterator or ordered value type.
        auto operator<=>(const metadata::pointer) const =
            delete /*("Comparison operators deleted by policy `traversal::rebinding` to prevent address comparisons. ConcretePtrUse `traversal::arithmetic` pointers for iterators.")*/;

    }; //class rebinding
} //namespace base::vocab::inline ptr::ptr_policies::traversal
