// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.meta.traits-type_categories.cppm
 * @version 0.0.4
 * @date August 18, 2026
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
 * @brief Traits for categorizing types.
 *
 * @details
 * - `is_class_type_v`: A type predicate to determine if a type is a "class type" according to the core language rules of the C++ Standard.
 */

//Module partition interface unit
export module base.meta.traits:type_categories;

import std;

export namespace base::meta::traits::inline predicates {
    /**
     * @brief `is_class_type_v`: Determines whether a type is a C++ Standard core language "class type".
     *
     * @tparam T The type to check.
     *
     * @details Evaluates to `true` if `T` is a "non-union class type" or "union type", and `false` otherwise.
     *
     * @note This corresponds to the C++ Standard's core language definition of a "class type", which includes both `struct`s/`class`es ("non-union class types") and `union`s ("union types").
     */
    template<typename T>
    inline constexpr bool is_class_type_v = std::is_class_v<T> || std::is_union_v<T>;
} //namespace base::meta::traits::inline predicates
