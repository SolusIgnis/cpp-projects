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

import :metadata;

namespace base::vocab::inline ptr::ptr_policies::nullability {
    struct policy_group_tag;

    template<template<typename> typename ConcretePtr, typename Pointee>
    class yes {
        using metadata = pointer_metadata<Pointee>;
    public:
        using policy_group = policy_group_tag;

        auto is_constructor_explicit() -> std::false_type;

        ///@brief Resolves address from no arguments to provide default constructor by returning `nullptr` converted to `pointer` type.
        [[nodiscard]] constexpr metadata::pointer resolve_address() { return nullptr; }

        auto is_constructor_explicit(std::nullptr_t) -> std::false_type;

        ///@brief Resolves address from `nullptr` by converting it to `pointer` type.
        [[nodiscard]] constexpr metadata::pointer resolve_address(std::nullptr_t null) { return null; }

        ///@brief Contextually converts to `bool` to test if the pointer is engaged.
        [[nodiscard]] constexpr explicit operator bool(this auto&& self) noexcept { return (self.get() != nullptr); }

        ///@brief Compares equality against `nullptr`.
        [[nodiscard]] friend constexpr bool operator==(const ConcretePtr<Pointee>& ptr, std::nullptr_t null) noexcept
        {
            return (ptr.get() == null);
        }

        ///@brief Returns a raw pointer to the stored address while disengaging the pointer.
        [[nodiscard]] constexpr metadata::pointer release(this auto&& self) noexcept { return std::exchange(self.address_, nullptr); }

        ///@brief Reset the pointer to `nullptr`.
        constexpr void reset(this auto& self, std::nullptr_t null = nullptr) noexcept { self = null; }

    protected:
        ///@brief Passes the pointer through unchecked because this policy accepts null pointers.
        [[nodiscard]] constexpr metadata::pointer validate_by_nullability(metadata::pointer source) { return source; }
    }; //class yes

    template<template<typename> typename ConcretePtr, typename Pointee>
    class no {
        using metadata = pointer_metadata<Pointee>;
    public:
        using policy_group = policy_group_tag;

        ///@brief Culled stub to provide a non-viable overload candidate for `using Policies::is_constructor_explicit...` expansion.
        auto is_constructor_explicit(policy_group) -> std::true_type requires false;

        ///@brief Culled stub to provide a non-viable overload candidate for `using Policies::resolve_address...` expansion.
        constexpr metadata::pointer resolve_address(policy_group) noexcept requires false;

        ///@brief Contextually converts to `bool` to "test" if the pointer is engaged. Always returns `true` to confirm invariant.
        [[nodiscard]] constexpr explicit operator bool() const noexcept { return true; }

        ///@brief Deleted default constructor to prevent sources of null initialization.
        no() =
            delete /*("Default constructor deleted by policy `nullability::no` to prevent null initialization. Use `std::optional<ptr_type<T>>` for default-constructible optional pointers.")*/
            ;

        ///@brief Deleted address resolution from no arguments to prevent indirect default construction.
        metadata::pointer resolve_address(this auto&&) =
            delete /*("No-argument address resolution deleted by policy `nullability::no` to prevent null initialization. Use `std::optional<ptr_type<T>>` for default-constructible optional pointers.")*/
            ;

        ///@brief Deleted constructor from `nullptr` to prevent sources of null initialization.
        no(std::nullptr_t) =
            delete /*("Constructor from `nullptr` deleted to prevent null initialization. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        ///@brief Deleted assignment from `nullptr` to prevent sources of invalid null rebinding.
        template<typename Self>
        Self& operator=(this Self&&, std::nullptr_t) =
            delete /*("Assignment from `nullptr` deleted by policy `nullability::no` to prevent null rebinding. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        ///@brief Deleted address resolution from `nullptr` to prevent sources of invalid null rebinding.
        metadata::pointer resolve_address(this auto&&, std::nullptr_t) =
            delete /*("Address resolution from `nullptr` deleted by policy `nullability::no` to prevent null rebinding. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

    protected:
        ///@brief Enforces the non-null invariant by only passing the address through when it is not null.
        [[nodiscard]] constexpr metadata::pointer validate_by_nullability(metadata::pointer source)
        {
            if (source == nullptr) [[unlikely]]
                throw std::invalid_argument("`nullability::no` pointers cannot be constructed or assigned from a null pointer.");
            return source;
        }
    }; //class no
} //namespace base::vocab::inline ptr::ptr_policies::nullability
