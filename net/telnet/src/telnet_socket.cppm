/** 
 * @file telnet_socket.cppm
 * @version 0.1.0
 * @release_date September 14, 2025
 * @brief Socket implementation for Telnet protocol operations.
 * @note Implements TelnetSocketConcept for use with ProtocolFSM.
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 */

export module telnet:telnet_socket;

import std; // For std::string, std::shared_ptr, std::vector, std::errc, std::tuple, std::system_error
import std.compat; // For compatibility types
import :telnet;    // Import Telnet protocol definitions

// Use #include for Boost.Asio (no official module support as of September 4, 2025)
#include <boost/asio.hpp>

// Namespace alias for convenience (not exported)
namespace asio = boost::asio;

export namespace telnet {
    /** 
     * @brief Socket class wrapping a lower-layer socket with Telnet protocol handling.
     * @note Implements TelnetSocketConcept for use with ProtocolFSM.
     */
    template<typename LowerLayerSocketT>
    requires TelnetSocketConcept<LowerLayerSocketT>
    class socket {
    public:
        /// Type alias for the executor type.
        using executor_type = typename next_layer_type::executor_type;
        /// Type alias for the lowest layer socket type.
        using lowest_layer_type = typename next_layer_type::lowest_layer_type;
        /// Type alias for the next layer socket type.
        using next_layer_type = LowerLayerSocketT;

        /** 
         * @brief Constructs a socket with a lower-layer socket.
         * @param socket The lower-layer socket to wrap.
         */
        explicit socket(next_layer_type socket) : next_layer_(std::move(socket)), fsm_() {}

        /// Gets the executor associated with the socket.
        executor_type get_executor() { return next_layer_.get_executor(); }

        /// Gets a reference to the lowest layer socket.
        lowest_layer_type& lowest_layer() { return next_layer_.lowest_layer(); }

        /// Gets a reference to the next layer socket.
        next_layer_type& next_layer() { return next_layer_; }

        /** 
         * @brief Asynchronously reads some data with Telnet processing.
         * @tparam MutableBufferSequence The type of mutable buffer sequence to read into.
         * @tparam CompletionToken The type of completion token.
         * @param buffers The mutable buffer sequence to read into.
         * @param token The completion token.
         * @return Result type deduced from the completion token.
         * @note Initiates a read from the underlying layer and processes Telnet data afterward.
         */
        template<typename MutableBufferSequence, asio::completion_token_for<void(std::error_code, std::size_t)> CompletionToken>
        auto async_read_some(MutableBufferSequence buffers, CompletionToken&& token)
            -> asio::async_result_t<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>
        {
            return asio::async_initiate<CompletionToken, void(std::error_code, std::size_t)>(
                InputProcessor<MutableBufferSequence, asio::default_completion_token_t<executor_type>>::create(*this, std::move(buffers)),
                token);
        }

        /** 
         * @brief Synchronously reads some data with Telnet processing.
         * @tparam MutableBufferSequence The type of mutable buffer sequence to read into.
         * @param buffers The mutable buffer sequence to read into.
         * @param ec The error code to set on failure.
         * @return The number of bytes read and processed.
         * @note Wraps the throwing overload and catches exceptions to set ec.
         */
        template<typename MutableBufferSequence>
        std::size_t read_some(MutableBufferSequence&& buffers, std::error_code& ec) noexcept {
            try {
                return read_some(std::forward<MutableBufferSequence>(buffers));
            }
            catch (const std::system_error& e) {
                ec = e.code();
                return 0;
            }
            catch (const std::bad_alloc& e) {
                ec = std::make_error_code(std::errc::not_enough_memory);
                return 0;
            }
            catch (...) {
                ec = std::make_error_code(asio::error::operation_aborted);
                return 0;
            }
        }

        /** 
         * @brief Synchronously reads some data with Telnet processing.
         * @tparam MutableBufferSequence The type of mutable buffer sequence to read into.
         * @param buffers The mutable buffer sequence to read into.
         * @return The number of bytes read and processed.
         * @throws std::system_error If an error occurs, with the error code from the operation.
         * @note Directly returns the result of `sync_await` on the asynchronous counterpart.
         */
        template<typename MutableBufferSequence>
        std::size_t read_some(MutableBufferSequence&& buffers) {
            return sync_await(async_read_some(std::forward<MutableBufferSequence>(buffers), asio::use_awaitable));
        }

        /** 
         * @brief Asynchronously writes some data with Telnet-specific IAC escaping.
         * @param data The data to write (string or binary).
         * @param token The completion token.
         * @return Result type deduced from the completion token.
         * @note Uses the next layer for I/O to respect protocol layering and applies `escape_telnet_output` for IAC escaping.
         */
        template<typename ConstBufferSequence, asio::completion_token_for<void(std::error_code, std::size_t)> CompletionToken>
        auto async_write_some(ConstBufferSequence data, CompletionToken&& token)
            -> asio::async_result_t<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>
        {
            auto [ec, escaped_data] = escape_telnet_output(data);
            if (ec) {
                return asio::async_initiate<CompletionToken, void(std::error_code, std::size_t)>(
                    [ec](auto handler) {
                        handler(ec, 0); // Propagate the error to the handler
                    }, token);
            }
            return asio::async_initiate<CompletionToken, void(std::error_code, std::size_t)>(
                [this, escaped_data = std::move(escaped_data)](auto handler) mutable { // Changed to std::move
                    asio::async_write(this->next_layer_, asio::buffer(escaped_data),
                        asio::bind_executor(this->get_executor(),
                            [handler](const std::error_code& ec, std::size_t bytes_written) mutable {
                                handler(ec, bytes_written);
                            })
                    );
                }, token);
        }

        /** 
         * @brief Synchronously writes some data with Telnet-specific IAC escaping.
         * @tparam ConstBufferSequence The type of constant buffer sequence to write.
         * @param data The data to write.
         * @param ec The error code to set on failure.
         * @return The number of bytes written.
         * @note Wraps the throwing overload and catches exceptions to set ec.
         */
        template<typename ConstBufferSequence>
        std::size_t write_some(ConstBufferSequence&& data, std::error_code& ec) noexcept {
            try {
                return write_some(std::forward<ConstBufferSequence>(data));
            }
            catch (const std::system_error& e) {
                ec = e.code();
                return 0;
            }
            catch (const std::bad_alloc& e) {
                ec = std::make_error_code(std::errc::not_enough_memory);
                return 0;
            }
            catch (...) {
                ec = std::make_error_code(asio::error::operation_aborted);
                return 0;
            }
        }

        /** 
         * @brief Synchronously writes some data with Telnet-specific IAC escaping.
         * @tparam ConstBufferSequence The type of constant buffer sequence to write.
         * @param data The data to write.
         * @return The number of bytes written.
         * @throws std::system_error If an error occurs, with the error code from the operation.
         * @note Directly returns the result of `sync_await` on the asynchronous counterpart.
         */
        template<typename ConstBufferSequence>
        std::size_t write_some(ConstBufferSequence&& data) {
            return sync_await(async_write_some(std::forward<ConstBufferSequence>(data), asio::use_awaitable));
        }

        /** 
         * @brief Asynchronously writes a Telnet command.
         * @param cmd The Telnet command to send.
         * @param token The completion token.
         * @return Result type deduced from the completion token.
         * @note Constructs a buffer with {IAC, static_cast<uint8_t>(cmd)} and uses the next layer for I/O to respect protocol layering.
         */
        template<asio::completion_token_for<void(std::error_code, std::size_t)> CompletionToken>
        auto async_write_command(TelnetCommand cmd, CompletionToken&& token)
            -> asio::async_result_t<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>
        {
            std::array<uint8_t, 2> command_buffer = {
                static_cast<uint8_t>(TelnetCommand::IAC),
                static_cast<uint8_t>(cmd)
            };
            return asio::async_initiate<CompletionToken, void(std::error_code, std::size_t)>(
                [this, command_buffer](auto handler) {
                    asio::async_write(this->next_layer_, asio::buffer(command_buffer),
                        asio::bind_executor(this->get_executor(),
                            [handler](const std::error_code& ec, std::size_t bytes_written) mutable {
                                handler(ec, bytes_written);
                            })
                    );
                }, token);
        }

        /** 
         * @brief Synchronously writes a Telnet command.
         * @param cmd The Telnet command to send.
         * @param ec The error code to set on failure.
         * @return The number of bytes written.
         * @note Wraps the throwing overload and catches exceptions to set ec.
         */
        std::size_t write_command(TelnetCommand&& cmd, std::error_code& ec) noexcept {
            try {
                return write_command(std::forward<TelnetCommand>(cmd));
            }
            catch (const std::system_error& e) {
                ec = e.code();
                return 0;
            }
            catch (const std::bad_alloc& e) {
                ec = std::make_error_code(std::errc::not_enough_memory);
                return 0;
            }
            catch (...) {
                ec = std::make_error_code(asio::error::operation_aborted);
                return 0;
            }
        }

        /** 
         * @brief Synchronously writes a Telnet command.
         * @param cmd The Telnet command to send.
         * @return The number of bytes written.
         * @throws std::system_error If an error occurs, with the error code from the operation.
         * @note Directly returns the result of `sync_await` on the asynchronous counterpart.
         */
        std::size_t write_command(TelnetCommand&& cmd) {
            return sync_await(async_write_command(std::forward<TelnetCommand>(cmd), asio::use_awaitable));
        }

        /** 
         * @brief Asynchronously writes a Telnet negotiation command with an option.
         * @param cmd The Telnet command to send.
         * @param opt The Telnet option to send.
         * @param token The completion token.
         * @return Result type deduced from the completion token.
         * @note Constructs a buffer with {IAC, static_cast<uint8_t>(cmd), static_cast<uint8_t>(opt)} and uses the next layer for I/O to respect protocol layering.
         * @note Sets error code to `std::errc::invalid_argument` if cmd is not WILL, WONT, DO, or DONT.
         */
        template<asio::completion_token_for<void(std::error_code, std::size_t)> CompletionToken>
        auto async_write_negotiation(TelnetCommand cmd, TelnetOption opt, CompletionToken&& token)
            -> asio::async_result_t<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>
        {
            // Validate command is one of WILL, WONT, DO, or DONT
            if (cmd != TelnetCommand::WILL && cmd != TelnetCommand::WONT &&
                cmd != TelnetCommand::DO && cmd != TelnetCommand::DONT) {
                return asio::async_initiate<CompletionToken, void(std::error_code, std::size_t)>(
                    [cmd](auto handler) {
                        handler(std::make_error_code(std::errc::invalid_argument), 0);
                    }, token);
            }

            std::array<uint8_t, 3> command_buffer = {
                static_cast<uint8_t>(TelnetCommand::IAC),
                static_cast<uint8_t>(cmd),
                static_cast<uint8_t>(opt)
            };
            return asio::async_initiate<CompletionToken, void(std::error_code, std::size_t)>(
                [this, command_buffer](auto handler) {
                    asio::async_write(this->next_layer_, asio::buffer(command_buffer),
                        asio::bind_executor(this->get_executor(),
                            [handler](const std::error_code& ec, std::size_t bytes_written) mutable {
                                handler(ec, bytes_written);
                            })
                    );
                }, token);
        }

        /** 
         * @brief Synchronously writes a Telnet negotiation command with an option.
         * @param cmd The Telnet command to send.
         * @param opt The Telnet option to send.
         * @param ec The error code to set on failure.
         * @return The number of bytes written.
         * @note Wraps the throwing overload and catches exceptions to set ec.
         */
        std::size_t write_negotiation(TelnetCommand&& cmd, TelnetOption&& opt, std::error_code& ec) noexcept {
            try {
                return write_negotiation(std::forward<TelnetCommand>(cmd), std::forward<TelnetOption>(opt));
            }
            catch (const std::system_error& e) {
                ec = e.code();
                return 0;
            }
            catch (const std::bad_alloc& e) {
                ec = std::make_error_code(std::errc::not_enough_memory);
                return 0;
            }
            catch (...) {
                ec = std::make_error_code(asio::error::operation_aborted);
                return 0;
            }
        }

        /** 
         * @brief Synchronously writes a Telnet negotiation command with an option.
         * @param cmd The Telnet command to send.
         * @param opt The Telnet option to send.
         * @return The number of bytes written.
         * @throws std::system_error If an error occurs, with the error code from the operation.
         * @note Directly returns the result of `sync_await` on the asynchronous counterpart.
         */
        std::size_t write_negotiation(TelnetCommand&& cmd, TelnetOption&& opt) {
            return sync_await(async_write_negotiation(std::forward<TelnetCommand>(cmd), std::forward<TelnetOption>(opt), asio::use_awaitable));
        }

        /** 
         * @brief Asynchronously writes a Telnet subnegotiation command.
         * @param opt The Telnet option for the subnegotiation.
         * @param subnegotiation_buffer The data buffer for the subnegotiation.
         * @param token The completion token.
         * @return Result type deduced from the completion token.
         * @note Validates that the option is enabled via FSM, reserves space for escaped data plus 5 bytes (IAC SB, opt, IAC SE), appends IAC SB opt, calls `escape_telnet_output` to append the escaped data, and appends IAC SE, with error handling for memory allocation.
         */
        template<asio::completion_token_for<void(std::error_code, std::size_t)> CompletionToken>
        auto async_write_subnegotiation(TelnetOption opt, const std::vector<uint8_t>& subnegotiation_buffer, CompletionToken&& token)
            -> asio::async_result_t<std::decay_t<CompletionToken>, void(std::error_code, std::size_t)>
        {
            std::error_code ec;
            if (!fsm_.is_enabled(opt)) {
                ec = std::make_error_code(std::errc::invalid_argument);
                return asio::async_initiate<CompletionToken, void(std::error_code, std::size_t)>(
                    [ec](auto handler) {
                        handler(ec, 0);
                    }, token);
            }

            std::vector<uint8_t> escaped_buffer;
            try {
                // Reserve space for the original size plus 10% for escaping and 5 bytes for framing (IAC SB, opt, IAC SE)
                escaped_buffer.reserve(subnegotiation_buffer.size() * 1.1 + 5);

                // Append initial framing: IAC SB opt
                escaped_buffer.push_back(static_cast<uint8_t>(TelnetCommand::IAC));
                escaped_buffer.push_back(static_cast<uint8_t>(TelnetCommand::SB));
                escaped_buffer.push_back(static_cast<uint8_t>(opt));

                // Escape the subnegotiation data
                auto [ec, _] = escape_telnet_output(escaped_buffer, subnegotiation_buffer);
                if (ec) {
                    return asio::async_initiate<CompletionToken, void(std::error_code, std::size_t)>(
                        [ec](auto handler) {
                            handler(ec, 0);
                        }, token);
                }

                // Append final framing: IAC SE
                escaped_buffer.push_back(static_cast<uint8_t>(TelnetCommand::IAC));
                escaped_buffer.push_back(static_cast<uint8_t>(TelnetCommand::SE));
            } catch (const std::bad_alloc&) {
                return asio::async_initiate<CompletionToken, void(std::error_code, std::size_t)>(
                    [](auto handler) {
                        handler(std::make_error_code(std::errc::not_enough_memory), 0);
                    }, token);
            } catch (...) {
                return asio::async_initiate<CompletionToken, void(std::error_code, std::size_t)>(
                    [](auto handler) {
                        handler(asio::error::make_error_code(asio::error::operation_aborted), 0);
                    }, token);
            }

            return asio::async_initiate<CompletionToken, void(std::error_code, std::size_t)>(
                [this, escaped_buffer = std::move(escaped_buffer)](auto handler) mutable { // Changed to std::move
                    asio::async_write(this->next_layer_, asio::buffer(escaped_buffer),
                        asio::bind_executor(this->get_executor(),
                            [handler](const std::error_code& ec, std::size_t bytes_written) mutable {
                                handler(ec, bytes_written);
                            })
                    );
                }, token);
        }

        /** 
         * @brief Synchronously writes a Telnet subnegotiation command.
         * @param opt The Telnet option for the subnegotiation.
         * @param subnegotiation_buffer The data buffer for the subnegotiation.
         * @param ec The error code to set on failure.
         * @return The number of bytes written.
         * @note Wraps the throwing overload and catches exceptions to set ec.
         */
        std::size_t write_subnegotiation(TelnetOption&& opt, std::vector<uint8_t>&& subnegotiation_buffer, std::error_code& ec) noexcept {
            try {
                return write_subnegotiation(std::forward<TelnetOption>(opt), std::forward<std::vector<uint8_t>>(subnegotiation_buffer));
            }
            catch (const std::system_error& e) {
                ec = e.code();
                return 0;
            }
            catch (const std::bad_alloc& e) {
                ec = std::make_error_code(std::errc::not_enough_memory);
                return 0;
            }
            catch (...) {
                ec = std::make_error_code(asio::error::operation_aborted);
                return 0;
            }
        }

        /** 
         * @brief Synchronously writes a Telnet subnegotiation command.
         * @param opt The Telnet option for the subnegotiation.
         * @param subnegotiation_buffer The data buffer for the subnegotiation.
         * @return The number of bytes written.
         * @throws std::system_error If an error occurs, with the error code from the operation.
         * @note Directly returns the result of `sync_await` on the asynchronous counterpart.
         */
        std::size_t write_subnegotiation(TelnetOption&& opt, std::vector<uint8_t>&& subnegotiation_buffer) {
            return sync_await(async_write_subnegotiation(std::forward<TelnetOption>(opt), std::forward<std::vector<uint8_t>>(subnegotiation_buffer), asio::use_awaitable));
        }

    private:
        next_layer_type next_layer_;
        ProtocolFSM<next_layer_type> fsm_; // FSM member to maintain state

        /** 
         * @brief Synchronously awaits the completion of an awaitable operation.
         * @tparam Awaitable The type of awaitable to execute.
         * @param a The awaitable to await.
         * @return The result of the awaitable operation.
         * @note Uses a temporary io_context and a jthread to run the awaitable, capturing its result or exception.
         * @note The return type of the awaitable determines the promise/future type, handling both void and non-void results.
         */
        template<typename Awaitable>
        static auto sync_await(Awaitable&& a) {
            using ResultType = typename asio::awaitable_traits<std::decay_t<Awaitable>>::return_type;
            asio::io_context temp_ctx;
            std::promise<ResultType> promise;
            std::future<ResultType> future = promise.get_future();
            
            asio::co_spawn(
                temp_ctx,
                std::forward<Awaitable>(a),
                [&promise](std::exception_ptr e, auto... result) {
                    if (e) {
                        promise.set_exception(e);
                    } else {
                        if constexpr (std::is_void_v<ResultType>) {
                            promise.set_value();
                        } else {
                            promise.set_value(std::get<0>(std::make_tuple(result...)));
                        }
                    }
                }
            );
            
            std::jthread runner_thread([&temp_ctx]{ temp_ctx.run(); });
            return future.get();
        }

        /** 
         * @brief Escapes Telnet output data by duplicating 0xFF (IAC) bytes into a provided vector.
         * @tparam ConstBufferSequence The type of constant buffer sequence to escape.
         * @param escaped_data The vector to store the escaped data.
         * @param data The input data to escape.
         * @return A tuple containing the error code (empty on success) and a reference to the modified escaped_data vector.
         * @note Processes the byte sequence directly using buffers_iterator and appends to escaped_data. Sets error code to not_enough_memory on memory allocation failure or operation_aborted for unexpected exceptions.
         * @requires The type `ConstBufferSequence` must model `asio::const_buffer_sequence`.
         */
        template<typename ConstBufferSequence>
        requires asio::const_buffer_sequence<ConstBufferSequence>
        std::tuple<std::error_code, std::vector<uint8_t>&> escape_telnet_output(std::vector<uint8_t>& escaped_data, const ConstBufferSequence& data) const noexcept {
            try {
                for (auto iter = asio::buffers_begin(data), end = asio::buffers_end(data); iter != end; ++iter) {
                    escaped_data.push_back(*iter);
                    if (*iter == static_cast<uint8_t>(TelnetCommand::IAC)) {
                        escaped_data.push_back(*iter); // Push it again if it's IAC
                    }
                }
                return {std::error_code(), escaped_data};
            } catch (const std::bad_alloc&) {
                return {std::make_error_code(std::errc::not_enough_memory), escaped_data};
            } catch (...) {
                return {asio::error::make_error_code(asio::error::operation_aborted), escaped_data};
            }
        }

        /** 
         * @brief Escapes Telnet output data by duplicating 0xFF (IAC) bytes.
         * @tparam ConstBufferSequence The type of constant buffer sequence to escape.
         * @param data The input data to escape.
         * @return A tuple containing the error code (empty on success) and a vector containing the escaped data.
         * @note Reserves 10% extra capacity to accommodate escaping and delegates to the overload with a provided vector. Returns an empty vector with not_enough_memory error if allocation fails or operation_aborted for unexpected exceptions.
         * @requires The type `ConstBufferSequence` must model `asio::const_buffer_sequence`.
         */
        template<typename ConstBufferSequence>
        requires asio::const_buffer_sequence<ConstBufferSequence>
        std::tuple<std::error_code, std::vector<uint8_t>> escape_telnet_output(const ConstBufferSequence& data) const noexcept {
            std::vector<uint8_t> escaped_data;
            try {
                escaped_data.reserve(asio::buffer_size(data) * 1.1); // 10% overhead for escaping, may reallocate if many IACs; sufficient as reallocation doubles to max *2
                return this->escape_telnet_output(escaped_data, data);
            } catch (const std::bad_alloc&) {
                return {std::make_error_code(std::errc::not_enough_memory), std::vector<uint8_t>()};
            } catch (...) {
                return {asio::error::make_error_code(asio::error::operation_aborted), std::vector<uint8_t>()};
            }
        }

        /** 
         * @brief Processes Telnet input byte-by-byte using the FSM for a buffer sequence.
         * @deprecated Use InputProcessor for integrated async processing.
         * @tparam MutableBufferSequence The type of mutable buffer sequence to process.
         * @param buffers The mutable buffer sequence to process.
         * @param bytes_read The number of bytes read into the sequence.
         * @return A tuple containing the error code (e.g., operation_aborted on invariant violation) and the number of bytes processed.
         * @note Returns {operation_aborted, 0} if the write iterator exceeds the buffer end.
         * @note Temporarily handles negotiation responses by spawning async_write_negotiation on a detached thread.
         */
        template<typename MutableBufferSequence>
        std::tuple<std::error_code, std::size_t> process_telnet_input(MutableBufferSequence buffers, std::size_t bytes_read) noexcept
            requires asio::mutable_buffer_sequence<MutableBufferSequence>
        {
            auto begin = asio::buffers_begin(buffers);
            auto end = begin + bytes_read; // Use random access to set end based on bytes_read
            auto write_it = begin;

            for (auto read_it = begin; read_it != end; ++read_it) {
                auto [ec, forward, negotiation_response] = this->fsm_.process_byte(*read_it); // Updated call
                if (ec) {
                    return {ec, 0};
                }
                if (forward) {
                    if (write_it >= end) {
                        return {asio::error::make_error_code(asio::error::operation_aborted), 0};
                    }
                    *write_it++ = *read_it;
                }
                // Temporary solution: Handle negotiation_response
                if (negotiation_response) {
                    auto [cmd, opt] = *negotiation_response; // Unpack the tuple
                    asio::co_spawn(this->get_executor(),
                                   async_write_negotiation(cmd, opt, asio::use_awaitable),
                                   asio::detached);
                }
            }

            return {std::error_code(), std::distance(begin, write_it)};
        }

        /**
         * @brief A private nested class template for processing Telnet input asynchronously.
         * @tparam MutableBufferSequence The type of mutable buffer sequence to process.
         * @tparam CompletionHandler The type of completion handler for result propagation.
         * @note Manages the stateful processing of Telnet input, handling reads and negotiation writes
         *       within a single shared_ptr-managed instance.
         * @note Uses a single is_processing_ flag to track the flow, eliminating the need for
         *       distinct reading, processing, or writing states. Future extensions may require a state machine review.
         */
        template<typename MutableBufferSequence, typename CompletionHandler>
        class InputProcessor {
        private:
            /// Reference to the parent socket managing the connection.
            socket& parent_socket_;
            /// Buffer sequence to process incoming Telnet data.
            MutableBufferSequence buffers_;
            /// Flag indicating if processing is active (true after initial read setup).
            bool is_processing_ = false;
            /// Optional completion handler to propagate results, initialized to nullopt.
            std::optional<CompletionHandler> completion_handler_ = std::nullopt;
            /// Iterator marking the beginning of the buffer sequence.
            using iterator_type = asio::buffers_iterator<MutableBufferSequence>;
            iterator_type buf_begin_;
            /// Iterator marking the end of the buffer sequence based on bytes read.
            iterator_type buf_end_;
            /// Iterator tracking the position for writing processed data.
            iterator_type write_it_;
            /// Iterator tracking the current position for reading data.
            iterator_type read_it_;

            // Private constructor and destructor to enforce shared_ptr-only creation
            InputProcessor(socket& parent_socket, MutableBufferSequence buffers) 
                : parent_socket_(parent_socket), buffers_(std::move(buffers)) {}
            ~InputProcessor() = default;

            // Friend declaration for shared_ptr construction
            friend class std::shared_ptr<InputProcessor<MutableBufferSequence, CompletionHandler>>;

        public:
            /**
             * @brief Creates a new InputProcessor instance with the given socket and buffer sequence.
             * @tparam Args Variadic template for constructor arguments.
             * @param parent_socket Reference to the parent socket managing the connection.
             * @param args Forwarded arguments for buffer sequence construction.
             * @return A shared pointer to the newly created InputProcessor instance.
             * @note Uses std::make_shared to ensure proper shared ownership.
             */
            template<typename... Args>
            static std::shared_ptr<InputProcessor> create(socket& parent_socket, Args&&... args) {
                return std::make_shared<InputProcessor>(parent_socket, std::forward<Args>(args)...);
            }

            /**
             * @brief Completion handler callback for asynchronous operations.
             * @tparam MutableBufferSequence The type of mutable buffer sequence to process.
             * @tparam CompletionHandler The type of completion handler for result propagation.
             * @param ec The error code indicating the outcome of the asynchronous operation.
             * @param bytes_transferred The number of bytes transferred in the operation.
             * @note Throws std::logic_error if invoked without prior initialization by asio::async_initiate.
             * @note Propagates errors via the completion handler if ec is set.
             * @note Initializes iterators and processing state on the first call, then processes data or handles write completions.
             * @see ProtocolFSM::process_byte in telnet.cppm for details on byte processing logic.
             */
            void operator()(const std::error_code& ec, std::size_t bytes_transferred) {
                if (!completion_handler_) {
                    throw std::logic_error("InputProcessor completion handler invoked without prior initialization by asio::async_initiate");
                }
                if (ec) {
                    (*completion_handler_)(ec, 0); // Propagate read or write error
                    return;
                }
                if (!is_processing_) {
                    buf_begin_ = asio::buffers_begin(buffers_);
                    buf_end_ = buf_begin_ + bytes_transferred; // Update end based on bytes read
                    write_it_ = buf_begin_;
                    read_it_ = buf_begin_; // Reset read and write iterators
                    is_processing_ = true; // Set processing flag early
                }
                // Trunk of the control flow
                for (; read_it_ != buf_end_; ++read_it_) {
                    auto [proc_ec, forward, negotiation_response] = parent_socket_.fsm_.process_byte(*read_it_);
                    if (proc_ec) {
                        (*completion_handler_)(proc_ec, 0); // Propagate processing error
                        return;
                    }
                    if (forward) {
                        if (write_it_ >= buf_end_) {
                            (*completion_handler_)(asio::error::make_error_code(asio::error::operation_aborted), 0);
                            return;
                        }
                        *write_it_++ = *read_it_;
                    }
                    // Handle negotiation response with async write
                    if (negotiation_response) {
                        ++read_it_; // Increment read_it_ before async call to mimic continue
                        auto [cmd, opt] = *negotiation_response;
                        parent_socket_.async_write_negotiation(
                            cmd,
                            opt,
                            asio::bind_executor(parent_socket_.get_executor(), shared_from_this())
                        );
                        return; // Exit after initiating async write
                    }
                }

                // Propagate result
                std::size_t bytes_processed = std::distance(buf_begin_, write_it_);
                (*completion_handler_)(std::error_code(), bytes_processed); // Propagate success
            }

            /**
             * @brief Initiator callback to set the completion handler for the InputProcessor.
             * @tparam CompletionHandler The type of completion handler to set.
             * @param handler The completion handler to be stored and used for result propagation.
             * @note Initiates an async_read_some operation on the parent socket's next layer.
             * @note Returns early with an error if a handler is already set.
             */
            void operator()(CompletionHandler&& handler) {
                if (completion_handler_) {
                    // Report error: invalid state since handler is already set
                    std::error_code ec = std::make_error_code(std::errc::invalid_argument);
                    handler(ec, 0); // Invoke the handler with an error
                    return; // Early return after error handling
                }
                // Assign the completion handler if not set
                completion_handler_ = std::move(handler);
                // Initiate async_read_some on the next layer
                parent_socket_.next_layer().async_read_some(
                    buffers_,
                    asio::bind_executor(parent_socket_.get_executor(), shared_from_this())
                );
            }
        };
    };
} // namespace telnet
