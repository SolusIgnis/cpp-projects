// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-forward_declarations.cppm
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
        using reference =
            std::conditional_t<std::is_void_v<Pointee>, std::add_lvalue_reference_t<void_reference>, std::add_lvalue_reference_t<Pointee>>;

        /**
         * @typedef rvalue_reference
         * @brief The rvalue reference type (`Pointee&&`).
         *
         * @remark When `Pointee` is `void`, uses `void_reference&&` because `void` as a function parameter is ill-formed.
         * @note Used only for deletion of invalid overloads to prevent binding to temporaries.
         */
        using rvalue_reference =
            std::conditional_t<std::is_void_v<Pointee>, std::add_rvalue_reference_t<void_reference>, std::add_rvalue_reference_t<Pointee>>;

        /**
         * @typedef difference_type
         * @brief Pointer difference type (`std::ptrdiff_t`).
         *
         * @note Provided to model pointer interface even when arithmetic is disabled.
         */
        using difference_type = std::ptrdiff_t;
    }; //struct pointer_metadata

    template<typename T>
    concept VocabPtr = requires { typename T::derived_from_ptr_core; };
} //namespace base::vocab::inline ptr

export namespace base::vocab::inline ptr {
    template<typename ConcretePtr, typename Pointee, typename... Policies>
        requires (!std::is_reference_v<Pointee> && !std::is_function_v<base::meta::traits::remove_all_indirections_t<Pointee>>)
    class ptr_core : public pointer_metadata<Pointee>, public Policies... {
    public:
        struct derived_from_ptr_core;

    private:
        pointer address_; ///<@brief The stored address used by all concrete pointer types.

        using Policies::resolve_address...;
        using Policies::is_constructor_explicit...;

    public:
        using Policies::Policies...;
        
        ///@brief Constructs a pointer when its new address can be resolved by its policies.
        template<typename... Args>
        constexpr explicit(decltype(is_constructor_explicit(std::declval<Args>()...))::value) ptr_core(Args... args)
            noexcept(noexcept(resolve_address(std::forward<Args>(args)...)))
            requires requires { resolve_address(std::forward<Args>(args)...); }
            : address_{resolve_address(std::forward<Args>(args)...)}
        {}

        ///@brief Assigns to a pointer when its new address can be resolved by its policies.
        template<typename Self, typename... Args>
            requires (!std::is_const_v<Self>)
        constexpr Self& operator=(this Self& self, Args... args)
            noexcept(noexcept(resolve_address(std::forward<Args>(args)...)))
            requires requires { resolve_address(std::forward<Args>(args)...); }
        {
            address_ = resolve_address(std::forward<Args>(args)...);
            return self;
        }

        ///@brief Swaps addresses.
        friend constexpr void swap(ptr_core& lhs, ptr_core& rhs) noexcept
        {
            using std::swap;
            swap(lhs.address_, rhs.address_);
        }

        //================================================================================
        // Pointer Operations
        //================================================================================

        ///@brief Provides member access to the pointee object.
        [[nodiscard]] constexpr pointer operator->(this auto&& self) noexcept
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
        [[nodiscard]] constexpr pointer get(this auto&& self) noexcept { return self.address_; }

        ///@brief Implicitly converts to the nested `pointer` type.
        [[nodiscard]] constexpr explicit(false) operator pointer() const noexcept { return this->get(); }

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
