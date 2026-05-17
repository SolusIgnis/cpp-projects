// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-core.cppm
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
 * @brief Core pointer vocabulary infrastructure.
 */

//Module partition interface unit
export module base.vocab.ptr:core;

import std;

import base.meta.traits;
import base.meta.concepts;

export import :core_policies;

import :metadata;

namespace base::vocab::inline ptr {
    template<typename T>
    concept VocabPtr = requires { typename T::derived_from_ptr_core; };
} //namespace base::vocab::inline ptr

export namespace base::vocab::inline ptr {
    template<template<typename> typename ConcretePtr, typename Pointee, template<template<typename> typename, typename> typename... Policies>
        requires (!std::is_reference_v<Pointee> && !std::is_function_v<base::meta::traits::remove_all_indirections_t<Pointee>>)
    class ptr_core : public pointer_metadata<Pointee>, public Policies<ConcretePtr, Pointee>... {
    public:
        struct derived_from_ptr_core;
        friend Policies<ConcretePtr, Pointee>...;

    private:
        using metadata = pointer_metadata<Pointee>;

        metadata::pointer address_; ///<@brief The stored address used by all concrete pointer types.

    protected:
        using Policies<ConcretePtr, Pointee>::resolve_address...;
        using Policies<ConcretePtr, Pointee>::is_constructor_explicit...;
        using Policies<ConcretePtr, Pointee>::validate_by_nullability...;

  public:
        //================================================================================
        // Construction, Assignment, and Swap
        //================================================================================

        ///@brief Using constructor deletions from the policies.
        using Policies<ConcretePtr, Pointee>::Policies...;

        ///@brief Using assignment operator deletions from the policies.
        using Policies<ConcretePtr, Pointee>::operator=...;

        ///@brief (Conversion) Implicitly converts from another `ptr_core` according to underlying pointer conversions.
        template<typename OtherPointee>
            requires (!std::same_as<OtherPointee, Pointee>) && std::convertible_to<std::add_pointer_t<OtherPointee>, typename metadata::pointer>
        constexpr explicit(false) ptr_core(const ConcretePtr<OtherPointee>& source) noexcept : Policies<ConcretePtr, Pointee>(true)..., address_(source.get())
        {}

        ///@brief Constructs a pointer when its new address can be resolved by its policies.
        template<typename... Args>
        constexpr explicit(decltype(is_constructor_explicit(std::declval<Args>()...))::value) ptr_core(Args&&... args)
            noexcept(noexcept(resolve_address(std::forward<Args>(args)...)))
            requires requires { resolve_address(std::forward<Args>(args)...); }
            : Policies<ConcretePtr, Pointee>(true)..., address_{resolve_address(std::forward<Args>(args)...)}
        {}

        ///@brief (Conversion) Assigns from another `ptr_core` according to nested `pointer` type conversions.
        template<typename Self, typename OtherPointee>
            requires (!std::is_const_v<Self>) && (!std::same_as<OtherPointee, Pointee>) && std::convertible_to<std::add_pointer_t<OtherPointee>, typename metadata::pointer>
        constexpr Self& operator=(this Self& self, const ConcretePtr<OtherPointee>& source) noexcept
        {
            self.address_ = source.get();
            return self;
        }

        ///@brief Assigns to a pointer when its new address can be resolved by its policies.
        //template<typename Self, typename... Args>
        //    requires (!std::is_const_v<Self>)
        constexpr ConcretePtr& operator=(/*this Self& self,*/ Args&&... args)
            noexcept(noexcept(resolve_address(std::forward<Args>(args)...)))
            requires requires { resolve_address(std::forward<Args>(args)...); }
        {
            address_ = resolve_address(std::forward<Args>(args)...);
            return *this;
        }

        ///@brief Swaps pointer addresses.
        friend constexpr void swap(ConcretePtr<Pointee>& lhs, ConcretePtr<Pointee>& rhs) noexcept
        {
            using std::swap;
            swap(lhs.address_, rhs.address_);
        }

        //================================================================================
        // Pointer Operations
        //================================================================================

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

        //================================================================================
        // Stream Output
        //================================================================================

        ///@brief Outputs a pointer address to a `std::basic_ostream`.
        template<typename CharT, typename Traits>
        friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& stream, const ptr_core& ptr)
        {
            // In order to support pointers to arbitrarily cv-qualified objects:
            // 1. `static_cast` to `const volatile void*` to preserve all qualifiers while converting the pointer to `void*`.
            // 2. `const_cast` to `const void*` to satisfy the inserter's interface which lacks `volatile void*` overloads.
            // This is safe because formatting is a read-only numerical operation on the address.
            return stream << const_cast<const void*>(static_cast<const volatile void*>(ptr.get()));
        }
    }; //class ptr_core
} //namespace base::vocab::inline ptr

/**
 * @brief Partial specialization of `std::hash` for `VocabPtr`s.
 *
 * @tparam T The concrete pointer type.
 *
 * @remark Hashes the underlying stored address rather than pointee object state or values.
 * @remark Consistent with `cursor_ptr` equality semantics.
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
