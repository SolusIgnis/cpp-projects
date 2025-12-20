/**
 * @file telnet-protocol_fsm-impl.cpp
 * @version 0.2.0
 * @release_date September 19, 2025
 * @brief Implementation of the Telnet protocol finite state machine.
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 * @see telnet-protocol_fsm.cppm for interface, RFC 854 for Telnet protocol, RFC 855 for option negotiation, telnet:types for TelnetCommand, telnet:options for option and option::id_num, telnet:errors for error codes, telnet:socket for FSM usage
 */
module; //Including Boost.Asio in the Global Module Fragment until importable header units are reliable.
#include <boost/asio.hpp>
namespace asio = boost::asio;

//Module partition implementation unit
module telnet:protocol_fsm;

import std;        // For std::nullopt, std::optional, std::tuple, std::make_tuple, std::make_optional, std::error_code
import std.compat; // For std::uint8_t

import :types;        ///< @see telnet-types.cppm for TelnetCommand
import :options;      ///< @see telnet-options.cppm for option and option::id_num
import :errors;       ///< @see telnet-errors.cppm for telnet::error codes

import :protocol_fsm; ///< @see telnet-protocol_fsm.cppm for protocol_fsm partition interface

namespace telnet {
    /**
     * @brief Changes the FSM state and resets temporary state if transitioning to Normal.
     * @param next_state The new state to transition to.
     * @see telnet-protocol_fsm.cppm for ProtocolState, RFC 854 for state transitions
     */
    template<typename ConfigT>
    void ProtocolFSM<ConfigT>::change_state(ProtocolState next_state) noexcept {
        if (next_state == ProtocolState::Normal) {
            current_command_ = std::nullopt;
            current_option_ = std::nullopt;
            subnegotiation_buffer_.clear();
        }
        current_state_ = next_state;
    }

    /**
     * @brief Processes a single byte, dispatching to the appropriate state handler.
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag, and optional negotiation response.
     * @see telnet-protocol_fsm.cppm for process_byte declaration, telnet:errors for error codes, RFC 854 for protocol processing
     */
    template<typename ConfigT>
    std::tuple<std::error_code, bool, std::optional<std::tuple<TelnetCommand, option::id_num>>>
    ProtocolFSM<ConfigT>::process_byte(std::uint8_t byte) noexcept {
        switch (current_state_) {
            case ProtocolState::Normal:
                return handle_state_normal(byte);
            case ProtocolState::IAC:
                return handle_state_iac(byte);
            case ProtocolState::OptionNegotiation:
                return handle_state_option_negotiation(byte);
            case ProtocolState::SubnegotiationOption:
                return handle_state_subnegotiation_option(byte);
            case ProtocolState::Subnegotiation:
                return handle_state_subnegotiation(byte);
            case ProtocolState::SubnegotiationIAC:
                return handle_state_subnegotiation_iac(byte);
            default:
                change_state(ProtocolState::Normal);
                ProtocolConfig::log_error(std::make_error_code(error::protocol_violation), byte);
                return {std::make_error_code(error::protocol_violation), false, std::nullopt};
        }
    }

    /**
     * @brief Handles bytes in the Normal state (data or IAC).
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag, and optional negotiation response.
     * @see telnet-protocol_fsm.cppm for ProtocolState, telnet:types for TelnetCommand, RFC 854 for Normal state behavior
     */
    template<typename ConfigT>
    std::tuple<std::error_code, bool, std::optional<std::tuple<TelnetCommand, option::id_num>>>
    ProtocolFSM<ConfigT>::handle_state_normal(std::uint8_t byte) noexcept {
        if (byte == static_cast<std::uint8_t>(TelnetCommand::IAC)) {
            change_state(ProtocolState::IAC);
            return {std::error_code(), false, std::nullopt}; //discard IAC byte
        }
        return {std::error_code(), true, std::nullopt}; //retain data byte
    }

    /**
     * @brief Handles bytes after IAC (commands like WILL, DO, SB, etc.).
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag, and optional negotiation response.
     * @see telnet-protocol_fsm.cppm for ProtocolState, telnet:types for TelnetCommand, telnet:errors for error codes, RFC 854 for IAC command processing
     */
    template<typename ConfigT>
    std::tuple<std::error_code, bool, std::optional<std::tuple<TelnetCommand, option::id_num>>>
    ProtocolFSM<ConfigT>::handle_state_iac(std::uint8_t byte) noexcept {
        if (byte == static_cast<std::uint8_t>(TelnetCommand::IAC)) {
            change_state(ProtocolState::Normal);
            return {std::error_code(), true, std::nullopt}; //retain 0xFF as data (escaped IAC)
        }
        current_command_ = static_cast<TelnetCommand>(byte);
        if (current_command_) {
            switch (*current_command_) {
                case TelnetCommand::WILL: [[fallthrough]];
                case TelnetCommand::WONT: [[fallthrough]];
                case TelnetCommand::DO:   [[fallthrough]];
                case TelnetCommand::DONT:
                    change_state(ProtocolState::OptionNegotiation);
                    break;
                case TelnetCommand::SB:
                    change_state(ProtocolState::SubnegotiationOption);
                    break;
                case TelnetCommand::SE:
                    ProtocolConfig::log_error(std::make_error_code(error::invalid_subnegotiation), byte);
                    change_state(ProtocolState::Normal);
                    break;
                default:
                    {
                        auto handler = ProtocolConfig::get_command_handler(*current_command_);
                        if (handler) {
                            (*handler)(*current_command_);
                        }
                    }
                    change_state(ProtocolState::Normal);
                    break;
            }
        } else {
            ProtocolConfig::log_error(std::make_error_code(error::invalid_command), byte);
        }
        return {std::error_code(), false, std::nullopt}; //discard command byte
    }

    /**
     * @brief Handles bytes in the OptionNegotiation state (option ID after WILL/WONT/DO/DONT).
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag, and optional negotiation response.
     * @see telnet-protocol_fsm.cppm for ProtocolState, telnet:options for option::id_num, telnet:errors for error codes, RFC 855 for option negotiation
     */
    template<typename ConfigT>
    std::tuple<std::error_code, bool, std::optional<std::tuple<TelnetCommand, option::id_num>>>
    ProtocolFSM<ConfigT>::handle_state_option_negotiation(std::uint8_t byte) noexcept {
        current_option_ = ProtocolConfig::get_option(static_cast<option::id_num>(byte));
        if (current_command_ && current_option_) {
            bool respond = false;
            bool request_to_enable = (*current_command_ == TelnetCommand::DO || *current_command_ == TelnetCommand::WILL);
            bool remote_direction = (*current_command_ == TelnetCommand::WILL || *current_command_ == TelnetCommand::WONT);
            bool enable = false;

            bool currently_enabled = (remote_direction ? is_remotely_enabled(*current_option_) : is_locally_enabled(*current_option_));
            respond = (request_to_enable != currently_enabled);
            if (respond) {
                enable = request_to_enable && (remote_direction ? current_option_->supports_local() : current_option_->supports_remote());
                if (remote_direction) {
                    remote_enabled_[*current_option_] = enable;
                } else {
                    local_enabled_[*current_option_] = enable;
                }
            }

            if (respond) {
                TelnetCommand cmd = remote_direction
                    ? (enable ? TelnetCommand::DO : TelnetCommand::DONT)
                    : (enable ? TelnetCommand::WILL : TelnetCommand::WONT);
                change_state(ProtocolState::Normal);
                return {std::error_code(), false, std::make_optional(std::make_tuple(cmd, *current_option_))};
            }
        } else {
            auto unknown_option_handler = ProtocolConfig::get_unknown_option_handler();
            if (unknown_option_handler && current_command_) {
                unknown_option_handler(static_cast<option::id_num>(byte));
                bool request_to_enable = (*current_command_ == TelnetCommand::WILL || *current_command_ == TelnetCommand::DO);
                bool remote_direction = (*current_command_ == TelnetCommand::WILL || *current_command_ == TelnetCommand::WONT);
                if (request_to_enable) {
                    TelnetCommand cmd = remote_direction ? TelnetCommand::DONT : TelnetCommand::WONT;
                    change_state(ProtocolState::Normal);
                    return {std::error_code(), false, std::make_optional(std::make_tuple(cmd, static_cast<option::id_num>(byte)))};
                }
            } else {
                ProtocolConfig::log_error(std::make_error_code(error::option_not_available), byte);
            }
        }
        change_state(ProtocolState::Normal);
        return {std::error_code(), false, std::nullopt}; //discard option byte
    }

    /**
     * @brief Handles bytes in the SubnegotiationOption state (option ID after IAC SB).
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag, and optional negotiation response.
     * @note Logs invalid_subnegotiation for unenabled or unregistered options but continues to Subnegotiation state to consume data.
     * @remark Buffer reservation occurs after option validation to ensure current_option_ is set, optimizing memory allocation based on option::max_subnegotiation_size().
     * @see telnet-protocol_fsm.cppm for ProtocolState, telnet:options for option::id_num, telnet:errors for error codes, RFC 855 for subnegotiation
     */
    template<typename ConfigT>
    std::tuple<std::error_code, bool, std::optional<std::tuple<TelnetCommand, option::id_num>>>
    ProtocolFSM<ConfigT>::handle_state_subnegotiation_option(std::uint8_t byte) noexcept {
        current_option_ = ProtocolConfig::get_option(static_cast<option::id_num>(byte));
        if (current_option_) {
            if (!is_locally_enabled(*current_option_) && !is_remotely_enabled(*current_option_) || !current_option_->supports_subnegotiation()) {
                ProtocolConfig::log_error(std::make_error_code(error::invalid_subnegotiation), byte);
            }
        } else {
            current_option_ = option{static_cast<option::id_num>(byte)};
            ProtocolConfig::register_option(*current_option_);
            ProtocolConfig::log_error(std::make_error_code(error::invalid_subnegotiation), byte);
        }
        subnegotiation_buffer_.reserve(current_option_->max_subnegotiation_size());
        change_state(ProtocolState::Subnegotiation);
        return {std::error_code(), false, std::nullopt}; //discard subnegotiation byte
    }

    /**
     * @brief Handles bytes in the Subnegotiation state (data after option ID).
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag, and optional negotiation response.
     * @note Returns error::protocol_violation if current_option_ is unset (indicating invalid state transition).
     * @note Checks for IAC before buffer overflow to handle cases where buffer is full but followed by IAC SE.
     * @see telnet-protocol_fsm.cppm for ProtocolState, telnet:options for option::id_num, telnet:errors for error codes, RFC 855 for subnegotiation
     */
    template<typename ConfigT>
    std::tuple<std::error_code, bool, std::optional<std::tuple<TelnetCommand, option::id_num>>>
    ProtocolFSM<ConfigT>::handle_state_subnegotiation(std::uint8_t byte) noexcept {
        if (!current_option_) {
            change_state(ProtocolState::Normal);
            ProtocolConfig::log_error(std::make_error_code(error::protocol_violation), byte);
            return {std::make_error_code(error::protocol_violation), false, std::nullopt};
        }
        if (byte == static_cast<std::uint8_t>(TelnetCommand::IAC)) {
            change_state(ProtocolState::SubnegotiationIAC);
        } else {
            size_t max_size = current_option_->max_subnegotiation_size();
            if (max_size > 0 && subnegotiation_buffer_.size() >= max_size) {
                change_state(ProtocolState::Normal);
                ProtocolConfig::log_error(std::make_error_code(error::subnegotiation_overflow), byte);
                return {std::make_error_code(error::subnegotiation_overflow), false, std::nullopt};
            }
            subnegotiation_buffer_.push_back(byte);
        }
        return {std::error_code(), false, std::nullopt}; //discard subnegotiation byte
    }

    /**
     * @brief Handles bytes in the SubnegotiationIAC state (IAC in subnegotiation data).
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag, and optional negotiation response.
     * @note Returns error::protocol_violation if current_option_ is unset (indicating invalid state transition).
     * @note For invalid commands (not SE or IAC), logs error::invalid_command and assumes peer forgot to escape 0xFF/IAC byte, appending both IAC and the byte to the buffer.
     * @see telnet-protocol_fsm.cppm for ProtocolState, telnet:options for option::id_num, telnet:errors for error codes, RFC 855 for subnegotiation
     * @todo Phase 5: Review subnegotiation_overflow handling for custom error handlers
     */
    template<typename ConfigT>
    std::tuple<std::error_code, bool, std::optional<std::tuple<TelnetCommand, option::id_num>>>
    ProtocolFSM<ConfigT>::handle_state_subnegotiation_iac(std::uint8_t byte) noexcept {
        if (!current_option_) {
            change_state(ProtocolState::Normal);
            ProtocolConfig::log_error(std::make_error_code(error::protocol_violation), byte);
            return {std::make_error_code(error::protocol_violation), false, std::nullopt};
        }
        if (byte == static_cast<std::uint8_t>(TelnetCommand::SE)) {
            if (current_option_->supports_subnegotiation()) {
                current_option_->handle_subnegotiation(subnegotiation_buffer_);
            } else {
                ProtocolConfig::log_error(std::make_error_code(error::invalid_subnegotiation), byte);
            }
            change_state(ProtocolState::Normal);
        } else {
            size_t max_size = current_option_->max_subnegotiation_size();
            if (max_size > 0 && subnegotiation_buffer_.size() >= max_size) {
                change_state(ProtocolState::Normal);
                ProtocolConfig::log_error(std::make_error_code(error::subnegotiation_overflow), byte);
                return {std::make_error_code(error::subnegotiation_overflow), false, std::nullopt};
            }
            subnegotiation_buffer_.push_back(static_cast<std::uint8_t>(TelnetCommand::IAC));
            if (byte != static_cast<std::uint8_t>(TelnetCommand::IAC)) {
                ProtocolConfig::log_error(std::make_error_code(error::invalid_command), byte);
                subnegotiation_buffer_.push_back(byte);
            }
            change_state(ProtocolState::Subnegotiation);
        }
        return {std::error_code(), false, std::nullopt}; //discard subnegotiation byte
    }
} //namespace telnet