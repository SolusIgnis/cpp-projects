// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @module base.meta.concepts
 * @file base.meta.concepts.cppm
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
 * @brief Primary module interface for the metaprogramming `concept`s module.
 * @details Exports partitions for:
 *   - `:instantiable_with` = Determines if a template can be instantiated with a set of arguments.
 */

//Primary module interface unit
export module base.meta.concepts;

//Export all partition interfaces
export import :instantiable_with; ///< @see "base.meta.concepts-instantiable_with.cppm"
