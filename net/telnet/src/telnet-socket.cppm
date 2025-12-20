/**
 * @file telnet-socket.cppm
 * @version 0.2.0
 * @release_date September 19, 2025
 * @brief Interface for Telnet socket operations.
 * @remark Implements TelnetSocketConcept for use with ProtocolFSM.
 * @remark To be used with implementation units telnet-socket-impl.cpp, telnet-socket-async-impl.cpp, and telnet-socket-sync-impl.cpp for complex function definitions.
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 * @see RFC 854 for Telnet protocol, RFC 855 for option negotiation, telnet-protocol_fsm.cppm for ProtocolFSM, telnet:types for TelnetCommand, telnet:options for option::id_num, telnet:errors for error codes
 * @todo Future Development: Monitor asio::async_result_t compatibility with evolving Boost.Asio and std::execution for potential transition to awaitable or standard async frameworks.
 */
module; //Including Boost.Asio in the Global Module Fragment until importable header units are reliable.
#include <boost/asio.hpp>
namespace asio = boost::asio;

//Module partition interface unit
export module telnet:socket;

import std;        // For std::error_code, std::size_t, std::vector, std::same_as
import std.compat; // For std::uint8_t

export import :types;        ///< @see telnet-types.cppm for TelnetCommand parameters
export import :options;      ///< @see telnet-options.cppm for option and option::id_num parameters
export import :errors;       ///< @see telnet-errors.cppm for telnet::error codes
export import :protocol_fsm; ///< @see telnet-protocol_fsm.cppm for ProtocolFSM

export namespace telnet {
    /**
     * @brief Concept defining the requirements for a socket type compatible with Telnet operations.
     * @remark Only methods common to `boost::asio::ip::tcp::socket` and `boost::asio::ssl::stream<...>` are required directly;
     * other methods must be accessed via `lowest_layer()`.
     * @see RFC 854 for Telnet protocol requirements, boost::asio::ip::tcp::socket and boost::asio::ssl::stream for socket types
     */
    template<typename SocketT>
    concept TelnetSocketConcept = requires(SocketT s, asio::mutable_buffer mb, asio::const_buffer cb) {
        { s.get_executor() } -> std::same_as<asio::any_io_executor>;
        { s.lowest_layer() } -> std::same_as<typename SocketT::lowest_layer_type&>;
        { s.lowest_layer().is_open() } -> std::same_as<bool>;
        { s.lowest_layer().close() };
        { s.async_read_some(mb, asio::use_awaitable) } -> std::same_as<asio::awaitable<std::tuple<std::error_code, std::size_t>>>;
        { s.async_write_some(cb, asio::use_awaitable) } -> std::same_as<asio::awaitable<std::tuple<std::error_code, std::size_t>>>;
    }; //concept TelnetSocketConcept

    /**
     * @brief Socket class wrapping a next-layer socket with Telnet protocol handling.
     * @remark Implements TelnetSocketConcept for use with ProtocolFSM.
     * @see telnet-protocol_fsm.cppm for ProtocolFSM, telnet:types for TelnetCommand, telnet:options for option::id_num, telnet:errors for error codes
     */
    template<typename NextLayerSocketT, typename ProtocolConfigT = DefaultProtocolFSMConfig>
      requires TelnetSocketConcept<NextLayerSocketT> && ProtocolFSMConfig<ProtocolConfigT>
    class socket {
    private:
        /// Type alias for the Boost.Asio completion handler signature.
        using asio_completion_signature = void(std::error_code, std::size_t);
        /// Template type alias for the async result type based on completion token.
        template<typename CompletionToken>
        using asio_result_type = asio::async_result_t<std::decay_t<CompletionToken>, asio_completion_signature>;

    public:
        /// Type alias for the executor type.
        using executor_type = typename next_layer_type::executor_type;
        /// Type alias for the lowest layer socket type.
        using lowest_layer_type = typename next_layer_type::lowest_layer_type;
        /// Type alias for the next layer socket type.
        using next_layer_type = NextLayerSocketT;
        /// Type alias for the configured Telnet protocol FSM
        using fsm_type = ProtocolFSM<ProtocolConfigT>;

        /**
         * @brief Constructs a socket with a next-layer socket.
         * @param socket The next-layer socket to wrap.
         * @see TelnetSocketConcept for socket requirements
         */
        explicit socket(next_layer_type&& socket);

        /**
         * @brief Gets the executor associated with the socket.
         * @return The executor of the next-layer socket.
         * @see TelnetSocketConcept for executor requirements
         */
        executor_type get_executor() noexcept { return next_layer_.get_executor(); }

        /**
         * @brief Gets a reference to the lowest layer socket.
         * @return Reference to the lowest layer socket.
         * @see TelnetSocketConcept for lowest_layer requirements
         */
        lowest_layer_type& lowest_layer() noexcept { return next_layer_.lowest_layer(); }

        /**
         * @brief Gets a reference to the next layer socket.
         * @return Reference to the next layer socket.
         */
        next_layer_type& next_layer() noexcept { return next_layer_; }

        /**
         * @brief Asynchronously reads some data with Telnet processing.
         * @tparam MutableBufferSequence The type of mutable buffer sequence to read into.
         * @tparam CompletionToken The type of completion token.
         * @param buffers The mutable buffer sequence to read into.
         * @param token The completion token.
         * @return Result type deduced from the completion token.
         * @remark Initiates a read from the underlying layer and processes Telnet data using async_compose.
         * @see InputProcessor for processing details, telnet-protocol_fsm.cppm for ProtocolFSM, telnet:errors for error codes
         */
        template<typename MutableBufferSequence, asio::completion_token_for<asio_completion_signature> CompletionToken>
        auto async_read_some(MutableBufferSequence buffers, CompletionToken&& token)
            -> asio_result_type<CompletionToken>;

        /**
         * @brief Synchronously reads some data with Telnet processing.
         * @tparam MutableBufferSequence The type of mutable buffer sequence to read into.
         * @param buffers The mutable buffer sequence to read into.
         * @param ec The error code to set on failure.
         * @return The number of bytes read and processed.
         * @remark Wraps the throwing overload and catches exceptions to set ec.
         * @see sync_await for synchronous operation, telnet:errors for error codes
         */
        template<typename MutableBufferSequence>
        std::size_t read_some(MutableBufferSequence& buffers, std::error_code& ec) noexcept;

        /**
         * @brief Synchronously reads some data with Telnet processing.
         * @tparam MutableBufferSequence The type of mutable buffer sequence to read into.
         * @param buffers The mutable buffer sequence to read into.
         * @return The number of bytes read and processed.
         * @throws std::system_error If an error occurs, with the error code from the operation.
         * @remark Directly returns the result of `sync_await` on the asynchronous counterpart.
         * @see sync_await for synchronous operation, telnet:errors for error codes
         */
        template<typename MutableBufferSequence>
        std::size_t read_some(MutableBufferSequence& buffers);

        /**
         * @brief Asynchronously writes some data with Telnet-specific IAC escaping.
         * @param data The data to write (string or binary).
         * @param token The completion token.
         * @return Result type deduced from the completion token.
         * @remark Uses the next layer for I/O to respect protocol layering and applies `escape_telnet_output` for IAC escaping.
         * @see escape_telnet_output for escaping details, telnet:errors for error codes, RFC 854 for IAC escaping
         */
        template<typename ConstBufferSequence, asio::completion_token_for<asio_completion_signature> CompletionToken>
        auto async_write_some(const ConstBufferSequence& data, CompletionToken&& token)
            -> asio_result_type<CompletionToken>;

        /**
         * @brief Synchronously writes some data with Telnet-specific IAC escaping.
         * @tparam ConstBufferSequence The type of constant buffer sequence to write.
         * @param data The data to write.
         * @param ec The error code to set on failure.
         * @return The number of bytes written.
         * @remark Wraps the throwing overload and catches exceptions to set ec.
         * @see sync_await for synchronous operation, escape_telnet_output for escaping details, telnet:errors for error codes
         */
        template<typename ConstBufferSequence>
        std::size_t write_some(const ConstBufferSequence& data, std::error_code& ec) noexcept;

        /**
         * @brief Synchronously writes some data with Telnet-specific IAC escaping.
         * @tparam ConstBufferSequence The type of constant buffer sequence to write.
         * @param data The data to write.
         * @return The number of bytes written.
         * @throws std::system_error If an error occurs, with the error code from the operation.
         * @remark Directly returns the result of `sync_await` on the asynchronous counterpart.
         * @see sync_await for synchronous operation, escape_telnet_output for escaping details, telnet:errors for error codes
         */
        template<typename ConstBufferSequence>
        std::size_t write_some(const ConstBufferSequence& data);

        /**
         * @brief Asynchronously writes a Telnet command.
         * @param cmd The Telnet command to send.
         * @param token The completion token.
         * @return Result type deduced from the completion token.
         * @remark Constructs a buffer with {IAC, static_cast<std::uint8_t>(cmd)} and uses the next layer for I/O to respect protocol layering.
         * @see telnet:types for TelnetCommand, telnet:errors for error codes, RFC 854 for command structure
         */
        template<asio::completion_token_for<asio_completion_signature> CompletionToken>
        auto async_write_command(TelnetCommand cmd, CompletionToken&& token)
            -> asio_result_type<CompletionToken>;

        /**
         * @brief Synchronously writes a Telnet command.
         * @param cmd The Telnet command to send.
         * @param ec The error code to set on failure.
         * @return The number of bytes written.
         * @remark Wraps the throwing overload and catches exceptions to set ec.
         * @see sync_await for synchronous operation, telnet:types for TelnetCommand, telnet:errors for error codes
         */
        std::size_t write_command(TelnetCommand cmd, std::error_code& ec) noexcept;

        /**
         * @brief Synchronously writes a Telnet command.
         * @param cmd The Telnet command to send.
         * @return The number of bytes written.
         * @throws std::system_error If an error occurs, with the error code from the operation.
         * @remark Directly returns the result of `sync_await` on the asynchronous counterpart.
         * @see sync_await for synchronous operation, telnet:types for TelnetCommand, telnet:errors for error codes
         */
        std::size_t write_command(TelnetCommand cmd);

        /**
         * @brief Asynchronously writes a Telnet negotiation command with an option.
         * @param cmd The Telnet command to send.
         * @param opt The option::id_num to send.
         * @param token The completion token.
         * @return Result type deduced from the completion token.
         * @remark Constructs a buffer with {IAC, static_cast<std::uint8_t>(cmd), static_cast<std::uint8_t>(opt)} and uses the next layer for I/O to respect protocol layering.
         * @remark Sets error code to `telnet::error::invalid_negotiation` if cmd is not WILL, WONT, DO, or DONT.
         * @see telnet:types for TelnetCommand, telnet:options for option::id_num, telnet:errors for invalid_negotiation, RFC 855 for negotiation
         */
        template<asio::completion_token_for<asio_completion_signature> CompletionToken>
        auto async_write_negotiation(TelnetCommand cmd, option::id_num opt, CompletionToken&& token)
            -> asio_result_type<CompletionToken>;

        /**
         * @brief Synchronously writes a Telnet negotiation command with an option.
         * @param cmd The Telnet command to send.
         * @param opt The option::id_num to send.
         * @param ec The error code to set on failure.
         * @return The number of bytes written.
         * @remark Wraps the throwing overload and catches exceptions to set ec.
         * @see sync_await for synchronous operation, telnet:types for TelnetCommand, telnet:options for option::id_num, telnet:errors for error codes
         */
        std::size_t write_negotiation(TelnetCommand cmd, option::id_num opt, std::error_code& ec) noexcept;

        /**
         * @brief Synchronously writes a Telnet negotiation command with an option.
         * @param cmd The Telnet command to send.
         * @param opt The option::id_num to send.
         * @return The number of bytes written.
         * @throws std::system_error If an error occurs, with the error code from the operation.
         * @remark Directly returns the result of `sync_await` on the asynchronous counterpart.
         * @see sync_await for synchronous operation, telnet:types for TelnetCommand, telnet:options for option::id_num, telnet:errors for error codes
         */
        std::size_t write_negotiation(TelnetCommand cmd, option::id_num opt);

        /**
         * @brief Asynchronously writes a Telnet subnegotiation command.
         * @param opt The option::id_num for the subnegotiation.
         * @param subnegotiation_buffer The data buffer for the subnegotiation.
         * @param token The completion token.
         * @return Result type deduced from the completion token.
         * @remark Validates that the option is enabled via FSM, reserves space for escaped data plus 5 bytes (IAC SB, opt, IAC SE), appends IAC SB opt, calls `escape_telnet_output` to append the escaped data, and appends IAC SE, with error handling for memory allocation.
         * @remark Uses ProtocolFSM to check if the option::id_num is enabled and supports subnegotiation, registering a default option if unregistered.
         * @see telnet:options for option::id_num, telnet:errors for error codes, escape_telnet_output for escaping, telnet-protocol_fsm.cppm for ProtocolFSM, RFC 855 for subnegotiation
         */
        template<asio::completion_token_for<asio_completion_signature> CompletionToken>
        auto async_write_subnegotiation(option::id_num opt, const std::vector<std::uint8_t>& subnegotiation_buffer, CompletionToken&& token)
            -> asio_result_type<CompletionToken>;

        /**
         * @brief Synchronously writes a Telnet subnegotiation command.
         * @param opt The option::id_num for the subnegotiation.
         * @param subnegotiation_buffer The data buffer for the subnegotiation.
         * @param ec The error code to set on failure.
         * @return The number of bytes written.
         * @remark Wraps the throwing overload and catches exceptions to set ec.
         * @see sync_away for synchronous operation, telnet:options for option::id_num, telnet:errors for error codes
         */
        std::size_t write_subnegotiation(option::id_num opt, const std::vector<std::uint8_t>& subnegotiation_buffer, std::error_code& ec) noexcept;

        /**
         * @brief Synchronously writes a Telnet subnegotiation command.
         * @param opt The option::id_num for the subnegotiation.
         * @param subnegotiation_buffer The data buffer for the subnegotiation.
         * @return The number of bytes written.
         * @throws std::system_error If an error occurs, with the error code from the operation.
         * @remark Directly returns the result of `sync_await` on the asynchronous counterpart.
         * @see sync_await for synchronous operation, telnet:options for option::id_num, telnet:errors for error codes
         */
        std::size_t write_subnegotiation(option::id_num opt, const std::vector<std::uint8_t>& subnegotiation_buffer);

        /**
         * @brief A private nested class template for processing Telnet input asynchronously.
         * @tparam MutableBufferSequence The type of mutable buffer sequence to process.
         * @remark Manages the stateful processing of Telnet input, handling reads and negotiation writes using async_compose.
         * @see telnet-protocol_fsm.cppm for ProtocolFSM, telnet:errors for error codes
         */
        template<typename MutableBufferSequence>
        class InputProcessor {
        public:
            /**
             * @brief Constructs an InputProcessor with the parent socket, FSM, and buffers.
             * @param parent_socket Reference to the parent socket managing the connection.
             * @param fsm Reference to the ProtocolFSM for Telnet state management.
             * @param buffers The mutable buffer sequence to read into.
             * @see socket for parent_socket, telnet-protocol_fsm.cppm for fsm_type
             */
            InputProcessor(socket& parent_socket, socket::fsm_type& fsm, MutableBufferSequence buffers);

            /**
             * @brief Asynchronous operation handler for processing Telnet input.
             * @tparam Self The type of the coroutine self reference.
             * @param self Reference to the coroutine self.
             * @param ec The error code from the previous operation.
             * @param bytes_transferred The number of bytes transferred in the previous operation.
             * @remark Manages state transitions (INITIALIZING, READING, PROCESSING, DONE) and processes Telnet bytes.
             * @see telnet-protocol_fsm.cppm for ProtocolFSM processing, telnet:errors for error codes
             */
            template<typename Self>
            void operator()(Self& self, std::error_code ec = {}, std::size_t bytes_transferred = 0);

        private:
            /**
             * @brief Completes the asynchronous operation with error code and bytes transferred.
             * @tparam Self The type of the coroutine self reference.
             * @param self Reference to the coroutine self.
             * @param ec The error code to report.
             * @param bytes_transferred The number of bytes to report.
             * @remark Sets state to DONE and calls self.complete.
             */
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
        };

    private:
        /**
         * @brief Synchronously awaits the completion of an awaitable operation.
         * @tparam Awaitable The type of awaitable to execute.
         * @param a The awaitable to await.
         * @return The result of the awaitable operation.
         * @remark Uses a temporary io_context and a jthread to run the awaitable, capturing its result or exception.
         * @remark The return type of the awaitable determines the promise/future type, handling both void and non-void results.
         * @see telnet-socket-sync-impl.cpp for implementation
         */
        template<typename Awaitable>
        static auto sync_await(Awaitable&& a);

        /**
         * @brief Escapes Telnet output data by duplicating 0xFF (IAC) bytes into a provided vector.
         * @tparam ConstBufferSequence The type of constant buffer sequence to escape.
         * @param escaped_data The vector to store the escaped data.
         * @param data The input data to escape.
         * @return A tuple containing the error code (empty on success) and a reference to the modified escaped_data vector.
         * @remark Processes the byte sequence directly using buffers_iterator and appends to escaped_data. Sets error code to not_enough_memory on memory allocation failure or internal_error for unexpected exceptions.
         * @see telnet:errors for error codes, RFC 854 for IAC escaping, telnet-socket-impl.cpp for implementation
         */
        template<typename ConstBufferSequence>
          requires asio::const_buffer_sequence<ConstBufferSequence>
        std::tuple<std::error_code, std::vector<std::uint8_t>&> escape_telnet_output(std::vector<std::uint8_t>& escaped_data, const ConstBufferSequence& data) const noexcept;

        /**
         * @brief Escapes Telnet output data by duplicating 0xFF (IAC) bytes.
         * @tparam ConstBufferSequence The type of constant buffer sequence to escape.
         * @param data The input data to escape.
         * @return A tuple containing the error code (empty on success) and a vector containing the escaped data.
         * @remark Reserves 10% extra capacity to accommodate escaping and delegates to the overload with a provided vector. Returns an empty vector with not_enough_memory error if allocation fails or internal_error for unexpected exceptions.
         * @see telnet:errors for error codes, RFC 854 for IAC escaping, telnet-socket-impl.cpp for implementation
         */
        template<typename ConstBufferSequence>
          requires asio::const_buffer_sequence<ConstBufferSequence>
        std::tuple<std::error_code, std::vector<std::uint8_t>> escape_telnet_output(const ConstBufferSequence& data) const noexcept;

        /**
         * @brief Asynchronously writes a temporary buffer to the next layer.
         * @param temp_buffer The temporary buffer to write.
         * @param token The completion token.
         * @return Result type deduced from the completion token.
         * @remark Used by async_write_some, async_write_command, async_write_negotiation, and async_write_subnegotiation to write prepared buffers.
         * @see telnet-socket-async-impl.cpp for implementation
         */
        template<typename CompletionToken>
          requires asio::completion_token_for<CompletionToken, asio_completion_signature>
        auto async_write_temp_buffer(std::vector<std::uint8_t>&& temp_buffer, CompletionToken&& token)
            -> asio_result_type<CompletionToken>;

        /**
         * @brief Asynchronously reports an error via the completion token.
         * @param ec The error code to report.
         * @param token The completion token.
         * @return Result type deduced from the completion token.
         * @remark Used to report errors asynchronously in async_write_some, async_write_command, async_write_negotiation, and async_write_subnegotiation.
         * @see telnet:errors for error codes, telnet-socket-async-impl.cpp for implementation
         */
        template<typename CompletionToken>
          requires asio::completion_token_for<CompletionToken, asio_completion_signature>
        auto async_report_error(std::error_code ec, CompletionToken&& token)
            -> asio_result_type<CompletionToken>;

        next_layer_type next_layer_;
        fsm_type fsm_; // FSM member to maintain state
    }; //class socket
} //namespace telnet