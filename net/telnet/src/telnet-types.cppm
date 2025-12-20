/**
 * @file telnet-types.cppm
 * @version 0.2.0
 * @release_date September 19, 2025
 * @brief Partition for Telnet protocol-related types.
 * @remark Defines TelnetCommand enumeration.
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
     * @brief Enumeration of Telnet commands as defined in RFC 854.
     * @remark Used in socket operations and protocol state machine to represent Telnet commands.
     * @see RFC 854 for Telnet protocol specification of command byte values.
     * @see telnet:protocol_fsm for reading commands from the Telnet incoming byte stream by the protocol state machine and telnet:socket for usage in transmitting commands to a Telnet peer.
     */
    enum class TelnetCommand : std::uint8_t {
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