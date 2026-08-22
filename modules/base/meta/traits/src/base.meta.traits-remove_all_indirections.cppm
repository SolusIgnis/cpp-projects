// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.meta.traits-remove_all_indirections.cppm
 * @version 0.0.4
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
 * @brief `remove_all_indirections_t`: A type transformation trait that removes all layers of indirection yielding the "core" type.
 *
 * @details `remove_all_indirections_t` recursively peels off all layers of
 * indirection from a type, including:
 *   - pointers (`T*`) with any cv-qualification.
 *   - lvalue references (`T&`).
 *   - rvalue references (`T&&`).
 *   - array extents (`T[]` and `T[N]`).
 *   - pointers-to-member (`T C::*`) with any cv-qualification.
 *
 * This yields the "core" type: the type remaining after all indirection
 * layers have been removed, with any object-level cv-qualification preserved
 * intact.
 *
 * The resulting type contains none of the following:
 *   - pointer types
 *   - pointer-to-member types
 *   - reference types
 *   - array extents
 *
 * The `remove_indirection_t` transformation is applied repeatedly until its fixed
 * point is reached--i.e., a type for which applying `remove_indirection_t`
 * produces no further change.
 *
 * Conceptually, this transformation is equivalent to repeatedly applying
 * `std::remove_pointer`, `std::remove_reference`, and `std::remove_extent`
 * until the result reaches that same fixed point with the additional rule that
 * pointers-to-member (`T C::*`) are treated analogously to pointers and
 * removed in the same manner.
 *
 * Unlike the standard transformations, which operate on a single level of
 * indirection, this trait applies the transformation recursively until the
 * resulting type contains no remaining indirection.
 */

//Module partition interface unit
export module base.meta.traits:remove_all_indirections;

import std;

import :remove_indirection;

namespace base::meta::traits::inline transformation {
    ///@internal Recursive worker: Peels one layer and continues if the type changed.
    template<typename T, typename Result = remove_indirection_t<T>>
    struct remove_all_indirections {
        using type = typename remove_all_indirections<Result>::type;
    };

    ///@internal Base case: The type did not change after a peel attempt.
    template<typename T>
    struct remove_all_indirections<T, T> {
        using type = T;
    };
} //namespace base::meta::traits::inline transformation

export namespace base::meta::traits::inline transformation {
    /**
     * @brief Alias for `remove_all_indirections<T>::type`.
     *
     * @tparam T The type to transform.
     *
     * @details Produces the ultimate referent type of `T` after recursively
     * removing all indirection layers.
     *
     * @par Example
     * @code
     * using A = remove_all_indirections_t<int* const*>;     // int
     * using B = remove_all_indirections_t<const int&>;      // const int
     * using C = remove_all_indirections_t<int[3][4]>;       // int
     * using D = remove_all_indirections_t<int C::*>;        // int
     * @endcode
     */
    template<typename T>
    using remove_all_indirections_t = typename remove_all_indirections<T>::type;
} //namespace base::meta::traits::inline transformation
