// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-pointer_to.cppm
 * @version 0.9.1
 * @date July 10, 2026
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
 * @brief Generic object-to-pointer factory for pointer-like types.
 *
 * @details Provides the free function `base::vocab::ptr::pointer_to`, which
 * constructs a pointer to an existing object using the static member function
 * `pointer_to` provided by `std::pointer_traits`. This supplies a concise,
 * declarative spelling for pointer formation while integrating naturally with
 * any pointer type that models the standard pointer interface, including the
 * vocabulary pointer types provided by this library.
 */

//Module partition interface unit
export module base.vocab.ptr:pointer_to;

import std;

export namespace base::vocab::inline ptr {
    /**
     * @brief Produces a `Pointer<Pointee>` instance bound to a given object.
     *
     * @tparam Pointer A class template modeling a pointer type.
     * @tparam Pointee The pointee type, typically deduced from `object`.
     *
     * @param object The object to which to form a pointer.
     *
     * @return A `Pointer` to the provided `object`.
     *
     * @note `Pointee` is deduced from the cv-qualified type of `object`; callers need specify only the pointer template.
     * @remark Delegates to `std::pointer_traits<Pointer<Pointee>>::pointer_to`, allowing any pointer type with a conforming `std::pointer_traits` specialization to participate.
     */
    template<template<typename...> typename Pointer, typename Pointee>
        requires requires(Pointee& object) {
                     { std::pointer_traits<Pointer<Pointee>>::pointer_to(object) } -> std::convertible_to<Pointer<Pointee>>;
                 }
    constexpr Pointer<Pointee> pointer_to(Pointee& object) noexcept(noexcept(std::pointer_traits<Pointer<Pointee>>::pointer_to(object)))
    {
        return std::pointer_traits<Pointer<Pointee>>::pointer_to(object);
    }
} //namespace base::vocab::inline ptr
