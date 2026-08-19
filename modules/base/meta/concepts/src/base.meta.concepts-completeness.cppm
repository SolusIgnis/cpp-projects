// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.meta.concepts-completeness.cppm
 * @version 0.0.3
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
 * @brief `concept`s that determine if a type is complete (generally), is complete enough to dereference a pointer to it, or is complete enough to access its members.
 */

//Module partition interface unit
export module base.meta.concepts:completeness;

import std;

import base.meta.traits;

export namespace base::meta::concepts {
    /**
     * @brief `CompleteType`: Determines whether a type is complete.
     *
     * @tparam T The type to check for completeness.
     *
     * @details This concept is satisfied if the expression `sizeof(T)` is well-formed.
     */
    template<typename T>
    concept CompleteType = requires { sizeof(T); };

    /**
     * @brief `CompletePointee`: Determines whether a type is complete for the purpose of dereferencing a pointer to it.
     *
     * @tparam T The type to check.
     *
     * @details This concept is satisfied if the type is both an object and a complete type.
     */
    template<typename T>
    concept CompletePointee = std::is_object_v<T> && CompleteType<T>;

    /**
     * @brief `CompleteClassType`: Determines whether a type is a complete C++ Standard class type.
     *
     * @tparam T The type to check.
     *
     * @details This concept is satisfied if the type is both a "class type" (i.e., `struct`/`class` or `union`) and a complete pointee.
     */
    template<typename T>
    concept CompleteClassType = base::meta::traits::is_class_type_v<T> && CompletePointee<T>;
} //namespace base::meta::concepts
