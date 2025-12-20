/**
 * @file telnet.cppm
 * @version 0.2.0
 * @release_date September 19, 2025
 * @brief Primary module interface for the Telnet library.
 * @details Exports partitions for:
 *   - :types        = Telnet command enumeration.
 *   - :errors       = Telnet-specific error codes.
 *   - :options      = Option management and factory functions.
 *   - :protocol_fsm = Telnet protocol state machine.
 *   - :socket       = Asynchronous and synchronous socket operations filtering Telnet data from the raw socket byte stream.
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @see RFC 854 for Telnet protocol specification
 */
//Primary module interface unit
export module telnet;

//Export all partition interfaces
export import :types;        ///< @see telnet-types.cppm
export import :errors;       ///< @see telnet-errors.cppm 
export import :options;      ///< @see telnet-options.cppm 
export import :protocol_fsm; ///< @see telnet-protocol_fsm.cppm
export import :socket;       ///< @see telnet-socket.cppm 