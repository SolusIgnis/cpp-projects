// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @module base.vocab.ptr
 * @file base.vocab.ptr.cppm
 * @version 0.7.1
 * @date July 5, 2026
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
 *   - `:alias_ptr`      = General-purpose nullable object alias pointer.
 *   - `:cursor_ptr`     = Non-owning arithmetic never-null pointer suitable for contiguous memory traversal.
 *   - `:dependency_ptr` = Non-owning non-arithmetic never-null pointer for dependency injection.
 *   - `:iterator_ptr`   = Non-owning arithmetic nullable pointer compatible with standard iterator concepts.
 *   - `:required_ptr`   = General-purpose required-object alias pointer.
 */

//Primary module interface unit
export module base.vocab.ptr;

//Export all partition interfaces
export import :alias_ptr;      ///< @see "base.vocab.ptr-alias_ptr.cppm"
export import :cursor_ptr;     ///< @see "base.vocab.ptr-cursor_ptr.cppm"
export import :dependency_ptr; ///< @see "base.vocab.ptr-dependency_ptr.cppm"
export import :iterator_ptr;   ///< @see "base.vocab.ptr-iterator_ptr.cppm"
export import :required_ptr;   ///< @see "base.vocab.ptr-required_ptr.cppm"
