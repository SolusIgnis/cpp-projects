/**
 * @file telnet-types.cppm
 * @version 0.3.0
 * @release_date September 29, 2025
 *
 * @brief Partition for Telnet protocol-related types.
 * @remark Defines `byte_t` type alias for the byte stream's underlying type.
 * @remark Defines `TelnetCommand` enumeration.
 *
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @remark This module is fully inline.
 */
//Module partition interface unit
export module telnet:types;

import std.compat; // For std::uint8_t

export namespace telnet {
    /**
     * @typedef byte_t
     * @brief Type alias for bytes underlying the Telnet stream.
     * @see `:socket` and `:protocol_fsm` for `byte_t` stream processing.
     * @todo Future Development: Consider switching from `std::uint8_t` to `std::byte` when C++23 support is better.
     */
    using byte_t = std::uint8_t; 
    
    /**
     * @brief Enumeration of Telnet commands as defined in RFC 854.
     * @remark Used in socket operations and protocol state machine to represent Telnet commands.
     * @see RFC 854 for Telnet protocol specification of command byte values.
     * @see `:protocol_fsm` for reading `TelnetCommand` from the Telnet incoming byte stream by the protocol state machine and `:socket` for usage in transmitting `TelnetCommand` to a Telnet peer.
     */
    enum class TelnetCommand : byte_t {
        SE   = 0xF0, ///< Subnegotiation End
        NOP  = 0xF1, ///< No Operation
        DM   = 0xF2, ///< Data Mark
        BRK  = 0xF3, ///< Break
        IP   = 0xF4, ///< Interrupt Process
        AO   = 0xF5, ///< Abort Output
        AYT  = 0xF6, ///< Are You There
        EC   = 0xF7, ///< Erase Character
        EL   = 0xF8, ///< Erase Line
        GA   = 0xF9, ///< Go Ahead
        SB   = 0xFA, ///< Subnegotiation Begin
        WILL = 0xFB, ///< Sender wants to enable option
        WONT = 0xFC, ///< Sender wants to disable option
        DO   = 0xFD, ///< Sender requests receiver to enable option
        DONT = 0xFE, ///< Sender requests receiver to disable option
        IAC  = 0xFF  ///< Interpret As Command
    }; //enum class TelnetCommand
} //namespace telnet