// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.meta.traits-is_indirection.cppm
 * @version 0.0.1
 * @date April 27, 2026
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
 * @brief `is_indirection_v`: A type predicate trait that indirection as would be removed by `remove_indirection_t`.
 *
 * @details `is_indirection_v` detects indirection, including:
 *   - pointers (`T*`) with any cv-qualification.
 *   - pointers-to-member (`T C::*`) with any cv-qualification.
 *   - lvalue references (`T&`).
 *   - rvalue references (`T&&`).
 *   - array extents (`T[]` and `T[N]`).
 *
 * Conceptually, this predicate is equivalent to applying the disjunction
 * of `std::is_pointer_v`, `std::is_member_pointer_v`, `std::is_reference_v`,
 * and `std::is_array_v`.
 */

//Module partition interface unit
export module base.meta.traits:is_indirection;

import std;

import :remove_indirection;

namespace base::meta::traits::inline predicates {
    ///@internal Primary template: Identity transformation for non-indirect types.
    template<typename T>
    struct is_indirection {
        static constexpr bool value = !std::is_same_v<T, remove_indirection_t<T>>;
    };
} //namespace base::meta::traits::inline predicates

export namespace base::meta::traits::inline predicates {
    /**
     * @brief Predicate trait to determine if a type possesses at least one layer of indirection.
     * @details Defined by checking if the type changes when a single-layer peel is attempted.
     */
    template<typename T>
    constexpr bool is_indirection_v = is_indirection<T>::value;
} //namespace base::meta::traits::inline predicates
