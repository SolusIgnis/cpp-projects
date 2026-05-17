// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-reference_binding_policies.cppm
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
 * @brief Policies governing pointer binding from lvalue references.
 */

//Module partition interface unit
export module base.vocab.ptr:reference_binding_policies;

import std;

import base.meta.traits;
import base.meta.concepts;

import :metadata;

namespace base::vocab::inline ptr::ptr_policies::reference_binding {
    struct policy_group_tag;

    template<template<typename> typename ConcretePtr, typename Pointee>
    class allowed {
        using metadata = pointer_metadata<Pointee>;
    public:
        using policy_group = policy_group_tag;

        static auto is_constructor_explicit(metadata::reference source) -> std::true_type requires (!std::is_void_v<Pointee>);

        ///@brief Resolves an address from a lvalue `reference` to another object.
        constexpr metadata::pointer resolve_address(metadata::reference source) noexcept requires (!std::is_void_v<Pointee>) { return std::addressof(source); }

        //================================================================================
        // Deleted Constructors and Assignment Operators: No Aliasing Temporaries
        //================================================================================

        ///@brief Deleted constructor from `rvalue_reference` to discourage dangling by rejecting direct binding to temporaries.
        allowed(metadata::rvalue_reference) =
            delete /*("Constructor from `rvalue_reference` deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;

        ///@brief Deleted assignment from `rvalue_reference` to discourage dangling by rejecting direct binding to temporaries.
        template<typename Self>
        Self& operator=(this Self&&, metadata::rvalue_reference) =
            delete /*("Assignment from `rvalue_reference` deleted to discourage dangling by rejecting direct binding to temporaries.")*/
            ;
            
        ///@brief Deleted address resolution from `rvalue_reference` to discourage dangling by rejecting direct binding to temporaries.
        constexpr metadata::pointer resolve_address(metadata::rvalue_reference) =
            delete /*("Address resolution from `rvalue_reference` deleted to discourage dangling by rejecting direct binding to temporaries.")*/
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
        static auto is_constructor_explicit(policy_group) -> std::true_type requires false;

        ///@brief Deleted constructor from `const reference` to forbid binding to lvalue or rvalue references.
        forbidden(const metadata::reference) =
            delete /*("Constructor from references deleted by policy `reference_binding::forbidden`. Try constructing from the address directly.")*/
            ;

        ///@brief Deleted assignment from `const references` to forbid binding to lvalue or rvalue references.
        template<typename Self>
        Self& operator=(this Self&&, const metadata::reference) =
            delete /*("Assignment from references deleted by policy `reference_binding::forbidden`. Try assigning from the address directly.")*/
            ;

        ///@brief Deleted address resolution from `const reference` to forbid binding to lvalue or rvalue references.
        constexpr metadata::pointer resolve_address(const metadata::reference) =
            delete /*("Address resolution from references deleted by policy `reference_binding::forbidden`. Try resolving from the address directly.")*/
            ;

    protected:
        ///@brief Culled stub to provide a non-viable overload candidate for `using Policies::validate_by_nullability...` expansion.
        void validate_by_nullability() requires false;
    }; //class forbidden
} //namespace base::vocab::inline ptr::ptr_policies::reference_binding
