// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.meta.concepts-instantiable_with.cppm
 * @version 0.0.3
 * @date April 26, 2026
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
 * @brief `instantiable_with`: A `concept` that determines if a type template can be instantiated by substituting a given set of arguments.
 */

//Module partition interface unit
export module base.meta.concepts:instantiable_with;

import std;

export namespace base::meta::concepts {
    /**
     * @brief `instantiable_with`: Determines whether a type template can be formed with a given set of arguments.
     *
     * @tparam Template A class template taking type parameters.
     * @tparam Args The template arguments to test.
     *
     * @details This concept is satisfied if the expression `typename Template<Args...>`
     * is well-formed, i.e., if substituting `Args...` into `Template` produces a valid
     * type name.
     *
     * @note This checks only for substitution validity. It does not require the resulting type to be complete, constructible, or otherwise usable.
     * @note Restricted to type templates of the form `template<typename...> typename`. Does not support templates with non-type or template template parameters.
     * @remark Useful for constraining templates based on the availability of a specialization without instantiating or requiring full semantic validity.
     */
    template<template<typename...> typename Template, typename... Args>
    concept instantiable_with = requires { typename Template<Args...>; };
} //namespace base::meta::concepts
