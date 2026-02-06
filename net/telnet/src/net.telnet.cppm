// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2025-2026 Jeremy Murphy and any Contributors
/**
 * @file net.telnet.cppm
 * @version 0.5.7
 * @date October 30, 2025
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
 * @brief Primary module interface for the Telnet library.
 * @details Exports partitions for:
 *   - `:types`        = Telnet command enumeration plus miscellaneous utility types/typedefs.
 *   - `:errors`       = Telnet-specific error codes.
 *   - `:concepts`     = Concepts for Telnet stream constraints and protocol finite state machine configuration.
 *   - `:options`      = Option management and factory functions.
 *   - `:protocol_fsm` = Telnet protocol state machine.
 *   - `:stream`       = Asynchronous and synchronous stream operations filtering Telnet data from the raw socket byte stream.
 * @remark Provides a modular, thread-safe, and performance-optimized interface for Telnet protocol operations, supporting compile-time configuration and runtime extensibility.
 * @note Designed for integration with asynchronous I/O via Boost.Asio.
 * @remark Compile-time configuration is provided through a template parameter to `stream` (in `:stream`) that is used to instantiate its `ProtocolFSM` (in `:protocol_fsm`).
 *
 * @see RFC 854 for Telnet protocol specification, RFC 1123 for evolution thereof, and RFC 1143 for option negotiation states.
 * @todo Future Development: Clean up exports to ensure types never accessed by external users are not exported.
 */

//Primary module interface unit
export module net.telnet;

//Export all partition interfaces
export import :types;        ///< @see "net.telnet-types.cppm"
export import :errors;       ///< @see "net.telnet-errors.cppm"
export import :concepts;     ///< @see "net.telnet-concepts.cppm"
export import :options;      ///< @see "net.telnet-options.cppm"
export import :awaitables;   ///< @see "net.telnet-awaitables.cppm"
export import :protocol_fsm; ///< @see "net.telnet-protocol_fsm.cppm"
export import :stream;       ///< @see "net.telnet-stream.cppm"
