// SPDX-License-Identifier: Apache-2.0
/**
 * @file net.telnet-errors.cppm
 * @version 0.5.7
 * @date October 30, 2025
 *
 * @copyright Copyright (c) 2025-2026 Jeremy Murphy and any Contributors
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
 * @brief Telnet-specific error codes, processing signals, and error categories for protocol and stream operations.
 * @remark Defines `::net::telnet::error` enumeration for errors and `::net::telnet::processing_signal` for non-error processing signals.
 * @remark Defines `telnet_error_category` and `telnet_processing_signal_category` for use with `std::error_code`.
 *
 * @remark This module is fully inline.
 * @todo Future Development: Define and integrate `std::error_condition` mappings for both `error` and `processing_signal`.
 */

//Module partition interface unit
export module net.telnet:errors;

import std; //NOLINT For std::error_category, std::error_code, std::string, std::true_type

export namespace net::telnet {
    /**
     * @brief Telnet-specific error codes for protocol and stream operations.
     * @see RFC 854 for `protocol_violation`, `:protocol_fsm` for `telnet::error` usage, `:stream` for stream-related errors
     */
    enum class error : std::uint8_t {
        protocol_violation = 1, ///< General RFC 854 violation or invalid state transition (@see RFC 854)
        internal_error,         ///< Unexpected internal error or uncaught exception (@see `:protocol_fsm`, `:stream`)
        invalid_command,        ///< Unrecognized command byte after `IAC` (@see `:protocol_fsm`)
        invalid_negotiation, ///< Invalid command in negotiation (not `WILL`/`WONT`/`DO`/`DONT`) (@see RFC 854, `:stream`)
        option_not_available,    ///< Option is unsupported, disabled, or not found (@see `:options`, `:protocol_fsm`)
        invalid_subnegotiation,  ///< Invalid or incomplete subnegotiation sequence (@see `:protocol_fsm`)
        subnegotiation_overflow, ///< Subnegotiation buffer exceeds maximum size allowed by `option` (@see `:options`)
        ignored_go_ahead,        ///< Go-Ahead command ignored due to `SUPPRESS_GO_AHEAD` (@see `:protocol_fsm`)
        user_handler_forbidden,  ///< Attempt to register handler for reserved option (@see `:protocol_fsm`)
        user_handler_not_found,  ///< No handler registered for requested option (@see `:protocol_fsm`)
        negotiation_queue_error ///< The negotiation queue bit was set in a forbidden `NegotiationState` (@see `:internal`)
    }; //enum class error

    /**
     * @brief Telnet-specific processing signals for non-error conditions that trigger early read completion.
     * @remark Signals act as "soft EOF" allowing reads to complete early and signal "short read" pause conditions.
     * @see RFC 854 for `end_of_line`, `go_ahead`, `erase_character`, `erase_line`, `abort_output`, `interrupt_process`, `telnet_break`, `data_mark`; RFC 885 for `end_of_record`
     */
    enum class processing_signal : std::uint8_t {
        end_of_line = 1, ///< Encountered End-of-Line (`\r\n`) in byte stream (@see RFC 854, `:protocol_fsm`)
        carriage_return, ///< Encountered Carriage-Return (`\r`) sequence in byte stream requiring special handling (@see RFC 854, `:protocol_fsm`)
        end_of_record,   ///< Encountered End-of-Record (`IAC EOR`) in byte stream (@see RFC 885, `:protocol_fsm`)
        go_ahead,        ///< Encountered Go-Ahead (`IAC GA`) in byte stream (@see RFC 854, `:protocol_fsm`)
        erase_character, ///< Encountered Erase Character (`IAC EC`) in byte stream (@see RFC 854, `:protocol_fsm`)
        erase_line,      ///< Encountered Erase Line (`IAC EL`) in byte stream (@see RFC 854, `:protocol_fsm`)
        abort_output,    ///< Encountered Abort Output (`IAC AO`) in byte stream (@see RFC 854, `:protocol_fsm`)
        interrupt_process, ///< Encountered Interrupt Process (`IAC IP`) in byte stream (@see RFC 854, `:protocol_fsm`)
        telnet_break,      ///< Encountered Break (`IAC BRK`) in byte stream (@see RFC 854, `:protocol_fsm`)
        data_mark          ///< Encountered Data Mark (`IAC DM`) in byte stream (@see RFC 854, `:protocol_fsm`)
    }; //enum class processing_signal

    /**
     * @brief Error category for `telnet::error` codes.
     * @remark Thread-safe singleton providing detailed error messages.
     * @see `:protocol_fsm` for `telnet::error` usage, `:stream` for stream operations
     */
    class telnet_error_category : public std::error_category {
    public:
        /**
         * @brief Gets the singleton instance of the `telnet_error_category`.
         * @return Reference to the static `telnet_error_category`.
         * @see `:protocol_fsm` for `telnet::error` usage
         */
        static const telnet_error_category& instance()
        {
            static const telnet_error_category category;
            return category;
        }

        /**
         * @brief Gets the name of the error category.
         * @return `"telnet"` as the category name.
         */
        [[nodiscard]] const char* name() const noexcept override { return "telnet"; }

        /**
         * @brief Gets the error message for a `telnet::error` code.
         * @param ev The error value (`telnet::error` cast to `int`).
         * @return Detailed error message string.
         * @see `telnet::error` for error codes
         * @remark The `[[unlikely]]` (theoretically unreachable) default case guards against an error code message being undefined.
         */
        [[nodiscard]] std::string message(int value) const override
        {
            switch (static_cast<error>(value)) {
                case error::protocol_violation:
                    return "Telnet protocol violation";
                case error::internal_error:
                    return "Unexpected internal Telnet error";
                case error::invalid_command:
                    return "Unrecognized Telnet command after IAC";
                case error::invalid_negotiation:
                    return "Invalid Telnet negotiation command";
                case error::option_not_available:
                    return "Telnet option not available";
                case error::invalid_subnegotiation:
                    return "Invalid or incomplete Telnet subnegotiation";
                case error::subnegotiation_overflow:
                    return "Telnet subnegotiation buffer overflow";
                case error::ignored_go_ahead:
                    return "Telnet Go-Ahead ignored due to SUPPRESS_GO_AHEAD";
                case error::user_handler_forbidden:
                    return "Attempt to register handler for reserved option";
                case error::user_handler_not_found:
                    return "No handler registered for requested option";
                case error::negotiation_queue_error:
                    return "Telnet negotiation queue bit can only be set when the NegotiationState is WANTYES or WANTNO.";
                default:
                    [[unlikely]] return "Unknown Telnet error"; // Impossible unless programmer error results in an error code without a defined message
            }
        } //message(int) const

        /**
         * @brief Maps `telnet::error` codes to standard `std::error_condition` values.
         * @param ev The error value (`telnet::error` cast to `int`).
         * @return Corresponding `std::error_condition` or empty condition for unknown errors.
         * @remark Uses `[[fallthrough]]` to group related protocol errors under `std::errc::protocol_error` and handler errors under `std::errc::operation_not_supported` or `operation_not_permitted`.
         * @see `telnet::error` for error codes
         */
        [[nodiscard]] std::error_condition default_error_condition(int value) const noexcept override
        {
            switch (static_cast<error>(value)) {
                case error::invalid_command:
                    [[fallthrough]];
                case error::invalid_subnegotiation:
                    [[fallthrough]];
                case error::invalid_negotiation:
                    [[fallthrough]];
                case error::protocol_violation:
                    return std::errc::protocol_error;
                case error::option_not_available:
                    return std::errc::not_supported;
                case error::subnegotiation_overflow:
                    return std::errc::message_size;
                case error::ignored_go_ahead:
                    [[fallthrough]];
                case error::user_handler_not_found:
                    return std::errc::operation_not_supported;
                case error::internal_error:
                    return std::errc::state_not_recoverable;
                case error::user_handler_forbidden:
                    [[fallthrough]];
                case error::negotiation_queue_error:
                    return std::errc::operation_not_permitted;
                default:
                    return {};
            }
        } //default_error_condition(int) const noexcept
    }; //class telnet_error_category

    /**
     * @brief Error category for `telnet::processing_signal` codes.
     * @remark Thread-safe singleton providing detailed signal messages.
     * @see `:protocol_fsm` for `telnet::processing_signal` usage, `:stream` for stream operations
     */
    class telnet_processing_signal_category : public std::error_category {
    public:
        /**
         * @brief Gets the singleton instance of the `telnet_processing_signal_category`.
         * @return Reference to the static `telnet_processing_signal_category`.
         * @see `:protocol_fsm` for `telnet::processing_signal` usage
         */
        static const telnet_processing_signal_category& instance()
        {
            static const telnet_processing_signal_category category;
            return category;
        }

        /**
         * @brief Gets the name of the error category.
         * @return `"telnet_processing_signal"` as the category name.
         */
        [[nodiscard]] const char* name() const noexcept override { return "telnet_processing_signal"; }

        /**
         * @brief Gets the message for a `telnet::processing_signal` code.
         * @param ev The signal value (`telnet::processing_signal` cast to `int`).
         * @return Detailed signal message string.
         * @see `telnet::processing_signal` for signal codes
         * @remark The `[[unlikely]]` (theoretically unreachable) default case guards against an undefined signal code message.
         */
        [[nodiscard]] std::string message(int value) const override
        {
            switch (static_cast<processing_signal>(value)) {
                case processing_signal::end_of_line:
                    return "Telnet encountered End-of-Line in the byte stream";
                case processing_signal::carriage_return:
                    return "Telnet encountered Carriage-Return sequence in the byte stream requiring special handling";
                case processing_signal::end_of_record:
                    return "Telnet encountered \"End-of-Record\" command in the byte stream";
                case processing_signal::go_ahead:
                    return "Telnet encountered \"Go-Ahead\" command in the byte stream";
                case processing_signal::erase_character:
                    return "Telnet encountered \"Erase Character\" command in the byte stream";
                case processing_signal::erase_line:
                    return "Telnet encountered \"Erase Line\" command in the byte stream";
                case processing_signal::abort_output:
                    return "Telnet encountered \"Abort Output\" command in the byte stream";
                case processing_signal::interrupt_process:
                    return "Telnet encountered \"Interrupt Process\" command in the byte stream";
                case processing_signal::telnet_break:
                    return "Telnet encountered \"Break\" command in the byte stream";
                case processing_signal::data_mark:
                    return "Telnet encountered \"Data Mark\" command in the byte stream";
                default:
                    [[unlikely]] return "Unknown Telnet processing signal"; // Impossible unless programmer error results in a signal code without a defined message
            }
        } //message(int) const
    }; //class telnet_processing_signal_category

    /**
     * @brief Creates a `std::error_code` from a `telnet::error`.
     * @param e The `telnet::error` value.
     * @return `std::error_code` with `telnet_error_category`.
     * @see `telnet_error_category` for category details
     */
    inline std::error_code make_error_code(error ec)
    {
        return {static_cast<int>(ec), telnet_error_category::instance()};
    }

    /**
     * @brief Creates a `std::error_code` from a `telnet::processing_signal`.
     * @param s The `telnet::processing_signal` value.
     * @return `std::error_code` with `telnet_processing_signal_category`.
     * @see `telnet_processing_signal_category` for category details
     */
    inline std::error_code make_error_code(processing_signal ec)
    {
        return {static_cast<int>(ec), telnet_processing_signal_category::instance()};
    }
} //namespace net::telnet

namespace std {
    /**
     * @brief Specializes `std::is_error_code_enum` for `::net::telnet::error`.
     * @see `::net::telnet::error` for error codes
     */
    template<>
    struct is_error_code_enum<::net::telnet::error> : std::true_type {};

    /**
     * @brief Specializes `std::is_error_code_enum` for `::net::telnet::processing_signal`.
     * @see `::net::telnet::processing_signal` for signal codes
     */
    template<>
    struct is_error_code_enum<::net::telnet::processing_signal> : std::true_type {};
} //namespace std
