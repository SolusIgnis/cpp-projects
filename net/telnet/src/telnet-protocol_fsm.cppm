/**
 * @file telnet-protocol_fsm.cppm
 * @version 0.4.0
 * @release_date October 03, 2025
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
 * @todo Future Development: Consider optional half-duplex support (RFC 854) if legacy peer requirements arise.
 * @todo Phase 5: Consider restructuring option negotiation results to call option enablement or disablement handlers if needed.
 * @todo Phase 5: Consider restructuring option negotiation results for internal implementation of some options.
 * @todo Phase 6: Consider moving concept `ProtocolFSMConfig` to a :concepts partition or a :protocol_config partition (with `DefaultProtocolFSMConfig`).
 */
module; //Including Boost.Asio in the Global Module Fragment until importable header units are reliable.
#include <boost/asio.hpp>
namespace asio = boost::asio;

//Module partition interface unit
export module telnet:protocol_fsm;

import std; // For std::function, std::optional, std::map, std::set, std::vector, std::shared_mutex, std::shared_lock, std::lock_guard, std::once_flag, std::cout, std::cerr, std::hex, std::setw, std::setfill, std::dec, std::format

export import :types;   ///< @see telnet-types.cppm for `byte_t`, `TelnetCommand`, and `NegotiationDirection`
export import :errors;  ///< @see telnet-errors.cppm for `telnet::error` codes
export import :options; ///< @see telnet-options.cppm for `option` and `option::id_num`

import :internal;       ///< @see telnet-internal.cppm for implementation classes

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
        typename T::TelnetCommandHandler;
        typename T::UnknownOptionHandler;
        typename T::ErrorLogger;
        { T::initialize() } -> std::same_as<void>;
        { T::set_unknown_command_handler(std::declval<typename T::TelnetCommandHandler>()) } -> std::same_as<void>;
        { T::get_unknown_command_handler() } -> std::same_as<std::optional<typename T::TelnetCommandHandler>>;
        { T::set_unknown_option_handler(std::declval<typename T::UnknownOptionHandler>()) } -> std::same_as<void>;
        { T::set_error_logger(std::declval<typename T::ErrorLogger>()) } -> std::same_as<void>;
        { T::initialize_command_handlers() } -> std::convertible_to<std::map<TelnetCommand, typename ProtocolFSM<T>::TelnetCommandHandler>>;
        { T::get_unknown_option_handler() } -> std::convertible_to<const typename ProtocolFSM<T>::UnknownOptionHandler&>;
        { T::log_error(ec, std::declval<std::string>()) } -> std::same_as<void>;
        { T::registered_options.get(opt) } -> std::convertible_to<std::optional<option>>;
        { T::registered_options.has(opt) } -> std::same_as<bool>;
        { T::registered_options.upsert(opt) } -> std::convertible_to<const option&>;
        { T::registered_options.upsert(full_opt, ec) } -> std::same_as<void>;
        { T::registered_options.upsert(full_opt) } -> std::convertible_to<const option&>;
        { T::get_ayt_response() } -> std::same_as<std::string_view>;
        { T::set_ayt_response(std::declval<std::string>()) } -> std::same_as<void>;
        requires std::convertible_to<typename T::TelnetCommandHandler, typename ProtocolFSM<T>::TelnetCommandHandler>;
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
         * @typedef TelnetCommandHandler
         * @brief Function type for handling `TelnetCommand` instances.
         *
         * @param cmd The `TelnetCommand` to process.
         * @return `asio::awaitable<void>` for asynchronous command handling.
         */
        using TelnetCommandHandler = std::function<asio::awaitable<void>(TelnetCommand)>;

        /**
         * @typedef SubnegotiationAwaitable
         * @brief Awaitable type for subnegotiation handlers.
         */
        using SubnegotiationAwaitable = asio::awaitable<void>;

        /**
         * @typedef SubnegotiationHandler
         * @brief Function type for processing subnegotiation data.
         *
         * @param id The `option::id_num` identifying the option.
         * @param data The `std::vector<byte_t>` containing subnegotiation data.
         * @return `SubnegotiationAwaitable` for asynchronous subnegotiation handling.
         */
        using SubnegotiationHandler = std::function<SubnegotiationAwaitable(option::id_num, std::vector<byte_t>)>;

        /**
         * @typedef UnknownOptionHandler
         * @brief Function type for handling unknown option negotiation attempts.
         *
         * @param id The `option::id_num` of the unknown option.
         */
        using UnknownOptionHandler = std::function<void(option::id_num)>;

        /**
         * @typedef ErrorLogger
         * @brief Function type for logging errors during FSM processing.
         *
         * @param ec The `std::error_code` describing the error.
         * @param msg The formatted error message.
         */
        using ErrorLogger = std::function<void(const std::error_code&, std::string)>;

        /// @brief Initializes the configuration once.
        static void initialize() {
            std::call_once(initialization_flag_, &init);
        }

        /// @brief Sets the handler for unknown `TelnetCommand` instances.
        static void set_unknown_command_handler(TelnetCommandHandler handler) {
            std::lock_guard<std::shared_mutex> lock(mutex_);
            unknown_command_handler_ = std::move(handler);
        }

        /// @brief Gets the handler for unknown `TelnetCommand` instances.
        static std::optional<TelnetCommandHandler> get_unknown_command_handler() {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            return unknown_command_handler_ ? unknown_command_handler_ : std::nullopt;
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
    
        /// @brief Initializes the command handler map.
        static std::map<TelnetCommand, TelnetCommandHandler> initialize_command_handlers() {
            std::map<TelnetCommand, TelnetCommandHandler> handlers;
            handlers[TelnetCommand::NOP] = [](TelnetCommand) { co_return; };
            return handlers;
        }
        
        static inline option_registry registered_options = initialize_option_registry();

    private:
        /// @brief Initializes the option registry with default options.
        static option_registry initialize_option_registry() {
            return {
                option{option::id_num::BINARY,                      "Binary Transmission"},
                option{option::id_num::ECHO,                        "Echo",
                    [](option::id_num o) { return o == option::id_num::ECHO; },
                    option::always_reject},
                option{option::id_num::SUPPRESS_GO_AHEAD,           "Suppress Go-Ahead", 
                    option::always_accept,
                    option::always_reject},
                option{option::id_num::NEGOTIATE_ABOUT_WINDOW_SIZE, "Negotiate About Window Size",
                    option::always_accept,
                    option::always_accept, false, 4}
            };
        } //initialize_option_registry()

        /// @brief Performs initialization for `initialize`.
        static void init() {
            std::lock_guard<std::shared_mutex> lock(mutex_);
            unknown_command_handler_ = [](TelnetCommand cmd) {
                std::cout << "Unknown command: " << std::to_underlying(cmd) << "\n";
                co_return;
            };
            unknown_option_handler_ = [](option::id_num opt) {
                std::cout << "Unknown option: " << std::to_underlying(opt) << "\n";
                return;
            };
            error_logger_ = [](const std::error_code& ec, std::string msg) {
                std::cerr << "Telnet FSM error: " << ec.message() << " (" << msg << ")" << std::endl;
            };
        } //init()

        static inline TelnetCommandHandler unknown_command_handler_;
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
     * @fn void DefaultProtocolFSMConfig::set_unknown_command_handler(TelnetCommandHandler handler)
     *
     * @param handler The `TelnetCommandHandler` to set for unknown `TelnetCommand` instances.
     *
     * @remark Thread-safe via `std::lock_guard<std::shared_mutex>`.
     */
    /**
     * @fn std::optional<TelnetCommandHandler> DefaultProtocolFSMConfig::get_unknown_command_handler()
     *
     * @return `std::optional` containing the `TelnetCommandHandler` if set, or `std::nullopt`.
     *
     * @remark Thread-safe via `std::shared_lock<std::shared_mutex>`.
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
     * @fn std::map<TelnetCommand, TelnetCommandHandler> DefaultProtocolFSMConfig::initialize_command_handlers()
     *
     * @return `std::map` of `TelnetCommand` to `TelnetCommandHandler` with default handlers.
     *
     * @remark Initializes a handler for `TelnetCommand::NOP` as a no-op.
     */
    /**
     * @fn option_registry DefaultProtocolFSMConfig::initialize_option_registry()
     *
     * @return `option_registry` with default `option` instances.
     *
     * @remark Initializes options for `BINARY`, `ECHO`, `SUPPRESS_GO_AHEAD`, and `NEGOTIATE_ABOUT_WINDOW_SIZE`.
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
         * @brief States for the Telnet protocol FSM.
         * @see RFC 854 for state transition details
         */
        enum class ProtocolState { 
            Normal,               ///< Default state for data processing
            IAC,                  ///< Processing IAC command byte
            OptionNegotiation,    ///< Processing option ID after WILL/WONT/DO/DONT
            SubnegotiationOption, ///< Processing option ID after SB
            Subnegotiation,       ///< Processing subnegotiation data
            SubnegotiationIAC     ///< Processing IAC during subnegotiation
        };

        /**
         * @typedef TelnetCommandHandler
         * @brief Function type for handling `TelnetCommand` instances.
         *
         * @param cmd The `TelnetCommand` to process.
         * @return `asio::awaitable<void>` for asynchronous command handling.
         */
        using TelnetCommandHandler = std::function<asio::awaitable<void>(TelnetCommand)>;

        /**
         * @typedef SubnegotiationAwaitable
         * @brief Awaitable type for subnegotiation handlers.
         */
        using SubnegotiationAwaitable = asio::awaitable<void>;

        /**
         * @typedef SubnegotiationHandler
         * @brief Function type for processing subnegotiation data.
         *
         * @param id The `option::id_num` identifying the option.
         * @param data The `std::vector<byte_t>` containing subnegotiation data.
         * @return `SubnegotiationAwaitable` for asynchronous subnegotiation handling.
         */
        using SubnegotiationHandler = std::function<SubnegotiationAwaitable(option::id_num, std::vector<byte_t>)>;

        /**
         * @typedef UnknownOptionHandler
         * @brief Function type for handling unknown option negotiation attempts.
         *
         * @param id The `option::id_num` of the unknown option.
         */
        using UnknownOptionHandler = std::function<void(option::id_num)>;

        /**
         * @typedef ErrorLogger
         * @brief Function type for logging errors during FSM processing.
         *
         * @param ec The `std::error_code` describing the error.
         * @param msg The formatted error message.
         */
        using ErrorLogger = std::function<void(const std::error_code&, std::string)>;

        /**
         * @typedef ProcessingReturnVariant
         * @brief Variant type for return values from byte processing.
         *
         * @remark Holds either a negotiation response (`std::tuple<TelnetCommand, option::id_num>`), direct socket write (`std::string`), command handler tuple (`std::tuple<TelnetCommand, TelnetCommandHandler>`), or subnegotiation result (`SubnegotiationAwaitable`).
         */
        using ProcessingReturnVariant = std::variant<
            std::tuple<TelnetCommand, option::id_num>,       // Negotiation response
            std::string,                                     // Direct socket write
            std::tuple<TelnetCommand, TelnetCommandHandler>, // Handler with command context
            SubnegotiationAwaitable                          // Subnegotiation result
        >;

        /// @brief Constructs the FSM, initializing `ProtocolConfig` once.
        ProtocolFSM() {
            ConfigT::initialize();
        }

        /// @brief Processes a single byte of Telnet input.
        std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>>
        process_byte(byte_t byte) noexcept;

        /// @brief Registers a handler for a `TelnetCommand`.
        std::error_code register_command_handler(TelnetCommand cmd, TelnetCommandHandler handler) {
            return command_handler_registry_.add(cmd, handler);
        }
        
        /// @brief Removes a handler for a `TelnetCommand`.
        void unregister_command_handler(TelnetCommand cmd) {
            command_handler_registry_.remove(cmd);
        }
        
        /// @brief Checks if a handler is registered for a `TelnetCommand`.
        bool is_command_handler_registered(TelnetCommand cmd) const {
            return command_handler_registry_.has(cmd);
        }
        
        /// @brief Checks if an option is enabled locally or remotely.
        bool is_enabled(const option& opt) { return option_status_[opt].is_enabled(); }

    private:
        /// @brief Makes a negotiation response tuple
        std::tuple<TelnetCommand, option::id_num> make_negotiation(NegotiationDirection direction, bool enable, option::id_num opt) noexcept;
    
        /// @brief Changes the FSM state.
        void change_state(ProtocolState next_state) noexcept;

        /// @brief Handles bytes in the `Normal` state (data or IAC).
        std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>>
        handle_state_normal(byte_t byte) noexcept;

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

        CommandHandlerRegistry<ProtocolConfig, TelnetCommandHandler> command_handler_registry_;
        OptionHandlerRegistry<ProtocolConfig, SubnegotiationAwaitable, SubnegotiationHandler> option_handler_registry_;
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
     * @fn std::error_code ProtocolFSM::register_command_handler(TelnetCommand cmd, TelnetCommandHandler handler)
     *
     * @param cmd The `TelnetCommand` to register a handler for.
     * @param handler The `TelnetCommandHandler` to associate with `cmd`.
     * @return `std::error_code` indicating success or failure (e.g., `error::user_handler_forbidden` for reserved commands).
     *
     * @remark Delegates to `CommandHandlerRegistry::add`.
     * @remark Thread-safe via `CommandHandlerRegistry` synchronization.
     */
    /**
     * @fn void ProtocolFSM::unregister_command_handler(TelnetCommand cmd)
     *
     * @param cmd The `TelnetCommand` whose handler is to be removed.
     *
     * @remark Delegates to `CommandHandlerRegistry::remove`.
     * @remark Thread-safe via `CommandHandlerRegistry` synchronization.
     */
    /**
     * @fn bool ProtocolFSM::is_command_handler_registered(TelnetCommand cmd) const
     *
     * @param cmd The `TelnetCommand` to check.
     * @return True if a handler is registered for `cmd`, false otherwise.
     *
     * @remark Delegates to `CommandHandlerRegistry::has`.
     * @remark Thread-safe via `CommandHandlerRegistry` synchronization.
     */
    /**
     * @fn bool ProtocolFSM::is_enabled(const option& opt)
     *
     * @param opt The `option` to check.
     * @return True if `opt` is enabled locally or remotely, false otherwise.
     *
     * @remark Queries `OptionStatusDB` for the option’s status.
     */
    /**
     * @fn std::tuple<TelnetCommand, option::id_num> make_negotiation(NegotiationDirection direction, bool enable, option::id_num opt) noexcept
     *
     * @param direction Negotiate local (DO/DONT) or remote (WILL/WONT) option?
     * @param enable Negotiate to enable?
     * @param opt The `option::id_num` to negotiate.
     * @return The {`TelnetCommand`, `option::id_num`} tuple representing the desired negotiation.
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
     * @fn std::tuple<std::error_code, bool, std::optional<ProcessingReturnVariant>> ProtocolFSM::handle_state_iac(byte_t byte) noexcept
     *
     * @param byte The byte to process.
     * @return Tuple of error code, forward flag (true for escaped IAC), and optional response (`std::tuple<TelnetCommand, option::id_num>`, `std::string`, or `std::tuple<TelnetCommand, TelnetCommandHandler>`).
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
     * @note Logs `error::protocol_violation` if no current option is set.
     * @note Logs `error::invalid_command` for invalid commands (not `SE` or `IAC`).
     * @see RFC 855 for subnegotiation, :options for `option::id_num`, :errors for error codes
     * @todo Phase 6: Review `subnegotiation_overflow` handling for custom error handlers
     */
} //namespace telnet