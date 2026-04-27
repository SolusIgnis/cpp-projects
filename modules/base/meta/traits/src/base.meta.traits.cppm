// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @module base.meta.traits
 * @file base.meta.traits.cppm
 * @version 0.0.1
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
 * @brief Primary module interface for the pointer vocabulary module.
 * @details Exports partitions for:
 *   - `:remove_all_indirections` = Removes all layers of indirection (pointer, pointer-to-member, lvalue reference, rvalue reference, or array) from a type to yield the core cv-qualified type.
 */

//Primary module interface unit
export module base.meta.traits;

//Export all partition interfaces
export import :remove_all_indirections; ///< @see "base.meta.traits-remove_all_indirections.cppm"
