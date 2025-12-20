/**
 * @file telnet-types.cppm
 * @version 0.5.7
 * @release_date October 30, 2025
 *
 * @brief Partition for Telnet protocol-related types.
 * @remark Defines `byte_t` type alias for the byte stream's underlying type.
 * @remark Defines `TelnetCommand` and `NegotiationDirection` enumerations.
 * @remark Defines custom formatters for `TelnetCommand` and `NegotiationDirection` for use with `std::format`.
 *
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @remark This module is fully inline.
 */
//Module partition interface unit
export module telnet:types;

import std;        // For std::format, std::string_view, std::format_context
import std.compat; // For std::uint8_t

export namespace telnet {
    /**
     * @typedef byte_t
     * @brief Type alias for bytes underlying the Telnet stream.
     * @see `:stream` and `:protocol_fsm` for `byte_t` stream processing.
     * @todo Future Development: Consider switching from `std::uint8_t` to `std::byte` when C++23 support is better.
     */
    using byte_t = std::uint8_t; 
    
    /**
     * @brief Enumeration of Telnet commands as defined in RFC 854.
     * @remark Used in stream operations and protocol state machine to represent Telnet commands.
     * @see RFC 854 for Telnet protocol specification of command byte values.
     * @see `:protocol_fsm` for reading `TelnetCommand` from the Telnet incoming byte stream by the protocol state machine and `:stream` for usage in transmitting `TelnetCommand` to a Telnet peer.
     */
    enum class TelnetCommand : byte_t {
        EOR  = 0xEF, ///< End of Record 
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
    
    /**
     * @brief Enumeration representing option negotiation directions (i.e., local/remote enablement).
     * @see RFC 854 for Telnet protocol specification and RFC 1143 for option negotiation guidelines.
     * @see `:protocol_fsm` for use in option negotiation.
     */
    enum class NegotiationDirection {
        LOCAL,  ///< Local side ("us", sends WILL/WONT, receives DO/DONT)
        REMOTE  ///< Remote side ("them", sends DO/DONT, receives WILL/WONT)
    }; //enum class NegotiationDirection
} //namespace telnet

export namespace std {
    /**
     * @brief Formatter specialization for `telnet::TelnetCommand` to support `std::format`.
     * @remark Formats `TelnetCommand` values for use in `ProtocolConfig::log_error` and other logging contexts.
     * @see `:protocol_fsm` for logging usage.
     */
    template<>
    struct formatter<telnet::TelnetCommand, char> {
        char presentation = 'd'; ///< Format specifier: 'd' for name (0xXX), 'n' for name only, 'x' for hex only.

        /**
         * @brief Parses the format specifier for `TelnetCommand`.
         * @param ctx The format parse context.
         * @return Iterator pointing to the end of the parsed format specifier.
         * @throws std::format_error if the specifier is invalid (not 'd', 'n', or 'x').
         * @remark Supports 'd' (default: name (0xXX)), 'n' (name only), and 'x' (hex only, 0xXX).
         */
        constexpr auto parse(format_parse_context& ctx) {
            auto it = ctx.begin();
            if (it != ctx.end() && (*it == 'n' || *it == 'x' || *it == 'd')) {
                presentation = *it;
                ++it;
            }
            if (it != ctx.end() && *it != '}') {
                throw std::format_error("Invalid format specifier for TelnetCommand");
            }
            return it;
        } //parse(format_parse_context&)

        /**
         * @brief Formats a `TelnetCommand` value.
         * @param cmd The `TelnetCommand` to format.
         * @param ctx The format context.
         * @return Output iterator after formatting.
         * @details Formats as:
         * - 'd': "name (0xXX)" (e.g., "WILL (0xFB)").
         * - 'n': "name" (e.g., "WILL").
         * - 'x': "0xXX" (e.g., "0xFB").
         * - Unknown commands format as "UNKNOWN" ('n') or "0xXX" ('x').
         */
        template<typename FormatContext>
        auto format(telnet::TelnetCommand cmd, FormatContext& ctx) const {
            string_view name;
            switch (cmd) {
                case telnet::TelnetCommand::EOR:  name = "EOR";  break;
                case telnet::TelnetCommand::SE:   name = "SE";   break;
                case telnet::TelnetCommand::NOP:  name = "NOP";  break;
                case telnet::TelnetCommand::DM:   name = "DM";   break;
                case telnet::TelnetCommand::BRK:  name = "BRK";  break;
                case telnet::TelnetCommand::IP:   name = "IP";   break;
                case telnet::TelnetCommand::AO:   name = "AO";   break;
                case telnet::TelnetCommand::AYT:  name = "AYT";  break;
                case telnet::TelnetCommand::EC:   name = "EC";   break;
                case telnet::TelnetCommand::EL:   name = "EL";   break;
                case telnet::TelnetCommand::GA:   name = "GA";   break;
                case telnet::TelnetCommand::SB:   name = "SB";   break;
                case telnet::TelnetCommand::WILL: name = "WILL"; break;
                case telnet::TelnetCommand::WONT: name = "WONT"; break;
                case telnet::TelnetCommand::DO:   name = "DO";   break;
                case telnet::TelnetCommand::DONT: name = "DONT"; break;
                case telnet::TelnetCommand::IAC:  name = "IAC";  break;
                default:                          name = "UNKNOWN"; break;
            }
            
            if (presentation == 'n') {
                return std::format_to(ctx.out(), "{}", name);
            } else if (presentation == 'x') {
                return std::format_to(ctx.out(), "0x{:02x}", std::to_underlying(cmd));
            } else { // 'd' (default: name (0xXX))
                return std::format_to(ctx.out(), "{} (0x{:02x})", name, std::to_underlying(cmd));
            }
        } //format(TelnetCommand, FormatContext&)
    }; //struct formatter<telnet::TelnetCommand>
} //namespace std

// Custom formatter for NegotiationDirection
export namespace std {
    /**
     * @brief Formatter specialization for `telnet::NegotiationDirection` to support `std::format`.
     * @remark Formats `NegotiationDirection` values for use in `ProtocolConfig::log_error` during option negotiation.
     * @see `:protocol_fsm` for logging usage.
     */
    template<>
    struct formatter<telnet::NegotiationDirection, char> {
        /**
         * @brief Parses the format specifier for `NegotiationDirection`.
         * @param ctx The format parse context.
         * @return Iterator pointing to the end of the parsed format specifier.
         * @throws std::format_error if the specifier is not '}' (only default {} is supported).
         */
        constexpr auto parse(format_parse_context& ctx) {
            auto it = ctx.begin();
            if (it != ctx.end() && *it != '}') {
                throw format_error("Invalid format specifier for NegotiationDirection");
            }
            return it;
        } //parse(format_parse_context&)

        /**
         * @brief Formats a `NegotiationDirection` value.
         * @param dir The `NegotiationDirection` to format.
         * @param ctx The format context.
         * @return Output iterator after formatting.
         * @details Formats as "local" for `LOCAL` or "remote" for `REMOTE`.
         */
        template<typename FormatContext>
        auto format(telnet::NegotiationDirection dir, FormatContext& ctx) const {
            string_view name = dir == telnet::NegotiationDirection::LOCAL ? "local" : "remote";
            return format_to(ctx.out(), "{}", name);
        } //format(NegotiationDirection, FormatContext&)
    }; //struct formatter<telnet::NegotiationDirection>
} //namespace std