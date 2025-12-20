/**
 * @file telnet-internal.cppm
 * @version 0.4.0
 * @release_date October 3, 2025
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
     * @remark Thread-safe by std::shared_mutex.
     * @see `:protocol_fsm` for handler usage, `:types` for `TelnetCommand`, `:errors` for error codes
     */
    template<typename ProtocolConfig, typename TelnetCommandHandler>
    class CommandHandlerRegistry {
    public:
        /// @brief Default Constructor @details Constructs a registry with handlers initialized by `ProtocolConfig`.
        CommandHandlerRegistry() : command_handlers_(ProtocolConfig::initialize_command_handlers()) {}
        
        /// @brief Registers a handler for a `TelnetCommand`.
        std::error_code add(TelnetCommand cmd, TelnetCommandHandler handler) {
            std::lock_guard<std::shared_mutex> lock(mutex_);
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
            std::lock_guard<std::shared_mutex> lock(mutex_);
            command_handlers_.erase(cmd);
        } //remove(TelnetCommand)
        
        /// @brief Checks if a `TelnetCommand` has a registered handler.
        bool has(TelnetCommand cmd) const {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            return command_handlers_.contains(cmd);
        } //has(TelnetCommand)
        
        /// @brief Retrieves a handler for a `TelnetCommand` or the default unknown handler.
        std::optional<TelnetCommandHandler> get(TelnetCommand cmd) const {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            auto it = command_handlers_.find(cmd);
            if (it == command_handlers_.end()) {
                return ProtocolConfig::get_unknown_command_handler();
            } else {
                return std::make_optional(it->second);
            }
        } //get(TelnetCommand)

    private:
        std::map<TelnetCommand, TelnetCommandHandler> command_handlers_;
        mutable std::shared_mutex mutex_;
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
     * @todo Phase 5: Add a `shared_mutex` to `OptionHandlerRegistry` if registration is not confined to initialization time.
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
        SubnegotiationAwaitable handle_subnegotiation(const option& opt, std::vector<byte_t> data) {
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
        static SubnegotiationAwaitable undefined_subnegotiation_handler(const option& opt, std::vector<byte_t>) {
            ProtocolConfig::log_error(
                std::make_error_code(error::user_handler_not_found),
                "cmd: {}, option: {}",
                TelnetCommand::SE,
                opt
            );
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
     * @note Implements RFC 1143 Q Method with 4-state FSMs plus queue bits for local (us) and remote (him) sides.
     * @see `:types` for `NegotiationDirection`, `:options` for `option::id_num`, `:protocol_fsm` for usage
     */
    class OptionStatusRecord {
    public:
        enum class NegotiationState : std::uint8_t { NO = 0, YES = 1, WANTNO = 2, WANTYES = 3 };

        // Local state queries (us)
        /// @brief Checks if the option is enabled locally.
        bool local_enabled() const noexcept { return local_state_ == std::to_underlying(NegotiationState::YES); }
        /// @brief Checks if the option is fully disabled locally. @note NOT equivalent to !local_enabled()
        bool local_disabled() const noexcept { return local_state_ == std::to_underlying(NegotiationState::NO); }
        /// @brief Checks if a local enablement request is pending.
        bool local_pending_enable() const noexcept { return local_state_ == std::to_underlying(NegotiationState::WANTYES); }
        /// @brief Checks if a local disablement request is pending.
        bool local_pending_disable() const noexcept { return local_state_ == std::to_underlying(NegotiationState::WANTNO); }
        /// @brief Checks if local negotiation is pending (WANTNO or WANTYES).
        bool local_pending() const noexcept { return local_pending_enable() || local_pending_disable(); }

        // Remote state queries (him)
        /// @brief Checks if the option is enabled remotely.
        bool remote_enabled() const noexcept { return remote_state_ == std::to_underlying(NegotiationState::YES); }
        /// @brief Checks if the option is fully disabled remotely. @note NOT equivalent to !remote_enabled()
        bool remote_disabled() const noexcept { return remote_state_ == std::to_underlying(NegotiationState::NO); }
        /// @brief Checks if a remote enablement request is pending.
        bool remote_pending_enable() const noexcept { return remote_state_ == std::to_underlying(NegotiationState::WANTYES); }
        /// @brief Checks if a remote disablement request is pending.
        bool remote_pending_disable() const noexcept { return remote_state_ == std::to_underlying(NegotiationState::WANTNO); }
        /// @brief Checks if remote negotiation is pending (WANTNO or WANTYES).
        bool remote_pending() const noexcept { return remote_pending_enable() || remote_pending_disable(); }

        // Bidirectional state queries
        /// @brief Checks if the option is enabled in the designated direction.
        bool enabled(NegotiationDirection direction)  const noexcept { return (direction == NegotiationDirection::REMOTE) ? remote_enabled()  : local_enabled();  }
        /// @brief Checks if the option is disabled in the designated direction. @note NOT equivalent to !enabled(direction)
        bool disabled(NegotiationDirection direction) const noexcept { return (direction == NegotiationDirection::REMOTE) ? remote_disabled() : local_disabled(); }
        /// @brief Checks if an enablement request is pending in the designated direction.
        bool pending_enable(NegotiationDirection direction) const noexcept { return (direction == NegotiationDirection::REMOTE) ? remote_pending_enable() : local_pending_enable(); }
        /// @brief Checks if a disablement request is pending in the designated direction.
        bool pending_disable(NegotiationDirection direction) const noexcept { return (direction == NegotiationDirection::REMOTE) ? remote_pending_disable() : local_pending_disable(); }
        /// @brief Checks if a negotiation is pending (WANTNO or WANTYES) in the designated direction.
        bool pending(NegotiationDirection direction) const noexcept { return (direction == NegotiationDirection::REMOTE) ? remote_pending() : local_pending(); }

        // Combined state query
        /// @brief Checks if the option is enabled locally or remotely.
        bool is_enabled() const noexcept { return local_enabled() || remote_enabled(); }

        // Queue queries (usq, himq)
        /// @brief Checks if a local user request is queued (OPPOSITE state).
        bool local_queued() const noexcept { return local_queue_; }
        /// @brief Checks if a remote user request is queued (OPPOSITE state).
        bool remote_queued() const noexcept { return remote_queue_; }
        /// @brief Checks if a user request is queued (OPPOSITE state) in the designated direction.
        bool queued(NegotiationDirection direction) const noexcept { return (direction == NegotiationDirection::REMOTE) ? remote_queued() : local_queued(); }
        /// @brief Checks if any user request is queued (local or remote). [Optional]
        bool has_queued_request() const noexcept { return local_queued() || remote_queued(); }

        // Local state setters (us)
        /// @brief Enables the option locally.
        void enable_local() noexcept { local_state_ = std::to_underlying(NegotiationState::YES); }
        /// @brief Disables the option locally.
        void disable_local() noexcept { local_state_ = std::to_underlying(NegotiationState::NO); }
        /// @brief Marks a local enablement request as pending.
        void pend_enable_local() noexcept { local_state_ = std::to_underlying(NegotiationState::WANTYES); }
        /// @brief Marks a local disablement request as pending.
        void pend_disable_local() noexcept { local_state_ = std::to_underlying(NegotiationState::WANTNO); }

        // Remote state setters (him)
        /// @brief Enables the option remotely.
        void enable_remote() noexcept { remote_state_ = std::to_underlying(NegotiationState::YES); }
        /// @brief Disables the option remotely.
        void disable_remote() noexcept { remote_state_ = std::to_underlying(NegotiationState::NO); }
        /// @brief Marks a remote enablement request as pending.
        void pend_enable_remote() noexcept { remote_state_ = std::to_underlying(NegotiationState::WANTYES); }
        /// @brief Marks a remote disablement request as pending.
        void pend_disable_remote() noexcept { remote_state_ = std::to_underlying(NegotiationState::WANTNO); }

        // Bidirectional state setters
        /// @brief Enables the option in the designated direction.
        void enable(NegotiationDirection direction) noexcept { if (direction == NegotiationDirection::REMOTE) { enable_remote(); } else { enable_local(); } }
        /// @brief Disables the option in the designated direction.
        void disable(NegotiationDirection direction) noexcept { if (direction == NegotiationDirection::REMOTE) { disable_remote(); } else { disable_local(); } }
        /// @brief Marks an enablement request as pending in the designated direction.
        void pend_enable(NegotiationDirection direction) noexcept { if (direction == NegotiationDirection::REMOTE) { pend_enable_remote(); } else { pend_enable_local(); } }
        /// @brief Marks a disablement request as pending in the designated direction.
        void pend_disable(NegotiationDirection direction) noexcept { if (direction == NegotiationDirection::REMOTE) { pend_disable_remote(); } else { pend_disable_local(); } }

        // Queue setters (usq, himq)
        /// @brief Sets a local user request as queued (OPPOSITE state).
        std::error_code enqueue_local()  noexcept { if (local_enabled()  || local_disabled())  { return std::make_error_code(error::negotiation_queue_error); } else { local_queue_  = true; return {}; } }
        /// @brief Sets a remote user request as queued (OPPOSITE state).
        std::error_code enqueue_remote() noexcept { if (remote_enabled() || remote_disabled()) { return std::make_error_code(error::negotiation_queue_error); } else { remote_queue_ = true; return {}; } }
        /// @brief Sets a user request as queued (OPPOSITE state) in the designated direction.
        std::error_code enqueue(NegotiationDirection direction) noexcept { if (direction == NegotiationDirection::REMOTE) { return enqueue_remote(); } else { return enqueue_local(); } }
        /// @brief Clears a queued local user request.
        void dequeue_local() noexcept { local_queue_ = false; }
        /// @brief Clears a queued remote user request.
        void dequeue_remote() noexcept { remote_queue_ = false; }
        /// @brief Clears a queued user request in the designated direction.
        void dequeue(NegotiationDirection direction) noexcept { if (direction == NegotiationDirection::REMOTE) { dequeue_remote(); } else { dequeue_local(); } }
        /// @brief Clears all queued requests.
        void clear_queued_requests() noexcept { local_queue_ = remote_queue_ = false; }

        // Utility
        /// @brief Resets all state to initial values (NO, no queued requests).
        void reset() noexcept {
            local_state_ = std::to_underlying(NegotiationState::NO);
            remote_state_ = std::to_underlying(NegotiationState::NO);
            local_queue_ = remote_queue_ = false;
        }
        /// @brief Checks if either local or remote negotiation is pending. [Optional]
        bool is_negotiating() const noexcept { return local_pending() || remote_pending(); }
        /// @brief Validates state consistency (e.g., queue flags false when not pending). [Optional]
        bool is_valid() const noexcept {
            return (!local_queue_ || local_pending()) && (!remote_queue_ || remote_pending());
        }
        /// @brief Gets the raw local negotiation state (for debugging).
        // NegotiationState local_state() const noexcept { return static_cast<NegotiationState>(local_state_); }
        /// @brief Gets the raw remote negotiation state (for debugging).
        // NegotiationState remote_state() const noexcept { return static_cast<NegotiationState>(remote_state_); }

    private:
        // Pack these 4 fields into 1 byte (6 bits used, 2 unused).
        std::uint8_t local_state_  : 2 = std::to_underlying(NegotiationState::NO); // us
        std::uint8_t remote_state_ : 2 = std::to_underlying(NegotiationState::NO); // him
        std::uint8_t local_queue_  : 1 = false; // usq (EMPTY=false, OPPOSITE=true)
        std::uint8_t remote_queue_ : 1 = false; // himq (EMPTY=false, OPPOSITE=true)
    };

    /**
     * @fn bool OptionStatusRecord::local_enabled() const noexcept
     * @return True if the option is enabled locally (state is YES), false otherwise.
     * @remark Derived from `local_state_` (us in RFC 1143).
     */
    /**
     * @fn bool OptionStatusRecord::local_disabled() const noexcept
     * @return True if the option is fully disabled locally (state is NO), false otherwise.
     * @remark Derived from `local_state_` (us in RFC 1143). Not equivalent to !local_enabled().
     */
    /**
     * @fn bool OptionStatusRecord::local_pending_enable() const noexcept
     * @return True if a local enablement request is pending (state is WANTYES), false otherwise.
     * @remark Derived from `local_state_` (us in RFC 1143).
     */
    /**
     * @fn bool OptionStatusRecord::local_pending_disable() const noexcept
     * @return True if a local disablement request is pending (state is WANTNO), false otherwise.
     * @remark Derived from `local_state_` (us in RFC 1143).
     */
    /**
     * @fn bool OptionStatusRecord::local_pending() const noexcept
     * @return True if a local negotiation is pending (WANTYES or WANTNO), false otherwise.
     * @remark Derived from `local_state_` (us in RFC 1143).
     */
    /**
     * @fn bool OptionStatusRecord::remote_enabled() const noexcept
     * @return True if the option is enabled remotely (state is YES), false otherwise.
     * @remark Derived from `remote_state_` (him in RFC 1143).
     */
    /**
     * @fn bool OptionStatusRecord::remote_disabled() const noexcept
     * @return True if the option is fully disabled remotely (state is NO), false otherwise.
     * @remark Derived from `remote_state_` (him in RFC 1143). Not equivalent to !remote_enabled().
     */
    /**
     * @fn bool OptionStatusRecord::remote_pending_enable() const noexcept
     * @return True if a remote enablement request is pending (state is WANTYES), false otherwise.
     * @remark Derived from `remote_state_` (him in RFC 1143).
     */
    /**
     * @fn bool OptionStatusRecord::remote_pending_disable() const noexcept
     * @return True if a remote disablement request is pending (state is WANTNO), false otherwise.
     * @remark Derived from `remote_state_` (him in RFC 1143).
     */
    /**
     * @fn bool OptionStatusRecord::remote_pending() const noexcept
     * @return True if a remote negotiation is pending (WANTYES or WANTNO), false otherwise.
     * @remark Derived from `remote_state_` (him in RFC 1143).
     */
    /**
     * @fn bool OptionStatusRecord::enabled(NegotiationDirection direction) const noexcept
     * @param direction The negotiation direction (LOCAL or REMOTE).
     * @return True if the option is enabled in the designated direction, false otherwise.
     * @remark Delegates to `local_enabled()` or `remote_enabled()` based on direction (us or him in RFC 1143).
     */
    /**
     * @fn bool OptionStatusRecord::disabled(NegotiationDirection direction) const noexcept
     * @param direction The negotiation direction (LOCAL or REMOTE).
     * @return True if the option is fully disabled in the designated direction, false otherwise.
     * @remark Delegates to `local_disabled()` or `remote_disabled()` based on direction (us or him in RFC 1143). Not equivalent to !enabled(direction).
     */
    /**
     * @fn bool OptionStatusRecord::pending_enable(NegotiationDirection direction) const noexcept
     * @param direction The negotiation direction (LOCAL or REMOTE).
     * @return True if an enablement request is pending in the designated direction (state is WANTYES), false otherwise.
     * @remark Delegates to `local_pending_enable()` or `remote_pending_enable()` based on direction (us or him in RFC 1143).
     */
    /**
     * @fn bool OptionStatusRecord::pending_disable(NegotiationDirection direction) const noexcept
     * @param direction The negotiation direction (LOCAL or REMOTE).
     * @return True if a disablement request is pending in the designated direction (state is WANTNO), false otherwise.
     * @remark Delegates to `local_pending_disable()` or `remote_pending_disable()` based on direction (us or him in RFC 1143).
     */
    /**
     * @fn bool OptionStatusRecord::pending(NegotiationDirection direction) const noexcept
     * @param direction The negotiation direction (LOCAL or REMOTE).
     * @return True if a negotiation is pending (WANTYES or WANTNO) in the designated direction, false otherwise.
     * @remark Delegates to `local_pending()` or `remote_pending()` based on direction (us or him in RFC 1143).
     */
    /**
     * @fn bool OptionStatusRecord::is_enabled() const noexcept
     * @return True if the option is enabled locally or remotely, false otherwise.
     * @remark Combines results of `local_enabled()` and `remote_enabled()`.
     */
    /**
     * @fn bool OptionStatusRecord::local_queued() const noexcept
     * @return True if a local user request is queued (OPPOSITE state), false otherwise.
     * @remark Accesses `local_queue_` (usq in RFC 1143).
     */
    /**
     * @fn bool OptionStatusRecord::remote_queued() const noexcept
     * @return True if a remote user request is queued (OPPOSITE state), false otherwise.
     * @remark Accesses `remote_queue_` (himq in RFC 1143).
     */
    /**
     * @fn bool OptionStatusRecord::queued(NegotiationDirection direction) const noexcept
     * @param direction The negotiation direction (LOCAL or REMOTE).
     * @return True if a user request is queued (OPPOSITE state) in the designated direction, false otherwise.
     * @remark Delegates to `local_queued()` or `remote_queued()` based on direction (usq or himq in RFC 1143).
     */
    /**
     * @fn bool OptionStatusRecord::has_queued_request() const noexcept
     * @return True if any user request is queued (local or remote), false otherwise.
     * @remark Combines results of `local_queued()` and `remote_queued()` (usq and himq in RFC 1143). [Optional]
     */
    /**
     * @fn void OptionStatusRecord::enable_local() noexcept
     * @brief Sets the local state to YES (enabled).
     * @remark Updates `local_state_` (us in RFC 1143).
     */
    /**
     * @fn void OptionStatusRecord::disable_local() noexcept
     * @brief Sets the local state to NO (disabled).
     * @remark Updates `local_state_` (us in RFC 1143).
     */
    /**
     * @fn void OptionStatusRecord::pend_enable_local() noexcept
     * @brief Sets the local state to WANTYES (pending enablement).
     * @remark Updates `local_state_` (us in RFC 1143).
     */
    /**
     * @fn void OptionStatusRecord::pend_disable_local() noexcept
     * @brief Sets the local state to WANTNO (pending disablement).
     * @remark Updates `local_state_` (us in RFC 1143).
     */
    /**
     * @fn void OptionStatusRecord::enable_remote() noexcept
     * @brief Sets the remote state to YES (enabled).
     * @remark Updates `remote_state_` (him in RFC 1143).
     */
    /**
     * @fn void OptionStatusRecord::disable_remote() noexcept
     * @brief Sets the remote state to NO (disabled).
     * @remark Updates `remote_state_` (him in RFC 1143).
     */
    /**
     * @fn void OptionStatusRecord::pend_enable_remote() noexcept
     * @brief Sets the remote state to WANTYES (pending enablement).
     * @remark Updates `remote_state_` (him in RFC 1143).
     */
    /**
     * @fn void OptionStatusRecord::pend_disable_remote() noexcept
     * @brief Sets the remote state to WANTNO (pending disablement).
     * @remark Updates `remote_state_` (him in RFC 1143).
     */
    /**
     * @fn void OptionStatusRecord::enable(NegotiationDirection direction) noexcept
     * @param direction The negotiation direction (LOCAL or REMOTE).
     * @brief Sets the state to YES (enabled) in the designated direction.
     * @remark Delegates to `enable_local()` or `enable_remote()` based on direction (us or him in RFC 1143).
     */
    /**
     * @fn void OptionStatusRecord::disable(NegotiationDirection direction) noexcept
     * @param direction The negotiation direction (LOCAL or REMOTE).
     * @brief Sets the state to NO (disabled) in the designated direction.
     * @remark Delegates to `disable_local()` or `disable_remote()` based on direction (us or him in RFC 1143).
     */
    /**
     * @fn void OptionStatusRecord::pend_enable(NegotiationDirection direction) noexcept
     * @param direction The negotiation direction (LOCAL or REMOTE).
     * @brief Sets the state to WANTYES (pending enablement) in the designated direction.
     * @remark Delegates to `pend_enable_local()` or `pend_enable_remote()` based on direction (us or him in RFC 1143).
     */
    /**
     * @fn void OptionStatusRecord::pend_disable(NegotiationDirection direction) noexcept
     * @param direction The negotiation direction (LOCAL or REMOTE).
     * @brief Sets the state to WANTNO (pending disablement) in the designated direction.
     * @remark Delegates to `pend_disable_local()` or `pend_disable_remote()` based on direction (us or him in RFC 1143).
     */
    /**
     * @fn void OptionStatusRecord::enqueue_local() noexcept
     * @brief Marks a local user request as queued (OPPOSITE state).
     * @return `negotiation_queue_error` if `local_state_` is YES or NO and {} otherwise. 
     * @remark Sets `local_queue_` to true (usq in RFC 1143) if `local_state_` is WANT*.
     */
    /**
     * @fn void OptionStatusRecord::enqueue_remote() noexcept
     * @brief Marks a remote user request as queued (OPPOSITE state).
     * @return `negotiation_queue_error` if `remote_state_` is YES or NO and {} otherwise
     * @remark Sets `remote_queue_` to true (himq in RFC 1143) if `remote_state_` is WANT*.
     */
    /**
     * @fn void OptionStatusRecord::enqueue(NegotiationDirection direction) noexcept
     * @param direction The negotiation direction (LOCAL or REMOTE).
     * @brief Marks a user request as queued (OPPOSITE state) in the designated direction.
     * @remark Delegates to `enqueue_local()` or `enqueue_remote()` based on direction (usq or himq in RFC 1143).
     */
    /**
     * @fn void OptionStatusRecord::dequeue_local() noexcept
     * @brief Clears a queued local user request.
     * @remark Sets `local_queue_` to false (usq in RFC 1143).
     */
    /**
     * @fn void OptionStatusRecord::dequeue_remote() noexcept
     * @brief Clears a queued remote user request.
     * @remark Sets `remote_queue_` to false (himq in RFC 1143).
     */
    /**
     * @fn void OptionStatusRecord::dequeue(NegotiationDirection direction) noexcept
     * @param direction The negotiation direction (LOCAL or REMOTE).
     * @brief Clears a queued user request in the designated direction.
     * @remark Delegates to `dequeue_local()` or `dequeue_remote()` based on direction (usq or himq in RFC 1143).
     */
    /**
     * @fn void OptionStatusRecord::clear_queued_requests() noexcept
     * @brief Clears all queued requests (local and remote).
     * @remark Sets `local_queue_` and `remote_queue_` to false (usq and himq in RFC 1143).
     */
    /**
     * @fn void OptionStatusRecord::reset() noexcept
     * @brief Resets all state to initial values (NO, no queued requests).
     * @remark Sets `local_state_`, `remote_state_`, `local_queue_`, and `remote_queue_` to initial values.
     */
    /**
     * @fn bool OptionStatusRecord::is_negotiating() const noexcept
     * @brief Checks if either local or remote negotiation is pending.
     * @return True if `local_pending()` or `remote_pending()` is true, false otherwise.
     * @remark Combines results of `local_pending()` and `remote_pending()`. [Optional]
     */
    /**
     * @fn bool OptionStatusRecord::has_queued_request() const noexcept
     * @brief Checks if any user request is queued (local or remote).
     * @return True if `local_queued()` or `remote_queued()` is true, false otherwise.
     * @remark Combines results of `local_queued()` and `remote_queued()`. [Optional]
     */
    /**
     * @fn bool OptionStatusRecord::is_valid() const noexcept
     * @brief Validates state consistency (e.g., queue flags false when not pending).
     * @return True if queue flags are consistent with pending states, false otherwise.
     * @remark Ensures `local_queue_` and `remote_queue_` are false unless respective states are WANTNO or WANTYES. [Optional]
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