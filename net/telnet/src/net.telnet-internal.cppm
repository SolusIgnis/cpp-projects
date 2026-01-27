/**
 * @file net.telnet-internal.cppm
 * @version 0.5.7
 * @release_date October 30, 2025
 *
 * @brief Internal partition defining implementation detail data structures for other partitions.
 * @remark Defines record and registry types used by `:protocol_fsm` for managing Telnet options and handlers.
 *
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @remark Not intended for direct use by external code; serves as an implementation detail for other partitions.
 * @see RFC 855 for Telnet option negotiation, `:types` for `telnet::command`, `:options` for `option` and `option::id_num`, `:errors` for error codes, `:protocol_fsm` for usage
 */

module; //Including Asio in the Global Module Fragment until importable header units are reliable.
#include <asio.hpp>

//Module partition interface unit
export module net.telnet:internal;

import std; //NOLINT For std::function, std::optional, std::map, std::set, std::vector, std::shared_mutex, std::shared_lock, std::lock_guard, std::once_flag, std::cout, std::cerr, std::hex, std::setw, std::setfill, std::dec
import std.compat; //NOLINT For std::uint8_t (needed for bit-field type specifier)

import :types;      ///< @see "net.telnet-types.cppm" for `byte_t` and `telnet::command`
import :errors;     ///< @see "net.telnet-errors.cppm" for `telnet::error` and `telnet::processing_signal` codes
import :concepts;   ///< @see "net.telnet-concepts.cppm" for `telnet::concepts::ProtocolFSMConfig`
import :options;    ///< @see "net.telnet-options.cppm" for `option` and `option::id_num`
import :awaitables; ///< @see "net.telnet-awaitables.cppm" for `TaggedAwaitable`, semantic tags, and type aliases

//namespace asio = boost::asio;

export namespace net::telnet {
    /**
     * @brief Registry for managing Telnet option handlers.
     * @tparam ProtocolConfig Configuration class providing logging and error handling.
     * @tparam OptionEnablementHandler Handler type for option enablement.
     * @tparam OptionDisablementHandler Handler type for option disablement.
     * @tparam SubnegotiationHandler Handler type for processing subnegotiation data.
     * @remark Used by `:protocol_fsm` to manage handlers for Telnet options.
     * @remark Instantiated per-`ProtocolFSM` and used in a single thread/strand.
     * @see `:protocol_fsm` for handler usage, `:options` for `option::id_num`, `:errors` for error codes
     */
    template<typename ProtocolConfig,
             typename OptionEnablementHandler,
             typename OptionDisablementHandler,
             typename SubnegotiationHandler>
    class option_handler_registry {
    private:
        /**
         * @brief Record for handlers registered to a single Telnet option.
         * @details Stores an optional enablement handler, an optional disablement handler, and an optional subnegotiation handler for processing option-specific data.
         * @see `:options` for `option::id_num`, `:protocol_fsm` for usage
         */
        struct option_handler_record {
            std::optional<OptionEnablementHandler> enablement_handler;
            std::optional<OptionDisablementHandler> disablement_handler;
            std::optional<SubnegotiationHandler> subnegotiation_handler;
        }; //struct option_handler_record

    public:
        /**
         * @brief Registers handlers for a Telnet option.
         * @param opt The `option::id_num` to register handlers for.
         * @param enablement_handler Optional handler for option enablement.
         * @param disablement_handler Optional handler for option disablement.
         * @param subnegotiation_handler Optional handler for subnegotiation (defaults to `std::nullopt`).
         * @remark Overwrites existing handlers for the specified option.
         * @see `:options` for `option::id_num`, `:awaitables` for handler return types
         */
        void register_handlers(option::id_num opt,
                               std::optional<OptionEnablementHandler> enablement_handler,
                               std::optional<OptionDisablementHandler> disablement_handler,
                               std::optional<SubnegotiationHandler> subnegotiation_handler = std::nullopt) {
            handlers_[opt] = option_handler_record{std::move(enablement_handler),
                                                 std::move(disablement_handler),
                                                 std::move(subnegotiation_handler)};
        } //register_handlers(option::id_num, std::optional<OptionEnablementHandler>, std::optional<OptionDisablementHandler>, std::optional<SubnegotiationHandler>)

        /**
         * @brief Unregisters all handlers for a Telnet option.
         * @param opt The `option::id_num` to unregister handlers for.
         * @remark Removes the handler record from the registry.
         * @see `:options` for `option::id_num`
         */
        void unregister_handlers(option::id_num opt) { handlers_.erase(opt); } //unregister_handlers(option::id_num)

        ///@brief Handles enablement for a Telnet option.
        awaitables::option_enablement_awaitable handle_enablement(const option opt, negotiation_direction direction) {
            auto iter = handlers_.find(opt);
            if ((iter != handlers_.end()) && iter->second.enablement_handler) {
                auto& handler = *(iter->second.enablement_handler);
                return handler(opt, direction);
            }
            return {};
        } //handle_enablement(const option&, negotiation_direction)

        ///@brief Handles disablement for a Telnet option.
        awaitables::option_disablement_awaitable handle_disablement(const option opt, negotiation_direction direction) {
            auto iter = handlers_.find(opt);
            if ((iter != handlers_.end()) && iter->second.disablement_handler) {
                auto& handler = *(iter->second.disablement_handler);
                return handler(opt, direction);
            }
            return {};
        } //handle_disablement(const option&, negotiation_direction)

        ///@brief Handles subnegotiation for a Telnet option.
        awaitables::subnegotiation_awaitable handle_subnegotiation(const option opt, std::vector<byte_t> data) {
            auto iter = handlers_.find(opt);
            if ((iter != handlers_.end()) && iter->second.subnegotiation_handler) {
                auto& handler = *(iter->second.subnegotiation_handler);
                return handler(opt, std::move(data));
            } else {
                return undefined_subnegotiation_handler<ProtocolConfig>(opt, std::move(data));
            }
        } //handle_subnegotiation(option::id_num, std::vector<byte_t>)
    private:
        ///@brief Default handler for undefined subnegotiation.
        awaitables::subnegotiation_awaitable undefined_subnegotiation_handler(option opt, std::vector<byte_t> /*unused*/) {
            ProtocolConfig::log_error(make_error_code(error::user_handler_not_found),
                                      "cmd: {}, option: {}",
                                      command::se,
                                      opt);
            co_return;
        } //undefined_subnegotiation_handler(option::id_num opt, std::vector<byte_t>)

        std::map<option::id_num, option_handler_record> handlers_;
    }; //class option_handler_registry

    /**
     * @fn void option_handler_registry::register_handlers(option::id_num opt, std::optional<OptionEnablementHandler> enablement_handler, std::optional<OptionDisablementHandler> disablement_handler, std::optional<SubnegotiationHandler> subnegotiation_handler)
     * @param opt The `option::id_num` to register handlers for.
     * @param enablement_handler Optional handler for option enablement.
     * @param disablement_handler Optional handler for option disablement.
     * @param subnegotiation_handler Optional handler for subnegotiation (defaults to `std::nullopt`).
     * @remark Overwrites existing handlers for the specified option.
     */
    /**
     * @fn void option_handler_registry::unregister_handlers(option::id_num opt)
     * @param opt The `option::id_num` to unregister handlers for.
     * @remark Removes the handler record from the registry.
     */
    /**
     * @fn option_enablement_awaitable option_handler_registry::handle_enablement(const option& opt, negotiation_direction direction)
     * @param opt The `option::id_num` of the Telnet option.
     * @param direction The negotiation direction (`local` or `remote`).
     * @return `option_enablement_awaitable` representing the asynchronous handling result.
     * @remark Invokes the registered enablement handler if present; otherwise, returns an empty awaitable.
     */
    /**
     * @fn option_disablement_awaitable option_handler_registry::handle_disablement(const option& opt, negotiation_direction direction)
     * @param opt The `option::id_num` of the Telnet option.
     * @param direction The negotiation direction (`local` or `remote`).
     * @return `option_disablement_awaitable` representing the asynchronous handling result.
     * @remark Invokes the registered disablement handler if present; otherwise, returns an empty awaitable.
     */
    /**
     * @fn subnegotiation_awaitable option_handler_registry::handle_subnegotiation(const option& opt, std::vector<byte_t> data)
     * @param opt The `option::id_num` of the Telnet option.
     * @param data The subnegotiation data to process.
     * @return `subnegotiation_awaitable` representing the asynchronous handling result.
     * @remark Invokes the registered subnegotiation handler if present; otherwise, calls `undefined_subnegotiation_handler`.
     */
    /**
     * @fn static subnegotiation_awaitable option_handler_registry::undefined_subnegotiation_handler<PC>(const option& opt, std::vector<byte_t>)
     * @tparam PC The `ProtocolConfig` type for logging.
     * @param opt The `option::id_num` of the Telnet option.
     * @param data The subnegotiation data (unused).
     * @return `subnegotiation_awaitable` representing an empty asynchronous result.
     * @remark Logs an `error::user_handler_not_found` error via `PC::log_error`.
     * @note Used as a fallback when no subnegotiation handler is registered.
     */

    /**
     * @brief Record for tracking the status of a single Telnet option.
     * @remark Optimized with bit-fields packed into a single byte for memory efficiency.
     * @note Implements RFC 1143 Q Method with 4-state FSMs plus queue bits for local (us) and remote (him) sides.
     * @remark Instantiated per-`ProtocolFSM` and used in a single thread/strand.
     * @see `:types` for `negotiation_direction`, `:options` for `option::id_num`, `:protocol_fsm` for usage
     */
    class option_status_record {
    public:
        enum class negotiation_state : std::uint8_t {
            no       = 0,
            yes      = 1,
            want_no  = 2,
            want_yes = 3
        };

        //Local state queries (us)
        ///@brief Checks if the option is enabled locally.
        [[nodiscard]] bool local_enabled() const noexcept { return local_state_ == std::to_underlying(negotiation_state::yes); }

        ///@brief Checks if the option is fully disabled locally. @note NOT equivalent to !local_enabled()
        [[nodiscard]] bool local_disabled() const noexcept { return local_state_ == std::to_underlying(negotiation_state::no); }

        ///@brief Checks if a local enablement request is pending.
        [[nodiscard]] bool local_pending_enable() const noexcept {
            return local_state_ == std::to_underlying(negotiation_state::want_yes);
        }

        ///@brief Checks if a local disablement request is pending.
        [[nodiscard]] bool local_pending_disable() const noexcept {
            return local_state_ == std::to_underlying(negotiation_state::want_no);
        }

        ///@brief Checks if local negotiation is pending (WANTNO or WANTYES).
        [[nodiscard]] bool local_pending() const noexcept { return local_pending_enable() || local_pending_disable(); }

        //Remote state queries (him)
        ///@brief Checks if the option is enabled remotely.
        [[nodiscard]] bool remote_enabled() const noexcept { return remote_state_ == std::to_underlying(negotiation_state::yes); }

        ///@brief Checks if the option is fully disabled remotely. @note NOT equivalent to !remote_enabled()
        [[nodiscard]] bool remote_disabled() const noexcept { return remote_state_ == std::to_underlying(negotiation_state::no); }

        ///@brief Checks if a remote enablement request is pending.
        [[nodiscard]] bool remote_pending_enable() const noexcept {
            return remote_state_ == std::to_underlying(negotiation_state::want_yes);
        }

        ///@brief Checks if a remote disablement request is pending.
        [[nodiscard]] bool remote_pending_disable() const noexcept {
            return remote_state_ == std::to_underlying(negotiation_state::want_no);
        }

        ///@brief Checks if remote negotiation is pending (WANTNO or WANTYES).
        [[nodiscard]] bool remote_pending() const noexcept { return remote_pending_enable() || remote_pending_disable(); }

        //Bidirectional state queries
        ///@brief Checks if the option is enabled in the designated direction.
        [[nodiscard]] bool enabled(negotiation_direction direction) const noexcept {
            return (direction == negotiation_direction::remote) ? remote_enabled() : local_enabled();
        }

        ///@brief Checks if the option is disabled in the designated direction. @note NOT equivalent to !enabled(direction)
        [[nodiscard]] bool disabled(negotiation_direction direction) const noexcept {
            return (direction == negotiation_direction::remote) ? remote_disabled() : local_disabled();
        }

        ///@brief Checks if an enablement request is pending in the designated direction.
        [[nodiscard]] bool pending_enable(negotiation_direction direction) const noexcept {
            return (direction == negotiation_direction::remote) ? remote_pending_enable() : local_pending_enable();
        }

        ///@brief Checks if a disablement request is pending in the designated direction.
        [[nodiscard]] bool pending_disable(negotiation_direction direction) const noexcept {
            return (direction == negotiation_direction::remote) ? remote_pending_disable() : local_pending_disable();
        }

        ///@brief Checks if a negotiation is pending (WANTNO or WANTYES) in the designated direction.
        [[nodiscard]] bool pending(negotiation_direction direction) const noexcept {
            return (direction == negotiation_direction::remote) ? remote_pending() : local_pending();
        }

        //Combined state query
        ///@brief Checks if the option is enabled locally or remotely.
        [[nodiscard]] bool is_enabled() const noexcept { return local_enabled() || remote_enabled(); }

        //Queue queries (usq, himq)
        ///@brief Checks if a local user request is queued (OPPOSITE state).
        [[nodiscard]] bool local_queued() const noexcept { return local_queue_; }

        ///@brief Checks if a remote user request is queued (OPPOSITE state).
        [[nodiscard]] bool remote_queued() const noexcept { return remote_queue_; }

        ///@brief Checks if a user request is queued (OPPOSITE state) in the designated direction.
        [[nodiscard]] bool queued(negotiation_direction direction) const noexcept {
            return (direction == negotiation_direction::remote) ? remote_queued() : local_queued();
        }

        ///@brief Checks if any user request is queued (local or remote). [Optional]
        [[nodiscard]] bool has_queued_request() const noexcept { return local_queued() || remote_queued(); }

        //Local state setters (us)
        ///@brief Enables the option locally.
        void enable_local() noexcept { local_state_ = std::to_underlying(negotiation_state::yes); }

        ///@brief Disables the option locally.
        void disable_local() noexcept { local_state_ = std::to_underlying(negotiation_state::no); }

        ///@brief Marks a local enablement request as pending.
        void pend_enable_local() noexcept { local_state_ = std::to_underlying(negotiation_state::want_yes); }

        ///@brief Marks a local disablement request as pending.
        void pend_disable_local() noexcept { local_state_ = std::to_underlying(negotiation_state::want_no); }

        //Remote state setters (him)
        ///@brief Enables the option remotely.
        void enable_remote() noexcept { remote_state_ = std::to_underlying(negotiation_state::yes); }

        ///@brief Disables the option remotely.
        void disable_remote() noexcept { remote_state_ = std::to_underlying(negotiation_state::no); }

        ///@brief Marks a remote enablement request as pending.
        void pend_enable_remote() noexcept { remote_state_ = std::to_underlying(negotiation_state::want_yes); }

        ///@brief Marks a remote disablement request as pending.
        void pend_disable_remote() noexcept { remote_state_ = std::to_underlying(negotiation_state::want_no); }

        //Bidirectional state setters
        ///@brief Enables the option in the designated direction.
        void enable(negotiation_direction direction) noexcept {
            if (direction == negotiation_direction::remote) {
                enable_remote();
            } else {
                enable_local();
            }
        }

        ///@brief Disables the option in the designated direction.
        void disable(negotiation_direction direction) noexcept {
            if (direction == negotiation_direction::remote) {
                disable_remote();
            } else {
                disable_local();
            }
        }

        ///@brief Marks an enablement request as pending in the designated direction.
        void pend_enable(negotiation_direction direction) noexcept {
            if (direction == negotiation_direction::remote) {
                pend_enable_remote();
            } else {
                pend_enable_local();
            }
        }

        ///@brief Marks a disablement request as pending in the designated direction.
        void pend_disable(negotiation_direction direction) noexcept {
            if (direction == negotiation_direction::remote) {
                pend_disable_remote();
            } else {
                pend_disable_local();
            }
        }

        //Queue setters (usq, himq)
        ///@brief Sets a local user request as queued (OPPOSITE state).
        [[nodiscard]] std::error_code enqueue_local() noexcept {
            if (local_enabled() || local_disabled()) {
                return make_error_code(error::negotiation_queue_error);
            } else {
                local_queue_ = true;
                return {};
            }
        }

        ///@brief Sets a remote user request as queued (OPPOSITE state).
        [[nodiscard]] std::error_code enqueue_remote() noexcept {
            if (remote_enabled() || remote_disabled()) {
                return make_error_code(error::negotiation_queue_error);
            } else {
                remote_queue_ = true;
                return {};
            }
        }

        ///@brief Sets a user request as queued (OPPOSITE state) in the designated direction.
        [[nodiscard]] std::error_code enqueue(negotiation_direction direction) noexcept {
            if (direction == negotiation_direction::remote) {
                return enqueue_remote();
            } else {
                return enqueue_local();
            }
        }

        ///@brief Clears a queued local user request.
        void dequeue_local() noexcept { local_queue_ = false; }

        ///@brief Clears a queued remote user request.
        void dequeue_remote() noexcept { remote_queue_ = false; }

        ///@brief Clears a queued user request in the designated direction.
        void dequeue(negotiation_direction direction) noexcept {
            if (direction == negotiation_direction::remote) {
                dequeue_remote();
            } else {
                dequeue_local();
            }
        }

        ///@brief Clears all queued requests.
        void clear_queued_requests() noexcept { local_queue_ = remote_queue_ = false; }

        //Utility
        ///@brief Resets all state to initial values (NO, no queued requests).
        void reset() noexcept {
            local_state_  = std::to_underlying(negotiation_state::no);
            remote_state_ = std::to_underlying(negotiation_state::no);
            local_queue_ = remote_queue_ = false;
        }

        ///@brief Checks if either local or remote negotiation is pending. [Optional]
        [[nodiscard]] bool is_negotiating() const noexcept { return local_pending() || remote_pending(); }

        ///@brief Validates state consistency (e.g., queue flags false when not pending). [Optional]
        [[nodiscard]] bool is_valid() const noexcept {
            return (!local_queue_ || local_pending()) && (!remote_queue_ || remote_pending());
        }

    private:
        //Pack these 4 fields into 1 byte (6 bits used, 2 unused).
        std::uint8_t local_state_  : 2 = std::to_underlying(negotiation_state::no); //us
        std::uint8_t remote_state_ : 2 = std::to_underlying(negotiation_state::no); //him
        std::uint8_t local_queue_  : 1 = static_cast<std::uint8_t>(false);         //usq  (EMPTY=false, OPPOSITE=true)
        std::uint8_t remote_queue_ : 1 = static_cast<std::uint8_t>(false);         //himq (EMPTY=false, OPPOSITE=true)
    }; //class option_status_record

    /**
     * @fn bool option_status_record::local_enabled() const noexcept
     * @return True if the option is enabled locally (state is YES), false otherwise.
     * @remark Derived from `local_state_` (us in RFC 1143).
     */
    /**
     * @fn bool option_status_record::local_disabled() const noexcept
     * @return True if the option is fully disabled locally (state is NO), false otherwise.
     * @remark Derived from `local_state_` (us in RFC 1143). Not equivalent to !local_enabled().
     */
    /**
     * @fn bool option_status_record::local_pending_enable() const noexcept
     * @return True if a local enablement request is pending (state is WANTYES), false otherwise.
     * @remark Derived from `local_state_` (us in RFC 1143).
     */
    /**
     * @fn bool option_status_record::local_pending_disable() const noexcept
     * @return True if a local disablement request is pending (state is WANTNO), false otherwise.
     * @remark Derived from `local_state_` (us in RFC 1143).
     */
    /**
     * @fn bool option_status_record::local_pending() const noexcept
     * @return True if a local negotiation is pending (WANTYES or WANTNO), false otherwise.
     * @remark Derived from `local_state_` (us in RFC 1143).
     */
    /**
     * @fn bool option_status_record::remote_enabled() const noexcept
     * @return True if the option is enabled remotely (state is YES), false otherwise.
     * @remark Derived from `remote_state_` (him in RFC 1143).
     */
    /**
     * @fn bool option_status_record::remote_disabled() const noexcept
     * @return True if the option is fully disabled remotely (state is NO), false otherwise.
     * @remark Derived from `remote_state_` (him in RFC 1143). Not equivalent to !remote_enabled().
     */
    /**
     * @fn bool option_status_record::remote_pending_enable() const noexcept
     * @return True if a remote enablement request is pending (state is WANTYES), false otherwise.
     * @remark Derived from `remote_state_` (him in RFC 1143).
     */
    /**
     * @fn bool option_status_record::remote_pending_disable() const noexcept
     * @return True if a remote disablement request is pending (state is WANTNO), false otherwise.
     * @remark Derived from `remote_state_` (him in RFC 1143).
     */
    /**
     * @fn bool option_status_record::remote_pending() const noexcept
     * @return True if a remote negotiation is pending (WANTYES or WANTNO), false otherwise.
     * @remark Derived from `remote_state_` (him in RFC 1143).
     */
    /**
     * @fn bool option_status_record::enabled(negotiation_direction direction) const noexcept
     * @param direction The negotiation direction (local or remote).
     * @return True if the option is enabled in the designated direction, false otherwise.
     * @remark Delegates to `local_enabled()` or `remote_enabled()` based on direction (us or him in RFC 1143).
     */
    /**
     * @fn bool option_status_record::disabled(negotiation_direction direction) const noexcept
     * @param direction The negotiation direction (local or remote).
     * @return True if the option is fully disabled in the designated direction, false otherwise.
     * @remark Delegates to `local_disabled()` or `remote_disabled()` based on direction (us or him in RFC 1143). Not equivalent to !enabled(direction).
     */
    /**
     * @fn bool option_status_record::pending_enable(negotiation_direction direction) const noexcept
     * @param direction The negotiation direction (local or remote).
     * @return True if an enablement request is pending in the designated direction (state is WANTYES), false otherwise.
     * @remark Delegates to `local_pending_enable()` or `remote_pending_enable()` based on direction (us or him in RFC 1143).
     */
    /**
     * @fn bool option_status_record::pending_disable(negotiation_direction direction) const noexcept
     * @param direction The negotiation direction (local or remote).
     * @return True if a disablement request is pending in the designated direction (state is WANTNO), false otherwise.
     * @remark Delegates to `local_pending_disable()` or `remote_pending_disable()` based on direction (us or him in RFC 1143).
     */
    /**
     * @fn bool option_status_record::pending(negotiation_direction direction) const noexcept
     * @param direction The negotiation direction (local or remote).
     * @return True if a negotiation is pending (WANTYES or WANTNO) in the designated direction, false otherwise.
     * @remark Delegates to `local_pending()` or `remote_pending()` based on direction (us or him in RFC 1143).
     */
    /**
     * @fn bool option_status_record::is_enabled() const noexcept
     * @return True if the option is enabled locally or remotely, false otherwise.
     * @remark Combines results of `local_enabled()` and `remote_enabled()`.
     */
    /**
     * @fn bool option_status_record::local_queued() const noexcept
     * @return True if a local user request is queued (OPPOSITE state), false otherwise.
     * @remark Accesses `local_queue_` (usq in RFC 1143).
     */
    /**
     * @fn bool option_status_record::remote_queued() const noexcept
     * @return True if a remote user request is queued (OPPOSITE state), false otherwise.
     * @remark Accesses `remote_queue_` (himq in RFC 1143).
     */
    /**
     * @fn bool option_status_record::queued(negotiation_direction direction) const noexcept
     * @param direction The negotiation direction (local or remote).
     * @return True if a user request is queued (OPPOSITE state) in the designated direction, false otherwise.
     * @remark Delegates to `local_queued()` or `remote_queued()` based on direction (usq or himq in RFC 1143).
     */
    /**
     * @fn bool option_status_record::has_queued_request() const noexcept
     * @return True if any user request is queued (local or remote), false otherwise.
     * @remark Combines results of `local_queued()` and `remote_queued()` (usq and himq in RFC 1143). [Optional]
     */
    /**
     * @fn void option_status_record::enable_local() noexcept
     * @brief Sets the local state to YES (enabled).
     * @remark Updates `local_state_` (us in RFC 1143).
     */
    /**
     * @fn void option_status_record::disable_local() noexcept
     * @brief Sets the local state to NO (disabled).
     * @remark Updates `local_state_` (us in RFC 1143).
     */
    /**
     * @fn void option_status_record::pend_enable_local() noexcept
     * @brief Sets the local state to WANTYES (pending enablement).
     * @remark Updates `local_state_` (us in RFC 1143).
     */
    /**
     * @fn void option_status_record::pend_disable_local() noexcept
     * @brief Sets the local state to WANTNO (pending disablement).
     * @remark Updates `local_state_` (us in RFC 1143).
     */
    /**
     * @fn void option_status_record::enable_remote() noexcept
     * @brief Sets the remote state to YES (enabled).
     * @remark Updates `remote_state_` (him in RFC 1143).
     */
    /**
     * @fn void option_status_record::disable_remote() noexcept
     * @brief Sets the remote state to NO (disabled).
     * @remark Updates `remote_state_` (him in RFC 1143).
     */
    /**
     * @fn void option_status_record::pend_enable_remote() noexcept
     * @brief Sets the remote state to WANTYES (pending enablement).
     * @remark Updates `remote_state_` (him in RFC 1143).
     */
    /**
     * @fn void option_status_record::pend_disable_remote() noexcept
     * @brief Sets the remote state to WANTNO (pending disablement).
     * @remark Updates `remote_state_` (him in RFC 1143).
     */
    /**
     * @fn void option_status_record::enable(negotiation_direction direction) noexcept
     * @param direction The negotiation direction (local or remote).
     * @brief Sets the state to YES (enabled) in the designated direction.
     * @remark Delegates to `enable_local()` or `enable_remote()` based on direction (us or him in RFC 1143).
     */
    /**
     * @fn void option_status_record::disable(negotiation_direction direction) noexcept
     * @param direction The negotiation direction (local or remote).
     * @brief Sets the state to NO (disabled) in the designated direction.
     * @remark Delegates to `disable_local()` or `disable_remote()` based on direction (us or him in RFC 1143).
     */
    /**
     * @fn void option_status_record::pend_enable(negotiation_direction direction) noexcept
     * @param direction The negotiation direction (local or remote).
     * @brief Sets the state to WANTYES (pending enablement) in the designated direction.
     * @remark Delegates to `pend_enable_local()` or `pend_enable_remote()` based on direction (us or him in RFC 1143).
     */
    /**
     * @fn void option_status_record::pend_disable(negotiation_direction direction) noexcept
     * @param direction The negotiation direction (local or remote).
     * @brief Sets the state to WANTNO (pending disablement) in the designated direction.
     * @remark Delegates to `pend_disable_local()` or `pend_disable_remote()` based on direction (us or him in RFC 1143).
     */
    /**
     * @fn void option_status_record::enqueue_local() noexcept
     * @brief Marks a local user request as queued (OPPOSITE state).
     * @return `negotiation_queue_error` if `local_state_` is YES or NO and {} otherwise.
     * @remark Sets `local_queue_` to true (usq in RFC 1143) if `local_state_` is WANT*.
     */
    /**
     * @fn void option_status_record::enqueue_remote() noexcept
     * @brief Marks a remote user request as queued (OPPOSITE state).
     * @return `negotiation_queue_error` if `remote_state_` is YES or NO and {} otherwise
     * @remark Sets `remote_queue_` to true (himq in RFC 1143) if `remote_state_` is WANT*.
     */
    /**
     * @fn void option_status_record::enqueue(negotiation_direction direction) noexcept
     * @param direction The navigation direction (local or remote).
     * @brief Marks a user request as queued (OPPOSITE state) in the designated direction.
     * @remark Delegates to `enqueue_local()` or `enqueue_remote()` based on direction (usq or himq in RFC 1143).
     */
    /**
     * @fn void option_status_record::dequeue_local() noexcept
     * @brief Clears a queued local user request.
     * @remark Sets `local_queue_` to false (usq in RFC 1143).
     */
    /**
     * @fn void option_status_record::dequeue_remote() noexcept
     * @brief Clears a queued remote user request.
     * @remark Sets `remote_queue_` to false (himq in RFC 1143).
     */
    /**
     * @fn void option_status_record::dequeue(negotiation_direction direction) noexcept
     * @param direction The negotiation direction (local or remote).
     * @brief Clears a queued user request in the designated direction.
     * @remark Delegates to `dequeue_local()` or `dequeue_remote()` based on direction (usq or himq in RFC 1143).
     */
    /**
     * @fn void option_status_record::clear_queued_requests() noexcept
     * @brief Clears all queued requests (local and remote).
     * @remark Sets `local_queue_` and `remote_queue_` to false (usq and himq in RFC 1143).
     */
    /**
     * @fn void option_status_record::reset() noexcept
     * @brief Resets all state to initial values (NO, no queued requests).
     * @remark Sets `local_state_`, `remote_state_`, `local_queue_`, and `remote_queue_` to initial values.
     */
    /**
     * @fn bool option_status_record::is_negotiating() const noexcept
     * @brief Checks if either local or remote negotiation is pending.
     * @return True if `local_pending()` or `remote_pending()` is true, false otherwise.
     * @remark Combines results of `local_pending()` and `remote_pending()`. [Optional]
     */
    /**
     * @fn bool option_status_record::has_queued_request() const noexcept
     * @brief Checks if any user request is queued (local or remote).
     * @return True if `local_queued()` or `remote_queued()` is true, false otherwise.
     * @remark Combines results of `local_queued()` and `remote_queued()`. [Optional]
     */
    /**
     * @fn bool option_status_record::is_valid() const noexcept
     * @brief Validates state consistency (e.g., queue flags false when not pending).
     * @return True if queue flags are consistent with pending states, false otherwise.
     * @remark Ensures `local_queue_` and `remote_queue_` are false unless respective states are WANTNO or WANTYES. [Optional]
     */

    /**
     * @brief Collection of `option_status_record` objects for tracking Telnet option statuses.
     * @remark Provides array-based access to option statuses by `option::id_num`.
     * @remark Used by `:protocol_fsm` to manage the state of Telnet options.
     * @remark Instantiated per-`ProtocolFSM` and used in a single thread/strand.
     * @see `:options` for `option::id_num`, `:protocol_fsm` for usage, `option_status_record` for status details
     */
    class option_status_db {
    public:
        ///@brief Accesses or creates an `option_status_record` for a Telnet option.
        option_status_record& operator[](option::id_num opt) { return status_records_[std::to_underlying(opt)]; } //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index): Safe by construction as the array bounds are defined to hold all values of the underlying type.

        ///@brief Retrieves an `option_status_record` for a Telnet option.
        const option_status_record& operator[](option::id_num opt) const {
            return status_records_[std::to_underlying(opt)]; //NOLINT(cppcoreguidelines-pro-bounds-constant-array-index): Safe by construction as the array bounds are defined to hold all values of the underlying type.
        }

        ///@brief The number of possible `option::id_num` values.
        static constexpr size_t max_option_count =
            (1U << std::numeric_limits<std::underlying_type_t<option::id_num>>::digits); //NOLINT(hicpp-signed-bitwise)

    private:
        //Byte array of Status Record bit-fields.
        std::array<option_status_record, max_option_count> status_records_;
    }; //class option_status_db

    /**
     * @fn option_status_record& option_status_db::operator[](option::id_num opt)
     *
     * @param opt The `option::id_num` of the Telnet option.
     * @return Reference to the `option_status_record` for `opt`.
     *
     * @remark Allows modification of the status record for the specified option.
     */
    /**
     * @fn const option_status_record& option_status_db::operator[](option::id_num opt) const
     *
     * @param opt The `option::id_num` of the Telnet option.
     * @return Const reference to the `option_status_record` for `opt`.
     *
     * @remark Provides read-only access to the status record.
     */
} //namespace net::telnet
