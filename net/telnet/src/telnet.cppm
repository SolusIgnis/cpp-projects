/** 
 * @file telnet.cppm
 * @version 0.1.0
 * @release_date September 14, 2025
 * @brief Primary module interface for Telnet protocol operations.
 * @note Static members (`command_handlers_`, `unknown_command_handler_`, `option_registry_`) are initialized per-template-specialization and protected by `fsm_mutex_` for write operations. Reads during `process_byte` are assumed immutable post-initialization. Initialization is performed once using `std::call_once`.
 * @note Implements basic Telnet protocol (RFC 854) including IAC command processing and stateful negotiation, deferring option-specific implementations to phase two.
 * @example
 *   telnet::ProtocolFSM fsm;
 *   fsm.register_option(telnet::TelnetOption::ECHO, "Echo", [](telnet::TelnetOption o) { return o == telnet::TelnetOption::ECHO; }, telnet::Option::always_reject);
 *   fsm.set_unknown_command_handler([](telnet::TelnetCommand) { std::cout << "Unknown command: " << static_cast<uint8_t>(cmd) << "\n"; });
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 */

export module telnet; //@todo Phase2: refactor module files to separate complex function definitions from interfaces.

import std; // For std::string, std::shared_ptr, std::mutex, std::lock_guard, std::map, std::set, std::vector, std::call_once, std::once_flag
import std.compat; // For compatibility types
import :telnet_socket; // Import the Socket class from the partition

// Use #include for Boost.Asio (no official module support as of September 4, 2025)
#include <boost/asio.hpp>

// Namespace alias for convenience (not exported)
namespace asio = boost::asio;

export namespace telnet {
    /// Enumeration of Telnet commands as defined in RFC 854.
    enum class TelnetCommand : uint8_t {
        SE   = 0xF0, ///< Subnegotiation End
        NOP  = 0xF1, ///< No Operation
        DM   = 0xF2, ///< Data Mark
        BRK  = 0xF3, ///< Break
        IP   = 0xF4, ///< Interrupt Process
        AO   = 0xF5, ///< Abort Output
        AYT  = 0xF6, ///< Are You There
        EC   = 0xF7, ///< Erase Character
        EL   = 0xF8, ///< Erase Line
        GA   = 0xF9, ///< Go Ahead
        SB   = 0xFA, ///< Subnegotiation Begin
        WILL = 0xFB, ///< Sender wants to enable option
        WONT = 0xFC, ///< Sender wants to disable option
        DO   = 0xFD, ///< Sender requests receiver to enable option
        DONT = 0xFE, ///< Sender requests receiver to disable option
        IAC  = 0xFF  ///< Interpret As Command
    };

    /// Enumeration of Telnet options as defined in the IANA Telnet Option Registry and MUD-specific extensions.
    enum class TelnetOption : uint8_t {
        BINARY = 0x00,                    ///< Binary Transmission (RFC 856)
        ECHO = 0x01,                      ///< Echo (RFC 857)
        RECONNECTION = 0x02,              ///< Reconnection (NIC 15391 of 1973)
        SUPPRESS_GO_AHEAD = 0x03,         ///< Suppress Go Ahead (RFC 858)
        APPROX_MESSAGE_SIZE_NEGOTIATION = 0x04, ///< Approx Message Size Negotiation (NIC 15393 of 1973)
        STATUS = 0x05,                    ///< Status (RFC 859)
        TIMING_MARK = 0x06,               ///< Timing Mark (RFC 860)
        REMOTE_CONTROLLED_TRANS_AND_ECHO = 0x07, ///< Remote Controlled Trans and Echo (RFC 726)
        OUTPUT_LINE_WIDTH = 0x08,         ///< Output Line Width (NIC 20196 of August 1978)
        OUTPUT_PAGE_SIZE = 0x09,          ///< Output Page Size (NIC 20197 of August 1978)
        OUTPUT_CARRIAGE_RETURN_DISPOSITION = 0x0A, ///< Output Carriage-Return Disposition (RFC 652)
        OUTPUT_HORIZONTAL_TAB_STOPS = 0x0B, ///< Output Horizontal Tab Stops (RFC 653)
        OUTPUT_HORIZONTAL_TAB_DISPOSITION = 0x0C, ///< Output Horizontal Tab Disposition (RFC 654)
        OUTPUT_FORMFEED_DISPOSITION = 0x0D, ///< Output Formfeed Disposition (RFC 655)
        OUTPUT_VERTICAL_TABSTOPS = 0x0E,  ///< Output Vertical Tabstops (RFC 656)
        OUTPUT_VERTICAL_TAB_DISPOSITION = 0x0F, ///< Output Vertical Tab Disposition (RFC 657)
        OUTPUT_LINEFEED_DISPOSITION = 0x10, ///< Output Linefeed Disposition (RFC 658)
        EXTENDED_ASCII = 0x11,            ///< Extended ASCII (RFC 698)
        LOGOUT = 0x12,                    ///< Logout (RFC 727)
        BYTE_MACRO = 0x13,                ///< Byte Macro (RFC 735)
        DATA_ENTRY_TERMINAL = 0x14,       ///< Data Entry Terminal (RFC 1043, RFC 732)
        SUPDUP = 0x15,                    ///< SUPDUP (RFC 736, RFC 734)
        SUPDUP_OUTPUT = 0x16,             ///< SUPDUP Output (RFC 749)
        SEND_LOCATION = 0x17,             ///< Send Location (RFC 779)
        TERMINAL_TYPE = 0x18,             ///< Terminal Type (RFC 1091)
        END_OF_RECORD = 0x19,             ///< End of Record (RFC 885)
        TACACS_USER_IDENTIFICATION = 0x1A, ///< TACACS User Identification (RFC 927)
        OUTPUT_MARKING = 0x1B,            ///< Output Marking (RFC 933)
        TERMINAL_LOCATION_NUMBER = 0x1C,  ///< Terminal Location Number (RFC 946)
        TELNET_3270_REGIME = 0x1D,        ///< Telnet 3270 Regime (RFC 1041)
        X_3_PAD = 0x1E,                   ///< X.3 PAD (RFC 1053)
        NEGOTIATE_ABOUT_WINDOW_SIZE = 0x1F, ///< Negotiate About Window Size (RFC 1073)
        TERMINAL_SPEED = 0x20,            ///< Terminal Speed (RFC 1079)
        REMOTE_FLOW_CONTROL = 0x21,       ///< Remote Flow Control (RFC 1372)
        LINEMODE = 0x22,                  ///< Linemode (RFC 1184)
        X_DISPLAY_LOCATION = 0x23,        ///< X Display Location (RFC 1096)
        ENVIRONMENT_OPTION = 0x24,        ///< Environment Option (RFC 1408)
        AUTHENTICATION_OPTION = 0x25,     ///< Authentication Option (RFC 2941)
        ENCRYPTION_OPTION = 0x26,         ///< Encryption Option (RFC 2946)
        NEW_ENVIRONMENT_OPTION = 0x27,    ///< New Environment Option (RFC 1572)
        TN3270E = 0x28,                   ///< TN3270E (RFC 2355)
        XAUTH = 0x29,                     ///< XAUTH (Rob Earhart)
        CHARSET = 0x2A,                   ///< CHARSET (RFC 2066)
        TELNET_REMOTE_SERIAL_PORT = 0x2B, ///< Telnet Remote Serial Port (Robert Barnes)
        COM_PORT_CONTROL_OPTION = 0x2C,   ///< Com Port Control Option (RFC 2217)
        TELNET_SUPPRESS_LOCAL_ECHO = 0x2D, ///< Telnet Suppress Local Echo (Wirt Atmar)
        TELNET_START_TLS = 0x2E,          ///< Telnet Start TLS (Michael Boe)
        KERMIT = 0x2F,                    ///< KERMIT (RFC 2840)
        SEND_URL = 0x30,                  ///< SEND-URL (David Croft)
        FORWARD_X = 0x31,                 ///< FORWARD_X (Jeffrey Altman)
        /* Options 50-137 (Bytes 0x32-0x89) Unassigned by IANA */
        TELOPT_PRAGMA_LOGON = 0x8A,       ///< TELOPT PRAGMA LOGON (Steve McGregory)
        TELOPT_SSPI_LOGON = 0x8B,         ///< TELOPT SSPI LOGON (Steve McGregory)
        TELOPT_PRAGMA_HEARTBEAT = 0x8C,   ///< TELOPT PRAGMA HEARTBEAT (Steve McGregory)
        /* Options 141-254 (Bytes 0x8D-0xFE) Unassigned by IANA */
        EXTENDED_OPTIONS_LIST = 0xFF,     ///< Extended-Options-List (RFC 861)
        MSDP = 0x45,                      ///< MUD Server Data Protocol
        MCCP1 = 0x55,                     ///< MUD Client Compression Protocol v.1
        MCCP2 = 0x56                      ///< MUD Client Compression Protocol v.2
    };

    /** 
     * @brief Concept defining the requirements for a socket type compatible with Telnet operations.
     * Only methods common to `boost::asio::ip::tcp::socket` and `boost::asio::ssl::stream<...>` are required directly;
     * other methods must be accessed via `lowest_layer()`.
     */
    template<typename SocketT>
    concept TelnetSocketConcept = requires(SocketT s, asio::mutable_buffer mb, asio::const_buffer cb, char delim) {
        { s.get_executor() } -> std::same_as<asio::any_io_executor>;
        { s.lowest_layer() } -> std::same_as<typename SocketT::lowest_layer_type&>;
        { s.lowest_layer().is_open() } -> std::same_as<bool>;
        { s.lowest_layer().close() };
        { s.async_read_some(mb, asio::use_awaitable) } -> std::same_as<asio::awaitable<std::tuple<std::error_code, std::size_t>>>;
        { s.async_write_some(cb, asio::use_awaitable) } -> std::same_as<asio::awaitable<std::tuple<std::error_code, std::size_t>>>;
    };

    /** 
     * @brief Class to encapsulate Telnet option data and negotiation logic.
     * Not thread-safe; static members are per-template-specialization. This class defines options without tracking enablement state,
     * which is managed per-session in `ProtocolFSM`.
     * @todo Phase3: revisit virtual methods (e.g., `OptionBase` with `handle_subnegotiation`) if custom option handlers require polymorphic behavior.
     * @todo Phase3: consider `std::vector<SubnegotiationHandler>` if distinct, independent actions cannot be combined into a single lambda.
     */
    class Option {
    public:
        /// Type alias for a predicate function determining option enablement.
        using EnablePredicate = std::function<bool(TelnetOption)>;
        /// Type alias for a handler function processing subnegotiation data.
        using SubnegotiationHandler = std::function<void(const TelnetOption, const std::vector<uint8_t>&)>;

        /** 
         * Constructs an Option with the given ID and optional parameters.
         * @param id The Telnet option ID.
         * @param name The option name (default empty).
         * @param local_pred Predicate for local enablement (default rejects).
         * @param remote_pred Predicate for remote enablement (default rejects).
         * @param subneg_handler Handler for subnegotiation data (default ignores).
         */
        explicit Option(TelnetOption id, std::string name = "",
                       EnablePredicate local_pred = Option::always_reject,
                       EnablePredicate remote_pred = Option::always_reject,
                       SubnegotiationHandler subneg_handler = Option::ignore_subnegotiation)
            : id_(id), name_(std::move(name)), local_predicate(local_pred), remote_predicate(remote_pred), subnegotiation_handler_(subneg_handler) {}

        /// Gets the option ID.
        TelnetOption get_id() const { return id_; }

        /// Gets the option name.
        const std::string& get_name() const { return name_; }

        /** 
         * Adds a handler callback for this option.
         * @param handler The callback function to add.
         */
        using Handler = std::function<void()>;
        void add_handler(Handler handler) { handlers_.push_back(std::move(handler)); }

        /// Gets the list of handler callbacks.
        const std::vector<Handler>& get_handlers() const { return handlers_; }

        /// Compares two Options based on ID.
        auto operator<=>(const Option& other) const = default;

        /// Predicate for local enablement.
        EnablePredicate local_predicate;
        /// Predicate for remote enablement.
        EnablePredicate remote_predicate;
        /// Handler for subnegotiation data.
        SubnegotiationHandler subnegotiation_handler_;

        /** 
         * Sets the subnegotiation handler for this option.
         * @param handler The new subnegotiation handler to set.
         */
        void set_subnegotiation_handler(SubnegotiationHandler handler) {
            subnegotiation_handler_ = std::move(handler);
        }

        /** 
         * Always accepts an option.
         * @param o The option to check.
         * @return True.
         */
        static bool always_accept(TelnetOption o) { return true; }

        /** 
         * Always rejects an option.
         * @param o The option to check.
         * @return False.
         */
        static bool always_reject(TelnetOption o) { return false; }

        /** 
         * Ignores subnegotiation data (default handler).
         * @param o The option ID.
         * @param buf The subnegotiation buffer.
         */
        static void ignore_subnegotiation(const TelnetOption o, const std::vector<uint8_t>& buf) {
            // Do nothing
        }

    private:
        TelnetOption id_;
        std::string name_;
        std::vector<Handler> handlers_;
    };

    /// Predicate struct for finding an Option by ID.
    struct OptionIdPredicate {
        /// The ID to match.
        TelnetOption id;
        /** 
         * Checks if an Option matches the ID.
         * @param opt The Option to check.
         * @return True if IDs match.
         */
        bool operator()(const Option& opt) const {
            return opt.get_id() == id;
        }
    };

    /** 
     * @brief Finite State Machine for Telnet protocol processing.
     * Initialized to `ProtocolState::Normal` per RFC 854. Static members are protected by `fsm_mutex_` during write operations;
     * reads during `process_byte` are assumed immutable post-initialization. Initialization is performed once using `std::call_once`.
     * @note Thread safety: Write operations on static members are protected by `fsm_mutex_`. Reads during `process_byte` assume immutability post-initialization; external synchronization is required for dynamic modifications.
     * @todo Phase2: template ProtocolFSM on a configuration class to register options and handlers.
     * @todo Phase2: in phase two, add std::shared_lock around static member reads in `process_byte` to ensure thread safety during concurrent modifications.
     * @example
     *   telnet::ProtocolFSM fsm;
     *   fsm.register_option(telnet::TelnetOption::ECHO, "Echo", [](telnet::TelnetOption o) { return o == telnet::TelnetOption::ECHO; }, telnet::Option::always_reject);
     *   fsm.set_unknown_command_handler([](telnet::TelnetCommand cmd) { std::cout << "Unknown command: " << static_cast<uint8_t>(cmd) << "\n"; });
     */
    class ProtocolFSM {
    public:
        /// States of the Telnet protocol FSM.
        enum class ProtocolState {
            Normal,            ///< Normal data transmission state
            IAC,               ///< Received IAC, awaiting command
            OptionNegotiation, ///< Processing option negotiation
            Subnegotiation,    ///< Collecting subnegotiation data
            SubnegotiationIAC  ///< Received IAC during subnegotiation
        };

        /// Type alias for command handler functions.
        using TelnetCommandHandler = std::function<void(TelnetCommand)>;
        /// Type alias for unknown option handler function.
        using UnknownOptionHandler = std::function<void(TelnetOption)>;

        /** 
         * Registers a handler for a specific Telnet command.
         * @param cmd The Telnet command to handle.
         * @param handler The function to execute when the command is received.
         * @note Thread-safe with `fsm_mutex_`.
         */
        static void register_handler(TelnetCommand cmd, TelnetCommandHandler handler) {
            std::lock_guard<std::mutex> lock(fsm_mutex_);
            command_handlers_[cmd] = std::move(handler);
        }

        /** 
         * Unregisters a handler for a specific Telnet command.
         * @param cmd The Telnet command to unregister.
         * @note Thread-safe with `fsm_mutex_`.
         */
        static void unregister_handler(TelnetCommand cmd) {
            std::lock_guard<std::mutex> lock(fsm_mutex_);
            command_handlers_.erase(cmd);
        }

        /** 
         * Sets the handler for unknown commands.
         * @param handler The function to execute for unhandled commands.
         * @note Thread-safe with `fsm_mutex_`. Defaults to logging the command byte.
         */
        static void set_unknown_command_handler(TelnetCommandHandler handler) {
            std::lock_guard<std::mutex> lock(fsm_mutex_);
            unknown_command_handler_ = std::move(handler);
        }

        /** 
         * Sets the handler for unknown options.
         * @param handler The function to execute for unrecognized options.
         * @note Thread-safe with `fsm_mutex_`. Defaults to logging the option byte.
         */
        static void set_unknown_option_handler(UnknownOptionHandler handler) {
            std::lock_guard<std::mutex> lock(fsm_mutex_);
            unknown_option_handler_ = std::move(handler);
        }

        /** 
         * Registers a new Telnet option with the specified parameters.
         * @tparam Args Variadic template parameter pack for Option constructor arguments.
         * @param args Arguments forwarded to the Option constructor.
         * @note Thread-safe with `fsm_mutex_`. Limited to 75 options before reconsideration of `std::set` vs. `std::unordered_map`.
         */
        template<typename... Args>
        static void register_option(Args&&... args) {
            std::lock_guard<std::mutex> lock(fsm_mutex_);
            option_registry_.emplace(std::forward<Args>(args)...);
        }

        /** 
         * @brief Sets a callback for logging errors during byte processing (e.g., invalid states).
         * @param handler The function to call with error details (default: no-op).
         * @note Currently unimplemented; reserved for future enhancement.
         * @todo Phase2: implement error logging with detailed context (e.g., invalid state, byte value).
         */
        static void set_error_logger(std::function<void(const std::error_code&, uint8_t)> handler) {
            // No-op for 0.1.0
        }

        /** 
         * Constructs the FSM with default options.
         * @note Initializes `current_state_` to `ProtocolState::Normal` per RFC 854, pre-reserves `subnegotiation_buffer_`, and triggers `initialize()` once using `std::call_once`.
         */
        ProtocolFSM() : current_state_(ProtocolState::Normal), subnegotiation_buffer_(MAX_SUBNEGOTIATION_SIZE / 8) {
            std::call_once(initialization_flag_, &ProtocolFSM::initialize);
        }

        /** 
         * @brief Processes a single byte of Telnet input.
         * @param byte The byte to process.
         * @return A tuple containing the error code (e.g., operation_aborted for invalid states), a boolean indicating if the byte should be forwarded, and an optional tuple of (TelnetCommand, TelnetOption) for negotiation responses.
         * @note Returns {operation_aborted, false, std::nullopt} and resets to Normal state if an invalid state is encountered. Subnegotiation data is limited to 1024 bytes, with excess silently truncated.
         * @todo Phase2: make the subnegotiation buffer size configurable or add a warning callback when the limit is exceeded.
         * @todo Phase2: consider refactoring with helper methods.
         */
        std::tuple<std::error_code, bool, std::optional<std::tuple<TelnetCommand, TelnetOption>>> process_byte(uint8_t byte) noexcept {
            switch (current_state_) {
                case ProtocolState::Normal:
                    if (byte == static_cast<uint8_t>(TelnetCommand::IAC)) {
                        change_state(ProtocolState::IAC);
                        return {std::error_code(), false, std::nullopt};
                    }
                    return {std::error_code(), true, std::nullopt};
                case ProtocolState::IAC:
                    if (byte == static_cast<uint8_t>(TelnetCommand::IAC)) {
                        change_state(ProtocolState::Normal);
                        return {std::error_code(), true, std::nullopt};
                    }
                    current_command_ = static_cast<TelnetCommand>(byte);
                    if (current_command_) {
                        switch (*current_command_) {
                            case TelnetCommand::WILL: [[fallthrough]];
                            case TelnetCommand::WONT: [[fallthrough]];
                            case TelnetCommand::DO: [[fallthrough]];
                            case TelnetCommand::DONT:
                                change_state(ProtocolState::OptionNegotiation);
                                break;
                            case TelnetCommand::SB:
                                change_state(ProtocolState::Subnegotiation);
                                break;
                            case TelnetCommand::SE:
                                // Ignore SE outside subnegotiation per RFC 854 (malformed input); reset to Normal as following byte is not a command
                                change_state(ProtocolState::Normal);
                                break;
                            default:
                                auto it = command_handlers_.find(*current_command_);
                                if (it != command_handlers_.end()) {
                                    it->second(*current_command_);
                                } else if (unknown_command_handler_) {
                                    unknown_command_handler_(*current_command_);
                                }
                                change_state(ProtocolState::Normal);
                                break;
                        }
                    }
                    return {std::error_code(), false, std::nullopt};
                case ProtocolState::OptionNegotiation:
                    current_option_ = static_cast<TelnetOption>(byte);
                    if (current_command_ && current_option_) {
                        bool respond = false;
                        bool request_to_enable = (*current_command_ == TelnetCommand::WILL || *current_command_ == TelnetCommand::DO);
                        bool remote_direction = (*current_command_ == TelnetCommand::WILL || *current_command_ == TelnetCommand::WONT);
                        bool enable = false;

                        auto it = std::find_if(option_registry_.begin(), option_registry_.end(), OptionIdPredicate{*current_option_});
                        if (it != option_registry_.end()) {
                            bool currently_enabled = (remote_direction ? remote_enabled_[*current_option_] : local_enabled_[*current_option_]);
                            respond = (request_to_enable != currently_enabled);
                            if (respond) {
                                enable = request_to_enable && (remote_direction ? it->local_predicate(*current_option_) : it->remote_predicate(*current_option_));
                                if (remote_direction) {
                                    remote_enabled_[*current_option_] = enable;
                                } else {
                                    local_enabled_[*current_option_] = enable;
                                }
                                for (const auto& handler : it->get_handlers()) {
                                    handler();
                                }
                            }
                        } else if (unknown_option_handler_) {
                            unknown_option_handler_(*current_option_);
                            respond = request_to_enable;
                            enable = false;
                        }

                        if (respond) {
                            TelnetCommand cmd = remote_direction
                                ? (enable ? TelnetCommand::DO : TelnetCommand::DONT)
                                : (enable ? TelnetCommand::WILL : TelnetCommand::WONT);
                            return {std::error_code(), false, std::make_optional(std::make_tuple(cmd, *current_option_))};
                        }
                    }
                    change_state(ProtocolState::Normal);
                    return {std::error_code(), false, std::nullopt};
                case ProtocolState::Subnegotiation:
                    if (subnegotiation_buffer_.size() >= MAX_SUBNEGOTIATION_SIZE) {
                        change_state(ProtocolState::Normal);
                        return {std::error_code(), false, std::nullopt};
                    }
                    if (byte == static_cast<uint8_t>(TelnetCommand::IAC)) {
                        change_state(ProtocolState::SubnegotiationIAC);
                    } else {
                        subnegotiation_buffer_.push_back(byte);
                    }
                    return {std::error_code(), false, std::nullopt};
                case ProtocolState::SubnegotiationIAC:
                    if (byte == static_cast<uint8_t>(TelnetCommand::SE)) {
                        if (!subnegotiation_buffer_.empty()) {
                            TelnetOption opt_id = static_cast<TelnetOption>(subnegotiation_buffer_[0]);
                            auto it = std::find_if(option_registry_.begin(), option_registry_.end(), OptionIdPredicate{opt_id});
                            if (it != option_registry_.end() && it->subnegotiation_handler_) {
                                it->subnegotiation_handler_(opt_id, subnegotiation_buffer_);
                            }
                        }
                        change_state(ProtocolState::Normal);
                    } else {
                        if (subnegotiation_buffer_.size() >= MAX_SUBNEGOTIATION_SIZE) {
                            change_state(ProtocolState::Normal);
                            return {std::error_code(), false, std::nullopt};
                        }
                        subnegotiation_buffer_.push_back(static_cast<uint8_t>(TelnetCommand::IAC));
                        if (byte != static_cast<uint8_t>(TelnetCommand::IAC)) {
                            subnegotiation_buffer_.push_back(byte);
                        }
                        change_state(ProtocolState::Subnegotiation);
                    }
                    return {std::error_code(), false, std::nullopt};
                default:
                    change_state(ProtocolState::Normal);
                    return {asio::error::make_error_code(asio::error::operation_aborted), false, std::nullopt};
            }
        }

        /** 
         * Checks if the specified option is locally enabled.
         * @param opt The Telnet option to check.
         * @return True if the option is locally enabled, false otherwise (including unregistered options).
         */
        bool is_locally_enabled(TelnetOption opt) const noexcept {
            auto it = local_enabled_.find(opt);
            return it != local_enabled_.end() ? it->second : false;
        }

        /** 
         * Checks if the specified option is remotely enabled.
         * @param opt The Telnet option to check.
         * @return True if the option is remotely enabled, false otherwise (including unregistered options).
         */
        bool is_remotely_enabled(TelnetOption opt) const noexcept {
            auto it = remote_enabled_.find(opt);
            return it != remote_enabled_.end() ? it->second : false;
        }

        /** 
         * Checks if the specified option is enabled (locally or remotely).
         * @param opt The Telnet option to check.
         * @return True if the option is enabled either locally or remotely, false otherwise (including unregistered options).
         */
        bool is_enabled(TelnetOption opt) const noexcept {
            return is_locally_enabled(opt) || is_remotely_enabled(opt);
        }

    private:
        /// Maximum size for subnegotiation buffer
        static constexpr size_t MAX_SUBNEGOTIATION_SIZE = 1024;

        inline static std::map<TelnetCommand, TelnetCommandHandler> command_handlers_;
        inline static TelnetCommandHandler unknown_command_handler_; // Reuses TelnetCommandHandler
        inline static UnknownOptionHandler unknown_option_handler_;
        inline static std::set<Option> option_registry_; // Registry of implemented options
        inline static std::mutex fsm_mutex_;            // Mutex for thread safety of static members
        inline static std::once_flag initialization_flag_; // Flag for one-time initialization

        /// Tracks local enablement state per connection.
        std::map<TelnetOption, bool> local_enabled_;
        /// Tracks remote enablement state per connection.
        std::map<TelnetOption, bool> remote_enabled_;

        ProtocolState current_state_; // Renamed from state_ for clarity
        std::optional<TelnetCommand> current_command_;
        std::optional<TelnetOption> current_option_;
        std::vector<uint8_t> subnegotiation_buffer_; // Pre-reserved for efficiency
        // @todo Phase3: revisit subnegotiation buffer strategy (size, type) for NAWS, MSDP, etc.

        /** 
         * Initializes static members with default options and handlers.
         * @note Called once via `std::call_once` to set up the FSM's static state. Relies on `register_option` and `set_*_handler` for thread-safe modifications.
         * @todo Phase3: implement default command handlers for NOP (do nothing) and AYT (respond IAC NOP; needs access to the socket)
         */
        static void initialize() {
            register_option(TelnetOption::BINARY, "Binary Transmission");
            register_option(TelnetOption::ECHO, "Echo", [](TelnetOption o) { return o == TelnetOption::ECHO; }, Option::always_reject);
            register_option(TelnetOption::SUPPRESS_GO_AHEAD, "Suppress Go-Ahead", Option::always_accept, Option::always_reject);
            register_option(TelnetOption::NEGOTIATE_ABOUT_WINDOW_SIZE, "Negotiate About Window Size", Option::always_accept, Option::always_accept);
            set_unknown_command_handler([](TelnetCommand cmd) {
                std::cout << "Unknown command: " << static_cast<uint8_t>(cmd) << "\n";
            });
            set_unknown_option_handler([](TelnetOption opt) {
                std::cout << "Unknown option: " << static_cast<uint8_t>(opt) << "\n";
            });
        }
    };
} // namespace telnet