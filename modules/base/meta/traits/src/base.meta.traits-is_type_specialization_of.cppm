// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.meta.traits-is_type_specialization_of.cppm
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
 * @brief `is_type_specialization_of_v`: A type predicate trait that determines if a type is a specialization of a given primary template.
 *
 * @details `is_type_specialization_of_v<T, PrimaryTemplate>` evaluates to `true` if `T` is a specialization
 * of `PrimaryTemplate` such that `PrimaryTemplate<Args...>` is the same type as `T` for some type arguments.
 */

//Module partition interface unit
export module base.meta.traits:is_type_specialization_of;

import std;

namespace base::meta::traits::inline predicates {
    template<typename T, template<typename...> typename PrimaryTemplate>
    inline constexpr bool is_type_specialization_of_impl = false;

    template<template<typename...> typename TT, typename... Args>
    inline constexpr bool is_type_specialization_of_impl<TT<Args...>, TT> = true;

    /**
     * @brief Checks if a type is a specialization of a given template.
     * @tparam T The type to check.
     * @tparam PrimaryTemplate The template to match against.
     */
    export template<typename T, template<typename...> typename PrimaryTemplate>
    inline constexpr bool is_type_specialization_of_v = is_type_specialization_of_impl<std::remove_cvref_t<T>, PrimaryTemplate>;
} //namespace base::meta::traits::inline predicates
