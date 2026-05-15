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

namespace base::vocab::inline ptr::ptr_policies::traversal {
    struct policy_group_tag;

    template<typename ConcretePtr>
        requires (!is_void_v<typename ConcretePtr::element_type>)
    struct arithmetic {
    private:
        using pointee = typename ConcretePtr::element_type;

    public:
        using policy_group = policy_group_tag;

        //================================================================================
        // Arithmetic Operators: Implemented for Iteration
        //================================================================================

        ///@brief Subscript operator provided solely to comply with random-access iterator requirements.
        [[nodiscard]] [[deprecated(
            "Subscript operator conflates pointers with arrays. Use `*(ptr + offset)` for explicit traversal or consider subscripting the container instead."
        )]]
        constexpr auto& operator[](this auto self, difference_type offset) noexcept
            requires base::meta::concepts::CompletePointee<pointee>
        {
            return *(self + offset);
        }

        ///@brief Prefix increment: increments the stored address.
        template<typename Self>
        constexpr decltype(auto) operator++(this Self&& self) noexcept
            requires base::meta::concepts::CompletePointee<pointee>
        {
            ++self.address_;
            return std::forward<Self>(self);
        }

        ///@brief Prefix decrement: decrements the stored address.
        template<typename Self>
        constexpr decltype(auto) operator--(this Self&& self) noexcept
            requires base::meta::concepts::CompletePointee<pointee>
        {
            --self.address_;
            return std::forward<Self>(self);
        }

        ///@brief Postfix increment: increments the stored address but return a pointer to the prior stored address.
        template<typename Self>
        constexpr auto operator++(this Self&& self, int) noexcept
            requires base::meta::concepts::CompletePointee<pointee>
        {
            std::decay_t<Self> old{self};
            ++self;
            return old;
        }

        ///@brief Postfix decrement: decrements the stored address but return a pointer to the prior stored address.
        template<typename Self>
        constexpr auto operator--(this Self&& self, int) noexcept
            requires base::meta::concepts::CompletePointee<pointee>
        {
            std::decay_t<Self> old{self};
            --self;
            return old;
        }

        ///@brief Addition assignment: increments the stored address by a given distance.
        template<typename Self>
        constexpr decltype(auto) operator+=(this Self&& self, difference_type diff) noexcept
            requires base::meta::concepts::CompletePointee<pointee>
        {
            self.address_ += diff;
            return std::forward<Self>(self);
        }

        ///@brief Subtraction assignment: decrements the stored address by a given distance.
        template<typename Self>
        constexpr decltype(auto) operator-=(this Self&& self, difference_type diff) noexcept
            requires base::meta::concepts::CompletePointee<pointee>
        {
            self.address_ -= diff;
            return std::forward<Self>(self);
        }

        ///@brief Pointer addition: gets a pointer to an address a given distance after the stored address.
        friend constexpr ConcretePtr operator+(ConcretePtr ptr, difference_type diff) noexcept requires base::meta::concepts::CompletePointee<pointee> { return ptr += diff; }

        ///@brief Pointer addition (commutative): gets a pointer to an address a given distance after the stored address.
        friend constexpr ConcretePtr operator+(difference_type diff, ConcretePtr ptr) noexcept requires base::meta::concepts::CompletePointee<pointee> { return ptr += diff; }

        ///@brief Pointer subtraction: gets a pointer to an address a given distance before the stored address.
        friend constexpr ConcretePtr operator-(ConcretePtr ptr, difference_type diff) noexcept requires base::meta::concepts::CompletePointee<pointee> { return ptr -= diff; }

        ///@brief Pointer subtraction (difference): computes the distance between the addresses stored in two pointers.
        friend constexpr difference_type operator-(ConcretePtr lhs, ConcretePtr rhs) noexcept requires base::meta::concepts::CompletePointee<pointee> { return lhs.get() - rhs.get(); }
    }; //struct arithmetic

    template<typename ConcretePtr>
    struct rebinding {
    private:
        using pointee = typename ConcretePtr::element_type;

    public:
        using policy_group = policy_group_tag;

        //================================================================================
        // Deleted Pointer Arithmetic Operators: Not an Iterator
        //================================================================================

        ///@brief Deleted prefix increment to prevent misuse as an iterator.
        template<typename Self>
        Self& operator++(this Self&&) =
            delete /*("Prefix increment deleted to prevent pointer arithmetic. `traversal::rebinding` pointers are not iterators.")*/;

        ///@brief Deleted prefix decrement to prevent misuse as an iterator.
        template<typename Self>
        Self& operator--(this Self&&) =
            delete /*("Prefix decrement deleted to prevent pointer arithmetic. `traversal::rebinding` pointers are not iterators.")*/;

        ///@brief Deleted postfix increment to prevent misuse as an iterator.
        template<typename Self>
        Self operator++(this Self&&, int) =
            delete /*("Postfix increment deleted to prevent pointer arithmetic. `traversal::rebinding` pointers are not iterators.")*/;

        ///@brief Deleted postfix decrement to prevent misuse as an iterator.
        template<typename Self>
        Self operator--(this Self&&, int) =
            delete /*("Postfix decrement deleted to prevent pointer arithmetic. `traversal::rebinding` pointers are not iterators.")*/;

        ///@brief Deleted addition assignment to prevent misuse as an iterator.
        template<typename Self>
        Self& operator+=(this Self&&, difference_type) =
            delete /*("Addition assignment deleted to prevent pointer arithmetic. `traversal::rebinding` pointers are not iterators.")*/;

        ///@brief Deleted subtraction assignment to prevent misuse as an iterator.
        template<typename Self>
        Self& operator-=(this Self&&, difference_type) =
            delete /*("Subtraction assignment deleted to prevent pointer arithmetic. `traversal::rebinding` pointers are not iterators.")*/;

        ///@brief Deleted pointer addition to prevent misuse as an iterator.
        friend ConcretePtr operator+(ConcretePtr, difference_type) =
            delete /*("Pointer addition deleted to prevent pointer arithmetic. `traversal::rebinding` pointers are not iterators.")*/;

        ///@brief Deleted pointer addition to prevent misuse as an iterator.
        friend ConcretePtr operator+(difference_type, ConcretePtr) =
            delete /*("Pointer addition deleted to prevent pointer arithmetic. `traversal::rebinding` pointers are not iterators.")*/;

        ///@brief Deleted pointer subtraction to prevent misuse as an iterator.
        friend difference_type operator-(ConcretePtr, ConcretePtr) =
            delete /*("Pointer subtraction deleted to prevent pointer arithmetic. `traversal::rebinding` pointers are not iterators.")*/;

        ///@brief Deleted pointer subtraction to prevent misuse as an iterator.
        friend ConcretePtr operator-(ConcretePtr, difference_type) =
            delete /*("Pointer subtraction deleted to prevent pointer arithmetic. `traversal::rebinding` pointers are not iterators.")*/;
    }; //struct rebinding
} //namespace base::vocab::inline ptr::ptr_policies::traversal
