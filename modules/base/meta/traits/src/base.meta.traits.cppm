// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @module base.meta.traits
 * @file base.meta.traits.cppm
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
 * @brief Primary module interface for the metaprogramming traits module.
 * @details Exports partitions for:
 *   - `:copy_cvref`                  = Apply value categories and/or cv-qualifications from one type to another.
 *   - `:remove_indirection`          = Remove one layer of indirection  (pointer, pointer-to-member, lvalue reference, rvalue reference, or array) from a type.
 *   - `:remove_all_indirections`     = Remove all layers of indirection from a type to yield the "core" cv-qualified type.
 *   - `:is_indirection`              = Determine if a type has at least one layer of indirection.
 *   - `:is_type_specialization_of`   = Determine if a type is a specialization of a given primary template.
 *   - `:type_categories`             = Determine if a type fits into a given category.
 */

//Primary module interface unit
export module base.meta.traits;

//Export all partition interfaces
export import :copy_cvref;                ///< @see "base.meta.traits-copy_cvref.cppm"
export import :remove_indirection;        ///< @see "base.meta.traits-remove_indirection.cppm"
export import :remove_all_indirections;   ///< @see "base.meta.traits-remove_all_indirections.cppm"
export import :is_indirection;            ///< @see "base.meta.traits-is_indirection.cppm"
export import :is_type_specialization_of; ///< @see "base.meta.traits-is_type_specialization_of.cppm"
export import :type_categories;           ///< @see "base.meta.traits-type_categories.cppm"
