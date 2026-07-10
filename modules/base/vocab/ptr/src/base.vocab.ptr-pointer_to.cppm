// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file base.vocab.ptr-pointer_to.cppm
 * @version 0.9.0
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
 * @brief 
 *
 * @details 
 */

//Module partition interface unit
export module base.vocab.ptr:pointer_to;

import std;

namespace base::vocab::inline ptr {
    template<template<typename...> typename Pointer, typename Pointee>
    Pointer<Pointee> pointer_to(Pointee& object) //noexcept(noexcept(std::pointer_traits<Pointer<Pointee>>::pointer_to(object)))
    {
       return nullptr;//std::pointer_traits<Pointer<Pointee>>::pointer_to(object);
    }
} //namespace base::vocab::inline ptr
