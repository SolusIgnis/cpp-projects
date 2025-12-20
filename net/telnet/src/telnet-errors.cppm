/**
 * @file telnet-errors.cppm
 * @version 0.3.0
 * @release_date September 29, 2025
 *
 * @brief Telnet-specific error codes and error category for protocol and socket operations.
 * @remark Defines `telnet::error` enumeration and `telnet_error_category` for use with `std::error_code`.
 * 
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @remark This module is fully inline.
 */
//Module partition interface unit
export module telnet:errors;

import std; // For std::error_category, std::error_code, std::string, std::true_type

export namespace telnet {
    /**
     * @brief Telnet-specific error codes for protocol and socket operations.
     * @see RFC 854 for `protocol_violation`, `:protocol_fsm` for `telnet::error` usage, `:socket` for socket-related errors
     */
    enum class error {
        protocol_violation = 1,  ///< General RFC 854 violation or invalid state transition (@see RFC 854)
        internal_error,          ///< Unexpected internal error or uncaught exception (@see `:protocol_fsm`, `:socket`)
        invalid_command,         ///< No handler found for a command byte after `IAC` (@see `:protocol_fsm`)
        invalid_negotiation,     ///< Invalid command in negotiation (not `WILL`/`WONT`/`DO`/`DONT`) (@see RFC 854, `:socket`)
        option_not_available,    ///< Option is unsupported, disabled, or not found (@see `:options`, `:protocol_fsm`)
        invalid_subnegotiation,  ///< Invalid or incomplete subnegotiation sequence (@see `:protocol_fsm`)
        subnegotiation_overflow, ///< Subnegotiation buffer exceeds `max_subneg_size_` (@see `:options`)
        ignored_go_ahead,        ///< Go Ahead command received and ignored (@see `:protocol_fsm`)
        user_handler_forbidden,  ///< A user tried to register a handler for a command/option reserved to the implementation (@see `:protocol_fsm`) 
        user_handler_not_found   ///< A needed handler was not registered by the user (@see `:protocol_fsm`)
    }; //enum class error

    /**
     * @brief Error category for `telnet::error` codes.
     * @remark Thread-safe singleton providing detailed error messages.
     * @see `:protocol_fsm` for `telnet::error` usage, `:socket` for socket operations
     */
    class telnet_error_category : public std::error_category {
    public:
        /**
         * @brief Gets the singleton instance of the `telnet_error_category`.
         * @return Reference to the static `telnet_error_category`.
         * @see `:protocol_fsm` for `telnet::error` usage
         */
        static const telnet_error_category& instance() {
            static const telnet_error_category category;
            return category;
        }

        /**
         * @brief Gets the name of the error category.
         * @return `"telnet"` as the category name.
         */
        const char* name() const noexcept override {
            return "telnet";
        }

        /**
         * @brief Gets the error message for a `telnet::error` code.
         * @param ev The error value (`telnet::error` cast to `int`).
         * @return Detailed error message string.
         * @see `telnet::error` for error codes
         * @remark The `[[unlikely]]` (theoretically unreachable) default case guards against an error code message being undefined.
         */
        std::string message(int ev) const override {
            switch (static_cast<error>(ev)) {
                case error::protocol_violation:
                    return "Telnet protocol violation";
                case error::internal_error:
                    return "Telnet internal error";
                case error::invalid_command:
                    return "No handler found for Telnet command byte";
                case error::invalid_negotiation:
                    return "Invalid Telnet negotiation command";
                case error::option_not_available:
                    return "Telnet option not available";
                case error::invalid_subnegotiation:
                    return "Invalid Telnet subnegotiation sequence";
                case error::subnegotiation_overflow:
                    return "Telnet subnegotiation buffer overflow";
                case error::ignored_go_ahead:
                    return "Telnet Go Ahead command received from peer and ignored";
                case error::user_handler_forbidden:
                    return "User attempted to register a handler for a command or option reserved to the Telnet implementation";
                case error::user_handler_not_found:
                    return "Telnet needed a handler that was not registered by the user";
                default:
                    [[unlikely]] //Impossible unless programmer error results in an error code without a defined message 
                    return "Unknown Telnet error";
            }
        } //message(int) const
    }; //class telnet_error_category

    /**
     * @brief Creates a `std::error_code` from a `telnet::error`.
     * @param e The `telnet::error` value.
     * @return `std::error_code` with `telnet_error_category`.
     * @see `telnet_error_category` for category details
     */
    inline std::error_code make_error_code(error e) {
        return std::error_code(static_cast<int>(e), telnet_error_category::instance());
    }
} //namespace telnet

namespace std {
    /**
     * @brief Specializes `std::is_error_code_enum` for `telnet::error`.
     * @see `telnet::error` for error codes
     */
    template <>
    struct is_error_code_enum<telnet::error> : std::true_type {};
} //namespace std