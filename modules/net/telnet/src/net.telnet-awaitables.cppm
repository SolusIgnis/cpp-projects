// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2025-2026 Jeremy Murphy and any Contributors
/**
 * @file net.telnet-awaitables.cppm
 * @version 0.5.8
 * @date April 17, 2026
 *
 * @copyright © 2025-2026 Jeremy Murphy and any Contributors
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
 * @brief Partition defining tagged awaitable types for type-safe asynchronous operations in the Telnet library.
 * @remark Defines semantic tag structs and `framework::coroutines::tagged_awaitable` type aliases for option negotiation and subnegotiation handlers.
 *
 * @see RFC 854 for Telnet protocol, RFC 855 for option negotiation, `:protocol_fsm` for handler usage, `:internal` for handler definitions, and `framework.coroutines.tagged_awaitable` for `tagged_awaitable`
 */

module; //Including Asio in the Global Module Fragment until importable header units are reliable.
#include <asio/awaitable.hpp>

// Module partition interface unit
export module net.telnet:awaitables;

import std; //NOLINT

import framework.coroutines.tagged_awaitable;

export import :options; ///< @see "net.telnet-options.cppm" for `option`

export namespace net::telnet::awaitables {
    /// @brief Semantic tag `struct`s to specialize `framework::coroutines::tagged_awaitable`. @see `framework.coroutines.tagged_awaitable`
    namespace tags {
        /// @brief Tag to specialize `framework::coroutines::tagged_awaitable` for option enablement handlers. @see `framework.coroutines.tagged_awaitable`
        struct option_enablement_tag;

        /// @brief Tag to specialize `framework::coroutines::tagged_awaitable` for option disablement handlers. @see `framework.coroutines.tagged_awaitable`
        struct option_disablement_tag;

        /// @brief Tag to specialize `framework::coroutines::tagged_awaitable` for subnegotiation handlers. @see `framework.coroutines.tagged_awaitable`
        struct subnegotiation_tag;
    } //namespace tags

    /**
     * @typedef option_enablement_awaitable
     * @brief Awaitable type for option enablement handlers.
     * @see `framework.coroutines.tagged_awaitable`, `tags::option_enablement_tag`, `:internal` (`option_handler_registry`), `:protocol_fsm` (for use)
     */
    using option_enablement_awaitable = framework::coroutines::tagged_awaitable<tags::option_enablement_tag, asio::awaitable<void>>;

    /**
     * @typedef option_disablement_awaitable
     * @brief Awaitable type for option disablement handlers.
     * @see `framework.coroutines.tagged_awaitable`, `tags::option_disablement_tag`, `:internal` (`option_handler_registry`), `:protocol_fsm` (for use)
     */
    using option_disablement_awaitable = framework::coroutines::tagged_awaitable<tags::option_disablement_tag, asio::awaitable<void>>;

    /**
     * @typedef subnegotiation_awaitable
     * @brief Awaitable type for subnegotiation handlers.
     * @see `framework.coroutines.tagged_awaitable`, `tags::subnegotiation_tag`, `:internal` (`option_handler_registry`), `:protocol_fsm` (for use)
     */
    using subnegotiation_awaitable =
        framework::coroutines::tagged_awaitable<tags::subnegotiation_tag, asio::awaitable<std::tuple<option, std::vector<byte_t>>>>;
} //namespace net::telnet::awaitables
