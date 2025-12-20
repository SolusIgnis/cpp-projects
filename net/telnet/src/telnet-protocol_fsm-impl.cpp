/**
 * @file telnet-protocol_fsm-impl.cpp
 * @version 0.3.0
 * @release_date September 29, 2025
 *
 * @brief Implementation of the Telnet protocol finite state machine.
 *
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @see :protocol_fsm for interface, RFC 854 for Telnet protocol, RFC 855 and RFC 1143 for option negotiation, :types for `TelnetCommand`, :options for `option` and `option::id_num`, :errors for error codes, :socket for FSM usage
 */
module; //Including Boost.Asio in the Global Module Fragment until importable header units are reliable.
#include <boost/asio.hpp>
namespace asio = boost::asio;

//Module partition implementation unit
module telnet:protocol_fsm;

import std; // For std::nullopt, std::optional, std::tuple, std::make_tuple, std::make_optional, std::error_code

import :types;        ///< @see telnet-types.cppm for `TelnetCommand`
import :errors;       ///< @see telnet-errors.cppm for `telnet::error` codes
import :options;      ///< @see telnet-options.cppm for `option` and `option::id_num`

import :protocol_fsm; ///< @see telnet-protocol_fsm.cppm for `protocol_fsm` partition interface

namespace telnet {
    /**
     * @internal
     * Transitions `ProtocolFSM`'s state.
     * @note Resets `current_command_`, `current_option_`, and `subnegotiation_buffer_` when transitioning to `ProtocolState::Normal`.
     */
    template<typename ConfigT>
    void ProtocolFSM<ConfigT>::change_state(ProtocolState next_state) noexcept {
        if (next_state == ProtocolState::Normal) {
            current_command_ = std::nullopt;
            current_option_ = std::nullopt;
            subnegotiation_buffer_.clear();
        }
        current_state_ = next_state;
    } //change_state(ProtocolState)

    /**
     * @internal
     * Dispatches to state-specific handlers based on `current_state_`.
     * Handles invalid states (e.g., memory corruption) by logging `error::protocol_violation`, resetting to `ProtocolState::Normal`, and returning an error tuple.
     * Uses `[[unlikely]]` for the default case to optimize for valid states.
     */
    template<typename ConfigT>
    std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>>
    ProtocolFSM<ConfigT>::process_byte(byte_t byte) noexcept {
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
                [[unlikely]] //Impossible unless a new enumerator has been added or memory has been corrupted.
                ProtocolConfig::log_error(std::make_error_code(error::protocol_violation), byte, current_command_, current_option_);
                change_state(ProtocolState::Normal);
                return {std::make_error_code(error::protocol_violation), false, std::nullopt};
        }
    } //process_byte(byte_t)

    /**
     * @internal
     * Transitions to `ProtocolState::IAC` if `byte` is `IAC` (0xFF), discarding the byte (returns `false` for forward flag).
     * For non-IAC bytes, retains the byte as data (returns `true` for forward flag).
     */
    template<typename ConfigT>
    std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>>
    ProtocolFSM<ConfigT>::handle_state_normal(byte_t byte) noexcept {
        if (byte == std::to_underlying(TelnetCommand::IAC)) {
            change_state(ProtocolState::IAC);
            return {std::error_code(), false, std::nullopt}; //discard IAC byte
        }
        return {std::error_code(), true, std::nullopt}; //retain data byte
    } //handle_state_normal(byte_t)

    /**
     * @internal
     * Handles escaped IAC (0xFF) by transitioning to `ProtocolState::Normal` and retaining the byte as data.
     * For command bytes, sets `current_command_` and transitions to appropriate states (`OptionNegotiation` for `WILL`/`WONT`/`DO`/`DONT`, `SubnegotiationOption` for `SB`, `Normal` for others).
     * Logs errors for invalid commands (`error::invalid_command`), `SE` outside subnegotiation (`error::invalid_subnegotiation`), or `GA` (`error::ignored_go_ahead`).
     * Returns `AYT` response from `ProtocolConfig::get_ayt_response` or handler from `command_handler_registry_` for custom commands.
     * Discards command bytes (returns `false` for forward flag).
     * Uses `[[likely]]` for valid `current_command_` cases.
     */
    template<typename ConfigT>
    std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>>
    ProtocolFSM<ConfigT>::handle_state_iac(byte_t byte) noexcept {
        if (byte == std::to_underlying(TelnetCommand::IAC)) {
            change_state(ProtocolState::Normal);
            return {std::error_code(), true, std::nullopt}; //retain 0xFF as data (escaped IAC)
        }
        
        std::optional<ProcessingReturnVariant> result = std::nullopt;
        
        current_command_ = static_cast<TelnetCommand>(byte);
        if (current_command_) [[likely]] {
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
                    //SE outside subnegotiation is a protocol-level error. Log it, ignore it and move on.
                    ProtocolConfig::log_error(std::make_error_code(error::invalid_subnegotiation), byte, current_command_, current_option_);
                    change_state(ProtocolState::Normal);
                    break;
                case TelnetCommand::DM:
                    //DM represents a useless but harmless signal from legacy implementations. Ignore it without logging.
                    change_state(ProtocolState::Normal);
                    break;
                case TelnetCommand::GA:
                    //Log GA but ultimately ignore it as we do not support half-duplex mode.
                    ProtocolConfig::log_error(std::make_error_code(error::ignored_go_ahead), byte, std::nullopt, std::nullopt);
                    change_state(ProtocolState::Normal);
                    break;
                case TelnetCommand::AYT:
                    result = ProtocolConfig::get_ayt_response();
                    change_state(ProtocolState::Normal);
                    break;
                default:
                    {
                        auto handler = command_handler_registry_.get(*current_command_);
                        if (handler) {
                            result = std::make_tuple(*current_command_, *handler);
                        }
                    }
                    change_state(ProtocolState::Normal);
                    break;
            }
        } else [[unlikely]] { //Impossible unless memory has been corrupted.
            ProtocolConfig::log_error(std::make_error_code(error::invalid_command), byte, current_command_, current_option_);
        }
        return {std::error_code(), false, result}; //discard command byte
    } //handle_state_iac(byte_t)

    /**
     * @internal
     * Sets `current_option_` by querying `ProtocolConfig::registered_options` with the option ID.
     * For known options, evaluates enablement state using `option_status_` and updates `local_enabled_` or `remote_enabled_` based on `current_command_` (`WILL`/`WONT` for remote, `DO`/`DONT` for local).
     * Generates a response (`WILL`/`WONT`/`DO`/`DONT`) if the request changes the enablement state and is supported.
     * For unknown options, invokes `ProtocolConfig::get_unknown_option_handler` or logs `error::option_not_available`, and responds with `DONT`/`WONT` for enablement requests.
     * Transitions to `ProtocolState::Normal` and discards the option byte (returns `false` for forward flag).
     * Uses `[[likely]]` for valid `current_command_` cases.
     */
    template<typename ConfigT>
    std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>>
    ProtocolFSM<ConfigT>::handle_state_option_negotiation(byte_t byte) noexcept {
        std::optional<ProcessingReturnVariant> response = std::nullopt;
        
        if (current_command_) [[likely]] {
            current_option_ = ProtocolConfig::registered_options.get(static_cast<option::id_num>(byte));
        
            if (current_option_) {
                auto& current_status = option_status_[*current_option_];
                bool respond = false;
                bool request_to_enable = (*current_command_ == TelnetCommand::DO || *current_command_ == TelnetCommand::WILL);
                bool remote_direction = (*current_command_ == TelnetCommand::WILL || *current_command_ == TelnetCommand::WONT);
                bool enable = false;
    
                bool currently_enabled = (remote_direction ? current_status.remote_enabled() : current_status.local_enabled());
                bool request_pending = (remote_direction ? current_status.remote_pending() : current_status.local_pending());
                respond = (request_to_enable != currently_enabled);
                if (respond) {
                    enable = request_to_enable && (request_pending || (remote_direction ? current_option_->supports_remote() : current_option_->supports_local()));
                    if (remote_direction) {
                        current_status.set_enabled_remote(enable);
                    } else {
                        current_status.set_enabled_local(enable);
                    }

                    if (!request_pending) {
                        TelnetCommand cmd = remote_direction
                            ? (enable ? TelnetCommand::DO : TelnetCommand::DONT)
                            : (enable ? TelnetCommand::WILL : TelnetCommand::WONT);
                        response = std::make_tuple(cmd, static_cast<option::id_num>(byte));
                    }
                }
            } else {
                auto unknown_option_handler = ProtocolConfig::get_unknown_option_handler();
                if (unknown_option_handler) {
                    unknown_option_handler(static_cast<option::id_num>(byte));
                } else {
                    ProtocolConfig::log_error(std::make_error_code(error::option_not_available), byte, current_command_, current_option_);
                }
                bool request_to_enable = (*current_command_ == TelnetCommand::WILL || *current_command_ == TelnetCommand::DO);
                bool remote_direction = (*current_command_ == TelnetCommand::WILL || *current_command_ == TelnetCommand::WONT);
                if (request_to_enable) {
                    TelnetCommand cmd = remote_direction ? TelnetCommand::DONT : TelnetCommand::WONT;
                    response = std::make_tuple(cmd, static_cast<option::id_num>(byte));
                }
            }
        } else [[unlikely]] { //Impossible unless memory has been corrupted.
            ProtocolConfig::log_error(std::make_error_code(error::protocol_violation), byte, current_command_, current_option_);
        }
        change_state(ProtocolState::Normal);
        return {std::error_code(), false, response}; //discard option byte
    } //handle_state_option_negotiation(byte_t)

    /**
     * @internal
     * Sets `current_option_` by querying `ProtocolConfig::registered_options` with the option ID.
     * For unknown options, memoizes a default `option` via `registry.upsert` and logs `error::invalid_subnegotiation`.
     * Logs `error::invalid_subnegotiation` for options that don’t support subnegotiation or aren’t enabled in `option_status_`.
     * Reserves `subnegotiation_buffer_` based on `current_option_->max_subnegotiation_size()`.
     * Transitions to `ProtocolState::Subnegotiation` and discards the option byte (returns `false` for forward flag).
     */
    template<typename ConfigT>
    std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>>
    ProtocolFSM<ConfigT>::handle_state_subnegotiation_option(byte_t byte) noexcept {
        option_registry& registry = ProtocolConfig::registered_options;
        current_option_ = registry.get(static_cast<option::id_num>(byte));
        
        if (!current_option_) {
            // Memoize a defaulted option object (automatic rejection) to avoid lookup failures on repeated bad requests.
            current_option_ = registry.upsert(static_cast<option::id_num>(byte));
            ProtocolConfig::log_error(std::make_error_code(error::invalid_subnegotiation), byte, current_command_, current_option_);
        } else if (!current_option_->supports_subnegotiation() || !(option_status_[*current_option_].is_enabled())) {
            ProtocolConfig::log_error(std::make_error_code(error::invalid_subnegotiation), byte, current_command_, current_option_);
        }
        subnegotiation_buffer_.reserve(current_option_->max_subnegotiation_size());
        change_state(ProtocolState::Subnegotiation);
        return {std::error_code(), false, std::nullopt}; //discard subnegotiation byte
    } //handle_state_subnegotiation_option(byte_t)

    /**
     * @internal
     * Logs `error::protocol_violation` and transitions to `ProtocolState::Normal` if `current_option_` is unset.
     * Transitions to `ProtocolState::SubnegotiationIAC` if `byte` is `IAC`.
     * Checks `subnegotiation_buffer_` size against `current_option_->max_subnegotiation_size()` and logs `error::subnegotiation_overflow` if exceeded, transitioning to `ProtocolState::Normal`.
     * Appends non-IAC bytes to `subnegotiation_buffer_` and discards them (returns `false` for forward flag).
     */
    template<typename ConfigT>
    std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>>
    ProtocolFSM<ConfigT>::handle_state_subnegotiation(byte_t byte) noexcept {
        if (!current_option_) {
            ProtocolConfig::log_error(std::make_error_code(error::protocol_violation), byte, current_command_, current_option_);
            change_state(ProtocolState::Normal);
            return {std::make_error_code(error::protocol_violation), false, std::nullopt};
        }
        if (byte == std::to_underlying(TelnetCommand::IAC)) {
            change_state(ProtocolState::SubnegotiationIAC);
        } else {
            size_t max_size = current_option_->max_subnegotiation_size();
            if (max_size > 0 && subnegotiation_buffer_.size() >= max_size) {
                ProtocolConfig::log_error(std::make_error_code(error::subnegotiation_overflow), byte, current_command_, current_option_);
                change_state(ProtocolState::Normal);
                return {std::make_error_code(error::subnegotiation_overflow), false, std::nullopt};
            }
            subnegotiation_buffer_.push_back(byte);
        }
        return {std::error_code(), false, std::nullopt}; //discard subnegotiation byte
    } //handle_state_subnegotiation(byte_t)

    /**
     * @internal
     * Logs `error::protocol_violation` and transitions to `ProtocolState::Normal` if `current_option_` is unset.
     * For `SE`, completes subnegotiation by invoking `option_handler_registry_.handle_subnegotiation` if supported and enabled, then transitions to `ProtocolState::Normal`.
     * For non-`SE`/non-`IAC` bytes, logs `error::invalid_command`, assumes an unescaped IAC, and appends both `IAC` and the byte to `subnegotiation_buffer_`.
     * Checks `subnegotiation_buffer_` size against `max_subnegotiation_size()` and logs `error::subnegotiation_overflow` if exceeded.
     * Transitions to `ProtocolState::Subnegotiation` for non-`SE` bytes and discards all bytes (returns `false` for forward flag).
     */
    template<typename ConfigT>
    std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>>
    ProtocolFSM<ConfigT>::handle_state_subnegotiation_iac(byte_t byte) noexcept {
        std::optional<ProcessingReturnVariant> result = std::nullopt;
        if (!current_option_) {
            ProtocolConfig::log_error(std::make_error_code(error::protocol_violation), byte, current_command_, current_option_);
            change_state(ProtocolState::Normal);
            return {std::make_error_code(error::protocol_violation), false, std::nullopt};
        }
        if (byte == std::to_underlying(TelnetCommand::SE)) {
            //Subnegotiation sequence completed, so pass the buffer to the handler if supported. 
            //If subnegotiation is not supported or the option is not enabled, the error was logged at the beginning of subnegotiation, so just discard the buffer.
            if (current_option_->supports_subnegotiation() && option_status_[*current_option_].is_enabled()) {
                result = option_handler_registry_.handle_subnegotiation(*current_option_, std::move(subnegotiation_buffer_));
            }
            change_state(ProtocolState::Normal);
        } else {
            std::size_t max_size = current_option_->max_subnegotiation_size();
            if (max_size > 0 && subnegotiation_buffer_.size() >= max_size) {
                ProtocolConfig::log_error(std::make_error_code(error::subnegotiation_overflow), byte, current_command_, current_option_);
                change_state(ProtocolState::Normal);
                return {std::make_error_code(error::subnegotiation_overflow), false, std::nullopt};
            }
            //We either have an escaped IAC or an invalid command, so append IAC to the buffer.
            subnegotiation_buffer_.push_back(std::to_underlying(TelnetCommand::IAC));
            if (byte != std::to_underlying(TelnetCommand::IAC)) {
                ProtocolConfig::log_error(std::make_error_code(error::invalid_command), byte, current_command_, current_option_);
                //Invalid subnegotiation command (not SE or IAC), assume peer forgot to escape 0xFF/IAC byte, so append the stray byte to the buffer.
                subnegotiation_buffer_.push_back(byte);
            }
            change_state(ProtocolState::Subnegotiation);
        }
        return {std::error_code(), false, result}; //discard subnegotiation byte
    } //handle_state_subnegotiation_iac(byte_t)
} //namespace telnet