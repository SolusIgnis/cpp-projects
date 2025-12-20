/**
 * @file telnet-protocol_fsm.cppm
 * @version 0.2.0
 * @release_date September 19, 2025
 * @brief Interface for the Telnet protocol finite state machine.
 * @remark Implements basic Telnet protocol (RFC 854) including IAC command processing and stateful negotiation.
 * @example
 *   telnet::ProtocolFSM<> fsm; // Uses DefaultProtocolFSMConfig
 *   telnet::ProtocolFSM<>::ProtocolConfig::set_unknown_command_handler([](telnet::TelnetCommand cmd) { std::cout << "Custom: " << static_cast<std::uint8_t>(cmd) << "\n"; });
 *   telnet::ProtocolFSM<>::ProtocolConfig::register_option(telnet::option::id_num::NEGOTIATE_ABOUT_WINDOW_SIZE, "NAWS", telnet::option::always_accept, telnet::option::always_accept, std::nullopt, 4);
 *   telnet::ProtocolFSM<>::ProtocolConfig::set_error_logger([](const std::error_code& ec, std::uint8_t byte) { std::cout << "Error: " << ec.message() << " on byte " << static_cast<std::uint32_t>(byte) << "\n"; });
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @see telnet-protocol_fsm-impl.cpp for this partition's implementation unit
 * @see RFC 854 for Telnet protocol, RFC 855 for option negotiation, telnet:types for TelnetCommand, telnet:options for option and option::id_num, telnet:errors for error codes, telnet:socket for FSM usage.
 */
module; //Including Boost.Asio in the Global Module Fragment until importable header units are reliable.
#include <boost/asio.hpp>
namespace asio = boost::asio;

//Module partition interface unit
export module telnet:protocol_fsm;

import std;        // For std::function, std::optional, std::map, std::set, std::vector, std::mutex, std::shared_lock, std::lock_guard, std::once_flag, std::cout, std::cerr, std::hex, std::setw, std::setfill, std::dec
import std.compat; // For std::uint8_t

export import :types;   ///< @see telnet-types.cppm for TelnetCommand
export import :options; ///< @see telnet-options.cppm for option and option::id_num
export import :errors;  ///< @see telnet-errors.cppm for telnet::error codes

export namespace telnet {
    /**
     * @brief Default configuration class for ProtocolFSM, encapsulating options and handlers.
     * @remark Provides thread-safe access via static getters and setters with mutex_.
     * @see RFC 854 for Telnet protocol, RFC 855 for option negotiation, telnet:options for option and option::id_num, telnet-protocol_fsm-impl.cpp for implementation (Phase 3), telnet-errors-impl.cpp for error handling (Phase 3).
     */
    class DefaultProtocolFSMConfig {
    public:
        using TelnetCommandHandler = std::function<void(TelnetCommand)>;
        using UnknownOptionHandler = std::function<void(option::id_num)>;
        using ErrorLogger = std::function<void(const std::error_code&, std::uint8_t)>;

        ///Initialize Once
        static void initialize() {
            std::call_once(initialization_flag_, &init);
        }

        ///Register a Telnet command handler
        static void register_handler(TelnetCommand cmd, TelnetCommandHandler handler) {
            std::lock_guard<std::mutex> lock(mutex_);
            command_handlers_[cmd] = std::move(handler);
        }

        ///Remove a registered Telnet command handler
        static void unregister_handler(TelnetCommand cmd) {
            std::lock_guard<std::mutex> lock(mutex_);
            command_handlers_.erase(cmd);
        }

        ///Set the handler for unknown Telnet commands
        static void set_unknown_command_handler(TelnetCommandHandler handler) {
            std::lock_guard<std::mutex> lock(mutex_);
            unknown_command_handler_ = std::move(handler);
        }

        ///Set the handler for attempts to negotiate unknown Telnet options
        static void set_unknown_option_handler(UnknownOptionHandler handler) {
            std::lock_guard<std::mutex> lock(mutex_);
            unknown_option_handler_ = std::move(handler);
        }

        ///Set the error logging handler
        static void set_error_logger(ErrorLogger handler) {
            std::lock_guard<std::mutex> lock(mutex_);
            error_logger_ = std::move(handler);
        }

        ///Register a Telnet option [factory to forward constructor arguments and directly emplace into registry]
        template<typename... Args>
        static void register_option(Args&&... args) {
            std::lock_guard<std::mutex> lock(mutex_);
            option_registry_.emplace(std::forward<Args>(args)...);
        }

        ///Get a handler for a Telnet command by its enumerated byte or the unknown_command_handler if none is registered
        static std::optional<TelnetCommandHandler> get_command_handler(TelnetCommand cmd) {
            std::shared_lock<std::mutex> lock(mutex_);
            auto it = command_handlers_.find(cmd);
            if (it != command_handlers_.end()) {
                return std::make_optional(it->second);
            }
            return unknown_command_handler_ ? std::make_optional(unknown_command_handler_) : std::nullopt;
        }

        ///Get the unknown option handler
        static const UnknownOptionHandler& get_unknown_option_handler() {
            std::shared_lock<std::mutex> lock(mutex_);
            return unknown_option_handler_;
        }

        ///Log an error_code and byte with the registered error logger
        static void log_error(const std::error_code& ec, std::uint8_t byte) {
            std::shared_lock<std::mutex> lock(mutex_);
            if (error_logger_) {
                error_logger_(ec, byte);
            }
        }

        ///Find an option in the registry by its TelOpt ID byte
        static std::optional<option> get_option(option::id_num opt) {
            std::shared_lock<std::mutex> lock(mutex_);
            auto it = std::find_if(option_registry_.begin(), option_registry_.end(),
                                   [opt](const option& o) { return o == opt; });
            return it != option_registry_.end() ? std::make_optional(*it) : std::nullopt;
        }

    private:
        ///Perform initialization @see initialize() which calls this once
        static void init() {
            std::lock_guard<std::mutex> lock(mutex_);
            option_registry_.emplace(option::make_option(option::id_num::BINARY, "Binary Transmission"));
            option_registry_.emplace(option::make_option(option::id_num::ECHO, "Echo",
                                                        [](option::id_num o) { return o == option::id_num::ECHO; },
                                                        option::always_reject));
            option_registry_.emplace(option::make_option(option::id_num::SUPPRESS_GO_AHEAD, "Suppress Go-Ahead",
                                                        option::always_accept, option::always_reject));
            option_registry_.emplace(option::make_option(option::id_num::NEGOTIATE_ABOUT_WINDOW_SIZE,
                                                        "Negotiate About Window Size",
                                                        option::always_accept, option::always_accept,
                                                        std::nullopt, 4));
            unknown_command_handler_ = [](TelnetCommand cmd) {
                std::cout << "Unknown command: " << static_cast<std::uint8_t>(cmd) << "\n";
            };
            unknown_option_handler_ = [](option::id_num opt) {
                std::cout << "Unknown option: " << static_cast<std::uint8_t>(opt) << "\n";
            };
            error_logger_ = [](const std::error_code& ec, std::uint8_t byte) {
                std::cerr << "Telnet FSM error: " << ec.message() << " on byte 0x"
                          << std::hex << std::setw(2) << std::setfill('0')
                          << static_cast<std::uint32_t>(byte) << std::dec << "\n";
            };
        } //init()

        static inline std::map<TelnetCommand, TelnetCommandHandler> command_handlers_;
        static inline TelnetCommandHandler unknown_command_handler_;
        static inline UnknownOptionHandler unknown_option_handler_;
        static inline ErrorLogger error_logger_;
        static inline std::set<option> option_registry_;
        static inline std::mutex mutex_; /// Mutex to protect shared static members
        static inline std::once_flag initialization_flag_; /// Ensures initialize() is idempotent; only invokes init() once
    }; //class DefaultProtocolFSMConfig

    /**
     * @brief Concept to constrain ProtocolConfig.
     * @remark Ensures ProtocolConfig provides necessary types and methods for ProtocolFSM, including initialization.
     * @see RFC 854 for Telnet protocol, RFC 855 for option negotiation, telnet:options for option and option::id_num, telnet-protocol_fsm-impl.cpp for implementation (Phase 3), telnet-errors-impl.cpp for error handling (Phase 3).
     */
    template<typename T>
    concept ProtocolFSMConfig = requires(T config, TelnetCommand cmd, option::id_num opt, std::error_code ec, std::uint8_t byte) {
        typename T::TelnetCommandHandler;
        typename T::UnknownOptionHandler;
        typename T::ErrorLogger;
        { T::initialize() } -> std::same_as<void>;
        { T::register_handler(cmd, std::declval<typename ProtocolFSM<T>::TelnetCommandHandler>()) } -> std::same_as<void>;
        { T::unregister_handler(cmd) } -> std::same_as<void>;
        { T::set_unknown_command_handler(std::declval<typename ProtocolFSM<T>::TelnetCommandHandler>()) } -> std::same_as<void>;
        { T::set_unknown_option_handler(std::declval<typename ProtocolFSM<T>::UnknownOptionHandler>()) } -> std::same_as<void>;
        { T::set_error_logger(std::declval<typename ProtocolFSM<T>::ErrorLogger>()) } -> std::same_as<void>;
        { T::get_command_handler(cmd) } -> std::convertible_to<std::optional<typename ProtocolFSM<T>::TelnetCommandHandler>>;
        { T::get_unknown_option_handler() } -> std::convertible_to<const typename ProtocolFSM<T>::UnknownOptionHandler&>;
        { T::log_error(ec, byte) } -> std::same_as<void>;
        { T::get_option(opt) } -> std::convertible_to<std::optional<option>>;
        requires std::convertible_to<typename T::TelnetCommandHandler, typename ProtocolFSM<T>::TelnetCommandHandler>;
        requires std::convertible_to<typename T::UnknownOptionHandler, typename ProtocolFSM<T>::UnknownOptionHandler>;
        requires std::convertible_to<typename T::ErrorLogger, typename ProtocolFSM<T>::ErrorLogger>;
    }; //concept ProtocolFSMConfig

    /**
     * @brief Finite State Machine for Telnet protocol processing.
     * @tparam ConfigT Configuration class defining options and handlers (defaults to DefaultProtocolFSMConfig).
     * @remark Initialized to `ProtocolState::Normal` per RFC 854. ProtocolConfig members are accessed statically.
     * @remark Thread safety: ProtocolConfig setters use mutex_. Getters and log_error use std::shared_lock.
     * @see RFC 854 for Telnet protocol, RFC 855 for option negotiation, telnet:options for option and option::id_num, telnet:types for TelnetCommand, telnet:errors for error codes, telnet:socket for FSM usage, telnet-protocol_fsm-impl.cpp for implementation (Phase 3), telnet-errors-impl.cpp for error handling (Phase 3).
     */
    template<typename ConfigT = DefaultProtocolFSMConfig>
    class ProtocolFSM {
    public:
        using ProtocolConfig = ConfigT; // Public nested type alias

        /// States for the Telnet protocol FSM
        enum class ProtocolState { Normal, IAC, OptionNegotiation, SubnegotiationOption, Subnegotiation, SubnegotiationIAC };

        using TelnetCommandHandler = std::function<void(TelnetCommand)>;
        using UnknownOptionHandler = std::function<void(option::id_num)>;
        using ErrorLogger = std::function<void(const std::error_code&, std::uint8_t)>;

        /**
         * @brief Constructs the FSM, initializing ProtocolConfig once.
         */
        ProtocolFSM() {
            ConfigT::initialize();
        }

        /**
         * @brief Processes a single byte of Telnet input.
         * @param byte The byte to process.
         * @return Tuple of error code, forward flag, and optional negotiation response.
         * @remark Returns {operation_aborted, false, std::nullopt} for invalid states.
         * @see RFC 854 for Telnet protocol, RFC 855 for option negotiation, telnet:errors for error codes.
         */
        std::tuple<std::error_code, bool, std::optional<std::tuple<TelnetCommand, option::id_num>>>
        process_byte(std::uint8_t byte) noexcept;

        ///Check if an option is locally enabled
        bool is_locally_enabled(option::id_num opt) const noexcept {
            auto it = local_enabled_.find(opt);
            return it != local_enabled_.end() ? it->second : false;
        }

        ///Check if an option is remotely enabled
        bool is_remotely_enabled(option::id_num opt) const noexcept {
            auto it = remote_enabled_.find(opt);
            return it != remote_enabled_.end() ? it->second : false;
        }

        ///Check if an option is enabled in either direction
        bool is_enabled(option::id_num opt) const noexcept {
            return is_locally_enabled(opt) || is_remotely_enabled(opt);
        }

    private:
        ///FSM state transitions; responsible for resetting FSM memory when state returns to Normal
        void change_state(ProtocolState next_state) noexcept;

        ///Process a byte from the Normal state
        std::tuple<std::error_code, bool, std::optional<std::tuple<TelnetCommand, option::id_num>>>
        handle_state_normal(std::uint8_t byte) noexcept;

        ///Process the byte following IAC (a command byte or escaped 0xFF)
        std::tuple<std::error_code, bool, std::optional<std::tuple<TelnetCommand, option::id_num>>>
        handle_state_iac(std::uint8_t byte) noexcept;

        ///Process the byte following an option negotiation command (the option byte)
        std::tuple<std::error_code, bool, std::optional<std::tuple<TelnetCommand, option::id_num>>>
        handle_state_option_negotiation(std::uint8_t byte) noexcept;

        ///Process the byte following IAC SB (the subnegotiation option byte)
        std::tuple<std::error_code, bool, std::optional<std::tuple<TelnetCommand, option::id_num>>>
        handle_state_subnegotiation_option(std::uint8_t byte) noexcept;

        ///Process a byte from the normal Subnegotiation state
        std::tuple<std::error_code, bool, std::optional<std::tuple<TelnetCommand, option::id_num>>>
        handle_state_subnegotiation(std::uint8_t byte) noexcept;

        ///Process the byte following IAC during subnegotiation
        std::tuple<std::error_code, bool, std::optional<std::tuple<TelnetCommand, option::id_num>>>
        handle_state_subnegotiation_iac(std::uint8_t byte) noexcept;

        std::map<option::id_num, bool> local_enabled_;
        std::map<option::id_num, bool> remote_enabled_;
        ProtocolState current_state_ = ProtocolState::Normal;
        std::optional<TelnetCommand> current_command_;
        std::optional<option> current_option_;
        std::vector<std::uint8_t> subnegotiation_buffer_;
    }; //class ProtocolFSM
} //namespace telnet