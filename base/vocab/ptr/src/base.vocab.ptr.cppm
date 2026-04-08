// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @module base.vocab.ptr
 * @file base.vocab.ptr.cppm
 * @version 0.1.0
 * @date March 11, 2026
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
 *   - `:alias_ptr`      = Non-owning nullable alias pointer.
 *   - `:dependency_ptr` = Non-owning never-null pointer for dependency injection.
 *   - `:required_ptr`   = Non-owning never-null alias pointer.
 */

//Primary module interface unit
export module base.vocab.ptr;

//Export all partition interfaces
export import :alias_ptr;      ///< @see "base.vocab.ptr-alias_ptr.cppm"
export import :dependency_ptr; ///< @see "base.vocab.ptr-dependency_ptr.cppm"
export import :required_ptr;   ///< @see "base.vocab.ptr-required_ptr.cppm"
