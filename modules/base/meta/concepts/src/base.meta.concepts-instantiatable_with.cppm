// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.meta.concepts-instantiatable_with.cppm
 * @version 0.2.0
 * @date April 25, 2026
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
 * @brief `instantiatable_with`: A `concept` that determines if a template can be instantiated with a set of arguments.
 */

//Module partition interface unit
export module base.meta.concepts:instantiatable_with;

import std;

export namespace base::meta::concepts {
    template<template<typename...> typename Template, typename... Args>
    concept instantiatable_with = requires { typename Template<Args...>; };
} //namespace base::meta::concepts
