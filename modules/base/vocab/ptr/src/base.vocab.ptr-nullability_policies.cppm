// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-nullability_policies.cppm
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
 * @brief Policies governing pointer nullability.
 */

//Module partition interface unit
export module base.vocab.ptr:nullability_policies;

import std;

import base.meta.traits;
import base.meta.concepts;

namespace base::vocab::inline ptr::ptr_policies::nullability {
    struct policy_group_tag;

    template<template<typename> typename ConcretePtr, typename Pointee>
    struct yes {
    private:
        using pointer = typename ConcretePtr<Pointee>::pointer;

    public:
        using policy_group = policy_group_tag;

        auto is_constructor_explicit() -> std::false_type;

        ///@brief Resolves address from no arguments to provide default consteuctor by returning `nullptr` converted to `pointer` type.
        [[nodiscard]] constexpr pointer resolve_address() { return nullptr; }

        auto is_constructor_explicit(std::nullptr_t) -> std::false_type;

        ///@brief Resolves address from `nullptr` by converting it to `pointer` type.
        [[nodiscard]] constexpr pointer resolve_address(std::nullptr_t null) { return null; }

        ///@brief Contextually converts to `bool` to test if the pointer is engaged.
        [[nodiscard]] constexpr explicit operator bool(this auto&& self) noexcept { return (self.get() != nullptr); }

        ///@brief Compares equality against `nullptr`.
        [[nodiscard]] friend constexpr bool operator==(const ConcretePtr<Pointee>& ptr, std::nullptr_t null) noexcept
        {
            return (ptr.get() == null);
        }

        ///@brief Returns a raw pointer to the stored address while disengaging the pointer.
        [[nodiscard]] constexpr pointer release(this auto&& self) noexcept { return std::exchange(self.address_, nullptr); }

        ///@brief Reset the pointer to `nullptr`.
        constexpr void reset(this auto& self, std::nullptr_t null = nullptr) noexcept { self = null; }

    protected:
        ///@brief Passes the pointer through unchecked because this policy accepts null pointers.
        [[nodiscard]] constexpr pointer validate_by_nullability(pointer source) { return source; }
    }; //struct yes

    template<template<typename> typename ConcretePtr, typename Pointee>
    struct no {
    private:
        using pointer = typename ConcretePtr<Pointee>::pointer;

    public:
        using policy_group = policy_group_tag;

        ///@brief Deleted default constructor to prevent sources of null initialization.
        no() =
            delete /*("Default constructor deleted to prevent null initialization. Use `std::optional<ptr_type<T>>` for default-constructible optional pointers.")*/
            ;

        ///@brief Deleted address resolution from no arguments to prevent indirect default construction.
        pointer resolve_address() =
            delete /*("No-argument address resolution deleted to prevent null initialization. Use `std::optional<ptr_type<T>>` for default-constructible optional pointers.")*/
            ;

        ///@brief Deleted constructor from `nullptr` to prevent sources of null initialization.
        no(std::nullptr_t) =
            delete /*("Constructor from `nullptr` deleted to prevent null initialization. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        ///@brief Deleted assignment from `nullptr` to prevent sources of invalid null rebinding.
        ConcretePtr<Pointee>& operator=(std::nullptr_t) =
            delete /*("Assignment from `nullptr` deleted to prevent null rebinding. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        ///@brief Deleted address resolution from `nullptr` to prevent sources of invalid null rebinding.
        pointer resolve_address(std::nullptr_t) =
            delete /*("Address resolution from `nullptr` deleted to prevent null rebinding. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        ///@brief Contextually converts to `bool` to "test" if the pointer is engaged. Always returns `true` to confirm invariant.
        [[nodiscard]] constexpr explicit operator bool() const noexcept { return true; }

    protected:
        ///@brief Enforces the non-null invariant by only passing the address through when it is not null.
        [[nodiscard]] constexpr pointer validate_by_nullability(pointer source)
        {
            if (source == nullptr) [[unlikely]]
                throw std::invalid_argument("`nullability::no` pointers cannot be constructed or assigned from a null pointer.");
            return source;
        }
    }; //struct no
} //namespace base::vocab::inline ptr::ptr_policies::nullability
