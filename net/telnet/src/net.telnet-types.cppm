/**
 * @file net.telnet-types.cppm
 * @version 0.5.7
 * @release_date October 30, 2025
 *
 * @brief Partition for Telnet protocol-related types.
 * @remark Defines `byte_t` type alias for the byte stream's underlying type.
 * @remark Defines `telnet::command` and `negotiation_direction` enumerations.
 * @remark Defines custom formatters for `telnet::command` and `negotiation_direction` for use with `std::format`.
 *
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @remark This module is fully inline.
 */
 
//Module partition interface unit
export module net.telnet:types;

import std;        //For std::format, std::string_view, std::format_context
import std.compat; //For std::uint8_t

export namespace net::telnet {
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
     * @see `:protocol_fsm` for reading `telnet::command` from the Telnet incoming byte stream by the protocol state machine and `:stream` for usage in transmitting `telnet::command` to a Telnet peer.
     */
    enum class command : byte_t {
        eor      = 0xEF, ///< End of Record 
        se       = 0xF0, ///< Subnegotiation End
        nop      = 0xF1, ///< No Operation
        dm       = 0xF2, ///< Data Mark
        brk      = 0xF3, ///< Break
        ip       = 0xF4, ///< Interrupt Process
        ao       = 0xF5, ///< Abort Output
        ayt      = 0xF6, ///< Are You There
        ec       = 0xF7, ///< Erase Character
        el       = 0xF8, ///< Erase Line
        ga       = 0xF9, ///< Go Ahead
        sb       = 0xFA, ///< Subnegotiation Begin
        will_opt = 0xFB, ///< Sender wants to enable option
        wont_opt = 0xFC, ///< Sender wants to disable option
        do_opt   = 0xFD, ///< Sender requests receiver to enable option
        dont_opt = 0xFE, ///< Sender requests receiver to disable option
        iac      = 0xFF  ///< Interpret As Command
    }; //enum class command
    
    /**
     * @brief Enumeration representing option negotiation directions (i.e., local/remote enablement).
     * @see RFC 854 for Telnet protocol specification and RFC 1143 for option negotiation guidelines.
     * @see `:protocol_fsm` for use in option negotiation.
     */
    enum class negotiation_direction {
        local,  ///< Local side ("us", sends WILL/WONT, receives DO/DONT)
        remote  ///< Remote side ("them", sends DO/DONT, receives WILL/WONT)
    }; //enum class NegotiationDirection
} // export namespace net::telnet

export namespace std {
    /**
     * @brief Formatter specialization for `::net::telnet::command` to support `std::format`.
     * @remark Formats `telnet::command` values for use in `ProtocolConfig::log_error` and other logging contexts.
     * @see `:protocol_fsm` for logging usage.
     */
    template<>
    struct formatter<::net::telnet::command, char> {
        char presentation = 'd'; ///< Format specifier: 'd' for name (0xXX), 'n' for name only, 'x' for hex only.

        /**
         * @brief Parses the format specifier for `telnet::command`.
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
                throw std::format_error("Invalid format specifier for telnet::command");
            }
            return it;
        } //parse(format_parse_context&)

        /**
         * @brief Formats a `telnet::command` value.
         * @param cmd The `telnet::command` to format.
         * @param ctx The format context.
         * @return Output iterator after formatting.
         * @details Formats as:
         * - 'd': "name (0xXX)" (e.g., "WILL (0xFB)").
         * - 'n': "name" (e.g., "WILL").
         * - 'x': "0xXX" (e.g., "0xFB").
         * - Unknown commands format as "UNKNOWN" ('n') or "0xXX" ('x').
         */
        template<typename FormatContext>
        auto format(::net::telnet::command cmd, FormatContext& ctx) const {
            string_view name;
            switch (cmd) {
                case ::net::telnet::command::eor:      name = "EOR";  break;
                case ::net::telnet::command::se:       name = "SE";   break;
                case ::net::telnet::command::nop:      name = "NOP";  break;
                case ::net::telnet::command::dm:       name = "DM";   break;
                case ::net::telnet::command::brk:      name = "BRK";  break;
                case ::net::telnet::command::ip:       name = "IP";   break;
                case ::net::telnet::command::ao:       name = "AO";   break;
                case ::net::telnet::command::ayt:      name = "AYT";  break;
                case ::net::telnet::command::ec:       name = "EC";   break;
                case ::net::telnet::command::el:       name = "EL";   break;
                case ::net::telnet::command::ga:       name = "GA";   break;
                case ::net::telnet::command::sb:       name = "SB";   break;
                case ::net::telnet::command::will_opt: name = "WILL"; break;
                case ::net::telnet::command::wont_opt: name = "WONT"; break;
                case ::net::telnet::command::do_opt:   name = "DO";   break;
                case ::net::telnet::command::dont_opt: name = "DONT"; break;
                case ::net::telnet::command::iac:      name = "IAC";  break;
                default:                                     name = "UNKNOWN"; break;
            }
            
            if (presentation == 'n') {
                return std::format_to(ctx.out(), "{}", name);
            } else if (presentation == 'x') {
                return std::format_to(ctx.out(), "0x{:02x}", std::to_underlying(cmd));
            } else { // 'd' (default: name (0xXX))
                return std::format_to(ctx.out(), "{} (0x{:02x})", name, std::to_underlying(cmd));
            }
        } //format(::net::telnet::command, FormatContext&)
    }; //struct formatter<::net::telnet::command>

    /**
     * @brief Formatter specialization for `::net::telnet::negotiation_direction` to support `std::format`.
     * @remark Formats `negotiation_direction` values for use in `ProtocolConfig::log_error` during option negotiation.
     * @see `:protocol_fsm` for logging usage.
     */
    template<>
    struct formatter<::net::telnet::negotiation_direction, char> {
        /**
         * @brief Parses the format specifier for `negotiation_direction`.
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
         * @brief Formats a `negotiation_direction` value.
         * @param dir The `negotiation_direction` to format.
         * @param ctx The format context.
         * @return Output iterator after formatting.
         */
        template<typename FormatContext>
        auto format(::net::telnet::negotiation_direction dir, FormatContext& ctx) const {
            string_view name = dir == ::net::telnet::negotiation_direction::local ? "local" : "remote";
            return format_to(ctx.out(), "{}", name);
        } //format(::net::telnet::negotiation_direction, FormatContext&)
    }; //struct formatter<::net::telnet::negotiation_direction>
} //export namespace std
