/**
 * @file net.telnet-protocol_fsm-impl.cpp
 * @version 0.5.7
 * @release_date October 30, 2025
 *
 * @brief Implementation of the Telnet protocol finite state machine.
 *
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @see "net.telnet-protocol_fsm.cppm" for interface, RFC 854 for Telnet protocol, RFC 855 and RFC 1143 for option negotiation, `:types` for `telnet::command` and `negotiation_direction`, `:options` for `option` and `option::id_num`, `:errors` for error codes, `:stream` for FSM usage
 */

//Module implementation unit
module net.telnet;

import std; //For std::nullopt, std::optional, std::tuple, std::make_tuple, std::make_optional, std::error_code, std::format, std::string_view

import :types;        ///< @see "net.telnet-types.cppm" for `telnet::command`, `negotiation_direction`
import :errors;       ///< @see "net.telnet-errors.cppm" for `telnet::error` and `telnet::processing_signal` codes
import :options;      ///< @see "net.telnet-options.cppm" for `option` and `option::id_num`
import :awaitables;   ///< @see "net.telnet-awaitables.cppm" for `OptionDisablementAwaitable`

import :protocol_fsm; ///< @see "net.telnet-protocol_fsm.cppm" for the partition being implemented.

namespace net::telnet {
    /**
     * @internal
     * Selects the `telnet::command` corresponding to a given `negotiation_direction` and enablement state 
     */
    template<typename PC>
    telnet::command ProtocolFSM<PC>::make_negotiation_command(negotiation_direction direction, bool enable) noexcept {
        using enum telnet::command;
        return (
                   (direction == negotiation_direction::remote)
                   ? (enable ? do_opt   : dont_opt)
                   : (enable ? will_opt : wont_opt)
               );
    } //make_negotiation_command(negotiation_direction, bool)
   
    /**
     * @internal
     * Validates option registration and updates `OptionStatusDB` based on the six states per RFC 1143 Q Method.
     * Handles redundant requests (`YES`, `WANTYES/EMPTY`, `WANTNO/OPPOSITE`) as idempotent successes, logging warnings.
     * Enqueues opposite requests in `WANTNO/EMPTY` to transition to `WANTNO/OPPOSITE`.
     * Transitions to `WANTYES`/`EMPTY` in `NO` state, returning a `NegotiationResponse` to initiate negotiation.
     * Does not invoke an enablement handler, as it initiates negotiation without enabling.
     * Logs errors for unregistered options (`error::option_not_available`), queue failures (`error::negotiation_queue_error`), or invalid states (`error::protocol_violation`).
     * @see RFC 1143 for Q Method, `:options` for `option::id_num`, `:errors` for error codes, `:types` for `negotiation_direction`, `:stream` for usage in `async_request_option`
     */
    template<typename PC>
    std::tuple<std::error_code, std::optional<typename ProtocolFSM<PC>::NegotiationResponse>> ProtocolFSM<PC>::request_option(
        option::id_num opt, negotiation_direction direction
    ) {
        if (!ProtocolConfig::registered_options.get(opt)) {
            ProtocolConfig::log_error(
                make_error_code(error::option_not_available),
                "Option {} not registered for {} negotiation",
                std::to_underlying(opt),
                direction
            );
            return {make_error_code(error::option_not_available), std::nullopt};
        }
        auto& status = option_status_[opt];
        //Six states: YES, WANTYES/EMPTY, WANTYES/OPPOSITE, WANTNO/EMPTY, WANTNO/OPPOSITE, NO
        if (status.enabled(direction)) { //YES
            ProtocolConfig::log_error(
                make_error_code(error::invalid_negotiation),
                "Redundant request for option {} in YES state, direction: {}",
                std::to_underlying(opt),
                direction
            );
            return {std::error_code{}, std::nullopt}; //Idempotent success
        } else if (status.pending_enable(direction) && !status.queued(direction)) { //WANTYES/EMPTY
            ProtocolConfig::log_error(
                make_error_code(error::invalid_negotiation),
                "Redundant request for option {} in WANTYES/EMPTY state, direction: {}",
                std::to_underlying(opt),
                direction
            );
            return {std::error_code{}, std::nullopt}; //Idempotent success
        } else if (status.pending_enable(direction) && status.queued(direction)) { //WANTYES/OPPOSITE
            status.dequeue(direction);
            return {std::error_code{}, std::nullopt};
        } else if (status.pending_disable(direction) && !status.queued(direction)) { //WANTNO/EMPTY
            if (auto ec = status.enqueue(direction); ec) {
                ProtocolConfig::log_error(
                    ec,
                    "Failed to enqueue request for option {} in WANTNO/EMPTY state, direction: {}",
                    std::to_underlying(opt),
                    direction
                );
                return {ec, std::nullopt};
            }
            return {std::error_code{}, std::nullopt};
        } else if (status.pending_disable(direction) && status.queued(direction)) { //WANTNO/OPPOSITE
            ProtocolConfig::log_error(
                make_error_code(error::invalid_negotiation),
                "Redundant request for option {} in WANTNO/OPPOSITE state, direction: {}",
                std::to_underlying(opt),
                direction
            );
            return {std::error_code{}, std::nullopt}; //Idempotent success
        } else if (status.disabled(direction)) { //NO
            status.pend_enable(direction);
            return {std::error_code{}, NegotiationResponse{direction, true, opt}};
        }
        ProtocolConfig::log_error(
            make_error_code(error::protocol_violation),
            "Invalid state for option {} in direction: {}",
            std::to_underlying(opt),
            direction
        );
        return {make_error_code(error::protocol_violation), std::nullopt};
    } //request_option(option::id_num, negotiation_direction)

    /**
     * @internal
     * Validates option registration and updates `OptionStatusDB` based on the six states per RFC 1143 Q Method.
     * Handles redundant disablements (`NO`, `WANTNO`/`EMPTY`, `WANTYES`/`OPPOSITE`) as idempotent successes, logging warnings.
     * Enqueues opposite requests in `WANTYES`/`EMPTY` to transition to `WANTYES`/`OPPOSITE`.
     * Transitions to `WANTNO`/`EMPTY` in `YES` state, returning a `NegotiationResponse` and `OptionDisablementAwaitable` via `option_handler_registry_.handle_disablement`.
     * Logs errors for unregistered options (`error::option_not_available`), queue failures (`error::negotiation_queue_error`), or invalid states (`error::protocol_violation`).
     * @see RFC 1143 for Q Method, `:options` for `option::id_num`, `:errors` for error codes, `:types` for `negotiation_direction`, `:awaitables` for `OptionDisablementAwaitable`, `:stream` for usage in `async_disable_option`
     */
    template<typename PC>
    std::tuple<std::error_code, std::optional<typename ProtocolFSM<PC>::NegotiationResponse>, std::optional<awaitables::OptionDisablementAwaitable>>
    ProtocolFSM<PC>::disable_option(option::id_num opt, negotiation_direction direction) {
        if (!ProtocolConfig::registered_options.get(opt)) {
            ProtocolConfig::log_error(
                make_error_code(error::option_not_available),
                "Option {} not registered for {} negotiation",
                std::to_underlying(opt),
                direction
            );
            return {make_error_code(error::option_not_available), std::nullopt, std::nullopt};
        }
        auto& status = option_status_[opt];
        //Six states: NO, WANTNO/EMPTY, WANTNO/OPPOSITE, WANTYES/EMPTY, WANTYES/OPPOSITE, YES
        if (status.disabled(direction)) { //NO
            ProtocolConfig::log_error(
                make_error_code(error::invalid_negotiation),
                "Redundant disable for option {} in NO state, direction: {}",
                std::to_underlying(opt),
                direction
            );
            return {std::error_code{}, std::nullopt, std::nullopt}; //Idempotent success
        } else if (status.pending_disable(direction) && !status.queued(direction)) { //WANTNO/EMPTY
            ProtocolConfig::log_error(
                make_error_code(error::invalid_negotiation),
                "Redundant disable for option {} in WANTNO/EMPTY state, direction: {}",
                std::to_underlying(opt),
                direction
            );
            return {std::error_code{}, std::nullopt, std::nullopt}; //Idempotent success
        } else if (status.pending_disable(direction) && status.queued(direction)) { //WANTNO/OPPOSITE
            status.dequeue(direction);
            return {std::error_code{}, std::nullopt, std::nullopt};
        } else if (status.pending_enable(direction) && !status.queued(direction)) { //WANTYES/EMPTY
            if (auto ec = status.enqueue(direction); ec) {
                ProtocolConfig::log_error(
                    ec,
                    "Failed to enqueue disable for option {} in WANTYES/EMPTY state, direction: {}",
                    std::to_underlying(opt),
                    direction
                );
                return {ec, std::nullopt, std::nullopt};
            }
            return {std::error_code{}, std::nullopt, std::nullopt};
        } else if (status.pending_enable(direction) && status.queued(direction)) { //WANTYES/OPPOSITE
            ProtocolConfig::log_error(
                make_error_code(error::invalid_negotiation),
                "Redundant disable for option {} in WANTYES/OPPOSITE state, direction: {}",
                std::to_underlying(opt),
                direction
            );
            return {std::error_code{}, std::nullopt, std::nullopt}; //Idempotent success
        } else if (status.enabled(direction)) { //YES
            status.pend_disable(direction);
            auto awaitable = option_handler_registry_.handle_disablement(opt, direction);
            return {std::error_code{}, NegotiationResponse{direction, false, opt}, std::move(awaitable)};
        }
        ProtocolConfig::log_error(
            make_error_code(error::protocol_violation),
            "Invalid state for option {} in direction: {}",
            std::to_underlying(opt),
            direction
        );
        return {make_error_code(error::protocol_violation), std::nullopt, std::nullopt};
    } //disable_option(option::id_num, negotiation_direction)
   
    /**
     * @internal
     * Transitions `ProtocolFSM`'s state.
     * @note Resets `current_command_`, `current_option_`, and `subnegotiation_buffer_` when transitioning to `ProtocolState::Normal`.
     */
    template<typename PC>
    void ProtocolFSM<PC>::change_state(ProtocolState next_state) noexcept {
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
    template<typename PC>
    std::tuple<std::error_code, bool, std::optional<typename ProtocolFSM<PC>::ProcessingReturnVariant>>
    ProtocolFSM<PC>::process_byte(byte_t byte) noexcept {
        switch (current_state_) {
            case ProtocolState::Normal:
                return handle_state_normal(byte);
            case ProtocolState::HasCR:
                return handle_state_has_cr(byte);
            case ProtocolState::HasIAC:
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
                ProtocolConfig::log_error(
                    make_error_code(error::protocol_violation),
                    "byte: 0x{:02x}, cmd: {}, opt: {}",
                    byte,
                    current_command_,
                    current_option_
                );
                change_state(ProtocolState::Normal);
                return {make_error_code(error::protocol_violation), false, std::nullopt};
        }
    } //process_byte(byte_t)

    /**
     * @internal
     * Transitions to `ProtocolState::HasIAC` if `byte` is `IAC` (0xFF), discarding the byte (returns `false` for forward flag).
     * Unless in `BINARY` mode, transitions to `ProtocolState::HasCR` if `byte` is `'\r'` (0x0D), forwarding the byte.
     * For non-`IAC` bytes, retains the byte as data (returns `true` for forward flag) unless it's nul (`'\0'`).
     */
    template<typename PC>
    std::tuple<std::error_code, bool, std::optional<typename ProtocolFSM<PC>::ProcessingReturnVariant>>
    ProtocolFSM<PC>::handle_state_normal(byte_t byte) noexcept {
        if (byte == std::to_underlying(telnet::command::iac)) {
            change_state(ProtocolState::HasIAC);
            return {std::error_code(), false, std::nullopt}; //discard IAC byte
        } else if ((byte == static_cast<byte_t>('\r')) && (!option_status_[option::id_num::binary].enabled(negotiation_direction::remote))) {
            change_state(ProtocolState::HasCR);
            return {std::error_code(), false, std::nullopt}; //discard CR byte
        } else if (byte == static_cast<byte_t>('\0')) {
            return {std::error_code(), false, std::nullopt}; //discard NUL byte
        }
        return {std::error_code(), true, std::nullopt}; //retain data byte
    } //handle_state_normal(byte_t)
    
    /**
     * @internal
     * For `'\n'`, forwards the byte (CR LF is EOL) and returns `processing_signal::end_of_line`.
     * For `'\0'`, discards the byte (CR NUL drops the NUL) and returns `processing_signal::carriage_return` to buffer the CR.
     * For `IAC`, logs a protocol violation and transitions to `ProtocolState::HasIAC`, discarding the byte and returns `processing_signal::carriage_return` to buffer the bare CR.
     * For all other bytes, retains the byte as data (returns `true` for forward flag) but logs a protocol violation, transitions to `ProtocolState::Normal`, and returns `processing_signal::carriage_return` to buffer the bare CR. 
     * In all cases other than `IAC`, transitions to `ProtocolState::Normal`.
     */
    template<typename PC>
    std::tuple<std::error_code, bool, std::optional<typename ProtocolFSM<PC>::ProcessingReturnVariant>>
    ProtocolFSM<PC>::handle_state_has_cr(byte_t byte) noexcept {
        ProtocolState next_state = ProtocolState::Normal;

        std::error_code result_ec;
        bool result_forward = false;
        
        if (byte == static_cast<byte_t>('\n')) {
            //valid Telnet End-of-Line sequence
            result_ec = make_error_code(processing_signal::end_of_line);
            result_forward = true; //retain LF byte
        } else if (byte == static_cast<byte_t>('\0')) {
            //valid Telnet Carriage-Return sequence
            result_ec = make_error_code(processing_signal::carriage_return);
            result_forward = false; //discard NUL byte
        } else if (byte == std::to_underlying(telnet::command::iac)) {
            ProtocolConfig::log_error(
                make_error_code(error::protocol_violation),
                "Invalid CR IAC sequence. Retained bare CR and transitioned to `ProtocolState::HasIAC`."
            );
            result_ec = make_error_code(processing_signal::carriage_return);
            result_forward = false; //discard IAC byte
            next_state = ProtocolState::HasIAC;
        } else { //any other sequence is invalid
            ProtocolConfig::log_error(
                make_error_code(error::protocol_violation),
                "Invalid CR 0x{:02x} sequence. Retained CR and data byte for data safety and transitioned back to `ProtocolState::Normal`.",
                byte
            );
            result_ec = make_error_code(processing_signal::carriage_return);
            result_forward = true; //retain data byte
        }
        
        change_state(next_state);
        return {result_ec, result_forward, std::nullopt};
    } //handle_state_has_cr(byte_t)

    /**
     * @internal
     * Handles escaped `IAC` (0xFF) by transitioning to `ProtocolState::Normal` and retaining the byte as data.
     * For command bytes, sets `current_command_` and transitions to appropriate states (`OptionNegotiation` for `WILL`/`WONT`/`DO`/`DONT`, `SubnegotiationOption` for `SB`, `Normal` for others).
     * Returns `std::error_code` with `telnet::processing_signal` for commands (`GA`, `EOR`, `EC`, `EL`, `AO`, `IP`, `BRK`, `DM`) to signal early completion or special handling.
     * Returns `ProcessingReturnVariant` for `AYT` with the response from `ProtocolConfig::get_ayt_response`.
     * Logs `error::invalid_subnegotiation` for `SE` outside subnegotiation, `error::ignored_go_ahead` for `GA` when `SUPPRESS_GO_AHEAD` is enabled, or `error::invalid_command` for unrecognized commands.
     * Logs `error::invalid_command` for commands outside of `telnet::command`.
     * Discards command bytes (returns `false` for forward flag).
     * Uses `[[likely]]` for valid `current_command_` cases.
     * @see RFC 854 for command definitions, `:errors` for `processing_signal` and error codes, `:stream` for `InputProcessor` handling
     */
    template<typename PC>
    std::tuple<std::error_code, bool, std::optional<typename ProtocolFSM<PC>::ProcessingReturnVariant>>
    ProtocolFSM<PC>::handle_state_iac(byte_t byte) noexcept {
        ProtocolState next_state = ProtocolState::Normal;
        
        std::error_code result_ec;
        bool result_forward = false;
        std::optional<ProcessingReturnVariant> result = std::nullopt;
        
        if (byte == std::to_underlying(telnet::command::iac)) {
            result_forward = true; //retain 0xFF as data (escaped IAC)
        } else {
            current_command_ = static_cast<telnet::command>(byte);
            if (current_command_) [[likely]] {
                using enum telnet::command;
                switch (*current_command_) {
                    case will_opt: [[fallthrough]];
                    case wont_opt: [[fallthrough]];
                    case do_opt:   [[fallthrough]];
                    case dont_opt:
                        next_state = ProtocolState::OptionNegotiation;
                        break;
                    case sb:
                        next_state = ProtocolState::SubnegotiationOption;
                        break;
                    case se:
                        //SE outside subnegotiation is a protocol-level error. Log it, ignore it and move on.
                        ProtocolConfig::log_error(
                            make_error_code(error::invalid_subnegotiation),
                            "byte: 0x{:02x}, cmd: {}, opt: {}",
                            byte,
                            telnet::command::se,
                            current_option_
                        );
                        break;
                    case dm:
                        result_ec = make_error_code(processing_signal::data_mark);
                        break;
                    case ga:
                        if (option_status_[option::id_num::suppress_go_ahead].enabled(negotiation_direction::remote)) {
                            //Log GA if SGA is active, but ultimately ignore it.
                            ProtocolConfig::log_error(
                                make_error_code(error::ignored_go_ahead),
                                "byte: 0x{:02x}, cmd: {}, opt: N/A",
                                byte,
                                telnet::command::ga
                            );
                        } else {
                            //Absent SGA, signal early completion on Go-Ahead.
                            result_ec = make_error_code(processing_signal::go_ahead);
                        }
                        break;
                    case ayt:
                        result = ProtocolConfig::get_ayt_response();
                        break;
                    case eor:
                        if (option_status_[option::id_num::end_of_record].enabled(negotiation_direction::remote)) {
                            result_ec = make_error_code(processing_signal::end_of_record); //signal early completion on End-of-Record
                        }
                        //If EOR is inactive, it's a no-op.
                        break;
                    case nop:
                        //No-Op
                        break;
                    case ec:
                        result_ec = make_error_code(processing_signal::erase_character);
                        break;
                    case el:
                        result_ec = make_error_code(processing_signal::erase_line);
                        break;
                    case ao:
                        result_ec = make_error_code(processing_signal::abort_output);
                        break;
                    case ip:
                        result_ec = make_error_code(processing_signal::interrupt_process);
                        break;
                    case brk:
                        result_ec = make_error_code(processing_signal::telnet_break);
                        break;
                    default:
                        ProtocolConfig::log_error(
                            make_error_code(error::invalid_command),
                            "byte: 0x{:02x}, cmd: {}, opt: {}",
                            byte,
                            *current_command_,
                            current_option_
                        );
                        break;
                } //switch(*current_command_)
            } else [[unlikely]] { //Impossible unless memory has been corrupted.
                ProtocolConfig::log_error(
                    make_error_code(error::invalid_command),
                    "byte: 0x{:02x}, cmd: N/A, opt: {}",
                    byte,
                    current_option_
                );
            }
        } //if (byte == ...)
        change_state(next_state);
        return {result_ec, result_forward, std::move(result)}; //discard command byte
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
    template<typename PC>
    std::tuple<std::error_code, bool, std::optional<typename ProtocolFSM<PC>::ProcessingReturnVariant>>
    ProtocolFSM<PC>::handle_state_option_negotiation(byte_t byte) noexcept {
        std::optional<ProcessingReturnVariant> response = std::nullopt;
        
        if (current_command_) [[likely]] {
            negotiation_direction direction = (*current_command_ == command::will_opt || *current_command_ == command::wont_opt) ? negotiation_direction::remote : negotiation_direction::local;
            
            current_option_ = ProtocolConfig::registered_options.get(static_cast<option::id_num>(byte));
        
            if (current_option_) {
                auto& current_status = option_status_[*current_option_];
                
                bool request_to_enable = (*current_command_ == command::do_opt || *current_command_ == command::will_opt);
                
                if ((request_to_enable && current_status.enabled(direction)) ||
                    (!request_to_enable && current_status.disabled(direction))) {
                    //Redundant WILL/DO in YES or WONT/DONT in NO: ignore
                    ProtocolConfig::log_error(
                        make_error_code(error::invalid_negotiation),
                        "byte: 0x{:02x}, cmd: {}, opt: {}, dir: {}",
                        byte,
                        *current_command_,
                        *current_option_,
                        direction
                    );
                } else if (request_to_enable) { //WILL/DO
                    if (current_status.pending_enable(direction)) {
                        //WANTYES
                        if (current_status.queued(direction)) {
                            //WANTYES with OPPOSITE queue bit.
                            current_status.dequeue(direction);
                            current_status.pend_disable(direction);
                            response = NegotiationResponse{direction, false, *current_option_};
                        } else {
                            //WILL/DO in WANTYES with EMPTY queue bit: complete negotiation.
                            current_status.enable(direction);
                            response = std::tuple{
                                option_handler_registry_.handle_enablement(*current_option_, direction),
                                std::nullopt
                            };
                        }
                    } else if (current_status.pending_disable(direction)) {
                        //WANTNO
                        if (current_status.queued(direction)) {
                            //WANTNO with OPPOSITE queue bit. Invalid Negotiation, but we're now in agreement, so accept gracefully.
                            current_status.dequeue(direction);
                            current_status.enable(direction);
                            response = std::tuple{
                                option_handler_registry_.handle_enablement(*current_option_, direction),
                                std::nullopt
                            };
                        } else {
                            //WANTNO with EMPTY queue bit. Invalid Negotiation.
                            ProtocolConfig::log_error(
                                make_error_code(error::invalid_negotiation),
                                "byte: 0x{:02x}, cmd: {}, opt: {}, dir: {}",
                                byte,
                                *current_command_,
                                *current_option_,
                                direction
                            );
                            current_status.disable(direction);
                        }
                    } else if (current_option_->supports(direction)) {
                        //WILL/DO in NO: accept if supported.
                        current_status.enable(direction);
                        response = std::tuple{
                            option_handler_registry_.handle_enablement(*current_option_, direction),
                            NegotiationResponse{direction, true, *current_option_}
                        };
                    } else {
                        //Unsupported option
                        response = NegotiationResponse{direction, false, *current_option_};
                    }
                } else { //WONT/DONT
                    if (current_status.pending_disable(direction)) {
                        //WANTNO
                        if (current_status.queued(direction)) {
                            //WANTNO with OPPOSITE queue bit.
                            current_status.dequeue(direction);
                            current_status.pend_enable(direction);
                            response = NegotiationResponse{direction, true, *current_option_};
                        } else {
                            //WONT/DONT in WANTNO with EMPTY queue bit: complete negotiation.
                            current_status.disable(direction);
                        }
                    } else if (current_status.pending_enable(direction)) {
                        //WANTYES
                        if (current_status.queued(direction)) {
                            //WANTYES with OPPOSITE queue bit: we now agree with rejection.
                            current_status.dequeue(direction);
                            current_status.disable(direction);
                        } else {
                            //WONT/DONT in WANTYES: disable.
                            current_status.disable(direction);
                        }
                    } else { //YES
                        //WONT/DONT in YES: disable.
                        current_status.disable(direction);
                        response = std::tuple{
                            option_handler_registry_.handle_disablement(*current_option_, direction),
                            NegotiationResponse{direction, false, *current_option_}
                        };
                    }
                }
            } else {
                //Peer is attempting to negotiate an unregistered option.
                auto unknown_option_handler = ProtocolConfig::get_unknown_option_handler();
                if (unknown_option_handler) {
                    unknown_option_handler(static_cast<option::id_num>(byte));
                } else {
                    ProtocolConfig::log_error(
                        make_error_code(error::option_not_available),
                        "byte: 0x{:02x}, cmd: {}, opt: N/A, dir: {}",
                        byte,
                        *current_command_,
                        direction
                    );
                }
                bool request_to_enable = (*current_command_ == command::do_opt || *current_command_ == command::will_opt);
                if (request_to_enable) { //Unregistered options are implicitly disabled, so requests to disable are ignored as redundant.
                    //Unregistered options MUST be refused per RFC 854 and RFC 1143
                    response = std::make_tuple(direction, false, static_cast<option::id_num>(byte));
                }
            }
        } else [[unlikely]] { //Impossible unless memory has been corrupted.
            ProtocolConfig::log_error(
                make_error_code(error::protocol_violation),
                "byte: 0x{:02x}, cmd: N/A, opt: {}",
                byte,
                current_option_
            );
        }
        change_state(ProtocolState::Normal);
        return {std::error_code(), false, std::move(response)}; //discard option byte
    } //handle_state_option_negotiation(byte_t)

    /**
     * @internal
     * Sets `current_option_` by querying `ProtocolConfig::registered_options` with the option ID.
     * For unknown options, memoizes a default `option` via `registry.upsert` and logs `error::invalid_subnegotiation`.
     * Logs `error::invalid_subnegotiation` for options that don’t support subnegotiation or aren’t enabled in `option_status_`.
     * Reserves `subnegotiation_buffer_` based on `current_option_->max_subnegotiation_size()`.
     * Transitions to `ProtocolState::Subnegotiation` and discards the option byte (returns `false` for forward flag).
     */
    template<typename PC>
    std::tuple<std::error_code, bool, std::optional<typename ProtocolFSM<PC>::ProcessingReturnVariant>>
    ProtocolFSM<PC>::handle_state_subnegotiation_option(byte_t byte) noexcept {
        option_registry& registry = ProtocolConfig::registered_options;
        current_option_ = registry.get(static_cast<option::id_num>(byte));
        
        if (!current_option_) {
            //Memoize a defaulted option object (automatic rejection) to avoid lookup failures on repeated bad requests.
            current_option_ = registry.upsert(static_cast<option::id_num>(byte));
            ProtocolConfig::log_error(
                make_error_code(error::invalid_subnegotiation),
                "byte: 0x{:02x}, cmd: {}, opt: {}",
                byte,
                telnet::command::sb,
                *current_option_
            );
        } else if (!current_option_->supports_subnegotiation() || !(option_status_[*current_option_].is_enabled())) {
            ProtocolConfig::log_error(
                make_error_code(error::invalid_subnegotiation),
                "byte: 0x{:02x}, cmd: {}, opt: {}",
                byte,
                telnet::command::sb,
                *current_option_
            );
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
     * Appends non-`IAC` bytes to `subnegotiation_buffer_` and discards them (returns `false` for forward flag).
     */
    template<typename PC>
    std::tuple<std::error_code, bool, std::optional<typename ProtocolFSM<PC>::ProcessingReturnVariant>>
    ProtocolFSM<PC>::handle_state_subnegotiation(byte_t byte) noexcept {
        if (!current_option_) {
            ProtocolConfig::log_error(
                make_error_code(error::protocol_violation),
                "byte: 0x{:02x}, cmd: {}, opt: N/A",
                byte,
                current_command_
            );
            change_state(ProtocolState::Normal);
            return {make_error_code(error::protocol_violation), false, std::nullopt};
        }
        if (byte == std::to_underlying(telnet::command::iac)) {
            change_state(ProtocolState::SubnegotiationIAC);
        } else {
            size_t max_size = current_option_->max_subnegotiation_size();
            if (max_size > 0 && subnegotiation_buffer_.size() >= max_size) {
                ProtocolConfig::log_error(
                    make_error_code(error::subnegotiation_overflow),
                    "byte: 0x{:02x}, cmd: {}, opt: {}",
                    byte,
                    current_command_,
                    *current_option_
                );
                change_state(ProtocolState::Normal);
                return {make_error_code(error::subnegotiation_overflow), false, std::nullopt};
            }
            subnegotiation_buffer_.push_back(byte);
        }
        return {std::error_code(), false, std::nullopt}; //discard subnegotiation byte
    } //handle_state_subnegotiation(byte_t)

    /**
     * @internal
     * Logs `error::protocol_violation` and transitions to `ProtocolState::Normal` if `current_option_` is unset.
     * For `SE`, completes subnegotiation by invoking `option_handler_registry_.handle_subnegotiation` if supported and enabled, then transitions to `ProtocolState::Normal`.
     * @note For `STATUS` subnegotiation, invokes dedicated `handle_status_subnegotiation` helper to yield the `SubnegotiationAwaitable` for internal processing.
     * For non-`SE`/non-`IAC` bytes, logs `error::invalid_command`, assumes an unescaped IAC, and appends both `IAC` and the byte to `subnegotiation_buffer_`.
     * Checks `subnegotiation_buffer_` size against `max_subnegotiation_size()` and logs `error::subnegotiation_overflow` if exceeded.
     * Transitions to `ProtocolState::Subnegotiation` for non-`SE` bytes and discards all bytes (returns `false` for forward flag).
     */
    template<typename PC>
    std::tuple<std::error_code, bool, std::optional<typename ProtocolFSM<PC>::ProcessingReturnVariant>>
    ProtocolFSM<PC>::handle_state_subnegotiation_iac(byte_t byte) noexcept {
        std::optional<ProcessingReturnVariant> response = std::nullopt;
        if (!current_option_) {
            ProtocolConfig::log_error(
                make_error_code(error::protocol_violation),
                "byte: 0x{:02x}, cmd: {}, opt: N/A",
                byte,
                current_command_
            );
            change_state(ProtocolState::Normal);
            return {make_error_code(error::protocol_violation), false, std::nullopt};
        }
        if (byte == std::to_underlying(telnet::command::se)) {
            //Subnegotiation sequence completed, so pass the buffer to the handler if supported. 
            //If subnegotiation is not supported or the option is not enabled, the error was logged at the beginning of subnegotiation, so just discard the buffer.
            if (current_option_->supports_subnegotiation() && option_status_[*current_option_].is_enabled()) {
                if (*current_option_ == option::id_num::status) {
                    response = handle_status_subnegotiation(*current_option_, std::move(subnegotiation_buffer_));
                } else {
                    response = option_handler_registry_.handle_subnegotiation(*current_option_, std::move(subnegotiation_buffer_));
                }
            }
            change_state(ProtocolState::Normal);
        } else {
            std::size_t max_size = current_option_->max_subnegotiation_size();
            if (max_size > 0 && subnegotiation_buffer_.size() >= max_size) {
                ProtocolConfig::log_error(
                    make_error_code(error::subnegotiation_overflow),
                    "byte: 0x{:02x}, cmd: {}, opt: {}",
                    byte,
                    current_command_,
                    *current_option_
                );
                change_state(ProtocolState::Normal);
                return {make_error_code(error::subnegotiation_overflow), false, std::nullopt};
            }
            //We either have an escaped IAC or an invalid command, so append IAC to the buffer.
            subnegotiation_buffer_.push_back(std::to_underlying(telnet::command::iac));
            if (byte != std::to_underlying(telnet::command::iac)) {
                ProtocolConfig::log_error(
                    make_error_code(error::invalid_command),
                    "byte: 0x{:02x}, cmd: {}, opt: {}",
                    byte,
                    current_command_,
                    *current_option_
                );
                //Invalid subnegotiation command (not SE or IAC), assume peer forgot to escape 0xFF/IAC byte, so append the stray byte to the buffer.
                subnegotiation_buffer_.push_back(byte);
            }
            change_state(ProtocolState::Subnegotiation);
        }
        return {std::error_code(), false, std::move(response)}; //discard subnegotiation byte
    } //handle_state_subnegotiation_iac(byte_t)
    
    /**
     * @internal
     * Processes an `IAC` `SB` `STATUS` `SEND` or `IS` sequence, validating the input buffer for `SEND` (1) or `IS` (0) and logging `telnet::error::invalid_subnegotiation` if invalid.
     * Validates enablement of `STATUS` option (local for `SEND`, remote for `IS`), logging `telnet::error::option_not_available` if not enabled.
     * For `IAC` `SB` `STATUS` `IS` ... `IAC` `SE`, delegates to a user-provided subnegotiation handler via `OptionHandlerRegistry`.
     * For `IAC` `SB` `STATUS` `SEND` `IAC` `SE`, constructs an `IS` [list] payload using `OptionStatusDB`, `co_return`ing a `SubnegotiationAwaitable`.
     * Iterates over `OptionStatusDB` to build the `SEND` payload with enabled options, excluding `STATUS`, escaping `IAC` (255) and `SE` (240) by doubling.
     * @see RFC 859, `:internal` for `OptionStatusDB`, `:options` for `option`, `:awaitables` for `SubnegotiationAwaitable`, `:stream` for `async_write_subnegotiation`
     */
    template<typename PC>
    awaitables::SubnegotiationAwaitable ProtocolFSM<PC>::handle_status_subnegotiation(const option opt, std::vector<byte_t> buffer) {
        constexpr byte_t IS   = static_cast<byte_t>(0);
        constexpr byte_t SEND = static_cast<byte_t>(1);

        if (buffer.empty()) {
            ProtocolConfig::log_error(error::invalid_subnegotiation, "Invalid STATUS subnegotiation: no data between IAC SB STATUS and IAC SE");
        } else if (buffer[0] == IS) {
            if (option_status_[option::id_num::status].remote_enabled()) {
                //Delegate processing of subcommand IS to user-provided handler.
                co_return co_await option_handler_registry_.handle_subnegotiation(opt, std::move(buffer));
            } else {
                ProtocolConfig::log_error(error::option_not_available, "STATUS subnegotiation IS received, but STATUS option is not remotely enabled.");
                co_return std::make_tuple(opt, std::vector<byte_t>{});
            }
        } else if (buffer[0] == SEND) {
            if (option_status_[option::id_num::status].local_enabled()) {        
                std::vector<byte_t> payload = {IS}; //IS
                for (std::size_t i = 0; i < OptionStatusDB::MAX_OPTION_COUNT; ++i) {
                    auto id = static_cast<option::id_num>(i);
                    
                    const auto& status = option_status_[id];
                    if (status.local_enabled()) {
                        payload.push_back(std::to_underlying(telnet::command::will_opt));
                        if (std::to_underlying(id) == std::to_underlying(telnet::command::iac) || std::to_underlying(id) == std::to_underlying(telnet::command::se)) { //Escape IAC or SE
                            payload.push_back(std::to_underlying(id));
                        }
                        payload.push_back(std::to_underlying(id));
                    }
                    if (status.remote_enabled()) {
                        payload.push_back(std::to_underlying(telnet::command::do_opt));
                        if (std::to_underlying(id) == std::to_underlying(telnet::command::iac) || std::to_underlying(id) == std::to_underlying(telnet::command::se)) { //Escape IAC or SE
                            payload.push_back(std::to_underlying(id));
                        }
                        payload.push_back(std::to_underlying(id));
                    }
                }
                co_return std::make_tuple(opt, std::move(payload));
            } else {
                ProtocolConfig::log_error(error::option_not_available, "STATUS subnegotiation SEND received, but STATUS option is not locally enabled.");
                co_return std::make_tuple(opt, std::vector<byte_t>{});
            }
        } else {
            ProtocolConfig::log_error(error::invalid_subnegotiation, "Invalid STATUS subnegotiation: expected IS (0) or SEND (1); received {}", buffer[0]);
            co_return std::make_tuple(opt, std::vector<byte_t>{});
        }
    } //handle_status_subnegotiation(const option&, std::vector<byte_t>)
} //namespace net::telnet
