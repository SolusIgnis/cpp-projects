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

import :metadata;

namespace base::vocab::inline ptr::ptr_policies::pointer_binding {
    struct policy_group_tag;

    template<template<typename> typename ConcretePtr, typename Pointee>
    class allowed {
        using metadata = pointer_metadata<Pointee>;
    public:
        using policy_group = policy_group_tag;

        template<typename P>
            requires std::is_pointer_v<std::remove_cvref_t<P>> && std::convertible_to<std::decay_t<P>, typename metadata::pointer>
        static auto is_constructor_explicit(P&&) -> std::conditional_t<std::is_void_v<Pointee>, std::true_type, std::false_type>;

        ///@brief Resolves address from a raw `pointer`.
        template<typename P>
            requires std::is_pointer_v<std::remove_cvref_t<P>> && std::convertible_to<std::decay_t<P>, typename metadata::pointer>
        constexpr metadata::pointer resolve_address(this auto& self, P&& source)
        {
            return self.validate_by_nullability(source);
        }

        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                  && requires(Pointer<Element, Args...> ptr) {
                         { std::as_const(ptr).get() } -> std::convertible_to<typename metadata::pointer>;
                     }
        static auto is_constructor_explicit(const Pointer<Element, Args...>&) -> std::conditional_t<std::is_void_v<Pointee>, std::true_type, std::false_type>;

        ///@brief Resolves address from another pointer-like type.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                  && requires(Pointer<Element, Args...> ptr) {
                         { std::as_const(ptr).get() } -> std::convertible_to<typename metadata::pointer>;
                     }
        constexpr metadata::pointer resolve_address(this auto& self, const Pointer<Element, Args...>& source)
        {
            return self.validate_by_nullability(source.get());
        }

        //================================================================================
        // Deleted Constructors and Assignment Operators: No Array-to-Pointer Decay
        //================================================================================

        ///@brief Deleted constructor from C-array to prevent array-to-pointer decay.
        template<typename AnyCArray>
            requires std::is_array_v<AnyCArray>
        allowed(AnyCArray&) =
            delete /*("Constructor from C-array deleted to prevent array-to-pointer decay. To point to the first element, alias it explicitly.")*/
            ;

        ///@brief Deleted assignment from C-array to prevent array-to-pointer decay.
        template<typename Self, typename AnyCArray>
            requires std::is_array_v<AnyCArray>
        Self& operator=(this Self&&, AnyCArray&) =
            delete /*("Assignment from C-array deleted to prevent array-to-pointer decay. To point to the first element, alias it explicitly.")*/
            ;

        ///@brief Deleted address resolution from C-array to prevent array-to-pointer decay.
        template<typename Self, typename AnyCArray>
            requires std::is_array_v<AnyCArray>
        metadata::pointer resolve_address(this auto&&, AnyCArray&) =
            delete /*("Address resolution from C-array deleted to prevent array-to-pointer decay. To point to the first element, alias it explicitly.")*/
            ;

        //================================================================================
        // Deleted Constructors and Assignment Operators: No Aliasing Temporaries
        //================================================================================

        ///@brief Deleted constructor from pointer-like object rvalue to discourage dangling by rejecting direct binding to temporaries.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                      && requires(Pointer<Element, Args...> ptr) {
                             { std::as_const(ptr).get() } -> std::convertible_to<typename metadata::pointer>;
                         }
        allowed(std::add_rvalue_reference_t<Pointer<Element, Args...>>) =
            delete /*("Constructor from pointer-like object rvalue deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        ///@brief Deleted assignment from pointer-like object rvalue to discourage dangling by rejecting direct binding to temporaries.
        template<typename Self, template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                      && requires(Pointer<Element, Args...> ptr) {
                             { std::as_const(ptr).get() } -> std::convertible_to<typename metadata::pointer>;
                         }
        Self& operator=(this Self&&, std::add_rvalue_reference_t<Pointer<Element, Args...>>) =
            delete /*("Assignment from pointer-like object rvalue deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;
            
        ///@brief Deleted address resolution from pointer-like object rvalue to discourage dangling by rejecting direct binding to temporaries.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                      && requires(Pointer<Element, Args...> ptr) {
                             { std::as_const(ptr).get() } -> std::convertible_to<typename metadata::pointer>;
                         }
        metadata::pointer resolve_address(this auto&&, std::add_rvalue_reference_t<Pointer<Element, Args...>>) =
            delete /*("Address resolution from pointer-like object rvalue deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

    protected:
        ///@brief Culled stub to provide a non-viable overload candidate for `using Policies::validate_by_nullability...` expansion.
        void validate_by_nullability() requires false;
    }; //class allowed

    template<template<typename> typename ConcretePtr, typename Pointee>
    class forbidden {
        using metadata = pointer_metadata<Pointee>;
    public:
        using policy_group = policy_group_tag;

        ///@brief Culled stub to provide a non-viable overload candidate for `using Policies::is_constructor_explicit...` expansion.
        auto is_constructor_explicit(policy_group) -> std::true_type requires false;

        ///@brief Culled stub to provide a non-viable overload candidate for `using Policies::resolve_address...` expansion.
        constexpr metadata::pointer resolve_address(policy_group) noexcept requires false;

        ///@brief Deleted constructor from `pointer` to structurally guarantee non-null initialization.
        forbidden(metadata::pointer) =
            delete /*("Constructor from `pointer` deleted by policy `pointer_binding::forbidden`. Dereference first to guarantee non-null initialization. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        ///@brief Deleted assignment from `pointer` to structurally guarantee non-null rebinding.
        template<typename Self>
        Self& operator=(this Self&&, metadata::pointer) =
            delete /*("Assignment from `pointer` deleted by policy `pointer_binding::forbidden`. Dereference first to guarantee non-null initialization. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        ///@brief Deleted address resolution from `pointer` to structurally guarantee non-null binding.
        metadata::pointer resolve_address(this auto&&, metadata::pointer) =
            delete /*("Address resolution from `pointer` deleted by policy `pointer_binding::forbidden`. Dereference first to guarantee non-null initialization. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;
            
        ///@brief Deleted constructor from another pointer-like type to structurally guarantee non-null initialization.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                  && requires(Pointer<Element, Args...> ptr) {
                         { std::as_const(ptr).get() } -> std::convertible_to<typename metadata::pointer>;
                     }
        forbidden(const Pointer<Element, Args...>&) =
            delete /*("Constructor from pointer-like types deleted by policy `pointer_binding::forbidden`. Dereference first to guarantee non-null initialization. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        ///@brief Deleted assignment from another pointer-like type to structurally guarantee non-null rebinding.
        template<typename Self, template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                  && requires(Pointer<Element, Args...> ptr) {
                         { std::as_const(ptr).get() } -> std::convertible_to<typename metadata::pointer>;
                     }
        Self& operator=(this Self&&, const Pointer<Element, Args...>&) =
            delete /*("Assignment from pointer-like types deleted by policy `pointer_binding::forbidden`. Dereference first to guarantee non-null initialization. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

        ///@brief Deleted address resolution from another pointer-like type to structurally guarantee non-null binding.
        template<template<typename, typename...> typename Pointer, typename Element, typename... Args>
            requires (!base::meta::traits::is_type_specialization_of_v<Pointer<Element, Args...>, ConcretePtr>)
                  && requires(Pointer<Element, Args...> ptr) {
                         { std::as_const(ptr).get() } -> std::convertible_to<typename metadata::pointer>;
                     }
        constexpr metadata::pointer resolve_address(this auto&, const Pointer<Element, Args...>&) =
            delete /*("Address resolution from pointer-like types deleted by policy `pointer_binding::forbidden`. Dereference first to guarantee non-null initialization. Use `std::optional<ptr_type<T>>` for optional pointers.")*/
            ;

    protected:
        ///@brief Culled stub to provide a non-viable overload candidate for `using Policies::validate_by_nullability...` expansion.
        void validate_by_nullability() requires false;
    }; //class forbidden
} //namespace base::vocab::inline ptr::ptr_policies::pointer_binding
