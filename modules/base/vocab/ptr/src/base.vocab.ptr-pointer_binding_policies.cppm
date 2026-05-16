// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-pointer_binding_policies.cppm
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
 * @brief Policies governing pointer binding from other pointers and pointer-like values.
 */

//Module partition interface unit
export module base.vocab.ptr:pointer_binding_policies;

import std;

import base.meta.traits;
import base.meta.concepts;

namespace base::vocab::inline ptr::ptr_policies::pointer_binding {
    struct policy_group_tag;

    template<template<typename> typename ConcretePtr, typename Pointee>
    struct allowed {
    private:
        using pointer = typename ConcretePtr<Pointee>::pointer;

    public:
        using policy_group = policy_group_tag;

        template<typename P>
            requires std::is_pointer_v<std::remove_cvref_t<P>> && std::convertible_to<std::decay_t<P>, pointer>
        auto is_constructor_explicit() -> std::conditional_t<std::is_void_v<Pointee>, std::true_type, std::false_type>;

        ///@brief Resolves address from a raw `pointer`.
        template<typename P>
            requires std::is_pointer_v<std::remove_cvref_t<P>> && std::convertible_to<std::decay_t<P>, pointer>
        constexpr pointer resolve_address(this auto& self, P&& source)
        {
            return self.validate_by_nullability(source);
        }

        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                  && requires(Pointer<Element, Args...> ptr) {
                         { std::as_const(ptr).get() } -> std::convertible_to<pointer>;
                     }
        auto is_constructor_explicit() -> std::conditional_t<std::is_void_v<Pointee>, std::true_type, std::false_type>;

        ///@brief Resolves address from another pointer-like type.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                  && requires(Pointer<Element, Args...> ptr) {
                         { std::as_const(ptr).get() } -> std::convertible_to<pointer>;
                     }
        constexpr pointer resolve_address(this auto& self, const Pointer<Element, Args...>& source)
        {
            return self.validate_by_nullability(source.get());
        }

        //================================================================================
        // Deleted Constructors and Assignment Operators: No Aliasing Temporaries
        //================================================================================

        ///@brief Deleted constructor from pointer-like object rvalue to discourage dangling by rejecting direct binding to temporaries.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, alias_ptr>)
                      && requires(Pointer<Element, Args...> ptr) {
                             { std::as_const(ptr).get() } -> std::convertible_to<pointer>;
                         }
        alias_ptr(const Pointer<Element, Args...>&&) =
            delete /*("Constructor from pointer-like object rvalue deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        ///@brief Deleted assignment from pointer-like object rvalue to discourage dangling by rejecting direct binding to temporaries.
        template<typename Self, template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, alias_ptr>)
                      && requires(Pointer<Element, Args...> ptr) {
                             { std::as_const(ptr).get() } -> std::convertible_to<pointer>;
                         }
        Self& operator=(this Self&&, const Pointer<Element, Args...>&&) =
            delete /*("Assignment from pointer-like object rvalue deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;
            
        ///@brief Deleted address resolution from pointer-like object rvalue to discourage dangling by rejecting direct binding to temporaries.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, alias_ptr>)
                      && requires(Pointer<Element, Args...> ptr) {
                             { std::as_const(ptr).get() } -> std::convertible_to<pointer>;
                         }
        pointer resolve_address(this auto&&, const Pointer<Element, Args...>&&) =
            delete /*("Address resolution from pointer-like object rvalue deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;
    }; //struct allowed

    template<template<typename> typename ConcretePtr, typename Pointee>
    struct forbidden {
    private:
        using pointer = typename ConcretePtr<Pointee>::pointer;

    public:
        using policy_group = policy_group_tag;

        ///@brief Deleted constructor from `pointer` to structurally guarantee non-null initialization.
        forbidden(pointer) =
            delete /*("Constructor from `pointer` deleted. Dereference first to guarantee non-null initialization. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        ///@brief Deleted assignment from `pointer` to structurally guarantee non-null rebinding.
        template<typename Self>
        Self& operator=(this Self&&, pointer) =
            delete /*("Assignment from `pointer` deleted. Dereference first to guarantee non-null initialization. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        ///@brief Deleted address resolution from `pointer` to structurally guarantee non-null rebinding.
        pointer resolve_address(this auto&&, pointer) =
            delete /*("Address resolution from `pointer` deleted. Dereference first to guarantee non-null initialization. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;
    }; //struct forbidden
} //namespace base::vocab::inline ptr::ptr_policies::pointer_binding
