/**
 * @file net.telnet.cppm
 * @version 0.5.7
 * @release_date October 30, 2025
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
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
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
