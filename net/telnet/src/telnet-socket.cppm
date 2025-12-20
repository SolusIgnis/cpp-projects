/**
 * @file telnet-socket.cppm
 * @version 0.4.0
 * @release_date October 3, 2025
 *
 * @brief Interface for Telnet socket operations.
 * @remark Defines `TelnetSocketConcept` concept to constrain lower-layer socket types.
 * @remark Defines `telnet::socket` class to provide a Telnet-aware socket wrapper around a lower-layer stream-oriented socket.
 * @remark Defines `socket::InputProcessor` for composed Telnet-aware async_read_some.
 * @see Partition implementation units `telnet-socket-impl.cpp`, `telnet-socket-async-impl.cpp`, and `telnet-socket-sync-impl.cpp` for function definitions.
 
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 *
 * @see RFC 854 for Telnet protocol, RFC 855 for option negotiation, :protocol_fsm for `ProtocolFSM`, :types for `TelnetCommand`, :options for `option` and `option::id_num`, :errors for error codes, :internal for implementation classes
 * @todo Phase 5: Consider a method for user code to request/offer an option that validates current state, sets the pending flag in the FSM, and writes the negotiation sequence.
 * @todo Phase 5: Consider making it possible to asynchronously await that method as a composed operation until the remote endpoint responds. (Probably better to have a unified enablement callback instead, but worth evaluating the two approaches.)
 * @todo Phase 6: Hand-tune next-layer socket constraints to exactly match Boost.Asio stream sockets.
 * @todo Phase 6: Add concept for TLS-aware next-layer socket to implement conditional TLS support.
 * @todo Future Development: Monitor `asio::async_result_t` compatibility with evolving Boost.Asio and `std::execution` for potential transition to awaitable or standard async frameworks.
 * @todo Future Development: Consider using `asio::get_associated_executor(token)` for handler execution in `telnet::socket` to support custom executors.
 */
module; // Including Boost.Asio in the Global Module Fragment until importable header units are reliable.
#include <boost/asio.hpp>
namespace asio = boost::asio;

// Module partition interface unit
export module telnet:socket;

import std; // For std::error_code, std::size_t, std::vector, std::same_as

export import :types;        ///< @see telnet-types.cppm for `byte_t` and `TelnetCommand`
export import :errors;       ///< @see telnet-errors.cppm for `telnet::error` codes
export import :options;      ///< @see telnet-options.cppm for `option` and `option::id_num`
export import :protocol_fsm; ///< @see telnet-protocol_fsm.cppm for `ProtocolFSM`

export namespace telnet {
    /**
     * @brief Concept defining the requirements for a socket type compatible with Telnet operations.
     * @remark Only methods common to `boost::asio::ip::tcp::socket` and `boost::asio::ssl::stream<...>` are required directly; other methods must be accessed via `lowest_layer()`.
     * @see RFC 854 for Telnet protocol requirements, `boost::asio::ip::tcp::socket` and `boost::asio::ssl::stream` for socket types
     */
    template<typename SocketT>
    concept TelnetSocketConcept = requires(SocketT s, asio::mutable_buffer mb, asio::const_buffer cb) {
        { s.get_executor() } -> std::same_as<asio::any_io_executor>;
        { s.lowest_layer() } -> std::same_as<typename SocketT::lowest_layer_type&>;
        { s.lowest_layer().is_open() } -> std::same_as<bool>;
        { s.lowest_layer().close() };
        { s.async_read_some(mb, asio::use_awaitable) } -> std::same_as<asio::awaitable<std::tuple<std::error_code, std::size_t>>>;
        { s.async_write_some(cb, asio::use_awaitable) } -> std::same_as<asio::awaitable<std::tuple<std::error_code, std::size_t>>>;
    }; // concept TelnetSocketConcept

    /**
     * @brief Socket class wrapping a next-layer socket with Telnet protocol handling.
     * @remark Implements `TelnetSocketConcept` for use with `ProtocolFSM`.
     * @see :protocol_fsm for `ProtocolFSM`, :types for `TelnetCommand`, :options for `option` and `option::id_num`, :errors for error codes
     */
    template<typename NextLayerSocketT, typename ProtocolConfigT = DefaultProtocolFSMConfig>
      requires TelnetSocketConcept<NextLayerSocketT> && ProtocolFSMConfig<ProtocolConfigT>
    class socket {
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
         * @param CompletionToken The type of the completion token.
         * @remark Deduced as `asio::async_result_t` for the given `CompletionToken` and `asio_completion_signature`.
         */
        template<typename CompletionToken>
        using asio_result_type = asio::async_result_t<std::decay_t<CompletionToken>, asio_completion_signature>;

    public:
        /**
         * @typedef executor_type
         * @brief Executor type for the socket.
         * @remark Matches the executor type of the next-layer socket.
         */
        using executor_type = typename next_layer_type::executor_type;

        /**
         * @typedef lowest_layer_type
         * @brief Type of the lowest layer socket.
         * @remark Provides access to the lowest layer of the socket stack.
         */
        using lowest_layer_type = typename next_layer_type::lowest_layer_type;

        /**
         * @typedef next_layer_type
         * @brief Type of the next-layer socket.
         * @remark Represents the wrapped socket type satisfying `TelnetSocketConcept`.
         */
        using next_layer_type = NextLayerSocketT;

        /**
         * @typedef fsm_type
         * @brief Type of the Telnet protocol finite state machine.
         * @remark Configured with `ProtocolConfigT`, defaults to `DefaultProtocolFSMConfig`.
         */
        using fsm_type = ProtocolFSM<ProtocolConfigT>;

        /// @brief Constructs a socket with a next-layer socket.
        explicit socket(next_layer_type&& socket);

        /// @brief Gets the executor associated with the socket.
        executor_type get_executor() noexcept { return next_layer_.get_executor(); }

        /// @brief Gets a reference to the lowest layer socket.
        lowest_layer_type& lowest_layer() noexcept { return next_layer_.lowest_layer(); }

        /// @brief Gets a reference to the next layer socket.
        next_layer_type& next_layer() noexcept { return next_layer_; }

        /// @brief Registers a handler for a `TelnetCommand`.
        std::error_code register_command_handler(TelnetCommand cmd, typename fsm_type::TelnetCommandHandler handler) {
            return fsm_.register_command_handler(cmd, std::move(handler));
        }

        /// @brief Removes a handler for a `TelnetCommand`.
        void unregister_command_handler(TelnetCommand cmd) {
            fsm_.unregister_command_handler(cmd);
        }

        /// @brief Checks if a handler is registered for a `TelnetCommand`.
        bool is_command_handler_registered(TelnetCommand cmd) const {
            return fsm_.is_command_handler_registered(cmd);
        }

        /// @brief Asynchronously reads some data with Telnet processing.
        template<typename MutableBufferSequence, asio::completion_token_for<asio_completion_signature> CompletionToken>
        auto async_read_some(MutableBufferSequence buffers, CompletionToken&& token)
            -> asio_result_type<CompletionToken>;

        /// @brief Synchronously reads some data with Telnet processing.
        template<typename MutableBufferSequence>
        std::size_t read_some(MutableBufferSequence&& buffers);

        /// @brief Synchronously reads some data with Telnet processing.
        template<typename MutableBufferSequence>
        std::size_t read_some(MutableBufferSequence&& buffers, std::error_code& ec) noexcept;

        /// @brief Asynchronously writes some data with Telnet-specific IAC escaping.
        template<typename ConstBufferSequence, asio::completion_token_for<asio_completion_signature> CompletionToken>
        auto async_write_some(const ConstBufferSequence& data, CompletionToken&& token)
            -> asio_result_type<CompletionToken>;

        /// @brief Synchronously writes some data with Telnet-specific IAC escaping.
        template<typename ConstBufferSequence>
        std::size_t write_some(const ConstBufferSequence& data);

        /// @brief Synchronously writes some data with Telnet-specific IAC escaping.
        template<typename ConstBufferSequence>
        std::size_t write_some(const ConstBufferSequence& data, std::error_code& ec) noexcept;

        /// @brief Asynchronously writes a pre-escaped buffer containing data and raw Telnet commands.
        template<typename ConstBufferSequence, typename CompletionToken> requires asio::completion_token_for<CompletionToken, typename socket<NextLayerSocketT, ProtocolConfigT>::asio_completion_signature>
        auto async_write_raw(const ConstBufferSequence& data, CompletionToken&& token)
            -> asio_result_type<CompletionToken>;

        /// @brief Synchronously writes a pre-escaped buffer containing data and raw Telnet commands.
        template<typename ConstBufferSequence>
          requires asio::const_buffer_sequence<ConstBufferSequence>
        std::size_t write_raw(const ConstBufferSequence& data);

        /// @brief Synchronously writes a pre-escaped buffer containing data and raw Telnet commands.
        template<typename ConstBufferSequence>
          requires asio::const_buffer_sequence<ConstBufferSequence>
        std::size_t write_raw(const ConstBufferSequence& data, std::error_code& ec) noexcept;

        /// @brief Asynchronously writes a Telnet command.
        template<asio::completion_token_for<asio_completion_signature> CompletionToken>
        auto async_write_command(TelnetCommand cmd, CompletionToken&& token)
            -> asio_result_type<CompletionToken>;

        /// @brief Synchronously writes a Telnet command.
        std::size_t write_command(TelnetCommand cmd);

        /// @brief Synchronously writes a Telnet command.
        std::size_t write_command(TelnetCommand cmd, std::error_code& ec) noexcept;

        /// @brief Asynchronously writes a Telnet negotiation command with an option.
        template<asio::completion_token_for<asio_completion_signature> CompletionToken>
        auto async_write_negotiation(TelnetCommand cmd, option::id_num opt, CompletionToken&& token)
            -> asio_result_type<CompletionToken>;

        /// @brief Synchronously writes a Telnet negotiation command with an option.
        std::size_t write_negotiation(TelnetCommand cmd, option::id_num opt);

        /// @brief Synchronously writes a Telnet negotiation command with an option.
        std::size_t write_negotiation(TelnetCommand cmd, option::id_num opt, std::error_code& ec) noexcept;

        /// @brief Asynchronously writes a Telnet subnegotiation command.
        template<asio::completion_token_for<asio_completion_signature> CompletionToken>
        auto async_write_subnegotiation(option opt, const std::vector<byte_t>& subnegotiation_buffer, CompletionToken&& token)
            -> asio_result_type<CompletionToken>;

        /// @brief Synchronously writes a Telnet subnegotiation command.
        std::size_t write_subnegotiation(option opt, const std::vector<byte_t>& subnegotiation_buffer);

        /// @brief Synchronously writes a Telnet subnegotiation command.
        std::size_t write_subnegotiation(option opt, const std::vector<byte_t>& subnegotiation_buffer, std::error_code& ec) noexcept;

        /**
         * @brief A private nested class template for processing Telnet input asynchronously.
         * @remark Manages the stateful processing of Telnet input, handling reads and negotiation writes using `async_compose`.
         * @see :protocol_fsm for `ProtocolFSM`, :errors for error codes
         */
        template<typename MutableBufferSequence>
        class InputProcessor {
        public:
            /// @brief Constructs an `InputProcessor` with the parent socket, FSM, and buffers.
            InputProcessor(socket& parent_socket, socket::fsm_type& fsm, MutableBufferSequence buffers);

            /// @brief Asynchronous operation handler for processing Telnet input.
            template<typename Self>
            void operator()(Self& self, std::error_code ec = {}, std::size_t bytes_transferred = 0);

        private:
            /// @brief Completes the asynchronous operation with error code and bytes transferred.
            template<typename Self>
            void complete(Self& self, const std::error_code& ec, std::size_t bytes_transferred);

            socket& parent_socket_;
            fsm_type& fsm_;
            MutableBufferSequence buffers_;
            using iterator_type = asio::buffers_iterator<MutableBufferSequence>;
            iterator_type buf_begin_;
            iterator_type buf_end_;
            iterator_type write_it_;
            iterator_type read_it_;
            enum class State { INITIALIZING, READING, PROCESSING, DONE } state_;
        }; //class InputProcessor

    private:
        /// @brief Synchronously awaits the completion of an awaitable operation.
        template<typename Awaitable>
        static auto sync_await(Awaitable&& a);

        /// @brief Escapes Telnet output data by duplicating 0xFF (IAC) bytes into a provided vector.
        template<typename ConstBufferSequence>
          requires asio::const_buffer_sequence<ConstBufferSequence>
        std::tuple<std::error_code, std::vector<byte_t>&> escape_telnet_output(std::vector<byte_t>& escaped_data, const ConstBufferSequence& data) const noexcept;

        /// @brief Escapes Telnet output data by duplicating 0xFF (IAC) bytes.
        template<typename ConstBufferSequence>
          requires asio::const_buffer_sequence<ConstBufferSequence>
        std::tuple<std::error_code, std::vector<byte_t>> escape_telnet_output(const ConstBufferSequence& data) const noexcept;

        /// @brief Asynchronously writes a temporary buffer to the next layer.
        template<typename CompletionToken>
          requires asio::completion_token_for<CompletionToken, asio_completion_signature>
        auto async_write_temp_buffer(std::vector<byte_t>&& temp_buffer, CompletionToken&& token)
            -> asio_result_type<CompletionToken>;

        /// @brief Asynchronously reports an error via the completion token.
        template<typename CompletionToken>
          requires asio::completion_token_for<CompletionToken, asio_completion_signature>
        auto async_report_error(std::error_code ec, CompletionToken&& token)
            -> asio_result_type<CompletionToken>;

        next_layer_type next_layer_;
        fsm_type fsm_; // FSM member to maintain state
    }; // class socket

    /**
     * @fn explicit socket::socket(next_layer_type&& socket)
     * @tparam NextLayerSocketT The type of the next-layer socket.
     * @tparam ProtocolConfigT The type of the protocol configuration.
     * @param socket The next-layer socket to wrap.
     * @remark Initializes the `next_layer_` member with the provided socket and constructs the `fsm_` member.
     * @see `TelnetSocketConcept` for socket requirements, :protocol_fsm for `fsm_type`, `telnet-socket-impl.cpp` for implementation
     */
    /**
     * @fn executor_type socket::get_executor() noexcept
     * @return The executor of the next-layer socket.
     * @remark Delegates to `next_layer_.get_executor()` to retrieve the associated executor.
     * @see `TelnetSocketConcept` for executor requirements
     */
    /**
     * @fn lowest_layer_type& socket::lowest_layer() noexcept
     * @return Reference to the lowest layer socket.
     * @remark Delegates to `next_layer_.lowest_layer()` to access the lowest layer of the socket stack.
     * @see `TelnetSocketConcept` for `lowest_layer` requirements
     */
    /**
     * @fn next_layer_type& socket::next_layer() noexcept
     * @return Reference to the next layer socket.
     * @remark Provides direct access to the wrapped `next_layer_` socket.
     */
    /**
     * @fn std::error_code socket::register_command_handler(TelnetCommand cmd, typename fsm_type::TelnetCommandHandler handler)
     * @param cmd The `TelnetCommand` to register a handler for.
     * @param handler The `TelnetCommandHandler` to associate with `cmd`.
     * @return `std::error_code` indicating success or failure (e.g., `error::user_handler_forbidden` for reserved commands).
     * @remark Delegates to `fsm_.register_command_handler` to add the handler to the `ProtocolFSM`’s registry.
     * @see :protocol_fsm for `ProtocolFSM` handler registration, :types for `TelnetCommand`, :errors for error codes
     */
    /**
     * @fn void socket::unregister_command_handler(TelnetCommand cmd)
     * @param cmd The `TelnetCommand` whose handler is to be removed.
     * @remark Delegates to `fsm_.unregister_command_handler` to remove the handler from the `ProtocolFSM`’s registry.
     * @see :protocol_fsm for `ProtocolFSM` handler management
     */
    /**
     * @fn bool socket::is_command_handler_registered(TelnetCommand cmd) const
     * @param cmd The `TelnetCommand` to check.
     * @return `true` if a handler is registered for `cmd`, `false` otherwise.
     * @remark Delegates to `fsm_.is_command_handler_registered` to query the `ProtocolFSM`’s registry.
     * @see :protocol_fsm for `ProtocolFSM` handler management
     */
    /**
     * @fn template<typename MutableBufferSequence, asio::completion_token_for<asio_completion_signature> CompletionToken>
     * auto socket::async_read_some(MutableBufferSequence buffers, CompletionToken&& token) -> asio_result_type<CompletionToken>
     * @tparam MutableBufferSequence The type of mutable buffer sequence to read into.
     * @tparam CompletionToken The type of completion token.
     * @param buffers The mutable buffer sequence to read into.
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @remark Initiates an asynchronous read from `next_layer_` using `asio::async_compose` with `InputProcessor` to process Telnet data.
     * @remark Binds the operation to the socket’s executor via `get_executor()`.
     * @see `InputProcessor` for processing details, :protocol_fsm for `ProtocolFSM`, :errors for error codes, `telnet-socket-async-impl.cpp` for implementation
     */
    /**
     * @fn template<typename MutableBufferSequence> std::size_t socket::read_some(MutableBufferSequence&& buffers)
     * @tparam MutableBufferSequence The type of mutable buffer sequence to read into.
     * @param buffers The mutable buffer sequence to read into.
     * @return The number of bytes read and processed.
     * @throws std::system_error If an error occurs, with the error code from the operation.
     * @remark Directly returns the result of `sync_await` on the asynchronous `async_read_some` operation.
     * @see `sync_await` for synchronous operation, :errors for error codes, `telnet-socket-sync-impl.cpp` for implementation
     */
    /**
     * @fn template<typename MutableBufferSequence> std::size_t socket::read_some(MutableBufferSequence&& buffers, std::error_code& ec) noexcept
     * @tparam MutableBufferSequence The type of mutable buffer sequence to read into.
     * @param buffers The mutable buffer sequence to read into.
     * @param ec The error code to set on failure.
     * @return The number of bytes read and processed, or 0 on error.
     * @remark Wraps the throwing `read_some` overload, catching exceptions to set `ec` with appropriate error codes (e.g., `std::system_error`, `not_enough_memory`, `internal_error`).
     * @see `sync_await` for synchronous operation, :errors for error codes, `telnet-socket-sync-impl.cpp` for implementation
     */
    /**
     * @fn template<typename ConstBufferSequence, asio::completion_token_for<asio_completion_signature> CompletionToken>
     * auto socket::async_write_some(const ConstBufferSequence& data, CompletionToken&& token) -> asio_result_type<CompletionToken>
     * @tparam ConstBufferSequence The type of constant buffer sequence to write.
     * @tparam CompletionToken The type of completion token.
     * @param data The data to write (string or binary).
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @remark Escapes input data using `escape_telnet_output` to duplicate 0xFF (IAC) bytes as per RFC 854, then writes the escaped data to `next_layer_` using `async_write_temp_buffer`.
     * @remark Returns `std::errc::not_enough_memory` or `telnet::error::internal_error` via `async_report_error` if escaping fails.
     * @see `escape_telnet_output` for escaping details, `async_write_temp_buffer` for buffer writing, :errors for error codes, RFC 854 for IAC escaping, `telnet-socket-async-impl.cpp` for implementation
     */
    /**
     * @fn template<typename ConstBufferSequence> std::size_t socket::write_some(const ConstBufferSequence& data)
     * @tparam ConstBufferSequence The type of constant buffer sequence to write.
     * @param data The data to write.
     * @return The number of bytes written.
     * @throws std::system_error If an error occurs, with the error code from the operation.
     * @remark Directly returns the result of `sync_await` on the asynchronous `async_write_some` operation.
     * @see `sync_await` for synchronous operation, `escape_telnet_output` for escaping details, :errors for error codes, `telnet-socket-sync-impl.cpp` for implementation
     */
    /**
     * @fn template<typename ConstBufferSequence> std::size_t socket::write_some(const ConstBufferSequence& data, std::error_code& ec) noexcept
     * @tparam ConstBufferSequence The type of constant buffer sequence to write.
     * @param data The data to write.
     * @param ec The error code to set on failure.
     * @return The number of bytes written, or 0 on error.
     * @remark Wraps the throwing `write_some` overload, catching exceptions to set `ec` with appropriate error codes (e.g., `std::system_error`, `not_enough_memory`, `internal_error`).
     * @see `sync_await` for synchronous operation, `escape_telnet_output` for escaping details, :errors for error codes, `telnet-socket-sync-impl.cpp` for implementation
     */
    /**
     * @fn template<typename ConstBufferSequence, typename CompletionToken> auto socket::async_write_raw(const ConstBufferSequence& data, CompletionToken&& token) -> asio_result_type<CompletionToken>
     * @tparam ConstBufferSequence The type of constant buffer sequence to write.
     * @tparam CompletionToken The type of completion token.
     * @param data The pre-escaped buffer containing data (with IAC doubled as 0xFF 0xFF) and raw command bytes (e.g., IAC GA as 0xFF 0xF9).
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @pre The input buffer must be RFC 854 compliant: data bytes of 0xFF must be doubled as 0xFF 0xFF; command sequences (e.g., IAC GA) must be included as raw bytes.
     * @remark Uses `asio::async_write` to transmit raw bytes to `next_layer_`, respecting protocol layering.
     * @remark Binds the operation to the socket’s executor using `asio::bind_executor`.
     * @see RFC 854 for rules on IAC escaping or command byte structuring, :errors for error codes, `telnet-socket-async-impl.cpp` for implementation
     */
    /**
     * @fn template<typename ConstBufferSequence> std::size_t socket::write_raw(const ConstBufferSequence& data)
     * @tparam ConstBufferSequence The type of constant buffer sequence to write.
     * @param data The pre-escaped buffer containing data (with IAC doubled as 0xFF 0xFF) and raw command bytes (e.g., IAC GA as 0xFF 0xF9).
     * @return The number of bytes written.
     * @throws std::system_error If an error occurs, with the error code from the operation.
     * @pre The input buffer must be RFC 854 compliant: data bytes of 0xFF must be doubled as 0xFF 0xFF; command sequences (e.g., IAC GA) must be included as raw bytes.
     * @remark Wraps `async_write_raw` using `sync_await`, propagating exceptions.
     * @remark Used for simple command responses like AYT (e.g., "[YES]\xFF\xF9" for [YES] IAC GA).
     * @remark No validation is performed; callers are responsible for ensuring RFC 854 compliance.
     * @see `async_write_raw` for async implementation, :errors for error codes, RFC 854 for IAC escaping, :protocol_fsm for AYT response configuration via `set_ayt_response`, `telnet-socket-sync-impl.cpp` for implementation
     */
    /**
     * @fn template<typename ConstBufferSequence> std::size_t socket::write_raw(const ConstBufferSequence& data, std::error_code& ec) noexcept
     * @tparam ConstBufferSequence The type of constant buffer sequence to write.
     * @param data The pre-escaped buffer containing data (with IAC doubled as 0xFF 0xFF) and raw command bytes (e.g., IAC GA as 0xFF 0xF9).
     * @param ec The error code to set on failure.
     * @return The number of bytes written, or 0 on error.
     * @pre The input buffer must be RFC 854 compliant: data bytes of 0xFF must be doubled as 0xFF 0xFF; command sequences (e.g., IAC GA) must be included as raw bytes.
     * @remark Wraps throwing `write_raw`, converting exceptions to error codes (`std::system_error`, `not_enough_memory`, `internal_error`).
     * @remark Used for simple command responses like AYT (e.g., "[YES]\xFF\xF9" for [YES] IAC GA).
     * @remark No validation is performed; callers are responsible for ensuring RFC 854 compliance.
     * @see `write_raw` for throwing version, `async_write_raw` for async implementation, :errors for error codes, RFC 854 for IAC escaping, :protocol_fsm for AYT response configuration via `set_ayt_response`, `telnet-socket-sync-impl.cpp` for implementation
     */
    /**
     * @fn template<asio::completion_token_for<asio_completion_signature> CompletionToken>
     * auto socket::async_write_command(TelnetCommand cmd, CompletionToken&& token) -> asio_result_type<CompletionToken>
     * @tparam CompletionToken The type of completion token.
     * @param cmd The `TelnetCommand` to send.
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @remark Constructs a 2-byte buffer with `{IAC, std::to_underlying(cmd)}` and writes it to `next_layer_` using `asio::async_write`.
     * @remark Binds the operation to the socket’s executor using `asio::bind_executor`.
     * @see :types for `TelnetCommand`, :errors for error codes, RFC 854 for command structure, `telnet-socket-async-impl.cpp` for implementation
     */
    /**
     * @fn std::size_t socket::write_command(TelnetCommand cmd)
     * @param cmd The `TelnetCommand` to send.
     * @return The number of bytes written.
     * @throws std::system_error If an error occurs, with the error code from the operation.
     * @remark Directly returns the result of `sync_await` on the asynchronous `async_write_command` operation.
     * @see `sync_await` for synchronous operation, :types for `TelnetCommand`, :errors for error codes, `telnet-socket-sync-impl.cpp` for implementation
     */
    /**
     * @fn std::size_t socket::write_command(TelnetCommand cmd, std::error_code& ec) noexcept
     * @param cmd The `TelnetCommand` to send.
     * @param ec The error code to set on failure.
     * @return The number of bytes written, or 0 on error.
     * @remark Wraps the throwing `write_command` overload, catching exceptions to set `ec` with appropriate error codes (e.g., `std::system_error`, `not_enough_memory`, `internal_error`).
     * @see `sync_await` for synchronous operation, :types for `TelnetCommand`, :errors for error codes, `telnet-socket-sync-impl.cpp` for implementation
     */
    /**
     * @fn template<asio::completion_token_for<asio_completion_signature> CompletionToken>
     * auto socket::async_write_negotiation(TelnetCommand cmd, option::id_num opt, CompletionToken&& token) -> asio_result_type<CompletionToken>
     * @tparam CompletionToken The type of completion token.
     * @param cmd The `TelnetCommand` to send.
     * @param opt The `option::id_num` to send.
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @remark Constructs a 3-byte buffer with `{IAC, std::to_underlying(cmd), std::to_underlying(opt)}` and writes it to `next_layer_` using `asio::async_write` if `cmd` is `WILL`, `WONT`, `DO`, or `DONT`.
     * @remark Returns `telnet::error::invalid_negotiation` via `async_report_error` if `cmd` is not `WILL`, `WONT`, `DO`, or `DONT`.
     * @remark Binds the operation to the socket’s executor using `asio::bind_executor`.
     * @see :types for `TelnetCommand`, :options for `option::id_num`, :errors for `invalid_negotiation`, RFC 855 for negotiation, `telnet-socket-async-impl.cpp` for implementation
     */
    /**
     * @fn std::size_t socket::write_negotiation(TelnetCommand cmd, option::id_num opt)
     * @param cmd The `TelnetCommand` to send.
     * @param opt The `option::id_num` to send.
     * @return The number of bytes written.
     * @throws std::system_error If an error occurs, with the error code from the operation.
     * @remark Directly returns the result of `sync_await` on the asynchronous `async_write_negotiation` operation.
     * @see `sync_await` for synchronous operation, :types for `TelnetCommand`, :options for `option::id_num`, :errors for error codes, `telnet-socket-sync-impl.cpp` for implementation
     */
    /**
     * @fn std::size_t socket::write_negotiation(TelnetCommand cmd, option::id_num opt, std::error_code& ec) noexcept
     * @param cmd The `TelnetCommand` to send.
     * @param ec The error code to set on failure.
     * @param opt The `option::id_num` to send.
     * @return The number of bytes written, or 0 on error.
     * @remark Wraps the throwing `write_negotiation` overload, catching exceptions to set `ec` with appropriate error codes (e.g., `std::system_error`, `not_enough_memory`, `internal_error`).
     * @see `sync_await` for synchronous operation, :types for `TelnetCommand`, :options for `option::id_num`, :errors for error codes, `telnet-socket-sync-impl.cpp` for implementation
     */
    /**
     * @fn template<asio::completion_token_for<asio_completion_signature> CompletionToken>
     * auto socket::async_write_subnegotiation(option opt, const std::vector<byte_t>& subnegotiation_buffer, CompletionToken&& token) -> asio_result_type<CompletionToken>
     * @tparam CompletionToken The type of completion token.
     * @param opt The `option` for the subnegotiation.
     * @param subnegotiation_buffer The data buffer for the subnegotiation.
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @remark Validates that `opt` supports subnegotiation via `opt.supports_subnegotiation()` and is enabled via `fsm_.is_enabled(opt)`.
     * @remark Constructs a buffer by reserving space for `subnegotiation_buffer` plus 10% for escaping and 5 bytes (IAC SB, `opt`, IAC SE), appending IAC SB `opt`, escaping the subnegotiation data with `escape_telnet_output`, and appending IAC SE.
     * @remark Returns `telnet::error::invalid_subnegotiation` if `opt` does not support subnegotiation, `telnet::error::option_not_available` if `opt` is not enabled, `std::errc::not_enough_memory` on allocation failure, or `telnet::error::internal_error` for other exceptions, all via `async_report_error`.
     * @note Subnegotiation overflow is handled by `ProtocolFSM::process_byte` for incoming subnegotiations, not outgoing writes.
     * @see :options for `option` and `option::id_num`, :errors for error codes, `escape_telnet_output` for escaping, :protocol_fsm for `ProtocolFSM`, RFC 855 for subnegotiation, `telnet-socket-async-impl.cpp` for implementation
     */
    /**
     * @fn std::size_t socket::write_subnegotiation(option opt, const std::vector<byte_t>& subnegotiation_buffer)
     * @param opt The `option` for the subnegotiation.
     * @param subnegotiation_buffer The data buffer for the subnegotiation.
     * @return The number of bytes written.
     * @throws std::system_error If an error occurs, with the error code from the operation.
     * @remark Directly returns the result of `sync_await` on the asynchronous `async_write_subnegotiation` operation.
     * @see `sync_await` for synchronous operation, :options for `option` and `option::id_num`, :errors for error codes, `telnet-socket-sync-impl.cpp` for implementation
     */
    /**
     * @fn std::size_t socket::write_subnegotiation(option opt, const std::vector<byte_t>& subnegotiation_buffer, std::error_code& ec) noexcept
     * @param opt The `option` for the subnegotiation.
     * @param subnegotiation_buffer The data buffer for the subnegotiation.
     * @param ec The error code to set on failure.
     * @return The number of bytes written, or 0 on error.
     * @remark Wraps the throwing `write_subnegotiation` overload, catching exceptions to set `ec` with appropriate error codes (e.g., `std::system_error`, `not_enough_memory`, `internal_error`).
     * @see `sync_await` for synchronous operation, :options for `option` and `option::id_num`, :errors for error codes, `telnet-socket-sync-impl.cpp` for implementation
     */
    /**
     * @fn template<typename MutableBufferSequence> socket::InputProcessor::InputProcessor(socket& parent_socket, socket::fsm_type& fsm, MutableBufferSequence buffers)
     * @tparam MutableBufferSequence The type of mutable buffer sequence to process.
     * @param parent_socket Reference to the parent `socket` managing the connection.
     * @param fsm Reference to the `ProtocolFSM` for Telnet state management.
     * @param buffers The mutable buffer sequence to read into.
     * @remark Initializes `parent_socket_`, `fsm_`, `buffers_`, and sets `state_` to `INITIALIZING`.
     * @see `socket` for parent_socket, :protocol_fsm for `fsm_type`, `telnet-socket-impl.cpp` for implementation
     */
    /**
     * @fn template<typename MutableBufferSequence, typename Self> void socket::InputProcessor::operator()(Self& self, std::error_code ec, std::size_t bytes_transferred)
     * @tparam MutableBufferSequence The type of mutable buffer sequence to process.
     * @tparam Self The type of the coroutine self reference.
     * @param self Reference to the coroutine self.
     * @param ec The error code from the previous operation.
     * @param bytes_transferred The number of bytes transferred in the previous operation.
     * @remark Manages state transitions (`INITIALIZING`, `READING`, `PROCESSING`, `DONE`), reading from `next_layer_` and processing bytes via `fsm_.process_byte`.
     * @remark Handles negotiation responses, raw writes, command handlers, and subnegotiation handlers via `std::visit` on `ProcessingReturnVariant`.
     * @note Checks `DONE` state early to prevent reentrancy issues after completion.
     * @invariant `(write_it_ <= read_it_) implies (write_it_ != buf_end_)` after buffer read.
     * @see :protocol_fsm for `ProtocolFSM` processing, :errors for error codes, :options for `option::id_num`, RFC 854 for Telnet protocol, `telnet-socket-async-impl.cpp` for implementation
     */
    /**
     * @fn template<typename MutableBufferSequence, typename Self> void socket::InputProcessor::complete(Self& self, const std::error_code& ec, std::size_t bytes_transferred)
     * @tparam MutableBufferSequence The type of mutable buffer sequence to process.
     * @tparam Self The type of the coroutine self reference.
     * @param self Reference to the coroutine self.
     * @param ec The error code to report.
     * @param bytes_transferred The number of bytes to report.
     * @remark Sets `state_` to `DONE` and invokes `self.complete` with `ec` and `bytes_transferred`.
     * @see `telnet-socket-async-impl.cpp` for implementation
     */
    /**
     * @fn template<typename Awaitable> auto socket::sync_await(Awaitable&& a)
     * @tparam Awaitable The type of awaitable to execute.
     * @param a The awaitable to await.
     * @return The result of the awaitable operation.
     * @remark Uses a temporary `io_context` and a `jthread` to run the awaitable, capturing its result or exception.
     * @warning Incurs overhead from creating a new `io_context` and `jthread` per call, which may impact performance in high-frequency synchronous operations, though this is minimal compared to blocking network I/O latency. Synchronous I/O inherently scales poorly.
     * @throws std::system_error If the operation fails with an error code.
     * @throws std::bad_alloc If memory allocation fails.
     * @throws std::exception For other unexpected errors.
     * @see `telnet-socket-sync-impl.cpp` for implementation
     */
    /**
     * @fn template<typename ConstBufferSequence> std::tuple<std::error_code, std::vector<byte_t>&> socket::escape_telnet_output(std::vector<byte_t>& escaped_data, const ConstBufferSequence& data) const noexcept
     * @tparam ConstBufferSequence The type of constant buffer sequence to escape.
     * @param escaped_data The vector to store the escaped data.
     * @param data The input data to escape.
     * @return A tuple containing the error code (empty on success) and a reference to the modified `escaped_data` vector.
     * @remark Processes the byte sequence directly using `buffers_iterator` and appends to `escaped_data`, duplicating 0xFF (IAC) bytes as required by RFC 854.
     * @remark Sets error code to `std::errc::not_enough_memory` on memory allocation failure or `telnet::error::internal_error` for unexpected exceptions.
     * @see :errors for error codes, RFC 854 for IAC escaping, `telnet-socket-impl.cpp` for implementation
     */
    /**
     * @fn template<typename ConstBufferSequence> std::tuple<std::error_code, std::vector<byte_t>> socket::escape_telnet_output(const ConstBufferSequence& data) const noexcept
     * @tparam ConstBufferSequence The type of constant buffer sequence to escape.
     * @param data The input data to escape.
     * @return A tuple containing the error code (empty on success) and a vector containing the escaped data.
     * @remark Reserves 10% extra capacity to accommodate escaping and delegates to the overload with a provided vector.
     * @remark Returns an empty vector with `std::errc::not_enough_memory` error on allocation failure or `telnet::error::internal_error` for unexpected exceptions.
     * @see :errors for error codes, RFC 854 for IAC escaping, `telnet-socket-impl.cpp` for implementation
     */
    /**
     * @fn template<typename CompletionToken> auto socket::async_write_temp_buffer(std::vector<byte_t>&& temp_buffer, CompletionToken&& token) -> asio_result_type<CompletionToken>
     * @tparam CompletionToken The type of completion token.
     * @param temp_buffer The temporary buffer to write.
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @remark Uses `asio::async_initiate` to write `temp_buffer` to `next_layer_` via `asio::async_write`, moving `temp_buffer` into a lambda to manage its lifetime.
     * @remark Binds the operation to the socket’s executor using `asio::bind_executor`.
     * @note Used by `async_write_some`, `async_write_command`, `async_write_negotiation`, and `async_write_subnegotiation` for writing prepared buffers.
     * @see `async_write_some`, `async_write_command`, `async_write_negotiation`, `async_write_subnegotiation`, :errors for error codes, `telnet-socket-async-impl.cpp` for implementation
     */
    /**
     * @fn template<typename CompletionToken> auto socket::async_report_error(std::error_code ec, CompletionToken&& token) -> asio_result_type<CompletionToken>
     * @tparam CompletionToken The type of completion token.
     * @param ec The error code to report.
     * @param token The completion token.
     * @return Result type deduced from the completion token.
     * @remark Uses `asio::async_initiate` to invoke the completion handler with `ec` and 0 bytes transferred.
     * @note Used by `async_write_some`, `async_write_command`, `async_write_negotiation`, and `async_write_subnegotiation` to report errors asynchronously.
     * @see :errors for error codes, `telnet-socket-async-impl.cpp` for implementation
     */
} // namespace telnet