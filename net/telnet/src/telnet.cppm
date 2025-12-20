/**
 * @file telnet.cppm
 * @version 0.5.0
 * @release_date October 17, 2025
 *
 * @brief Primary module interface for the Telnet library.
 * @details Exports partitions for:
 *   - `:types`        = Telnet command enumeration plus miscellaneous utility types/typedefs.
 *   - `:errors`       = Telnet-specific error codes.
 *   - `:options`      = Option management and factory functions.
 *   - `:protocol_fsm` = Telnet protocol state machine.
 *   - `:socket`       = Asynchronous and synchronous socket operations filtering Telnet data from the raw socket byte stream.
 * @remark Provides a modular, thread-safe, and performance-optimized interface for Telnet protocol operations, supporting compile-time configuration and runtime extensibility.
 * @note Designed for integration with asynchronous I/O via Boost.Asio.
 * @remark Compile-time configuration is provided through a template parameter to `socket` (in `:socket`) that is used to instantiate its `ProtocolFSM` (in `:protocol_fsm`).
 *
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @see RFC 854 for Telnet protocol specification, RFC 1123 for evolution thereof, and RFC 1143 for option negotiation states.
 * @todo Future Development: Clean up exports to ensure types never accessed by external users are not exported. 
 */
//Primary module interface unit
export module telnet;

//Export all partition interfaces
export import :types;        ///< @see telnet-types.cppm
export import :errors;       ///< @see telnet-errors.cppm 
export import :options;      ///< @see telnet-options.cppm 
export import :protocol_fsm; ///< @see telnet-protocol_fsm.cppm
export import :socket;       ///< @see telnet-socket.cppm