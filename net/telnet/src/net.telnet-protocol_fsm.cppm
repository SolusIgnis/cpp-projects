/**
 * @file net.telnet-protocol_fsm.cppm
 * @version 0.5.7
 * @release_date October 30, 2025
 *
 * @brief Interface for the Telnet protocol finite state machine.
 * @remark Implements basic Telnet protocol (RFC 854) including IAC command processing and stateful negotiation.
 * @example
 *   telnet::protocol_fsm<> fsm; //Uses `default_protocol_fsm_config` from `:protocol_config`
 *   telnet::protocol_fsm<>::protocol_config_type::set_unknown_command_handler([](telnet::command cmd) { std::cout << "Custom: " << std::to_underlying(cmd) << "\n"; });
 *   telnet::protocol_fsm<>::protocol_config_type::registered_options.upsert(telnet::option::id_num::negotiate_about_window_size, "NAWS", telnet::option::always_accept, telnet::option::always_accept, true, 4);
 *   telnet::protocol_fsm<>::protocol_config_type::set_error_logger([](const std::error_code& ec, std::string msg) { std::cout << "Error: " << ec.message() << " - " << msg << std::endl; });
 *
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @see RFC 854 for Telnet protocol, RFC 855 and RFC 1143 for option negotiation, `:types` for `telnet::command` and `negotiation_direction`, `:options` for `option` and `option::id_num`, `:errors` for error codes, `:stream` for FSM usage, `:internal` for implementation classes, , `:protocol_config` for `default_protocol_fsm_config`, and `:concepts` for `ProtocolFSMConfig`
 * @todo Phase 6: Add optional half-duplex support (RFC 854) if legacy peer requirements arise.
 */

//Module partition interface unit
export module net.telnet:protocol_fsm;

import std; //NOLINT For std::function, std::optional, std::map, std::set, std::vector, std::shared_mutex, std::shared_lock, std::lock_guard, std::once_flag, std::cout, std::cerr, std::hex, std::setw, std::setfill, std::dec, std::format

export import :types;    ///< @see "net.telnet-types.cppm" for `byte_t`, `telnet::command`, and `negotiation_direction`
export import :errors;   ///< @see "net.telnet-errors.cppm" for `telnet::error` and `telnet::processing_signal` codes
export import :concepts; ///< @see "net.telnet-concepts.cppm" for `telnet::concepts::ProtocolFSMConfig`
export import :options;  ///< @see "net.telnet-options.cppm" for `option` and `option::id_num`
export import :awaitables; ///< @see "net.telnet-awaitables.cppm" for `TaggedAwaitable`, semantic tags, and type aliases

import :internal;        ///< @see "net.telnet-internal.cppm" for implementation classes
import :protocol_config; ///< @see "net.telnet-protocol_config.cppm" for `default_protocol_fsm_config`

//namespace asio = boost::asio;

namespace net::telnet {
    //Non-exported using declarations to simplify template constraints below.
    using concepts::ProtocolFSMConfig;
} //namespace net::telnet

export namespace net::telnet {
    /**
     * @brief Finite State Machine for Telnet protocol processing.
     * @tparam ConfigT Configuration class defining options and handlers (defaults to `default_protocol_fsm_config`).
     * @remark Initialized to `protocol_state::normal` per RFC 854.
     * @remark Thread-safe for `protocol_config_type` access via `mutex_`.
     * @see RFC 854 for Telnet protocol, RFC 855 for option negotiation, `:options` for `option` and `option::id_num`, `:types` for `telnet::command`, `:errors` for error codes, `:stream` for FSM usage, `:internal` for implementation classes
     */
    template<typename ConfigT = default_protocol_fsm_config>
    class protocol_fsm {
        static_assert(ProtocolFSMConfig<ConfigT>);

    public:
        /**
         * @typedef protocol_config_type
         * @brief Configuration class type for the FSM, defaulting to `default_protocol_fsm_config`.
         */
        using protocol_config_type = ConfigT;

        /**
         * @typedef option_enablement_handler_type
         * @brief Function type for handling option enablement.
         * @param id The `option::id_num` being enabled.
         * @param direction The negotiation direction (`local` or `remote`).
         * @return `option_enablement_awaitable` representing the asynchronous handling result.
         */
        using option_enablement_handler_type =
            std::function<awaitables::option_enablement_awaitable(option::id_num /*id*/, negotiation_direction /*direction*/)>;

        /**
         * @typedef option_disablement_handler_type
         * @brief Function type for handling option disablement.
         * @param id The `option::id_num` being disabled.
         * @param direction The negotiation direction (`local` or `remote`).
         * @return `option_disablement_awaitable` representing the asynchronous handling result.
         */
        using option_disablement_handler_type =
            std::function<awaitables::option_disablement_awaitable(option::id_num /*id*/, negotiation_direction /*direction*/)>;

        /**
         * @typedef subnegotiation_handler_type
         * @brief Function type for processing subnegotiation data.
         * @param id The `option::id_num` identifying the option.
         * @param data The `std::vector<byte_t>` containing subnegotiation data.
         * @return `subnegotiation_awaitable` for asynchronous subnegotiation handling.
         */
        using subnegotiation_handler_type =
            std::function<awaitables::subnegotiation_awaitable(const option& /*id*/, std::vector<byte_t> /*data*/)>;

        /**
         * @typedef unknown_option_handler_type
         * @brief Function type for handling unknown option negotiation attempts.
         * @param id The `option::id_num` of the unknown option.
         */
        using unknown_option_handler_type = std::function<void(option::id_num /*id*/)>;

        /**
         * @typedef error_logger_type
         * @brief Function type for logging errors during FSM processing.
         * @param ec The `std::error_code` describing the error.
         * @param msg The formatted error message.
         */
        using error_logger_type = std::function<void(const std::error_code& /*ec*/, const std::string& /*msg*/)>;

        /**
         * @typedef negotiation_response_type
         * @brief Tuple type representing an option negotiation.
         * @remark Holds the `negotiation_direction`, enablement state, and `option::id_num`.
         */
        using negotiation_response_type = std::tuple<negotiation_direction, bool, option::id_num>;

        /**
         * @typedef processing_return_variant
         * @brief Variant type for return values from byte processing.
         * @remark Holds either a `negotiation_response_type`, direct stream write (`std::string`), handler awaitable (`option_enablement_awaitable` or `option_disablement_awaitable`) paired with an optional `negotiation_response_type`, or subnegotiation awaitable (`subnegotiation_awaitable`).
         */
        using processing_return_variant =
            std::variant<negotiation_response_type,
                         std::string,
                         std::tuple<awaitables::option_enablement_awaitable, std::optional<negotiation_response_type>>,
                         std::tuple<awaitables::option_disablement_awaitable, std::optional<negotiation_response_type>>,
                         awaitables::subnegotiation_awaitable>;

        ///@brief Constructs the FSM, initializing `protocol_config_type` once.
        protocol_fsm() { ConfigT::initialize(); }

        ///@brief Registers handlers for option enablement, disablement, and subnegotiation.
        void register_option_handlers(option::id_num opt,
                                      std::optional<option_enablement_handler_type> enable_handler,
                                      std::optional<option_disablement_handler_type> disable_handler,
                                      std::optional<subnegotiation_handler_type> subneg_handler = std::nullopt) {
            option_handler_registry_.register_handlers(opt,
                                                       std::move(enable_handler),
                                                       std::move(disable_handler),
                                                       std::move(subneg_handler));
        }

        ///@brief Unregisters handlers for an option.
        void unregister_option_handlers(option::id_num opt) { option_handler_registry_.unregister_handlers(opt); }

        ///@brief Processes a single byte of Telnet input.
        std::tuple<std::error_code, bool, std::optional<processing_return_variant>> process_byte(byte_t byte) noexcept;

        ///@brief Checks if an option is enabled locally or remotely.
        bool is_enabled(option::id_num opt) { return option_status_[opt].is_enabled(); }

        ///@brief Checks if an option is enabled in a specified direction.
        bool is_enabled(option::id_num opt, negotiation_direction dir) { return option_status_[opt].enabled(dir); }

        ///@brief Makes a negotiation response command
        static telnet::command make_negotiation_command(negotiation_direction direction, bool enable) noexcept;

        ///@brief Requests an option to be enabled (WILL/DO), synchronously updating option_status_db and returning a negotiation response.
        std::tuple<std::error_code, std::optional<negotiation_response_type>> request_option(option::id_num opt,
                                                                                       negotiation_direction direction);

        ///@brief Disables an option (WONT/DONT), synchronously updating option_status_db and returning a negotiation response and optional disablement awaitable.
        std::tuple<std::error_code,
                   std::optional<negotiation_response_type>,
                   std::optional<awaitables::option_disablement_awaitable>>
            disable_option(option::id_num opt, negotiation_direction direction);

    private:
        enum class protocol_state : std::uint8_t {
            normal,                ///< Default state for data processing
            has_cr,                ///< Processing the byte after '\r'
            has_iac,               ///< Processing IAC command byte
            option_negotiation,    ///< Processing option ID after WILL/WONT/DO/DONT
            subnegotiation_option, ///< Processing option ID after SB
            subnegotiation,        ///< Processing subnegotiation data
            subnegotiation_iac     ///< Processing IAC during subnegotiation
        };

        ///@brief Changes the FSM state.
        void change_state(protocol_state next_state) noexcept;

        ///@brief Handles bytes in the `Normal` state (data or IAC).
        std::tuple<std::error_code, bool, std::optional<processing_return_variant>>
            handle_state_normal(byte_t byte) noexcept;

        ///@brief Handles bytes immediately after '\r' ('\0', '\n', or error)
        std::tuple<std::error_code, bool, std::optional<processing_return_variant>>
            handle_state_has_cr(byte_t byte) noexcept;

        ///@brief Handles bytes after IAC (commands like WILL, DO, SB, etc.).
        std::tuple<std::error_code, bool, std::optional<processing_return_variant>>
            handle_state_iac(byte_t byte) noexcept;

        ///@brief Handles bytes in the `OptionNegotiation` state (option ID after WILL/WONT/DO/DONT).
        std::tuple<std::error_code, bool, std::optional<processing_return_variant>>
            handle_state_option_negotiation(byte_t byte) noexcept;

        ///@brief Handles bytes in the `SubnegotiationOption` state (option ID after IAC SB).
        std::tuple<std::error_code, bool, std::optional<processing_return_variant>>
            handle_state_subnegotiation_option(byte_t byte) noexcept;

        ///@brief Handles bytes in the `Subnegotiation` state (data after option ID).
        std::tuple<std::error_code, bool, std::optional<processing_return_variant>>
            handle_state_subnegotiation(byte_t byte) noexcept;

        ///@brief Handles bytes in the `SubnegotiationIAC` state (IAC in subnegotiation data).
        std::tuple<std::error_code, bool, std::optional<processing_return_variant>>
            handle_state_subnegotiation_iac(byte_t byte) noexcept;

        ///@brief Handles STATUS subnegotiation (RFC 859), returning an awaitable with the IS [list] payload or user-handled result.
        auto handle_status_subnegotiation(option opt, std::vector<byte_t> buffer)
            -> awaitables::subnegotiation_awaitable;

        //Data Members
        option_handler_registry<protocol_config_type, option_enablement_handler_type, option_disablement_handler_type, subnegotiation_handler_type>
            option_handler_registry_;
        option_status_db option_status_;

        protocol_state current_state_ = protocol_state::normal;
        std::optional<telnet::command> current_command_;
        std::optional<option> current_option_;
        std::vector<byte_t> subnegotiation_buffer_;
    }; //class protocol_fsm

    /**
     * @fn protocol_fsm::protocol_fsm()
     *
     * @remark Initializes `protocol_config_type` using `ConfigT::initialize`.
     */
    /**
     * @fn void protocol_fsm::register_option_handlers(option::id_num opt, std::optional<option_enablement_handler_type> enable_handler, std::optional<option_disablement_handler_type> disable_handler, std::optional<subnegotiation_handler_type> subneg_handler)
     *
     * @param opt The `option::id_num` for which to register handlers.
     * @param enable_handler Optional `option_enablement_handler_type` to handle option enablement.
     * @param disable_handler Optional `option_disablement_handler_type` to handle option disablement.
     * @param subneg_handler Optional `subnegotiation_handler_type` to handle subnegotiation data (defaults to `std::nullopt`).
     *
     * @remark Forwards to `OptionHandlerRegistry::register_handlers`.
     */
    /**
     * @fn void protocol_fsm::unregister_option_handlers(option::id_num opt)
     *
     * @param opt The `option::id_num` for which to unregister handlers.
     *
     * @remark Forwards to `OptionHandlerRegistry::unregister_handlers`.
     */
    /**
     * @fn std::tuple<std::error_code, bool, std::optional<processing_return_variant>> protocol_fsm::process_byte(byte_t byte) noexcept
     *
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag (true to pass byte to application), and optional negotiation response.
     *
     * @remark Dispatches to state-specific handlers based on `current_state_`.
     * @remark Returns `error::protocol_violation` for invalid states.
     * @see RFC 854 for Telnet protocol, RFC 855 for option negotiation, :errors for error codes
     */
    /**
     * @fn bool protocol_fsm::is_enabled(const option& opt)
     *
     * @param opt The `option::id_num` to check.
     * @return True if `opt` is enabled either locally or remotely, false otherwise.
     *
     * @remark Queries `option_status_db` for the option’s status.
     */
    /**
     * @overload bool protocol_fsm::is_enabled(option::id_num opt, negotiation_direction dir)
     *
     * @param opt The `option::id_num` to check.
     * @param dir The `negotiation_direction` to check.
     * @return True if `opt` is enabled in the designated direction, false otherwise.
     *
     * @remark Queries `option_status_db` for the option’s status.
     */
    /**
     * @fn telnet::command make_negotiation_command(negotiation_direction direction, bool enable) noexcept
     *
     * @param direction Negotiate local (DO/DONT) or remote (WILL/WONT) option?
     * @param enable Negotiate to enable?
     * @return The `telnet::command` representing the desired negotiation.
     */
    /**
     * @fn std::tuple<std::error_code, std::optional<negotiation_response_type>> protocol_fsm::request_option(option::id_num opt, negotiation_direction direction)
     *
     * @param opt The Telnet option (`option::id_num`) to request.
     * @param direction The negotiation direction (`LOCAL` or `REMOTE`).
     * @return std::tuple<std::error_code, std::optional<negotiation_response_type>> with error code and optional response (true for enable, false for disable).
     *
     * @remark Validates option registration and state per RFC 1143 Q Method, handling six states: YES, WANTYES/EMPTY, WANTYES/OPPOSITE, WANTNO/EMPTY, WANTNO/OPPOSITE, NO.
     * @remark Enqueues opposite request in WANTNO/EMPTY; treats redundant requests (YES, WANTYES/EMPTY, WANTNO/OPPOSITE) as idempotent successes, logging warnings.
     * @remark Does not invoke an enablement handler, as it initiates negotiation without enabling.
     * @throws None; errors returned via error_code (e.g., `error::option_not_available`, `error::negotiation_queue_error`, `error::protocol_violation`).
     * @see RFC 1143 for Q Method, `:options` for `option::id_num`, `:errors` for error codes, `:types` for `negotiation_direction`, `:stream` for usage in async_request_option
     */
    /**
     * @fn std::tuple<std::error_code, std::optional<negotiation_response_type>, std::optional<awaitables::option_disablement_awaitable>> protocol_fsm::disable_option(option::id_num opt, negotiation_direction direction)
     *
     * @param opt The Telnet option (`option::id_num`) to disable.
     * @param direction The negotiation direction (`LOCAL` or `REMOTE`).
     * @return std::tuple<std::error_code, std::optional<negotiation_response_type>, std::optional<awaitables::option_disablement_awaitable>> with error code, optional response, and optional disablement awaitable.
     *
     * @remark Validates option registration and state per RFC 1143 Q Method, handling six states: NO, WANTNO/EMPTY, WANTNO/OPPOSITE, WANTYES/EMPTY, WANTYES/OPPOSITE, YES.
     * @remark Enqueues opposite request in WANTYES/EMPTY; treats redundant disablements (NO, WANTNO/EMPTY, WANTYES/OPPOSITE) as idempotent successes, logging warnings.
     * @remark Returns `option_disablement_awaitable` in YES state via `OptionHandlerRegistry::handle_disablement` (no-op if unregistered).
     * @throws None; errors returned via error_code (e.g., `error::option_not_available`, `error::negotiation_queue_error`, `error::protocol_violation`).
     * @see RFC 1143 for Q Method, `:options` for `option::id_num`, `:errors` for error codes, `:types` for `negotiation_direction`, `:awaitables` for `option_disablement_awaitable`, `:stream` for usage in async_disable_option
     */
    /**
     * @fn void protocol_fsm::change_state(protocol_state next_state) noexcept
     *
     * @param next_state The new `protocol_state` to transition to.
     *
     * @remark Updates the FSM’s `current_state_`.
     * @see RFC 854 for state transitions
     */
    /**
     * @fn std::tuple<std::error_code, bool, std::optional<processing_return_variant>> protocol_fsm::handle_state_normal(byte_t byte) noexcept
     *
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag (true for data bytes), and optional negotiation response.
     *
     * @remark Processes data bytes or IAC in the `Normal` state.
     * @see RFC 854 for Normal state behavior, `:types` for `telnet::command`
     */
    /**
     * @fn std::tuple<std::error_code, bool, std::optional<processing_return_variant>> protocol_fsm::handle_state_has_cr(byte_t byte) noexcept
     *
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag (true for data bytes), and optional negotiation response.
     *
     * @remark Processes data bytes or IAC in the `HasCR` state.
     * @see RFC 854 for CR NUL or CR LF state behavior, `:types` for `telnet::command`
     */
    /**
     * @fn std::tuple<std::error_code, bool, std::optional<processing_return_variant>> protocol_fsm::handle_state_iac(byte_t byte) noexcept
     *
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag (true for escaped IAC), and optional response (`processing_return_variant`)
     *
     * @remark Processes commands like `WILL`, `DO`, `SB`, etc., after IAC.
     * @see RFC 854 for IAC command processing, `:types` for `telnet::command`, `:errors` for error codes
     */
    /**
     * @fn std::tuple<std::error_code, bool, std::optional<processing_return_variant>> protocol_fsm::handle_state_option_negotiation(byte_t byte) noexcept
     *
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag (false), and optional negotiation response (`std::tuple<telnet::command, telnet::option::id_num>`).
     *
     * @remark Processes option ID after `WILL`, `WONT`, `DO`, or `DONT`.
     * @see RFC 855 for option negotiation, `:options` for `option::id_num`, `:errors` for error codes
     */
    /**
     * @fn std::tuple<std::error_code, bool, std::optional<processing_return_variant>> protocol_fsm::handle_state_subnegotiation_option(byte_t byte) noexcept
     *
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag (false), and optional negotiation response.
     *
     * @note Logs `error::invalid_subnegotiation` for unenabled or unregistered options.
     * @see RFC 855 for subnegotiation, `:options` for `option::id_num`, `:errors` for error codes
     */
    /**
     * @fn std::tuple<std::error_code, bool, std::optional<processing_return_variant>> protocol_fsm::handle_state_subnegotiation(byte_t byte) noexcept
     *
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag (false), and optional negotiation response.
     *
     * @note Logs `error::protocol_violation` if no current option is set.
     * @note Checks for IAC before buffer overflow.
     * @see RFC 855 for subnegotiation, `:options` for `option::id_num`, `:errors` for error codes
     */
    /**
     * @fn std::tuple<std::error_code, bool, std::optional<processing_return_variant>> protocol_fsm::handle_state_subnegotiation_iac(byte_t byte) noexcept
     *
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag (false), and optional negotiation response (`subnegotiation_awaitable`).
     *
     * @note Relies on `option_handler_registry::handle_subnegotiation` to log `error::user_handler_not_found` when no handler is registered for the option.
     * @note Logs `error::protocol_violation` if no current option is set.
     * @note Logs `error::invalid_command` for invalid commands (not `SE` or `IAC`).
     * @see RFC 855 for subnegotiation, `:options` for `option::id_num`, `:errors` for error codes
     * @todo Phase 6: Review `subnegotiation_overflow` handling for custom error handlers
     */
    /**
     * @fn protocol_fsm::handle_status_subnegotiation(const option& opt, std::vector<byte_t> buffer)
     * @param opt The `status` option being negotiated (expected to be `option::id_num::status`)
     * @param buffer The subnegotiation input data (expected to contain `SEND` (1) or `IS` (0))
     * @return `subnegotiation_awaitable` yielding `std::tuple<const option&, std::vector<byte_t>>` containing the `status` option and the `IS` [list] payload for `SEND`, or user-defined result for `IS`
     * @remark For `SEND` (1), validates that `STATUS` is locally enabled; constructs a payload starting with `IS` (0), followed by pairs of [`WILL` (251), opt] for locally enabled options and [`DO` (252), opt] for remotely enabled options, with `IAC` (255) and `SE` (240) option codes escaped by doubling.
     * @remark For `IS` (0), validates that `STATUS` is remotely enabled and delegates to the user-provided subnegotiation handler via `OptionHandlerRegistry`.
     * @remark Logs `telnet::error::option_not_available` and returns an empty payload if `status` is not enabled in the required direction.
     * @remark Logs `telnet::error::invalid_subnegotiation` for invalid subcommands.
     * @remark The returned `subnegotiation_awaitable` is processed by `InputProcessor` to pass the payload to `stream::async_write_subnegotiation`, which adds `IAC` `SB` `status` ... `IAC` `SE` framing.
     * @see RFC 859, `:internal` for `option_status_db`, `:options` for `option`, `:awaitables` for `subnegotiation_awaitable`, `:stream` for `async_write_subnegotiation`
     */
} //namespace net::telnet
