/**
 * @file telnet-errors.cppm
 * @version 0.2.0
 * @release_date September 19, 2025
 * @brief Telnet-specific error codes and error category for protocol and socket operations.
 * @remark Defines telnet::error enum and telnet_error_category for use with std::error_code.
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
     * @see RFC 854 for protocol_violation, telnet:protocol_fsm for error usage, telnet:options for subnegotiation_overflow, telnet:socket for socket-related errors
     */
    enum class error {
        protocol_violation = 1,              ///< General RFC 854 violation or invalid state transition (@see RFC 854)
        internal_error,                      ///< Unexpected internal error or uncaught exception (@see telnet:protocol_fsm, telnet:socket)
        invalid_command,                     ///< No handler found for a command byte after IAC (@see telnet:protocol_fsm)
        invalid_negotiation,                 ///< Invalid command in negotiation (not WILL/WONT/DO/DONT) (@see RFC 854, telnet:socket)
        option_not_available,                ///< Option is unsupported, disabled, or not found (@see telnet:options, telnet:protocol_fsm)
        invalid_subnegotiation,              ///< Invalid or incomplete subnegotiation sequence (@see telnet:protocol_fsm)
        subnegotiation_overflow              ///< Subnegotiation buffer exceeds max_subneg_size_ (@see telnet:options)
    }; //enum class error

    /**
     * @brief Error category for telnet::error codes.
     * @remark Thread-safe singleton providing detailed error messages.
     * @see telnet:protocol_fsm for error usage, telnet:socket for socket operations
     */
    class telnet_error_category : public std::error_category {
    public:
        /**
         * @brief Gets the singleton instance of the error category.
         * @return Reference to the static telnet_error_category.
         * @see telnet:protocol_fsm for error usage
         */
        static const telnet_error_category& instance() {
            static const telnet_error_category category;
            return category;
        }

        /**
         * @brief Gets the name of the error category.
         * @return "telnet" as the category name.
         */
        const char* name() const noexcept override {
            return "telnet";
        }

        /**
         * @brief Gets the error message for a telnet::error code.
         * @param ev The error value (telnet::error cast to int).
         * @return Detailed error message string.
         * @see telnet::error for error codes
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
                default:
                    return "Unknown Telnet error";
            }
        } //message(int) const
    }; //class telnet_error_category

    /**
     * @brief Creates an std::error_code from a telnet::error.
     * @param e The telnet::error value.
     * @return std::error_code with telnet_error_category.
     * @see telnet_error_category for category details
     */
    inline std::error_code make_error_code(error e) {
        return std::error_code(static_cast<int>(e), telnet_error_category::instance());
    }
} //namespace telnet

namespace std {
    /**
     * @brief Specializes std::is_error_code_enum for telnet::error.
     * @see telnet::error for error codes
     */
    template <>
    struct is_error_code_enum<telnet::error> : std::true_type {};
} //namespace std