/**
 * @file telnet-stream.cppm
 * @version 0.5.7
 * @release_date October 30, 2025
 *
 * @brief Interface for Telnet stream operations.
 * @remark Defines `telnet::stream` class to provide a Telnet-aware stream wrapper around a lower-layer stream-oriented socket.
 * @remark Defines `stream::InputProcessor` for composed Telnet-aware async_read_some.
 * @see Partition implementation units "telnet-stream-impl.cpp", "telnet-stream-async-impl.cpp", and "telnet-stream-sync-impl.cpp" for function definitions.
 
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @see RFC 854 for Telnet protocol, RFC 855 for option negotiation, `:protocol_fsm` for `ProtocolFSM`, `:types` for `TelnetCommand`, `:options` for `option` and `option::id_num`, `:errors` for error codes, `:internal` for implementation classes
 * @todo Phase 6: Hand-tune next-layer stream constraints to exactly match Boost.Asio stream sockets.
 * @todo Phase 6: Add concept for TLS-aware next-layer stream to implement conditional TLS support.
 * @todo Future Development: Monitor `asio::async_result_t` compatibility with evolving Boost.Asio and `std::execution` for potential transition to awaitable or standard async frameworks.
 * @todo Future Development: Consider using `asio::get_associated_executor(token)` for handler execution in `telnet::stream` to support custom executors.
 */
module; // Including Boost.Asio in the Global Module Fragment until importable header units are reliable.
#include <boost/asio.hpp>

// Module partition interface unit
export module telnet:stream;

import std; // For std::error_code, std::size_t, std::vector, std::same_as

export import :types;        ///< @see "telnet-types.cppm" for `byte_t` and `TelnetCommand`
export import :errors;       ///< @see "telnet-errors.cppm" for `telnet::error` and `telnet::processing_signal` codes
export import :concepts;     ///< @see "telnet-concepts.cppm" for `telnet::concepts::LayerableSocketStream`
export import :options;      ///< @see "telnet-options.cppm" for `option` and `option::id_num`
export import :protocol_fsm; ///< @see "telnet-protocol_fsm.cppm" for `ProtocolFSM`
export import :awaitables;   ///< @see "telnet-awaitables.cppm" for `TaggedAwaitable`

namespace asio = boost::asio;

namespace telnet {
    // Non-exported using declarations to simplify template constraints below.
    using concepts::LayerableSocketStream;
    using concepts::MutableBufferSequence;
    using concepts::ConstBufferSequence;
    using concepts::ReadToken;
    using concepts::WriteToken;
    using concepts::ProtocolFSMConfig; 
} // namespace telnet

export namespace telnet {
    /**
     * @brief Stream class wrapping a next-layer stream/socket with Telnet protocol handling.
     * @remark Implements a stream interface (read/write) over a layered stream/socket.
     * @see `:protocol_fsm` for `ProtocolFSM`, `:types` for `TelnetCommand`, `:options` for `option` and `option::id_num`, `:errors` for error codes
     */
    template<LayerableSocketStream NextLayerT, ProtocolFSMConfig ProtocolConfigT = DefaultProtocolFSMConfig>
    class stream {
    private:
        /**
         * @typedef asio_completion_signature
         * @brief Completion handler signature for Boost.Asio asynchronous operations.
         * @remark Defines the signature `void(std::error_code, std::size_t)` for completion handlers.
         */
        using asio_completion_signature = void(std::error_code, std::size_t);

        /**
         * @typedef asio_result_type
         * @brief Template type for the async result based on a completion token.
         * @tparam CompletionToken The type of the completion token.
         * @remark Deduced as `asio::async_result_t` for the given `CompletionToken` and `asio_completion_signature`.
         */
        template<typename CompletionToken>
        using asio_result_type = asio::async_result_t<std::decay_t<CompletionToken>, asio_completion_signature>;
        
        /**
         * @typedef side_buffer_type
         * @brief Type for input and output side buffers.
         */
        using side_buffer_type = asio::streambuf;
        
    public:
        /**
         * @typedef executor_type
         * @brief Executor type for the stream.
         * @remark Matches the executor type of the next-layer stream.
         */
        using executor_type = typename next_layer_type::executor_type;

        /**
         * @typedef lowest_layer_type
         * @brief Type of the lowest layer stream.
         * @remark Provides access to the lowest layer of the stream stack.
         */
        using lowest_layer_type = typename next_layer_type::lowest_layer_type;

        /**
         * @typedef next_layer_type
         * @brief Type of the next-layer stream.
         * @remark Represents the wrapped stream type satisfying `LayerableSocketStream`.
         */
        using next_layer_type = NextLayerT;

        /**
         * @typedef fsm_type
         * @brief Type of the Telnet protocol finite state machine.
         * @remark Configured with `ProtocolConfigT`, defaults to `DefaultProtocolFSMConfig`.
         */
        using fsm_type = ProtocolFSM<ProtocolConfigT>;

        /// @brief Constructs a stream with a next-layer stream.
        explicit stream(next_layer_type&& next_layer_stream);

        /// @brief Gets the executor associated with the stream.
        executor_type get_executor() noexcept { return next_layer_.get_executor(); }

        /// @brief Gets a reference to the lowest layer stream.
        lowest_layer_type& lowest_layer() noexcept { return next_layer_.lowest_layer(); }

        /// @brief Gets a reference to the next layer stream.
        next_layer_type& next_layer() noexcept { return next_layer_; }

        /// @brief Registers handlers for option enablement, disablement, and subnegotiation.
        void register_option_handlers(option::id_num opt, std::optional<fsm_type::OptionEnablementHandler> enable_handler, std::optional<fsm_type::OptionDisablementHandler> disable_handler, std::optional<fsm_type::SubnegotiationHandler> subneg_handler = std::nullopt) { fsm_.register_option_handlers(opt, std::move(enable_handler), std::move(disable_handler), std::move(subneg_handler)); }

        /// @brief Unregisters handlers for an option.
        void unregister_option_handlers(option::id_num opt) { fsm_.unregister_option_handlers(opt); }

        /// @brief Asynchronously requests or offers an option, sending IAC WILL/DO.
        template<typename CompletionToken>
        auto async_request_option(option::id_num opt, NegotiationDirection direction, CompletionToken&& token;

        /// @brief Synchronously requests or offers an option, sending IAC WILL/DO.
        std::size_t request_option(option::id_num opt, NegotiationDirection direction);

        /// @brief Synchronously requests or offers an option, sending IAC WILL/DO.
        std::size_t request_option(option::id_num opt, NegotiationDirection direction, std::error_code& ec) noexcept;

        /// @brief Asynchronously disables an option, sending IAC WONT/DONT.
        template<typename CompletionToken>
        auto async_disable_option(option::id_num opt, NegotiationDirection direction, CompletionToken&& token;

        /// @brief Synchronously disables an option, sending IAC WONT/DONT.
        std::size_t disable_option(option::id_num opt, NegotiationDirection direction);

        /// @brief Synchronously disables an option, sending IAC WONT/DONT.
        std::size_t disable_option(option::id_num opt, NegotiationDirection direction, std::error_code& ec) noexcept;

        /// @brief Asynchronously reads some data with Telnet processing.
        template<MutableBufferSequence MBufSeq, ReadToken CompletionToken>
        auto async_read_some(const MBufSeq& buffers, CompletionToken&& token);

        /// @brief Synchronously reads some data with Telnet processing.
        template<MutableBufferSequence MBufSeq>
        std::size_t read_some(MBufSeq&& buffers);

        /// @brief Synchronously reads some data with Telnet processing.
        template<MutableBufferSequence MBufSeq>
        std::size_t read_some(MBufSeq&& buffers, std::error_code& ec) noexcept;

        /// @brief Asynchronously writes some data with Telnet-specific IAC escaping.
        template<ConstBufferSequence CBufSeq, WriteToken CompletionToken>
        auto async_write_some(const CBufSeq& data, CompletionToken&& token;

        /// @brief Synchronously writes some data with Telnet-specific IAC escaping.
        template<ConstBufferSequence CBufSeq>
        std::size_t write_some(const CBufSeq& data);

        /// @brief Synchronously writes some data with Telnet-specific IAC escaping.
        template<ConstBufferSequence CBufSeq>
        std::size_t write_some(const CBufSeq& data, std::error_code& ec) noexcept;

        /// @brief Asynchronously writes a pre-escaped buffer containing data and raw Telnet commands.
        template<ConstBufferSequence CBufSeq, typename CompletionToken> requires asio::completion_token_for<CompletionToken, typename stream<NextLayerT, ProtocolConfigT>::asio_completion_signature>
        auto async_write_raw(const CBufSeq& data, CompletionToken&& token;

        /// @brief Synchronously writes a pre-escaped buffer containing data and raw Telnet commands.
        template<ConstBufferSequence CBufSeq>
          requires asio::const_buffer_sequence<ConstBufferSequence>
        std::size_t write_raw(const CBufSeq& data);

        /// @brief Synchronously writes a pre-escaped buffer containing data and raw Telnet commands.
        template<ConstBufferSequence CBufSeq>
          requires asio::const_buffer_sequence<ConstBufferSequence>
        std::size_t write_raw(const CBufSeq& data, std::error_code& ec) noexcept;

        /// @brief Asynchronously writes a Telnet command.
        template<WriteToken CompletionToken>
        auto async_write_command(TelnetCommand cmd, CompletionToken&& token;

        /// @brief Synchronously writes a Telnet command.
        std::size_t write_command(TelnetCommand cmd);

        /// @brief Synchronously writes a Telnet command.
        std::size_t write_command(TelnetCommand cmd, std::error_code& ec) noexcept;

        /// @todo Future Development: make this private
        /// @brief Asynchronously writes a Telnet negotiation command with an option.
        template<WriteToken CompletionToken>
        auto async_write_negotiation(typename fsm_type::NegotiationResponse response, CompletionToken&& token;

        /// @brief Synchronously writes a Telnet negotiation command with an option.
        std::size_t write_negotiation(typename fsm_type::NegotiationResponse response);

        /// @brief Synchronously writes a Telnet negotiation command with an option.
        std::size_t write_negotiation(typename fsm_type::NegotiationResponse response, std::error_code& ec) noexcept;

        /// @brief Asynchronously writes a Telnet subnegotiation command.
        template<WriteToken CompletionToken>
        auto async_write_subnegotiation(option opt, const std::vector<byte_t>& subnegotiation_buffer, CompletionToken&& token;

        /// @brief Asynchronously sends Telnet Synch sequence (NUL bytes and IAC DM).
        template<WriteToken CompletionToken>
        auto async_send_synch(CompletionToken&& token;

        /// @brief Synchronously sends Telnet Synch sequence (NUL bytes and IAC DM).
        std::size_t send_synch();

        /// @brief Synchronously sends Telnet Synch sequence (NUL bytes and IAC DM).
        std::size_t send_synch(std::error_code& ec) noexcept;

        /// @brief Asynchronously waits (via 0-byte receive) for notification that OOB data is in the stream.
        void launch_wait_for_urgent_data();

    private:
        /**
         * @brief A private nested struct for holding processing context to share with `InputProcessor`.
         */
        struct context_type {
        private:
            /**
             * @brief A private nested class to track the state of the TCP urgent notification and receipt of `TelnetCommand::DM`.
             */
            class urgent_data_tracker {
            private:
                /// @typedef ProtocolConfig @brief Aliases `fsm_type::ProtocolConfig` to access the error logger.
                using ProtocolConfig = fsm_type::ProtocolConfig;
                /// @brief Scoped enumeration for the urgent data tracking state.
                enum class UrgentDataState : std::byte { NO_URGENT_DATA, HAS_URGENT_DATA, UNEXPECTED_DATA_MARK };
                std::atomic<UrgentDataState> state_{UrgentDataState::NO_URGENT_DATA};
            public:
                /// @brief Updates the state when the OOB notification arrives.
                void saw_urgent();
                /// @brief Updates the state when the `TelnetCommand::DM` arrives.
                void saw_data_mark();
                /// @brief Reports if the urgent notification is currently active.
                bool has_urgent_data() const noexcept { return (state_.load(std::memory_order_relaxed) == UrgentDataState::HAS_URGENT_DATA); }
                /// @brief Implicitly converts to bool reporting if the urgent notification is currently active.
                operator bool() const noexcept { return has_urgent_data(); }
            };
        public:
            side_buffer_type    input_side_buffer;
            side_buffer_type    output_side_buffer;
            std::error_code     deferred_transport_error;
            std::error_code     deferred_processing_signal;
            urgent_data_tracker urgent_data_state;
            std::atomic<bool>   waiting_for_urgent_data{false};
        }; // struct context_type
    
        /**
         * @brief A private nested class template for processing Telnet input asynchronously.
         * @remark Manages the stateful processing of Telnet input, handling reads and negotiation writes using `async_compose`.
         * @see :protocol_fsm for `ProtocolFSM`, :errors for error codes
         */
        template<MutableBufferSequence MBufSeq>
        class InputProcessor {
        public:
            /// @brief Constructs an `InputProcessor` with the parent stream, FSM, and buffers.
            InputProcessor(stream& parent_stream, stream::fsm_type& fsm, context_type& context, MutableBufferSequence buffers);

            /// @brief Asynchronous operation handler for processing Telnet input.
            template<typename Self>
            void operator()(Self& self, std::error_code ec = {}, std::size_t bytes_transferred = 0);

        private:
            /// @brief Completes the asynchronous operation with error code and bytes transferred.
            template<typename Self>
            void complete(Self& self, const std::error_code& ec, std::size_t bytes_transferred);
            
            /// @brief Defers write errors for later reporting and logs multi-error pile-ups.
            void process_write_error(std::error_code `ec`);
            
            /// @brief Handles processing_signal error codes from fsm_.process_byte, modifying the user buffer or urgent data state.
            void process_fsm_signals(std::error_code& `signal_ec`);

            /// @brief Handle a `NegotiationResponse` by initiating an async write of the negotiation.
            template<typename Self>
            void do_response(typename stream::fsm_type::NegotiationResponse response, Self&& self);
    
            /// @brief Handle a `std::string` by initiating an async write of raw data.
            template<typename Self>
            void do_response(std::string response, Self&& self);
    
            /// @brief Handle a `SubnegotiationAwaitable` by spawning a coroutine to process it.
            template<typename Self>
            void do_response(awaitables::SubnegotiationAwaitable awaitable, Self&& self);
    
            /// @brief Handle any `TaggedAwaitable` with optional `NegotiationResponse`.
            template<typename Self, typename Tag, typename T, typename Awaitable>
            void do_response(std::tuple<awaitables::TaggedAwaitable<Tag, T, Awaitable>, std::optional<typename stream::fsm_type::NegotiationResponse>> response, Self&& self);
    
            static inline constexpr std::size_t READ_BLOCK_SIZE = 1024;

            stream& parent_stream_;
            fsm_type& fsm_;
            context_type& context_;
           
            MutableBufferSequence buffers_;
            using iterator_type = asio::buffers_iterator<MutableBufferSequence>;
            iterator_type user_buf_begin_;
            iterator_type user_buf_end_;
            iterator_type write_it_;
            
            enum class State { INITIALIZING, READING, PROCESSING, DONE } state_;
        }; //class InputProcessor

        /// @brief Asynchronously sends a single NUL byte, optionally as OOB.
        template<WriteToken CompletionToken>
        auto async_send_nul(bool out_of_band, CompletionToken&& token;

        /// @brief Synchronously awaits the completion of an awaitable operation.
        template<typename Awaitable>
        static auto sync_await(Awaitable&& a);

        /// @brief Escapes Telnet output data by duplicating 0xFF (IAC) bytes into a provided vector.
        template<ConstBufferSequence CBufSeq>
        std::tuple<std::error_code, std::vector<byte_t>&> escape_telnet_output(std::vector<byte_t>& escaped_data, const CBufSeq& data) const noexcept;

        /// @brief Escapes Telnet output data by duplicating 0xFF (IAC) bytes.
        template<ConstBufferSequence CBufSeq>
        std::tuple<std::error_code, std::vector<byte_t>> escape_telnet_output(const CBufSeq& data) const noexcept;

        /// @brief Asynchronously writes a temporary buffer to the next layer.
        template<WriteToken CompletionToken> 
        auto async_write_temp_buffer(std::vector<byte_t>&& temp_buffer, CompletionToken&& token;

        /// @brief Asynchronously reports an error via the completion token.
        template<WriteToken CompletionToken>
        auto async_report_error(std::error_code ec, CompletionToken&& token;

        next_layer_type next_layer_;
        fsm_type fsm_; // FSM member to maintain state
        context_type context_;
    }; // class stream

    /**
     * @fn explicit stream::stream(next_layer_type&& next_layer_stream)
     * @param next_layer_stream The next-layer stream to wrap.
     * @remark Initializes the `next_layer_` member with the provided stream and constructs the `fsm_` member.
     * @see `LayerableSocketStream` for stream requirements, :protocol_fsm for `fsm_type`, "telnet-stream-impl.cpp" for implementation
     */
    /**
     * @fn executor_type stream::get_executor() noexcept
     * @return The executor of the next-layer stream.
     * @remark Delegates to `next_layer_.get_executor()` to retrieve the associated executor.
     * @see `LayerableSocketStream` for executor requirements
     */
    /**
     * @fn lowest_layer_type& stream::lowest_layer() noexcept
     * @return Reference to the lowest layer stream.
     * @remark Delegates to `next_layer_.lowest_layer()` to access the lowest layer of the stream stack.
     * @see `LayerableSocketStream` for `lowest_layer` requirements
     */
    /**
     * @fn next_layer_type& stream::next_layer() noexcept
     * @return Reference to the next layer stream.
     * @remark Provides direct access to the wrapped `next_layer_` stream.
     */
    /**
     * @fn void stream::register_option_handlers(option::id_num opt, std::optional<fsm_type::OptionEnablementHandler> enable_handler, std::optional<fsm_type::OptionDisablementHandler> disable_handler, std::optional<fsm_type::SubnegotiationHandler> subneg_handler)
     * @param opt The `option::id_num` for which to register handlers.
     * @param enable_handler Optional handler for option enablement.
     * @param disable_handler Optional handler for option disablement.
     * @param subneg_handler Optional handler for subnegotiation data (defaults to `std::nullopt`).
     * @remark Forwards to `fsm_.register_option_handlers` to register handlers for the specified option.
     * @remark Accepts `option::id_num` or `option` via implicit conversion.
     * @see `:protocol_fsm` for `ProtocolFSM`, :options for `option`, "telnet-stream-impl.cpp" for implementation
     */
    /**
     * @fn void stream::unregister_option_handlers(option::id_num opt)
     * @param opt The `option::id_num` for which to unregister handlers.
     * @remark Forwards to `fsm_.unregister_option_handlers` to remove handlers for the specified option.
     * @see `:protocol_fsm` for `ProtocolFSM`, `:options` for `option`, "telnet-stream-impl.cpp" for implementation
     */
    /**
     * @fn auto stream::async_request_option(option::id_num opt, NegotiationDirection direction, CompletionToken&& token)
     * @tparam CompletionToken The type of completion token.
     * @param opt The `option::id_num` to request or offer.
     * @param direction The negotiation direction (`LOCAL` for WILL, `REMOTE` for DO).
     * @param token The completion token (e.g., `asio::use_awaitable`).
     * @return Result type deduced from the completion token.
     * @remark Uses RFC 1143 Q Method via `fsm_.request_option` to validate state and set pending flags, sending `IAC WILL` or `IAC DO` via `async_write_negotiation` if needed.
     * @throws System errors (e.g., `telnet::error::internal_error`) via `async_report_error`.
     * @see RFC 1143, :protocol_fsm for `request_option`, :options for `option::id_num`, :errors for error codes, :types for `NegotiationDirection`, "telnet-stream-async-impl.cpp" for implementation
     */
    /**
     * @fn std::size_t stream::request_option(option::id_num opt, NegotiationDirection direction)
     * @param opt The `option::id_num` to request or offer.
     * @param direction The negotiation direction (`LOCAL` for WILL, `REMOTE` for DO).
     * @return The number of bytes written (typically 3 for IAC WILL/DO + option).
     * @throws std::system_error If an error occurs (e.g., `telnet::error::internal_error`, `telnet::error::option_not_available`).
     * @remark Uses RFC 1143 Q Method via `fsm_.request_option` to validate state and set pending flags, sending `IAC WILL` or `IAC DO` via `sync_await` on `async_request_option`.
     * @see RFC 1143, :protocol_fsm for `request_option`, :options for `option::id_num`, :errors for error codes, :types for `NegotiationDirection`, "telnet-stream-sync-impl.cpp" for implementation
     */
    /**
     * @overload std::size_t stream::request_option(option::id_num opt, NegotiationDirection direction, std::error_code& ec) noexcept
     * @param opt The `option::id_num` to request or offer.
     * @param direction The negotiation direction (`LOCAL` for WILL, `REMOTE` for DO).
     * @param[out] ec The error code to set on failure (e.g., `telnet::error::internal_error`, `telnet::error::option_not_available`).
     * @return The number of bytes written (typically 3 for IAC WILL/DO + option), or 0 on error.
     * @remark Wraps the throwing `request_option` overload, catching exceptions to set `ec` with appropriate error codes.
     * @remark Uses RFC 1143 Q Method via `fsm_.request_option` and `sync_await` on `async_request_option`.
     * @see RFC 1143, :protocol_fsm for `request_option`, :options for `option::id_num`, :errors for error codes, :types for `NegotiationDirection`, "telnet-stream-sync-impl.cpp" for implementation
     */
    /**
     * @fn auto stream::async_disable_option(option::id_num opt, NegotiationDirection direction, CompletionToken&& token)
     * @tparam CompletionToken The type of completion token.
     * @param opt The `option::id_num` to disable.
     * @param direction The negotiation direction (`LOCAL` for WONT, `REMOTE` for DONT).
     * @param token The completion token (e.g., `asio::use_awaitable`).
     * @return Result type deduced from the completion token.
     * @remark Uses RFC 1143 Q Method via `fsm_.disable_option` to validate state and set pending flags, sending `IAC WONT` or `IAC DONT` via `async_write_negotiation` if needed, and awaiting disablement handlers if registered.
     * @throws System errors (e.g., `telnet::error::internal_error`) via `async_report_error`.
     * @see RFC 1143, :protocol_fsm for `disable_option`, :options for `option::id_num`, :errors for error codes, :types for `NegotiationDirection`, :awaitables for `OptionDisablementAwaitable`, "telnet-stream-async-impl.cpp" for implementation
     */
    /**
     * @fn std::size_t stream::disable_option(option::id_num opt, NegotiationDirection direction)
     * @param opt The `option::id_num` to disable.
     * @param direction The negotiation direction (`LOCAL` for WONT, `REMOTE` for DONT).
     * @return The number of bytes written (typically 3 for IAC WONT/DONT + option).
     * @throws std::system_error If an error occurs (e.g., `telnet::error::internal_error`, `telnet::error::option_not_available`).
     * @remark Uses RFC 1143 Q Method via `fsm_.disable_option` to validate state and set pending flags, sending `IAC WONT` or `IAC DONT` via `sync_await` on `async_disable_option`, awaiting disablement handlers if registered.
     * @see RFC 1143, :protocol_fsm for `disable_option`, :options for `option::id_num`, :errors for error codes, :types for `NegotiationDirection`, :awaitables for `OptionDisablementAwaitable`, "telnet-stream-sync-impl.cpp" for implementation
     */
    /**
     * @overload std::size_t stream::disable_option(option::id_num opt, NegotiationDirection direction, std::error_code& ec) noexcept
     * @param opt The `option::id_num` to disable.
     * @param direction The negotiation direction (`LOCAL` for WONT, `REMOTE` for DONT).
     * @param[out] ec The error code to set on failure (e.g., `telnet::error::internal_error`, `telnet::error::option_not_available`).
     * @return The number of bytes written (typically 3 for IAC WONT/DONT + option), or 0 on error.
     * @remark Wraps the throwing `disable_option` overload, catching exceptions to set `ec` with appropriate error codes.
     * @remark Uses RFC 1143 Q Method via `fsm_.disable_option` and `sync_await` on `async_disable_option`, awaiting disablement handlers if registered.
     * @see RFC 1143, :protocol_fsm for `disable_option`, :options for `option::id_num`, :errors for error codes, :types for `NegotiationDirection`, :awaitables for `OptionDisablementAwaitable`, "telnet-stream-sync-impl.cpp" for implementation
     */
    /**
     * @fn auto stream::async_read_some(const MBufSeq& buffers, CompletionToken&& token)
     * @tparam MBufSeq The type of mutable buffer sequence to read into.
     * @tparam CompletionToken The type of completion token.
     * @param buffers The mutable buffer sequence to read into.
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @remark Initiates an asynchronous read from `next_layer_` using `asio::async_compose` with `InputProcessor` to process Telnet data.
     * @remark Binds the operation to the stream’s executor via `get_executor()`.
     * @see `InputProcessor` for processing details, `:protocol_fsm` for `ProtocolFSM`, `:errors` for error codes, "telnet-stream-async-impl.cpp" for implementation
     */
    /**
     * @fn std::size_t stream::read_some(MBufSeq&& buffers)
     * @tparam MBufSeq The type of mutable buffer sequence to read into.
     * @param buffers The mutable buffer sequence to read into.
     * @return The number of bytes read and processed.
     * @throws std::system_error If an error occurs, with the error code from the operation.
     * @remark Directly returns the result of `sync_await` on the asynchronous `async_read_some` operation.
     * @see `sync_await` for synchronous operation, `:errors` for error codes, "telnet-stream-sync-impl.cpp" for implementation
     */
    /**
     * @overload std::size_t stream::read_some(MBufSeq&& buffers, std::error_code& ec) noexcept
     * @tparam MBufSeq The type of mutable buffer sequence to read into.
     * @param buffers The mutable buffer sequence to read into.
     * @param ec The error code to set on failure.
     * @return The number of bytes read and processed, or 0 on error.
     * @remark Wraps the throwing `read_some` overload, catching exceptions to set `ec` with appropriate error codes (e.g., `std::system_error`, `not_enough_memory`, `internal_error`).
     * @see `sync_await` for synchronous operation, `:errors` for error codes, "telnet-stream-sync-impl.cpp" for implementation
     */
    /**
     * @fn auto stream::async_write_some(const CBufSeq& data, CompletionToken&& token)
     * @tparam CBufSeq The type of constant buffer sequence to write.
     * @tparam CompletionToken The type of completion token.
     * @param data The data to write (string or binary).
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @remark Escapes input data using `escape_telnet_output` to duplicate 0xFF (IAC) bytes as per RFC 854, then writes the escaped data to `next_layer_` using `async_write_temp_buffer`.
     * @remark Returns `std::errc::not_enough_memory` or `telnet::error::internal_error` via `async_report_error` if escaping fails.
     * @see `escape_telnet_output` for escaping details, `async_write_temp_buffer` for buffer writing, `:errors` for error codes, RFC 854 for IAC escaping, "telnet-stream-async-impl.cpp" for implementation
     */
    /**
     * @fn std::size_t stream::write_some(const CBufSeq& data)
     * @tparam CBufSeq The type of constant buffer sequence to write.
     * @param data The data to write.
     * @return The number of bytes written.
     * @throws std::system_error If an error occurs, with the error code from the operation.
     * @remark Directly returns the result of `sync_await` on the asynchronous `async_write_some` operation.
     * @see `sync_await` for synchronous operation, `escape_telnet_output` for escaping details, `:errors` for error codes, "telnet-stream-sync-impl.cpp" for implementation
     */
    /**
     * @overload std::size_t stream::write_some(const CBufSeq& data, std::error_code& ec) noexcept
     * @tparam CBufSeq The type of constant buffer sequence to write.
     * @param data The data to write.
     * @param ec The error code to set on failure.
     * @return The number of bytes written, or 0 on error.
     * @remark Wraps the throwing `write_some` overload, catching exceptions to set `ec` with appropriate error codes (e.g., `std::system_error`, `not_enough_memory`, `internal_error`).
     * @see `sync_await` for synchronous operation, `escape_telnet_output` for escaping details, `:errors` for error codes, "telnet-stream-sync-impl.cpp" for implementation
     */
    /**
     * @fn auto stream::async_write_raw(const CBufSeq& data, CompletionToken&& token)
     * @tparam CBufSeq The type of constant buffer sequence to write.
     * @tparam CompletionToken The type of completion token.
     * @param data The pre-escaped buffer containing data (with IAC doubled as 0xFF 0xFF) and raw command bytes (e.g., IAC GA as 0xFF 0xF9).
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @pre The input buffer must be RFC 854 compliant: data bytes of 0xFF must be doubled as 0xFF 0xFF; command sequences (e.g., IAC GA) must be included as raw bytes.
     * @remark Uses `asio::async_write` to transmit raw bytes to `next_layer_`, respecting protocol layering.
     * @remark Binds the operation to the stream’s executor using `asio::bind_executor`.
     * @see RFC 854 for rules on IAC escaping or command byte structuring, `:errors` for error codes, "telnet-stream-async-impl.cpp" for implementation
     */
    /**
     * @fn std::size_t stream::write_raw(const CBufSeq& data)
     * @tparam CBufSeq The type of constant buffer sequence to write.
     * @param data The pre-escaped buffer containing data (with IAC doubled as 0xFF 0xFF) and raw command bytes (e.g., IAC GA as 0xFF 0xF9).
     * @return The number of bytes written.
     * @throws std::system_error If an error occurs, with the error code from the operation.
     * @pre The input buffer must be RFC 854 compliant: data bytes of 0xFF must be doubled as 0xFF 0xFF; command sequences (e.g., IAC GA) must be included as raw bytes.
     * @remark Wraps `async_write_raw` using `sync_await`, propagating exceptions.
     * @remark Used for simple command responses like AYT (e.g., "[YES]\xFF\xF9" for [YES] IAC GA).
     * @remark No validation is performed; callers are responsible for ensuring RFC 854 compliance.
     * @see `async_write_raw` for async implementation, `:errors` for error codes, RFC 854 for IAC escaping, `:protocol_fsm` for AYT response configuration via `set_ayt_response`, "telnet-stream-sync-impl.cpp" for implementation
     */
    /**
     * @overload std::size_t stream::write_raw(const CBufSeq& data, std::error_code& ec) noexcept
     * @tparam CBufSeq The type of constant buffer sequence to write.
     * @param data The pre-escaped buffer containing data (with IAC doubled as 0xFF 0xFF) and raw command bytes (e.g., IAC GA as 0xFF 0xF9).
     * @param ec The error code to set on failure.
     * @return The number of bytes written, or 0 on error.
     * @pre The input buffer must be RFC 854 compliant: data bytes of 0xFF must be doubled as 0xFF 0xFF; command sequences (e.g., IAC GA) must be included as raw bytes.
     * @remark Wraps throwing `write_raw`, converting exceptions to error codes (`std::system_error`, `not_enough_memory`, `internal_error`).
     * @remark Used for simple command responses like AYT (e.g., "[YES]\xFF\xF9" for [YES] IAC GA).
     * @remark No validation is performed; callers are responsible for ensuring RFC 854 compliance.
     * @see `write_raw` for throwing version, `async_write_raw` for async implementation, `:errors` for error codes, RFC 854 for IAC escaping, `:protocol_fsm` for AYT response configuration via `set_ayt_response`, "telnet-stream-sync-impl.cpp" for implementation
     */
    /**
     * @fn auto stream::async_write_command(TelnetCommand cmd, CompletionToken&& token)
     * @tparam CompletionToken The type of completion token.
     * @param cmd The `TelnetCommand` to send.
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @remark Constructs a 2-byte buffer with `{IAC, std::to_underlying(cmd)}` and writes it to `next_layer_` using `asio::async_write`.
     * @remark Binds the operation to the stream’s executor using `asio::bind_executor`.
     * @see `:types` for `TelnetCommand`, `:errors` for error codes, RFC 854 for command structure, "telnet-stream-async-impl.cpp" for implementation
     */
    /**
     * @fn std::size_t stream::write_command(TelnetCommand cmd)
     * @param cmd The `TelnetCommand` to send.
     * @return The number of bytes written.
     * @throws std::system_error If an error occurs, with the error code from the operation.
     * @remark Directly returns the result of `sync_await` on the asynchronous `async_write_command` operation.
     * @see `sync_await` for synchronous operation, :types for `TelnetCommand`, :errors for error codes, "telnet-stream-sync-impl.cpp" for implementation
     */
    /**
     * @overload std::size_t stream::write_command(TelnetCommand cmd, std::error_code& ec) noexcept
     * @param cmd The `TelnetCommand` to send.
     * @param ec The error code to set on failure.
     * @return The number of bytes written, or 0 on error.
     * @remark Wraps the throwing `write_command` overload, catching exceptions to set `ec` with appropriate error codes (e.g., `std::system_error`, `not_enough_memory`, `internal_error`).
     * @see `sync_await` for synchronous operation, :types for `TelnetCommand`, :errors for error codes, "telnet-stream-sync-impl.cpp" for implementation
     */
    /**
     * @fn auto stream::async_write_negotiation(typename fsm_type::NegotiationResponse response, CompletionToken&& token)
     * @tparam CompletionToken The type of completion token.
     * @param response The negotiation response to write.
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @remark Constructs a 3-byte buffer with `{IAC, std::to_underlying(cmd), std::to_underlying(opt)}` and writes it to `next_layer_` using `asio::async_write` if `cmd` is `WILL`, `WONT`, `DO`, or `DONT`.
     * @remark Returns `telnet::error::invalid_negotiation` via `async_report_error` if `cmd` is not `WILL`, `WONT`, `DO`, or `DONT`.
     * @remark Binds the operation to the stream’s executor using `asio::bind_executor`.
     * @see :types for `TelnetCommand`, :options for `option::id_num`, :errors for `invalid_negotiation`, RFC 855 for negotiation, "telnet-stream-async-impl.cpp" for implementation
     */
    /**
     * @fn auto stream::async_write_subnegotiation(option opt, const std::vector<byte_t>& subnegotiation_buffer, CompletionToken&& token)
     * @tparam CompletionToken The type of completion token.
     * @param opt The `option` for the subnegotiation.
     * @param subnegotiation_buffer The data buffer for the subnegotiation.
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @remark Validates that `opt` supports subnegotiation via `opt.supports_subnegotiation()` and is enabled via `fsm_.is_enabled(opt)`.
     * @remark Constructs a buffer by reserving space for `subnegotiation_buffer` plus 10% for escaping and 5 bytes (IAC SB, `opt`, IAC SE), appending IAC SB `opt`, escaping the subnegotiation data with `escape_telnet_output`, and appending IAC SE.
     * @remark Returns `telnet::error::invalid_subnegotiation` if `opt` does not support subnegotiation, `telnet::error::option_not_available` if `opt` is not enabled, `std::errc::not_enough_memory` on allocation failure, or `telnet::error::internal_error` for other exceptions, all via `async_report_error`.
     * @note Subnegotiation overflow is handled by `ProtocolFSM::process_byte` for incoming subnegotiations, not outgoing writes.
     * @see :options for `option` and `option::id_num`, :errors for error codes, `escape_telnet_output` for escaping, :protocol_fsm for `ProtocolFSM`, RFC 855 for subnegotiation, "telnet-stream-async-impl.cpp" for implementation
     */
    /**
     * @fn std::size_t stream::write_subnegotiation(option opt, const std::vector<byte_t>& subnegotiation_buffer)
     * @param opt The `option` for the subnegotiation.
     * @param subnegotiation_buffer The data buffer for the subnegotiation.
     * @return The number of bytes written.
     * @throws std::system_error If an error occurs, with the error code from the operation.
     * @remark Directly returns the result of `sync_await` on the asynchronous `async_write_subnegotiation` operation.
     * @see `sync_await` for synchronous operation, :options for `option` and `option::id_num`, :errors for error codes, "telnet-stream-sync-impl.cpp" for implementation
     */
    /**
     * @overload std::size_t stream::write_subnegotiation(option opt, const std::vector<byte_t>& subnegotiation_buffer, std::error_code& ec) noexcept
     * @param opt The `option` for the subnegotiation.
     * @param subnegotiation_buffer The data buffer for the subnegotiation.
     * @param ec The error code to set on failure.
     * @return The number of bytes written, or 0 on error.
     * @remark Wraps the throwing `write_subnegotiation` overload, catching exceptions to set `ec` with appropriate error codes (e.g., `std::system_error`, `not_enough_memory`, `internal_error`).
     * @see `sync_await` for synchronous operation, :options for `option` and `option::id_num`, :errors for error codes, "telnet-stream-sync-impl.cpp" for implementation
     */
    /**
     * @fn void stream::launch_wait_for_urgent_data()
     * @remark Updated `context_.urgent_data_state` when OOB data is available, enabling Synch mode (RFC 854).
     * @remark Sets `context_.waiting_for_urgent_data` to `true` before `async_wait` and `false` on completion.
     * @see `stream::context_type::urgent_data_state`, `stream::context_type::waiting_for_urgent_data`, RFC 854
     */
    /**
     * @fn auto stream::async_send_synch(CompletionToken&& token)
     * @tparam CompletionToken The type of completion token.
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @remark Sends a Telnet Synch sequence (three NUL bytes, one as OOB, followed by IAC DM) to `next_layer_` using `async_send_nul` and `async_write_command`, as per RFC 854.
     * @remark Binds the operation to the stream’s executor using `asio::bind_executor`.
     * @note Used in response to `abort_output` to flush output and signal urgency, supporting client/server symmetry.
     * @see `async_send_nul` for OOB sending, `async_write_command` for command writing, :types for `TelnetCommand`, :errors for error codes, RFC 854 for Synch procedure, "telnet-stream-async-impl.cpp" for implementation
     */
    /**
     * @fn std::size_t stream::send_synch()
     * @return The number of bytes written.
     * @throws std::system_error If an error occurs, with the error code from the operation.
     * @remark Directly returns the result of `sync_await` on the asynchronous `async_send_synch` operation.
     * @note Used in response to `abort_output` to flush output and signal urgency, supporting client/server symmetry.
     * @see `async_send_synch` for async implementation, `sync_await` for synchronous operation, :types for `TelnetCommand`, :errors for error codes, RFC 854 for Synch procedure, "telnet-stream-sync-impl.cpp" for implementation
     */
    /**
     * @overload std::size_t stream::send_synch(std::error_code& ec) noexcept
     * @param[out] ec The error code to set on failure.
     * @return The number of bytes written, or 0 on error.
     * @remark Wraps the throwing `send_synch` overload, catching exceptions to set `ec` with appropriate error codes (e.g., `asio::error::operation_not_supported`, `std::errc::not_enough_memory`, `telnet::error::internal_error`).
     * @note Used in response to `abort_output` to flush output and signal urgency, supporting client/server symmetry.
     * @see `async_send_synch` for async implementation, `sync_await` for synchronous operation, :types for `TelnetCommand`, :errors for error codes, RFC 854 for Synch procedure, "telnet-stream-sync-impl.cpp" for implementation
     */
    /**
     * @fn stream::InputProcessor::InputProcessor(stream& parent_stream, stream::fsm_type& fsm, context_type& context, MutableBufferSequence buffers)
     * @param parent_stream Reference to the parent `stream` managing the connection.
     * @param fsm Reference to the `ProtocolFSM` for Telnet state management.
     * @param context Reference to the context containing input/output buffers and processing state.
     * @param buffers The mutable buffer sequence to read into.
     * @remark Initializes `parent_stream_`, `fsm_`, `context_`, `buffers_`, and sets `state_` to `INITIALIZING`.
     * @see `stream` for parent_stream, `context_type` for context, "telnet-stream-impl.cpp" for implementation
     */
    /**
     * @fn void stream::InputProcessor::operator()(Self& self, std::error_code ec, std::size_t bytes_transferred)
     * @tparam Self The type of the coroutine self reference.
     * @param self Reference to the coroutine self.
     * @param ec The error code from the previous operation.
     * @param bytes_transferred The number of bytes transferred in the previous operation.
     * @remark Manages state transitions (`INITIALIZING`, `READING`, `PROCESSING`, `DONE`), reading from `next_layer_` and processing bytes via `fsm_.process_byte`.
     * @note Loop guards against user buffer overrun via condition; response pausing consumes before async.
     * @remark Handles negotiation responses, raw writes, command handlers, and subnegotiation handlers via `std::visit` on `ProcessingReturnVariant`.
     * @note Checks `DONE` state early to prevent reentrancy issues after completion.
     * @see :protocol_fsm for `ProtocolFSM` processing, :errors for error codes, :options for `option::id_num`, RFC 854 for Telnet protocol, "telnet-stream-async-impl.cpp" for implementation
     */
    /**
     * @fn void stream::InputProcessor::complete(Self& self, const std::error_code& ec, std::size_t bytes_transferred)
     * @tparam Self The type of the coroutine self reference.
     * @param self Reference to the coroutine self.
     * @param ec The error code to report.
     * @param bytes_transferred The number of bytes to report.
     * @remark Sets `state_` to `DONE` and invokes `self.complete` with `ec` and `bytes_transferred`.
     * @see "telnet-stream-async-impl.cpp" for implementation
     */
    /**
     * @fn void stream::InputProcessor::process_write_error(std::error_code ec)
     * @param ec The error code to defer.
     * @remark Updates `context_.deferred_transport_error` with `ec` if no prior error exists, otherwise logs the new error using `ProtocolConfig::log_error` with the prior error as context.
     * @pre `ec` is a write error from an asynchronous operation (e.g., `async_write_negotiation`).
     * @post If `context_.deferred_transport_error` was set, it is unchanged; otherwise, `context_.deferred_transport_error` is set to `ec`.
     * @see `:errors` for error codes, `:protocol_fsm` for `ProtocolConfig`, "telnet-stream-impl.cpp" for implementation
     */
    /**
     * @fn void stream::InputProcessor::process_fsm_signals(std::error_code& signal_ec)
     * @param[in,out] signal_ec The error code from `fsm_.process_byte`, cleared for handled `processing_signal` values.
     * @remark Handles `processing_signal::carriage_return` by appending `\r` to the user buffer, `processing_signal::erase_character` by decrementing the write iterator if not at the buffer start, `processing_signal::erase_line` by resetting the write iterator to the buffer start, and `processing_signal::data_mark` by updating `context_.urgent_data_state` and relaunching urgent data wait.
     * @remark Unhandled signals (e.g., `processing_signal::abort_output`) are not cleared and propagate to the caller for higher-level notification.
     * @see `:errors` for `processing_signal`, `:protocol_fsm` for `ProtocolFSM`, `:stream` for `context_type`, "telnet-stream-impl.cpp" for implementation, RFC 854 for Telnet protocol
     */     
    /**
     * @fn void stream::InputProcessor::do_response(typename stream::fsm_type::NegotiationResponse response, Self&& self)
     * @tparam Self The type of the coroutine self reference.
     * @param response The negotiation response to write (e.g., `WILL`/`DO`/`WONT`/`DONT` with option).
     * @param self The completion handler to forward.
     * @remark Initiates an asynchronous write of the negotiation response using `async_write_negotiation`.
     * @see `:protocol_fsm` for `NegotiationResponse`, `:errors` for error codes, RFC 855 for negotiation, "telnet-stream-async-impl.cpp" for `async_write_negotiation`
     */
    /**
     * @overload void stream::InputProcessor::do_response(std::string response, Self&& self)
     * @tparam Self The type of the coroutine self reference.
     * @param response The string data to write (pre-escaped).
     * @param self The completion handler to forward.
     * @remark Initiates an asynchronous write of raw data using `async_write_raw`, wrapping the string in a `std::shared_ptr<std::string>` for lifetime management.
     * @see `:errors` for error codes, RFC 854 for IAC escaping, "telnet-stream-async-impl.cpp" for `async_write_raw`
     */
    /**
     * @overload void stream::InputProcessor::do_response(awaitables::SubnegotiationAwaitable awaitable, Self&& self)
     * @tparam Self The type of the coroutine self reference.
     * @param awaitable The subnegotiation awaitable to process.
     * @param self The completion handler to forward.
     * @remark Spawns a coroutine to process the subnegotiation, writing the result via `async_write_subnegotiation` if non-empty.
     * @throws `std::system_error` for system errors, `telnet::error::internal_error` for unexpected exceptions.
     * @see `:awaitables` for `SubnegotiationAwaitable`, `:errors` for error codes, RFC 855 for subnegotiation, "telnet-stream-async-impl.cpp" for `async_write_subnegotiation`
     */
    /**
     * @overload void stream::InputProcessor::do_response(std::tuple<awaitables::TaggedAwaitable<Tag, T, Awaitable>, std::optional<typename stream::fsm_type::NegotiationResponse>> response, Self&& self)
     * @tparam Self The type of the coroutine self reference.
     * @tparam Tag The semantic tag for TaggedAwaitable.
     * @tparam T The underlying value type for TaggedAwaitable.
     * @tparam Awaitable The underlying Awaitable type for TaggedAwaitable.
     * @param response Tuple of awaitable and optional negotiation response.
     * @param self The completion handler to forward.
     * @remark Spawns a coroutine to process the awaitable, optionally writing a negotiation response via `async_write_negotiation`.
     * @throws `std::system_error` for system errors, `telnet::error::internal_error` for unexpected exceptions.
     * @see `:awaitables` for `TaggedAwaitable`, `:protocol_fsm` for `NegotiationResponse`, `:errors` for error codes, RFC 855 for negotiation, "telnet-stream-async-impl.cpp" for `async_write_negotiation`
     */
    /**
     * @fn auto stream::sync_await(Awaitable&& a)
     * @tparam Awaitable The type of awaitable to execute.
     * @param a The awaitable to await.
     * @return The result of the awaitable operation.
     * @remark Uses a temporary `io_context` and a `jthread` to run the awaitable, capturing its result or exception.
     * @warning Incurs overhead from creating a new `io_context` and `jthread` per call, which may impact performance in high-frequency synchronous operations, though this is minimal compared to blocking network I/O latency. Synchronous I/O inherently scales poorly.
     * @throws std::system_error If the operation fails with an error code.
     * @throws std::bad_alloc If memory allocation fails.
     * @throws std::exception For other unexpected errors.
     * @see "telnet-stream-sync-impl.cpp" for implementation
     */
    /**
     * @fn std::tuple<std::error_code, std::vector<byte_t>&> stream::escape_telnet_output(std::vector<byte_t>& escaped_data, const CBufSeq& data) const noexcept
     * @tparam CBufSeq The type of constant buffer sequence to escape.
     * @param escaped_data The vector to store the escaped data.
     * @param data The input data to escape.
     * @return A tuple containing the error code (empty on success) and a reference to the modified `escaped_data` vector.
     * @remark Processes the byte sequence directly using `buffers_iterator` and appends to `escaped_data`, duplicating 0xFF (IAC) bytes and transforming LF to CR LF and CR to CR NUL as required by RFC 854.
     * @remark Sets error code to `std::errc::not_enough_memory` on memory allocation failure or `telnet::error::internal_error` for unexpected exceptions.
     * @see :errors for error codes, RFC 854 for IAC escaping, "telnet-stream-impl.cpp" for implementation
     */
    /**
     * @overload std::tuple<std::error_code, std::vector<byte_t>> stream::escape_telnet_output(const CBufSeq& data) const noexcept
     * @tparam CBufSeq The type of constant buffer sequence to escape.
     * @param data The input data to escape.
     * @return A tuple containing the error code (empty on success) and a vector containing the escaped data.
     * @remark Reserves 10% extra capacity to accommodate escaping and delegates to the overload with a provided vector.
     * @remark Returns an empty vector with `std::errc::not_enough_memory` error on allocation failure or `telnet::error::internal_error` for unexpected exceptions.
     * @see `:errors` for error codes, RFC 854 for IAC escaping, "telnet-stream-impl.cpp" for implementation
     */
    /**
     * @fn auto async_send_nul(bool out_of_band, CompletionToken&& token)
     * @tparam CompletionToken The type of completion token.
     * @param out_of_band Is this byte OOB/Urgent?
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @remark Uses `asio::async_send` to write a NUL byte (`'\0'`) with optional urgent flag.
     * @see `async_send_synch`
     */
    /**
     * @fn auto stream::async_write_temp_buffer(std::vector<byte_t>&& temp_buffer, CompletionToken&& token)
     * @tparam CompletionToken The type of completion token.
     * @param temp_buffer The temporary buffer to write.
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @remark Uses `asio::async_initiate` to write `temp_buffer` to `next_layer_` via `asio::async_write`, moving `temp_buffer` into a lambda to manage its lifetime.
     * @remark Binds the operation to the stream’s executor using `asio::bind_executor`.
     * @note Used by `async_write_some`, `async_write_command`, `async_write_negotiation`, and `async_write_subnegotiation` for writing prepared buffers.
     * @see `async_write_some`, `async_write_command`, `async_write_negotiation`, `async_write_subnegotiation`, :errors for error codes, "telnet-stream-async-impl.cpp" for implementation
     */
    /**
     * @fn auto stream::async_report_error(std::error_code ec, CompletionToken&& token)
     * @tparam CompletionToken The type of completion token.
     * @param ec The error code to report.
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @remark Uses `asio::async_initiate` to invoke the completion handler with `ec` and 0 bytes transferred.
     * @note Used by `async_write_some`, `async_write_command`, `async_write_negotiation`, and `async_write_subnegotiation` to report errors asynchronously.
     * @see :errors for error codes, "telnet-stream-async-impl.cpp" for implementation
     */
} // namespace telnet