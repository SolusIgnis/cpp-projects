// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-forward_declarations.cppm
 * @version 0.3.0
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
 * @brief Forward declarations for the pointer types to break dependency cycles.
 */

//Module partition interface unit
export module base.vocab.ptr:forward_declarations;

import std; 

import base.meta.traits;

namespace base::vocab::inline ptr {
    template<typename T>
        requires (
            !std::is_reference_v<T> && !std::is_void_v<T>
            && !std::is_function_v<base::meta::traits::remove_all_indirections_t<T>>
        )
    class [[nodiscard]] dependency_ptr;
    
    template<typename T>
        requires (!std::is_reference_v<T> && !std::is_function_v<base::meta::traits::remove_all_indirections_t<T>>)
    class required_ptr;
} //namespace base::vocab::inline ptr
