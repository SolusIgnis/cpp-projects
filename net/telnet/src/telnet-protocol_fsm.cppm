/**
 * @file telnet-protocol_fsm.cppm
 * @version 0.5.0
 * @release_date October 17, 2025
 *
 * @brief Interface for the Telnet protocol finite state machine.
 * @remark Implements basic Telnet protocol (RFC 854) including IAC command processing and stateful negotiation.
 * @example
 *   telnet::ProtocolFSM<> fsm; // Uses `DefaultProtocolFSMConfig`
 *   telnet::ProtocolFSM<>::ProtocolConfig::set_unknown_command_handler([](telnet::TelnetCommand cmd) { std::cout << "Custom: " << std::to_underlying(cmd) << "\n"; });
 *   telnet::ProtocolFSM<>::ProtocolConfig::registered_options.upsert(telnet::option::id_num::NEGOTIATE_ABOUT_WINDOW_SIZE, "NAWS", telnet::option::always_accept, telnet::option::always_accept, true, 4);
 *   telnet::ProtocolFSM<>::ProtocolConfig::set_error_logger([](const std::error_code& ec, std::string msg) { std::cout << "Error: " << ec.message() << " - " << msg << std::endl; });
 *
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @see RFC 854 for Telnet protocol, RFC 855 and RFC 1143 for option negotiation, :types for `TelnetCommand`, :options for `option` and `option::id_num`, :errors for error codes, :socket for FSM usage, :internal for implementation classes
 * @todo Phase 6: Add optional half-duplex support (RFC 854) if legacy peer requirements arise.
 * @todo Phase 6: Consider moving concept `ProtocolFSMConfig` to a :concepts partition or a :protocol_config partition (with `DefaultProtocolFSMConfig`).
 */
module; //Including Boost.Asio in the Global Module Fragment until importable header units are reliable.
#include <boost/asio.hpp>
namespace asio = boost::asio;

//Module partition interface unit
export module telnet:protocol_fsm;

import std; // For std::function, std::optional, std::map, std::set, std::vector, std::shared_mutex, std::shared_lock, std::lock_guard, std::once_flag, std::cout, std::cerr, std::hex, std::setw, std::setfill, std::dec, std::format

export import :types;      ///< @see telnet-types.cppm for `byte_t`, `TelnetCommand`, and `NegotiationDirection`
export import :errors;     ///< @see telnet-errors.cppm for `telnet::error` codes
export import :options;    ///< @see telnet-options.cppm for `option` and `option::id_num`
export import :awaitables; ///< @see telnet-awaitables.cppm for `TaggedAwaitable`, semantic tags, and type aliases

import :internal;          ///< @see telnet-internal.cppm for implementation classes

export namespace telnet {
    //Forward declaration referenced in the concept definition.
    template<typename ConfigT> class ProtocolFSM; 
    
    /**
     * @brief Concept to constrain `ProtocolConfig` for `ProtocolFSM`.
     * @remark Ensures `ProtocolConfig` provides necessary types and methods for `ProtocolFSM`, including initialization.
     * @see RFC 854 for Telnet protocol, RFC 855 for option negotiation, :options for `option` and `option::id_num`, :errors for error codes, :internal for implementation classes
     */
    template<typename T>
    concept ProtocolFSMConfig = requires(T config, TelnetCommand cmd, option full_opt, option::id_num opt, std::error_code ec, byte_t byte, std::string msg) {
        typename T::UnknownOptionHandler;
        typename T::ErrorLogger;
        { T::initialize() } -> std::same_as<void>;
        { T::set_unknown_option_handler(std::declval<typename T::UnknownOptionHandler>()) } -> std::same_as<void>;
        { T::set_error_logger(std::declval<typename T::ErrorLogger>()) } -> std::same_as<void>;
        { T::get_unknown_option_handler() } -> std::convertible_to<const typename ProtocolFSM<T>::UnknownOptionHandler&>;
        { T::log_error(ec, std::declval<std::string>()) } -> std::same_as<void>;
        { T::registered_options.get(opt) } -> std::convertible_to<std::optional<option>>;
        { T::registered_options.has(opt) } -> std::same_as<bool>;
        { T::registered_options.upsert(opt) } -> std::convertible_to<const option&>;
        { T::registered_options.upsert(full_opt, ec) } -> std::same_as<void>;
        { T::registered_options.upsert(full_opt) } -> std::convertible_to<const option&>;
        { T::get_ayt_response() } -> std::same_as<std::string_view>;
        { T::set_ayt_response(std::declval<std::string>()) } -> std::same_as<void>;
        requires std::convertible_to<typename T::UnknownOptionHandler, typename ProtocolFSM<T>::UnknownOptionHandler>;
        requires std::convertible_to<typename T::ErrorLogger, typename ProtocolFSM<T>::ErrorLogger>;
    }; //concept ProtocolFSMConfig
    
    /**
     * @brief Default configuration class for `ProtocolFSM`, encapsulating options and handlers.
     * @remark Provides thread-safe access to static members via `mutex_`.
     * @see RFC 854 for Telnet protocol, RFC 855 for option negotiation, :options for `option` and `option::id_num`, :errors for error codes, :internal for implementation classes
     */
    class DefaultProtocolFSMConfig {
    public:
        /**
         * @typedef UnknownOptionHandler
         * @brief Function type for handling unknown option negotiation attempts.
         * @param id The `option::id_num` of the unknown option.
         */
        using UnknownOptionHandler = std::function<void(option::id_num)>;

        /**
         * @typedef ErrorLogger
         * @brief Function type for logging errors during FSM processing.
         * @param ec The `std::error_code` describing the error.
         * @param msg The formatted error message.
         */
        using ErrorLogger = std::function<void(const std::error_code&, std::string)>;

        /// @brief Initializes the configuration once.
        static void initialize() {
            std::call_once(initialization_flag_, &init);
        }

        /// @brief Sets the handler for unknown option negotiation attempts.
        static void set_unknown_option_handler(UnknownOptionHandler handler) {
            std::lock_guard<std::shared_mutex> lock(mutex_);
            unknown_option_handler_ = std::move(handler);
        }

        /// @brief Sets the error logging handler.
        static void set_error_logger(ErrorLogger handler) {
            std::lock_guard<std::shared_mutex> lock(mutex_);
            error_logger_ = std::move(handler);
        }

        /// @brief Gets the handler for unknown option negotiation attempts.
        static const UnknownOptionHandler& get_unknown_option_handler() {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            return unknown_option_handler_;
        }
        
        /// @brief Logs an error with the registered error logger using a formatted string.
        template<typename ...Args>
        static void log_error(const std::error_code& ec, std::format_string<auto> fmt, Args&&... args) {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            if (error_logger_) {
                error_logger_(ec, std::format(fmt, std::forward<Args>(args)...));
            }
        }
        
        /// @brief Gets the AYT response string.
        static std::string_view get_ayt_response() {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            return ayt_response_;
        }
    
        /// @brief Sets the AYT response string.
        static void set_ayt_response(std::string response) {
            std::lock_guard<std::shared_mutex> lock(mutex_);
            ayt_response_ = std::move(response);
        }
        
        static inline option_registry registered_options = initialize_option_registry();

    private:
        /// @brief Initializes the option registry with default options.
        static option_registry initialize_option_registry() {
            return {
                option{option::id_num::BINARY,            "Binary Transmission", option::always_accept, option::always_accept},
                option{option::id_num::SUPPRESS_GO_AHEAD, "Suppress Go-Ahead",   option::always_accept, option::always_accept},
                option{option::id_num::STATUS,            "Status",              option::always_accept, option::always_reject, true}           
            };
        } //initialize_option_registry()

        /// @brief Performs initialization for `initialize`.
        static void init() {
            std::lock_guard<std::shared_mutex> lock(mutex_);
            unknown_option_handler_ = [](option::id_num opt) {
                std::cout << "Unknown option: " << std::to_underlying(opt) << "\n";
                return;
            };
            error_logger_ = [](const std::error_code& ec, std::string msg) {
                std::cerr << "Telnet FSM error: " << ec.message() << " (" << msg << ")" << std::endl;
            };
        } //init()

        static inline UnknownOptionHandler unknown_option_handler_;
        static inline ErrorLogger error_logger_;
        static inline std::string ayt_response_ = "Telnet system is active."; /// Default AYT response
        static inline std::shared_mutex mutex_; /// Mutex to protect shared static members
        static inline std::once_flag initialization_flag_; /// Ensures initialize() is idempotent; only invokes init() once
    }; //class DefaultProtocolFSMConfig

    /**
     * @fn void DefaultProtocolFSMConfig::initialize()
     *
     * @remark Initializes static members using `init` under a `std::once_flag`.
     * @remark Thread-safe via `std::call_once`.
     */
    /**
     * @fn void DefaultProtocolFSMConfig::set_unknown_option_handler(UnknownOptionHandler handler)
     *
     * @param handler The `UnknownOptionHandler` to set for unknown option negotiations.
     *
     * @remark Thread-safe via `std::lock_guard<std::shared_mutex>`.
     */
    /**
     * @fn void DefaultProtocolFSMConfig::set_error_logger(ErrorLogger handler)
     *
     * @param handler The `ErrorLogger` to set for error reporting.
     *
     * @remark Thread-safe via `std::lock_guard<std::shared_mutex>`.
     */
    /**
     * @fn const UnknownOptionHandler& DefaultProtocolFSMConfig::get_unknown_option_handler()
     *
     * @return Const reference to the `UnknownOptionHandler`.
     *
     * @remark Thread-safe via `std::shared_lock<std::shared_mutex>`.
     */
    /**
     * @fn template<typename ...Args> void DefaultProtocolFSMConfig::log_error(const std::error_code& ec, std::format_string<auto> fmt, Args&&... args)
     *
     * @tparam Args Variadic argument type pack.
     * @param ec The error code to log.
     * @param fmt The format string for the error message.
     * @param args Arguments to format the error message.
     *
     * @remark Formats the message using std::format and invokes the registered `error_logger_`.
     * @remark Thread-safe via `std::shared_lock<std::shared_mutex>`.
     */
    /**
     * @fn std::string_view DefaultProtocolFSMConfig::get_ayt_response()
     *
     * @return `std::string_view` of the AYT response string.
     *
     * @remark Thread-safe via `std::shared_lock<std::shared_mutex>`.
     */
    /**
     * @fn void DefaultProtocolFSMConfig::set_ayt_response(std::string response)
     *
     * @param response The AYT response string to set.
     *
     * @remark Thread-safe via `std::lock_guard<std::shared_mutex>`.
     */
    /**
     * @fn option_registry DefaultProtocolFSMConfig::initialize_option_registry()
     *
     * @return `option_registry` with default `option` instances.
     *
     * @remark Initializes options for `BINARY`, `SUPPRESS_GO_AHEAD`, and `STATUS` to support default implementations.
     * @note `STATUS` is supported locally but not remotely by default as the core implementation can send a status report but will not request one and cannot understand receipt of one.
     */
    /**
     * @fn void DefaultProtocolFSMConfig::init()
     *
     * @remark Initializes `unknown_command_handler_`, `unknown_option_handler_`, and `error_logger_`.
     * @remark Called once by `initialize` under `std::call_once`.
     */
     
    /**
     * @brief Finite State Machine for Telnet protocol processing.
     * @tparam ConfigT Configuration class defining options and handlers (defaults to `DefaultProtocolFSMConfig`).
     * @remark Initialized to `ProtocolState::Normal` per RFC 854.
     * @remark Thread-safe for `ProtocolConfig` access via `mutex_`.
     * @see RFC 854 for Telnet protocol, RFC 855 for option negotiation, :options for `option` and `option::id_num`, :types for `TelnetCommand`, :errors for error codes, :socket for FSM usage, :internal for implementation classes
     */
    template<ProtocolFSMConfig ConfigT = DefaultProtocolFSMConfig>
    class ProtocolFSM {
    public:
        /**
         * @typedef ProtocolConfig
         * @brief Configuration class type for the FSM, defaulting to `DefaultProtocolFSMConfig`.
         */
        using ProtocolConfig = ConfigT;

        /**
         * @typedef OptionEnablementHandler
         * @brief Function type for handling option enablement.
         * @param id The `option::id_num` being enabled.
         * @param direction The negotiation direction (`local` or `remote`).
         * @return `OptionEnablementAwaitable` representing the asynchronous handling result.
         */
        using OptionEnablementHandler = std::function<awaitables::OptionEnablementAwaitable(option::id_num, NegotiationDirection)>;

        /**
         * @typedef OptionDisablementHandler
         * @brief Function type for handling option disablement.
         * @param id The `option::id_num` being disabled.
         * @param direction The negotiation direction (`local` or `remote`).
         * @return `OptionDisablementAwaitable` representing the asynchronous handling result.
         */
        using OptionDisablementHandler = std::function<awaitables::OptionDisablementAwaitable(option::id_num, NegotiationDirection)>;

        /**
         * @typedef SubnegotiationHandler
         * @brief Function type for processing subnegotiation data.
         * @param id The `option::id_num` identifying the option.
         * @param data The `std::vector<byte_t>` containing subnegotiation data.
         * @return `SubnegotiationAwaitable` for asynchronous subnegotiation handling.
         */
        using SubnegotiationHandler = std::function<awaitables::SubnegotiationAwaitable(const option&, std::vector<byte_t>)>;

        /**
         * @typedef UnknownOptionHandler
         * @brief Function type for handling unknown option negotiation attempts.
         * @param id The `option::id_num` of the unknown option.
         */
        using UnknownOptionHandler = std::function<void(option::id_num)>;

        /**
         * @typedef ErrorLogger
         * @brief Function type for logging errors during FSM processing.
         * @param ec The `std::error_code` describing the error.
         * @param msg The formatted error message.
         */
        using ErrorLogger = std::function<void(const std::error_code&, std::string)>;

        /**
         * @typedef NegotiationResponse
         * @brief Tuple type representing an option negotiation.
         * @remark Holds the `NegotiationDirection`, enablement state, and `option::id_num`.
         */
        using NegotiationResponse = std::tuple<NegotiationDirection, bool, option::id_num>;

        /**
         * @typedef ProcessingReturnVariant
         * @brief Variant type for return values from byte processing.
         * @remark Holds either a `NegotiationResponse`, direct socket write (`std::string`), handler awaitable (`OptionEnablementAwaitable` or `OptionDisablementAwaitable`) paired with an optional `NegotiationResponse`, or subnegotiation awaitable (`SubnegotiationAwaitable`).
         */
        using ProcessingReturnVariant = std::variant<
            NegotiationResponse,
            std::string,
            std::tuple<OptionEnablementAwaitable, std::optional<NegotiationResponse>>,
            std::tuple<OptionDisablementAwaitable, std::optional<NegotiationResponse>>,
            SubnegotiationAwaitable
        >;

        /// @brief Constructs the FSM, initializing `ProtocolConfig` once.
        ProtocolFSM() {
            ConfigT::initialize();
        }

        /// @brief Registers handlers for option enablement, disablement, and subnegotiation.
        void register_option_handlers(option::id_num opt, std::optional<OptionEnablementHandler> enable_handler, std::optional<OptionDisablementHandler> disable_handler, std::optional<SubnegotiationHandler> subneg_handler = std::nullopt) { option_handler_registry_.register_handlers(opt, std::move(enable_handler), std::move(disable_handler), std::move(subneg_handler)); }

        /// @brief Unregisters handlers for an option.
        void unregister_option_handlers(option::id_num opt) { option_handler_registry_.unregister_handlers(opt); }

        /// @brief Processes a single byte of Telnet input.
        std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>>
        process_byte(byte_t byte) noexcept;

        /// @brief Checks if an option is enabled locally or remotely.
        bool is_enabled(option::id_num opt) { return option_status_[opt].is_enabled(); }
        
        /// @brief Checks if an option is enabled in a specified direction.
        bool is_enabled(option::id_num opt, NegotiationDirection dir) { return option_status_[opt].enabled(dir); }

        /// @brief Makes a negotiation response command
        static TelnetCommand make_negotiation_command(NegotiationDirection direction, bool enable) noexcept;

        /// @brief Requests an option to be enabled (WILL/DO), synchronously updating OptionStatusDB and returning a negotiation response.
        std::tuple<std::error_code, std::optional<NegotiationResponse>> request_option(option::id_num opt, NegotiationDirection direction);

        /// @brief Disables an option (WONT/DONT), synchronously updating OptionStatusDB and returning a negotiation response and optional disablement awaitable.
        std::tuple<std::error_code, std::optional<NegotiationResponse>, std::optional<awaitables::OptionDisablementAwaitable>> disable_option(option::id_num opt, NegotiationDirection direction);

    private:
        enum class ProtocolState { 
            Normal,               ///< Default state for data processing
            HasCR,                ///< Processing the byte after '\r'
            IAC,                  ///< Processing IAC command byte
            OptionNegotiation,    ///< Processing option ID after WILL/WONT/DO/DONT
            SubnegotiationOption, ///< Processing option ID after SB
            Subnegotiation,       ///< Processing subnegotiation data
            SubnegotiationIAC     ///< Processing IAC during subnegotiation
        };

        /// @brief Changes the FSM state.
        void change_state(ProtocolState next_state) noexcept;

        /// @brief Handles bytes in the `Normal` state (data or IAC).
        std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>>
        handle_state_normal(byte_t byte) noexcept;

        /// @brief Handles bytes immediately after '\r' ('\0', '\n', or error)
        std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>>
        handle_state_has_cr(byte_t byte) noexcept;

        /// @brief Handles bytes after IAC (commands like WILL, DO, SB, etc.).
        std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>>
        handle_state_iac(byte_t byte) noexcept;

        /// @brief Handles bytes in the `OptionNegotiation` state (option ID after WILL/WONT/DO/DONT).
        std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>>
        handle_state_option_negotiation(byte_t byte) noexcept;

        /// @brief Handles bytes in the `SubnegotiationOption` state (option ID after IAC SB).
        std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>>
        handle_state_subnegotiation_option(byte_t byte) noexcept;

        /// @brief Handles bytes in the `Subnegotiation` state (data after option ID).
        std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>>
        handle_state_subnegotiation(byte_t byte) noexcept;

        /// @brief Handles bytes in the `SubnegotiationIAC` state (IAC in subnegotiation data).
        std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>>
        handle_state_subnegotiation_iac(byte_t byte) noexcept;

        /// @brief Handles STATUS subnegotiation (RFC 859), returning an awaitable with the IS [list] payload or user-handled result.
        auto handle_status_subnegotiation(const option& opt, std::vector<byte_t> buffer) -> awaitables::SubnegotiationAwaitable;

        // Data Members
        OptionHandlerRegistry<ProtocolConfig, OptionEnablementHandler, OptionDisablementHandler, SubnegotiationHandler> option_handler_registry_;
        OptionStatusDB option_status_;
        
        ProtocolState current_state_ = ProtocolState::Normal;
        std::optional<TelnetCommand> current_command_;
        std::optional<option> current_option_;
        std::vector<byte_t> subnegotiation_buffer_;
    }; //class ProtocolFSM

    /**
     * @fn ProtocolFSM::ProtocolFSM()
     *
     * @remark Initializes `ProtocolConfig` using `ConfigT::initialize`.
     */
    /**
     * @fn void ProtocolFSM::register_option_handlers(option::id_num opt, std::optional<OptionEnablementHandler> enable_handler, std::optional<OptionDisablementHandler> disable_handler, std::optional<SubnegotiationHandler> subneg_handler)
     *
     * @param opt The `option::id_num` for which to register handlers.
     * @param enable_handler Optional `OptionEnablementHandler` to handle option enablement.
     * @param disable_handler Optional `OptionDisablementHandler` to handle option disablement.
     * @param subneg_handler Optional `SubnegotiationHandler` to handle subnegotiation data (defaults to `std::nullopt`).
     *
     * @remark Forwards to `OptionHandlerRegistry::register_handlers`.
     */
    /**
     * @fn void ProtocolFSM::unregister_option_handlers(option::id_num opt)
     *
     * @param opt The `option::id_num` for which to unregister handlers.
     *
     * @remark Forwards to `OptionHandlerRegistry::unregister_handlers`.
     */
    /**
     * @fn std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>> ProtocolFSM::process_byte(byte_t byte) noexcept
     *
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag (true to pass byte to application), and optional negotiation response.
     *
     * @remark Dispatches to state-specific handlers based on `current_state_`.
     * @remark Returns `error::protocol_violation` for invalid states.
     * @see RFC 854 for Telnet protocol, RFC 855 for option negotiation, :errors for error codes
     */
    /**
     * @fn bool ProtocolFSM::is_enabled(const option& opt)
     *
     * @param opt The `option::id_num` to check.
     * @return True if `opt` is enabled either locally or remotely, false otherwise.
     *
     * @remark Queries `OptionStatusDB` for the option’s status.
     */
    /**
     * @overload bool ProtocolFSM::is_enabled(option::id_num opt, NegotiationDirection dir)
     *
     * @param opt The `option::id_num` to check.
     * @param dir The `NegotiationDirection` to check.
     * @return True if `opt` is enabled in the designated direction, false otherwise.
     *
     * @remark Queries `OptionStatusDB` for the option’s status.
     */
    /**
     * @fn TelnetCommand make_negotiation_command(NegotiationDirection direction, bool enable) noexcept
     *
     * @param direction Negotiate local (DO/DONT) or remote (WILL/WONT) option?
     * @param enable Negotiate to enable?
     * @return The `TelnetCommand` representing the desired negotiation.
     */
    /**
     * @fn std::tuple<std::error_code, std::optional<NegotiationResponse>> ProtocolFSM::request_option(option::id_num opt, NegotiationDirection direction)
     *
     * @param opt The Telnet option (`option::id_num`) to request.
     * @param direction The negotiation direction (`LOCAL` or `REMOTE`).
     * @return std::tuple<std::error_code, std::optional<NegotiationResponse>> with error code and optional response (true for enable, false for disable).
     *
     * @remark Validates option registration and state per RFC 1143 Q Method, handling six states: YES, WANTYES/EMPTY, WANTYES/OPPOSITE, WANTNO/EMPTY, WANTNO/OPPOSITE, NO.
     * @remark Enqueues opposite request in WANTNO/EMPTY; treats redundant requests (YES, WANTYES/EMPTY, WANTNO/OPPOSITE) as idempotent successes, logging warnings.
     * @remark Does not invoke an enablement handler, as it initiates negotiation without enabling.
     * @throws None; errors returned via error_code (e.g., `error::option_not_available`, `error::negotiation_queue_error`, `error::protocol_violation`).
     * @see RFC 1143 for Q Method, :options for `option::id_num`, :errors for error codes, :types for `NegotiationDirection`, :socket for usage in async_request_option
     */
    /**
     * @fn std::tuple<std::error_code, std::optional<NegotiationResponse>, std::optional<awaitables::OptionDisablementAwaitable>> ProtocolFSM::disable_option(option::id_num opt, NegotiationDirection direction)
     *
     * @param opt The Telnet option (`option::id_num`) to disable.
     * @param direction The negotiation direction (`LOCAL` or `REMOTE`).
     * @return std::tuple<std::error_code, std::optional<NegotiationResponse>, std::optional<awaitables::OptionDisablementAwaitable>> with error code, optional response, and optional disablement awaitable.
     *
     * @remark Validates option registration and state per RFC 1143 Q Method, handling six states: NO, WANTNO/EMPTY, WANTNO/OPPOSITE, WANTYES/EMPTY, WANTYES/OPPOSITE, YES.
     * @remark Enqueues opposite request in WANTYES/EMPTY; treats redundant disablements (NO, WANTNO/EMPTY, WANTYES/OPPOSITE) as idempotent successes, logging warnings.
     * @remark Returns `OptionDisablementAwaitable` in YES state via `OptionHandlerRegistry::handle_disablement` (no-op if unregistered).
     * @throws None; errors returned via error_code (e.g., `error::option_not_available`, `error::negotiation_queue_error`, `error::protocol_violation`).
     * @see RFC 1143 for Q Method, :options for `option::id_num`, :errors for error codes, :types for `NegotiationDirection`, :awaitables for `OptionDisablementAwaitable`, :socket for usage in async_disable_option
     */
    /**
     * @fn void ProtocolFSM::change_state(ProtocolState next_state) noexcept
     *
     * @param next_state The new `ProtocolState` to transition to.
     *
     * @remark Updates the FSM’s `current_state_`.
     * @see RFC 854 for state transitions
     */
    /**
     * @fn std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>> ProtocolFSM::handle_state_normal(byte_t byte) noexcept
     *
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag (true for data bytes), and optional negotiation response.
     *
     * @remark Processes data bytes or IAC in the `Normal` state.
     * @see RFC 854 for Normal state behavior, :types for `TelnetCommand`
     */
    /**
     * @fn std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>> ProtocolFSM::handle_state_has_cr(byte_t byte) noexcept
     *
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag (true for data bytes), and optional negotiation response.
     *
     * @remark Processes data bytes or IAC in the `HasCR` state.
     * @see RFC 854 for CR NUL or CR LF state behavior, :types for `TelnetCommand`
     */
    /**
     * @fn std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>> ProtocolFSM::handle_state_iac(byte_t byte) noexcept
     *
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag (true for escaped IAC), and optional response (`ProcessingReturnVariant`)
     *
     * @remark Processes commands like `WILL`, `DO`, `SB`, etc., after IAC.
     * @see RFC 854 for IAC command processing, :types for `TelnetCommand`, :errors for error codes
     */
    /**
     * @fn std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>> ProtocolFSM::handle_state_option_negotiation(byte_t byte) noexcept
     *
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag (false), and optional negotiation response (`std::tuple<TelnetCommand, option::id_num>`).
     *
     * @remark Processes option ID after `WILL`, `WONT`, `DO`, or `DONT`.
     * @see RFC 855 for option negotiation, :options for `option::id_num`, :errors for error codes
     */
    /**
     * @fn std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>> ProtocolFSM::handle_state_subnegotiation_option(byte_t byte) noexcept
     *
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag (false), and optional negotiation response.
     *
     * @note Logs `error::invalid_subnegotiation` for unenabled or unregistered options.
     * @see RFC 855 for subnegotiation, :options for `option::id_num`, :errors for error codes
     */
    /**
     * @fn std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>> ProtocolFSM::handle_state_subnegotiation(byte_t byte) noexcept
     *
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag (false), and optional negotiation response.
     *
     * @note Logs `error::protocol_violation` if no current option is set.
     * @note Checks for IAC before buffer overflow.
     * @see RFC 855 for subnegotiation, :options for `option::id_num`, :errors for error codes
     */
    /**
     * @fn std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>> ProtocolFSM::handle_state_subnegotiation_iac(byte_t byte) noexcept
     *
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag (false), and optional negotiation response (`SubnegotiationAwaitable`).
     *
     * @note Relies on `OptionHandlerRegistry::handle_subnegotiation` to log `error::user_handler_not_found` when no handler is registered for the option.
     * @note Logs `error::protocol_violation` if no current option is set.
     * @note Logs `error::invalid_command` for invalid commands (not `SE` or `IAC`).
     * @see RFC 855 for subnegotiation, :options for `option::id_num`, :errors for error codes
     * @todo Phase 6: Review `subnegotiation_overflow` handling for custom error handlers
     */
    /**
     * @fn ProtocolFSM::handle_status_subnegotiation(const option& opt, std::vector<byte_t> buffer)
     * @param opt The `STATUS` option being negotiated (expected to be `option::id_num::STATUS`)
     * @param buffer The subnegotiation input data (expected to contain `SEND` (1) or `IS` (0))
     * @return `SubnegotiationAwaitable` yielding `std::tuple<const option&, std::vector<byte_t>>` containing the `STATUS` option and the `IS` [list] payload for `SEND`, or user-defined result for `IS`
     * @remark For `SEND` (1), validates that `STATUS` is locally enabled; constructs a payload starting with `IS` (0), followed by pairs of [`WILL` (251), opt] for locally enabled options and [`DO` (252), opt] for remotely enabled options, with `IAC` (255) and `SE` (240) option codes escaped by doubling.
     * @remark For `IS` (0), validates that `STATUS` is remotely enabled and delegates to the user-provided subnegotiation handler via `OptionHandlerRegistry`.
     * @remark Logs `telnet::error::option_not_available` and returns an empty payload if `STATUS` is not enabled in the required direction.
     * @remark Logs `telnet::error::invalid_subnegotiation` for invalid subcommands.
     * @remark The returned `SubnegotiationAwaitable` is processed by `InputProcessor` to pass the payload to `socket::async_write_subnegotiation`, which adds `IAC` `SB` `STATUS` ... `IAC` `SE` framing.
     * @see RFC 859, `:internal` for `OptionStatusDB`, `:options` for `option`, `:awaitables` for `SubnegotiationAwaitable`, `:socket` for `async_write_subnegotiation`
     */
} //namespace telnet