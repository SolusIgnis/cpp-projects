/**
 * @file telnet-internal.cppm
 * @version 0.3.0
 * @release_date September 29, 2025
 *
 * @brief Internal partition defining implementation detail data structures for other partitions.
 * @remark Defines record and registry types used by `:protocol_fsm` for managing Telnet command and option handlers.
 *
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @remark Not intended for direct use by external code; serves as an implementation detail for other partitions.
 * @see RFC 855 for Telnet option negotiation, `:types` for `TelnetCommand`, `:options` for `option` and `option::id_num`, `:errors` for error codes, `:protocol_fsm` for usage
 * @todo Phase 5: Consider adding option enablement and disablement handlers to `OptionHandlerRecord`.
 */
module; //Including Boost.Asio in the Global Module Fragment until importable header units are reliable.
#include <boost/asio.hpp>
namespace asio = boost::asio;

//Module partition interface unit
export module telnet:internal;

import std;        // For std::function, std::optional, std::map, std::set, std::vector, std::shared_mutex, std::shared_lock, std::lock_guard, std::once_flag, std::cout, std::cerr, std::hex, std::setw, std::setfill, std::dec
import std.compat; // For std::uint8_t (needed for bit-field type specifier)

import :types;   ///< @see telnet-types.cppm for `byte_t` and `TelnetCommand`
import :errors;  ///< @see telnet-errors.cppm for `telnet::error` codes
import :options; ///< @see telnet-options.cppm for `option` and `option::id_num`

export namespace telnet {
    /**
     * @brief Registry for managing Telnet command handlers.
     * @tparam ProtocolConfig Configuration class providing initialization and fallback handlers (e.g., `ProtocolConfig::initialize_command_handlers`).
     * @tparam TelnetCommandHandler Handler type for processing `TelnetCommand` instances.
     * @remark Used by `:protocol_fsm` to store and query command handlers.
     * @remark Thread-safe for concurrent access via internal synchronization (assumed by `std::map` usage).
     * @see `:protocol_fsm` for handler usage, `:types` for `TelnetCommand`, `:errors` for error codes
     */
    template<typename ProtocolConfig, typename TelnetCommandHandler>
    class CommandHandlerRegistry {
    public:
        /// @brief Default Constructor @details Constructs a registry with handlers initialized by `ProtocolConfig`.
        CommandHandlerRegistry() : command_handlers_(ProtocolConfig::initialize_command_handlers()) {}
        
        /// @brief Registers a handler for a `TelnetCommand`.
        std::error_code add(TelnetCommand cmd, TelnetCommandHandler handler) {
            switch (cmd) {
                case TelnetCommand::IAC:  [[fallthrough]];
                case TelnetCommand::WILL: [[fallthrough]];
                case TelnetCommand::WONT: [[fallthrough]];
                case TelnetCommand::DO:   [[fallthrough]];
                case TelnetCommand::DONT: [[fallthrough]];
                case TelnetCommand::SB:   [[fallthrough]];
                case TelnetCommand::SE:   [[fallthrough]];
                case TelnetCommand::DM:   [[fallthrough]];
                case TelnetCommand::GA:   [[fallthrough]];
                case TelnetCommand::AYT:  
                    return std::make_error_code(error::user_handler_forbidden);
                default: 
                    command_handlers_[cmd] = std::move(handler);
                    return {};
            }
        } //add(TelnetCommand, TelnetCommandHandler)

        /// @brief Removes a registered handler for a `TelnetCommand`.
        void remove(TelnetCommand cmd) {
            command_handlers_.erase(cmd);
        } //remove(TelnetCommand)
        
        /// @brief Checks if a `TelnetCommand` has a registered handler.
        bool has(TelnetCommand cmd) const {
            return command_handlers_.contains(cmd);
        } //has(TelnetCommand)
        
        /// @brief Retrieves a handler for a `TelnetCommand` or the default unknown handler.
        std::optional<TelnetCommandHandler> get(TelnetCommand cmd) const {
            auto it = command_handlers_.find(cmd);
            if (it == command_handlers_.end()) {
                return ProtocolConfig::get_unknown_command_handler();
            } else {
                return std::make_optional(it->second);
            }
        } //get(TelnetCommand)

    private:
        std::map<TelnetCommand, TelnetCommandHandler> command_handlers_;
    }; //class CommandHandlerRegistry

    /**
     * @fn std::error_code CommandHandlerRegistry::add(TelnetCommand cmd, TelnetCommandHandler handler)
     *
     * @param cmd The `TelnetCommand` to register a handler for.
     * @param handler The `TelnetCommandHandler` to associate with `cmd`.
     * @return `std::error_code` indicating success or failure (e.g., `error::user_handler_forbidden` for reserved commands).
     *
     * @remark Fails with `error::user_handler_forbidden` for reserved commands (`IAC`, `WILL`, `WONT`, `DO`, `DONT`, `SB`, `SE`, `DM`, `GA`, `AYT`).
     */
    /**
     * @fn void CommandHandlerRegistry::remove(TelnetCommand cmd)
     *
     * @param cmd The `TelnetCommand` whose handler is to be removed.
     *
     * @remark Removes the handler if present; no-op if not found.
     * @remark Performs O(log n) search to remove.
     */
    /**
     * @fn bool CommandHandlerRegistry::has(TelnetCommand cmd) const
     *
     * @param cmd The `TelnetCommand` to check.
     * @return True if a handler is registered for `cmd`, false otherwise.
     *
     * @remark Performs O(log n) lookup.
     */
    /**
     * @fn std::optional<TelnetCommandHandler> CommandHandlerRegistry::get(TelnetCommand cmd) const
     *
     * @param cmd The `TelnetCommand` to retrieve a handler for.
     * @return `std::optional` containing the handler if found, or `ProtocolConfig::get_unknown_command_handler()` if not.
     *
     * @remark Performs O(log n) lookup.
     * @remark Falls back to `ProtocolConfig::get_unknown_command_handler` for unregistered commands.
     */



    /**
     * @brief Registry for managing Telnet option handlers.
     * @tparam ProtocolConfig Configuration class providing logging and error handling.
     * @tparam SubnegotiationAwaitable Awaitable type for subnegotiation handlers.
     * @tparam SubnegotiationHandler Handler type for processing subnegotiation data.
     * @remark Used by `:protocol_fsm` to manage subnegotiation handlers for Telnet options.
     * @see `:protocol_fsm` for handler usage, `:options` for `option::id_num`, `:errors` for error codes
     */
    template<typename ProtocolConfig, typename SubnegotiationAwaitable, typename SubnegotiationHandler>
    class OptionHandlerRegistry {
    private:    
        /**
         * @brief Record for handlers registered to a single Telnet option.
         * @details Stores an optional subnegotiation handler for processing option-specific data.
         * @see `:options` for `option::id_num`, `:protocol_fsm` for usage
         */
        struct OptionHandlerRecord {
            std::optional<SubnegotiationHandler> subnegotiation_handler;
        }; //struct OptionHandlerRecord

    public:
        /// @brief Handles subnegotiation for a Telnet option.
        SubnegotiationAwaitable handle_subnegotiation(option::id_num opt, std::vector<byte_t> data) {
            auto it = handlers_.find(opt);
            if ((it != handlers_.end()) && it->second.subnegotiation_handler) {
                auto& handler = *(it->second.subnegotiation_handler);
                return handler(opt, std::move(data));
            } else {
                return undefined_subnegotiation_handler(opt, std::move(data));
            }
        } //handle_subnegotiation(option::id_num, std::vector<byte_t>)

        /// @brief Accesses or creates an `OptionHandlerRecord` for a Telnet option.
        OptionHandlerRecord& operator[](option::id_num opt) { return handlers_[opt]; }

        /// @brief Retrieves an `OptionHandlerRecord` for a Telnet option.
        const OptionHandlerRecord& operator[](option::id_num opt) const { return handlers_[opt]; }

    private:
        /// @brief Default handler for undefined subnegotiation.
        static SubnegotiationAwaitable undefined_subnegotiation_handler(option::id_num opt, std::vector<byte_t>) {
            ProtocolConfig::log_error(std::make_error_code(error::user_handler_not_found), std::to_underlying(opt), TelnetCommand::SE, opt);
            co_return;
        } //undefined_subnegotiation_handler(option::id_num opt, std::vector<byte_t>)

        std::map<option::id_num, OptionHandlerRecord> handlers_;
    }; //class OptionHandlerRegistry

    /**
     * @fn SubnegotiationAwaitable OptionHandlerRegistry::handle_subnegotiation(option::id_num opt, std::vector<byte_t> data)
     *
     * @param opt The `option::id_num` of the Telnet option.
     * @param data The subnegotiation data to process.
     * @return `SubnegotiationAwaitable` representing the asynchronous handling result.
     *
     * @remark Invokes the registered subnegotiation handler if present; otherwise, calls `undefined_subnegotiation_handler`.
     */
    /**
     * @fn OptionHandlerRegistry::OptionHandlerRecord& OptionHandlerRegistry::operator[](option::id_num opt)
     *
     * @param opt The `option::id_num` of the Telnet option.
     * @return Reference to the `OptionHandlerRecord` for `opt`, creating it if not present.
     *
     * @remark Allows modification of the handler record for the specified option.
     * @deprecated @todo Phase6: Remove if no use cases found. 
     */
    /**
     * @fn const OptionHandlerRegistry::OptionHandlerRecord& OptionHandlerRegistry::operator[](option::id_num opt) const
     *
     * @param opt The `option::id_num` of the Telnet option.
     * @return Const reference to the `OptionHandlerRecord` for `opt`.
     *
     * @remark Provides read-only access to the handler record.
     * @deprecated @todo Phase 6: Remove if no use cases found.
     */
    /**
     * @fn static SubnegotiationAwaitable OptionHandlerRegistry::undefined_subnegotiation_handler(option::id_num opt, std::vector<byte_t>)
     *
     * @param opt The `option::id_num` of the Telnet option.
     * @param data The subnegotiation data (unused).
     * @return `SubnegotiationAwaitable` representing an empty asynchronous result.
     *
     * @remark Logs an `error::user_handler_not_found` error via `ProtocolConfig::log_error`.
     * @note Used as a fallback when no subnegotiation handler is registered.
     */

    /**
     * @brief Record for tracking the status of a single Telnet option.
     * @remark Optimized with bit-fields packed into a single byte for memory efficiency.
     * @see `:options` for `option::id_num`, `:protocol_fsm` for usage
     */
    class OptionStatusRecord {
    public:
        /// @brief Checks if the option is enabled locally.
        bool local_enabled() const noexcept { return local_enabled_; }

        /// @brief Checks if a local enablement request is pending.
        bool local_pending() const noexcept { return local_pending_; }

        /// @brief Checks if the option is enabled remotely.
        bool remote_enabled() const noexcept { return remote_enabled_; }

        /// @brief Checks if a remote enablement request is pending.
        bool remote_pending() const noexcept { return remote_pending_; }
        
        /// @brief Checks if the option is enabled locally or remotely.
        bool is_enabled() const noexcept { return (local_enabled() || remote_enabled()); }
        
        /// @brief Sets the local enablement state.
        void set_enabled_local(bool enable) noexcept { local_pending_ = false; local_enabled_ = enable; }

        /// @brief Sets the remote enablement state.
        void set_enabled_remote(bool enable) noexcept { remote_pending_ = false; remote_enabled_ = enable; }
        
        /// @brief Enables the option locally.
        void enable_local() noexcept { local_pending_ = false; local_enabled_ = true; }

        /// @brief Disables the option locally.
        void disable_local() noexcept { local_pending_ = false; local_enabled_ = false; }

        /// @brief Marks a local enablement request as pending.
        void pend_local() noexcept { local_pending_ = true; local_enabled_ = false; }

        /// @brief Enables the option remotely.
        void enable_remote() noexcept { remote_pending_ = false; remote_enabled_ = true; }

        /// @brief Disables the option remotely.
        void disable_remote() noexcept { remote_pending_ = false; remote_enabled_ = false; }

        /// @brief Marks a remote enablement request as pending.
        void pend_remote() noexcept { remote_pending_ = true; remote_enabled_ = false; }
        
        /// @brief Resets all status flags to false.
        void reset() noexcept { local_enabled_ = local_pending_ = remote_enabled_ = remote_pending_ = false; }

    private:
        //Pack these 4 fields into 1 byte.
        std::uint8_t local_enabled_  : 1 = false;
        std::uint8_t local_pending_  : 1 = false;
        std::uint8_t remote_enabled_ : 1 = false;
        std::uint8_t remote_pending_ : 1 = false;
    }; //class OptionStatusRecord

    /**
     * @fn bool OptionStatusRecord::local_enabled() const noexcept
     *
     * @return True if the option is enabled locally, false otherwise.
     *
     * @remark Accesses the `local_enabled_` bit-field.
     */
    /**
     * @fn bool OptionStatusRecord::local_pending() const noexcept
     *
     * @return True if a local enablement request is pending, false otherwise.
     *
     * @remark Accesses the `local_pending_` bit-field.
     */
    /**
     * @fn bool OptionStatusRecord::remote_enabled() const noexcept
     *
     * @return True if the option is enabled remotely, false otherwise.
     *
     * @remark Accesses the `remote_enabled_` bit-field.
     */
    /**
     * @fn bool OptionStatusRecord::remote_pending() const noexcept
     *
     * @return True if a remote enablement request is pending, false otherwise.
     *
     * @remark Accesses the `remote_pending_` bit-field.
     */
    /**
     * @fn bool OptionStatusRecord::is_enabled() const noexcept
     *
     * @return True if the option is enabled locally or remotely, false otherwise.
     *
     * @remark Combines results of `local_enabled` and `remote_enabled`.
     */
    /**
     * @fn void OptionStatusRecord::set_enabled_local(bool enable) noexcept
     *
     * @param enable The desired local enablement state.
     *
     * @remark Clears `local_pending_` and sets `local_enabled_` to `enable`.
     */
    /**
     * @fn void OptionStatusRecord::set_enabled_remote(bool enable) noexcept
     *
     * @param enable The desired remote enablement state.
     *
     * @remark Clears `remote_pending_` and sets `remote_enabled_` to `enable`.
     */


  
    /**
     * @brief Collection of `OptionStatusRecord` objects for tracking Telnet option statuses.
     * @remark Provides array-based access to option statuses by `option::id_num`.
     * @remark Used by `:protocol_fsm` to manage the state of Telnet options.
     * @see `:options` for `option::id_num`, `:protocol_fsm` for usage, `OptionStatusRecord` for status details
     */
    class OptionStatusDB {
    public:
        /// @brief Accesses or creates an `OptionStatusRecord` for a Telnet option.
        OptionStatusRecord& operator[](option::id_num opt) { return status_records_[std::to_underlying(opt)]; }

        /// @brief Retrieves an `OptionStatusRecord` for a Telnet option.
        const OptionStatusRecord& operator[](option::id_num opt) const { return status_records_[std::to_underlying(opt)]; }

    private:
        /// @brief The number of possible `option::id_num` values.
        static inline constexpr size_t MAX_OPTION_COUNT = (1 << std::numeric_limits<std::underlying_type_t<option::id_num>>::digits);

        //Byte array of Status Record bit-fields.
        std::array<OptionStatusRecord, MAX_OPTION_COUNT> status_records_;
    }; //class OptionStatusDB

    /**
     * @fn OptionStatusRecord& OptionStatusDB::operator[](option::id_num opt)
     *
     * @param opt The `option::id_num` of the Telnet option.
     * @return Reference to the `OptionStatusRecord` for `opt`.
     *
     * @remark Allows modification of the status record for the specified option.
     */
    /**
     * @fn const OptionStatusRecord& OptionStatusDB::operator[](option::id_num opt) const
     *
     * @param opt The `option::id_num` of the Telnet option.
     * @return Const reference to the `OptionStatusRecord` for `opt`.
     *
     * @remark Provides read-only access to the status record.
     */
} //namespace telnet