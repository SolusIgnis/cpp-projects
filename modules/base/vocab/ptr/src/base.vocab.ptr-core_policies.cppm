// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-core_policies.cppm
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
export module base.vocab.ptr:core_policies;

import std;

import base.meta.traits;
import base.meta.concepts;

export import :nullability_policies;
export import :pointer_binding_policies;
export import :reference_binding_policies;
export import :traversal_policies;

namespace base::vocab::inline ptr {
    template<typename... Ts>
    struct type_list {};

    template<typename Group, typename... Policies>
    inline constexpr std::size_t group_count = (0u + ... + std::same_as<typename Policies::policy_group, Group>);

    template<typename Group, typename... Policies>
    inline constexpr bool exactly_one_policy_v = (group_count<Group, Policies...> == 1);

    template<typename GroupList, typename... Policies>
    struct valid_policy_pack_impl;

    template<typename... Groups, typename... Policies>
    struct valid_policy_pack_impl<
        type_list<Groups...>,
        Policies...
    > : std::bool_constant<
            (... && exactly_one_policy_v<Groups, Policies...>)
        >
    {};

    template<typename GroupList, typename... Policies>
    concept valid_policy_pack = valid_policy_pack_impl<GroupList, Policies...>::value;
       // (pointer_policy<Policies> && ...) &&
        
} //namespace base::vocab::inline ptr
