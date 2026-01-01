/**
 * @file net.telnet-protocol_config.cppm
 * @version 0.5.7
 * @release_date October 30, 2025
 *
 * @brief Default configuration implementation for `ProtocolFSM`.
 * @remark Provides thread-safe, static configuration with option registry and handlers.
 * @example
 *   telnet::ProtocolFSM<> fsm;
 *   telnet::DefaultProtocolFSMConfig::set_error_logger([](const std::error_code& ec, std::string msg) {
 *       std::cerr << "Error: " << ec.message() << " - " << msg << std::endl;
 *   });
 *
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @see `:protocol_fsm` for `ProtocolFSM`, `:concepts` for `ProtocolFSMConfig`, `:options` for `option`
 */
 
//Module partition interface unit
export module net.telnet:protocol_config;

import std; //For std::shared_mutex, std::lock_guard, std::shared_lock, std::function, std::error_code, std::string, std::once_flag 

export import :types;    ///< @see "net.telnet-types.cppm" for `byte_t` and `telnet::command`
export import :errors;   ///< @see "net.telnet-errors.cppm" for `telnet::error` and `telnet::processing_signal` codes
export import :concepts; ///< @see "net.telnet-concepts.cppm" for `telnet::concepts::ProtocolFSMConfig`
export import :options;  ///< @see "net.telnet-options.cppm" for `option` and `option::id_num`

export namespace net::telnet {
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

        ///@brief Initializes the configuration once.
        static void initialize() {
            std::call_once(initialization_flag_, &init);
        }

        ///@brief Sets the handler for unknown option negotiation attempts.
        static void set_unknown_option_handler(UnknownOptionHandler handler) {
            std::lock_guard<std::shared_mutex> lock(mutex_);
            unknown_option_handler_ = std::move(handler);
        }

        ///@brief Sets the error logging handler.
        static void set_error_logger(ErrorLogger handler) {
            std::lock_guard<std::shared_mutex> lock(mutex_);
            error_logger_ = std::move(handler);
        }

        ///@brief Gets the handler for unknown option negotiation attempts.
        static const UnknownOptionHandler& get_unknown_option_handler() {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            return unknown_option_handler_;
        }
        
        ///@brief Logs an error with the registered error logger using a formatted string.
        template<typename ...Args>
        static void log_error(const std::error_code& ec, std::format_string<std::remove_cvref_t<Args>...> fmt, Args&&... args) {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            if (error_logger_) {
                error_logger_(ec, std::format(fmt, std::forward<Args>(args)...));
            }
        }
        
        ///@brief Gets the AYT response string.
        static std::string_view get_ayt_response() {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            return ayt_response_;
        }
    
        ///@brief Sets the AYT response string.
        static void set_ayt_response(std::string response) {
            std::lock_guard<std::shared_mutex> lock(mutex_);
            ayt_response_ = std::move(response);
        }

    private:
        ///@brief Initializes the option registry with default options.
        static option_registry initialize_option_registry() {
            return {
                option{option::id_num::binary,            "Binary Transmission", option::always_accept, option::always_accept},
                option{option::id_num::suppress_go_ahead, "Suppress Go-Ahead",   option::always_accept, option::always_accept},
                option{option::id_num::status,            "Status",              option::always_accept, option::always_reject, true}           
            };
        } //initialize_option_registry()

    public:
        static inline option_registry registered_options = initialize_option_registry();

    private:
        ///@brief Performs initialization for `initialize`.
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
        static inline std::string ayt_response_ = "Telnet system is active."; ///Default AYT response
        static inline std::shared_mutex mutex_; ///Mutex to protect shared static members
        static inline std::once_flag initialization_flag_; ///Ensures initialize() is idempotent; only invokes init() once
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
} //export namespace net::telnet
