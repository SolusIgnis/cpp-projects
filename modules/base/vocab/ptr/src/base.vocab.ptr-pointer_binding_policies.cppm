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
    }; //struct allowed

    template<template<typename> typename ConcretePtr, typename Pointee>
    struct forbidden {
    private:
        using pointer = typename ConcretePtr<Pointee>::pointer;

    public:
        using policy_group = policy_group_tag;

        ///@brief Deleted constructor from `pointer` to structurally guarantee non-null initialization.
        forbidden(pointer) =
            delete /*("Constructor from `pointer` deleted. Dereference first to guarantee non-null initialization. Use `std::optional<dependency_ptr<T>>` for optional dependencies.")*/
            ;

        ///@brief Deleted assignment from `pointer` to structurally guarantee non-null rebinding.
        ConcretePtr<Pointee>& operator=(pointer) =
            delete /*("Assignment from `pointer` deleted. Dereference first to guarantee non-null initialization. Use `std::optional<dependency_ptr<T>>` for optional dependencies.")*/
            ;
    }; //struct forbidden
} //namespace base::vocab::inline ptr::ptr_policies::pointer_binding
