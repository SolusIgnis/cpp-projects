// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @module base.meta.sequences
 * @file base.meta.sequences.cppm
 * @version 0.0.1
 * @date May 18, 2026
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
 *   - `:core`      = Definition of the `type_list`, `value_list`, and `uniform_value_list` type templates.
 *   - `:access`    = Inspect elements and properties of a sequence without transforming it.
 *   - `:query`     = Produce sequence information as compile-time values, predicates, or scalar results.
 *   - `:modify`    = Directly modify sequence structure.
 *   - `:select`    = Select subsequences according to positions, predicates, or extraction criteria.
 *   - `:transform` = Produce new sequences through mapping, composition, expansion, or higher-order structural transformation.
 *   - `:reduce`    = Fold or accumulate sequence elements into a single synthesized meta-result through iterative reduction.
 *   - `:invoke`    = Metafunction invocation, binding, quoting, and deferred-evaluation utilities supporting higher-order metaprogramming.
 *
 * @todo Implement this diagram:
 * base.meta.sequences
 * ├── :core ✔️
 * ├── :access
 * ​│    ├── size_v ✔️
 * ​│    ├── empty_v ✔️
 * ​│    ├── EmptySequence ✔️
 * ​│    ├── NonEmptySequence ✔️
 * ​│    ├── front_t ✔️
 * ​│    ├── front_v ✔️
​​ * │    ├── find_type_if_t ✔️
​ * │    ├── find_value_if_v ✔️
 * │    ├── at_t (📌 needs C++26 pack indexing)
 * │    ├── at_v (📌 needs C++26 pack indexing)
​ * │    ├── back_t (⛓️ needs at_t)
 * ​│    └── back_v (⛓️ needs at_v)
 * ├── :query
 * ​│    ├── value_equivalent_v ✔️
 * ​│    ├── uniform_equivalent_v ✔️
​ * │    ├── contains_v
 * │    ├── contains_type_v ✔️
 * │    ├── contains_value_v ✔️
​ * │    ├── count_if_v
 * │    ├── count_type_if_v ✔️
 * │    ├── count_value_if_v ✔️
 * │    ├── exactly_one_type_if_v ✔️
 * │    ├── exactly_one_value_if_v ✔️
​ * │    ├── count_v
 * │    ├── count_type_v ✔️
 * │    ├── count_value_v ✔️
 * │    ├── exactly_one_of_v
 * │    ├── exactly_one_type_of_v ✔️
 * │    ├── exactly_one_value_of_v ✔️
​ * │    ├── equal_v (📌 needs C++26 pack indexing)
 * ​│    ├── is_unique_v
​ * │    ├── index_of_v
 * │    ├── index_of_type_v
 * │    ├── index_of_value_v
​​ * ​│    ├── any_of_type_v ✔️
​​ * ​│    ├── any_of_value_v ✔️
 * ​│    ├── all_of_type_v ✔️
 * ​│    ├── all_of_value_v ✔️
 * ​│    ├── none_of_type_v ✔️
​ * │    └── none_of_value_v ✔️
 * ├── :select
 * ​│    ├── filter_t
​ * │    ├── remove_t
​ * │    ├── try_find_type_if_t ✔️
 * ​│    ├── extract_t
 * ​│    ├── partition_t
 * ​│    ├── drop_t
 * ​│    └── take_t
 * ├── :modify
​ * │    ├── append_t
​ * │    ├── prepend_t
​ * │    ├── concat_t
 * ​│    ├── insert_t
​ * │    ├── erase_t
​ * │    ├── replace_t
 * ​│    ├── unique_t
​ * │    └── reverse_t
 * ├── :transform
​ * │    ├── transform_t
 * ​│    ├── zip_t
 * ​│    ├── flatten_t
 * ​│    └── cartesian_product_t
 * ├── :reduce // optional later
 * ​└── :invoke
​ *      ├── apply_t
 * ​     ├── bind_front_t
 * ​     ├── bind_back_t
 * ​     ├── quote_t
 * ​     └── defer_t
 */

//Primary module interface unit
export module base.meta.sequences;

//Export all partition interfaces
export import :core;   ///< @see "base.meta.sequences-core.cppm"
export import :access; ///< @see "base.meta.sequences-access.cppm"
export import :query;  ///< @see "base.meta.sequences-query.cppm"
//export import :select;    ///< @see "base.meta.sequences-select.cppm"
//export import :modify;    ///< @see "base.meta.sequences-modify.cppm"
//export import :transform; ///< @see "base.meta.sequences-transform.cppm"
//export import :reduce;    ///< @see "base.meta.sequences-reduce.cppm"
//export import :invoke;    ///< @see "base.meta.sequences-invoke.cppm"
