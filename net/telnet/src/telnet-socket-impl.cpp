/**
 * @file telnet-socket-impl.cpp
 * @version 0.2.0
 * @release_date September 19, 2025
 * @brief Implementation of Telnet socket constructor, InputProcessor, and utility functions.
 * @remark Contains socket constructor, InputProcessor, sync_await, and escape_telnet_output.
 * @copyright (c) 2025 [it's mine!]. All rights reserved.
 * @license See LICENSE file for details
 * @see telnet-socket.cppm for interface, RFC 854 for Telnet protocol, RFC 855 for option negotiation, telnet:types for TelnetCommand, telnet:options for option::id_num, telnet:errors for error codes, telnet:protocol_fsm for ProtocolFSM
 */
module; //Including Boost.Asio in the Global Module Fragment until importable header units are reliable.
#include <boost/asio.hpp>
namespace asio = boost::asio;

//Module partition implementation unit
module telnet:socket;

import std;        // For std::promise, std::future, std::jthread, std::exception_ptr, std::make_tuple
import std.compat; // For std::uint8_t

import :errors;       ///< @see telnet-errors.cppm for telnet::error codes 
import :protocol_fsm; ///< @see telnet-protocol_fsm.cppm for ProtocolFSM
import :options;      ///< @see telnet-options.cppm for option::id_num

import :socket;       ///< @see telnet-socket.cppm for socket partition interface

namespace telnet {
    /**
     * @brief Constructs a socket with a next-layer socket.
     * @param socket The next-layer socket to wrap.
     * @see telnet-socket.cppm for interface, TelnetSocketConcept for socket requirements
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC>
    socket<NLS, PC>::socket(next_layer_type&& socket)
        : next_layer_(std::move(socket)), fsm_() {}

    /**
     * @brief Synchronously awaits the completion of an awaitable operation.
     * @tparam Awaitable The type of awaitable to execute.
     * @param a The awaitable to await.
     * @return The result of the awaitable operation.
     * @remark Uses a temporary io_context and a jthread to run the awaitable, capturing its result or exception.
     * @remark The return type of the awaitable determines the promise/future type, handling both void and non-void results.
     * @warning Incurs overhead from creating a new io_context and jthread per call, which may impact performance in high-frequency synchronous operations, though this is minimal compared to blocking network I/O latency. Synchronous I/O inherently scales poorly.
     * @throws std::system_error If the operation fails with an error code.
     * @throws std::bad_alloc If memory allocation fails.
     * @throws std::exception For other unexpected errors.
     * @see telnet-socket.cppm for interface
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename Awaitable>
    auto socket<NLS, PC>::sync_await(Awaitable&& a) {
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
            } //lambda
        ); //co_spawn

        std::jthread runner_thread([&temp_ctx]{ temp_ctx.run(); });
        return future.get();
    } //socket::sync_await(Awaitable&&)

    /**
     * @brief Escapes Telnet output data by duplicating 0xFF (IAC) bytes into a provided vector.
     * @tparam ConstBufferSequence The type of constant buffer sequence to escape.
     * @param escaped_data The vector to store the escaped data.
     * @param data The input data to escape.
     * @return A tuple containing the error code (empty on success) and a reference to the modified escaped_data vector.
     * @remark Processes the byte sequence directly using buffers_iterator and appends to escaped_data. Sets error code to not_enough_memory on memory allocation failure or internal_error for unexpected exceptions.
     * @see telnet-socket.cppm for interface, telnet:errors for not_enough_memory and internal_error, RFC 854 for IAC escaping
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename ConstBufferSequence>
    std::tuple<std::error_code, std::vector<std::uint8_t>&> socket<NLS, PC>::escape_telnet_output(std::vector<std::uint8_t>& escaped_data, const ConstBufferSequence& data) const noexcept {
        try {
            for (auto iter = asio::buffers_begin(data), end = asio::buffers_end(data); iter != end; ++iter) {
                escaped_data.push_back(*iter);
                if (*iter == static_cast<std::uint8_t>(TelnetCommand::IAC)) {
                    escaped_data.push_back(*iter);
                }
            }
            return {std::error_code(), escaped_data};
        } catch (const std::bad_alloc&) {
        	escaped_data.clear();
            return {std::make_error_code(std::errc::not_enough_memory), escaped_data};
        } catch (...) {
            escaped_data.clear();
            return {std::make_error_code(error::internal_error), escaped_data};
        }
    } //socket::escape_telnet_output(std::vector<std::uint8_t>&, const ConstBufferSequence&) const noexcept

    /**
     * @brief Escapes Telnet output data by duplicating 0xFF (IAC) bytes.
     * @tparam ConstBufferSequence The type of constant buffer sequence to escape.
     * @param data The input data to escape.
     * @return A tuple containing the error code (empty on success) and a vector containing the escaped data.
     * @remark Reserves 10% extra capacity to accommodate escaping and delegates to the overload with a provided vector. Returns an empty vector with not_enough_memory error if allocation fails or internal_error for unexpected exceptions.
     * @see telnet-socket.cppm for interface, telnet:errors for not_enough_memory and internal_error, RFC 854 for IAC escaping
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename ConstBufferSequence>
    std::tuple<std::error_code, std::vector<std::uint8_t>> socket<NLS, PC>::escape_telnet_output(const ConstBufferSequence& data) const noexcept {
        std::vector<std::uint8_t> escaped_data;
        try {
            escaped_data.reserve(asio::buffer_size(data) * 1.1);
            return this->escape_telnet_output(escaped_data, data);
        } catch (const std::bad_alloc&) {
            return {std::make_error_code(std::errc::not_enough_memory), std::vector<std::uint8_t>()};
        } catch (...) {
            return {std::make_error_code(error::internal_error), std::vector<std::uint8_t>()};
        }
    } //socket::escape_telnet_output(const ConstBufferSequence&) const noexcept

    /**
     * @brief Constructs an InputProcessor with the parent socket, FSM, and buffers.
     * @param parent_socket Reference to the parent socket managing the connection.
     * @param fsm Reference to the ProtocolFSM for Telnet state management.
     * @param buffers The mutable buffer sequence to read into.
     * @see telnet-socket.cppm for interface, socket for parent_socket, telnet-protocol_fsm.cppm for fsm_type
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename MutableBufferSequence>
    socket<NLS, PC>::InputProcessor<MutableBufferSequence>::InputProcessor(
        socket& parent_socket, socket::fsm_type& fsm, MutableBufferSequence buffers)
        : parent_socket_(parent_socket),
          fsm_(fsm),
          buffers_(buffers),
          state_(State::INITIALIZING) {}

    /**
     * @brief Asynchronous operation handler for processing Telnet input.
     * @tparam Self The type of the coroutine self reference.
     * @param self Reference to the coroutine self.
     * @param ec The error code from the previous operation.
     * @param bytes_transferred The number of bytes transferred in the previous operation.
     * @remark Manages state transitions (INITIALIZING, READING, PROCESSING, DONE) and processes Telnet bytes.
     * @note Checks DONE state early to prevent reentrancy issues after completion.
     * @see telnet-socket.cppm for interface, telnet-protocol_fsm.cppm for ProtocolFSM processing, telnet:errors for error codes, telnet:options for option::id_num, RFC 854 for Telnet protocol
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename MutableBufferSequence>
    template<typename Self>
    void socket<NLS, PC>::InputProcessor<MutableBufferSequence>::operator()(Self& self, std::error_code ec, std::size_t bytes_transferred) {
        if (state_ == State::DONE) {
          return; //complete has already been called; unsafe to do anything else
        } else if (ec) {
            complete(self, ec, 0);
            return;
        }
        switch (state_) {
	        case State::INITIALIZING:
	            state_ = State::READING;
	            parent_socket_.next_layer().async_read_some(
	                buffers_,
	                asio::bind_executor(parent_socket_.get_executor(), std::move(self))
	            );
	            break;
	        case State::READING:
	            buf_begin_ = asio::buffers_begin(buffers_);
	            buf_end_ = buf_begin_ + bytes_transferred;
	            write_it_ = buf_begin_;
	            read_it_ = buf_begin_;
	            state_ = State::PROCESSING;
	            [[fallthrough]];
	        case State::PROCESSING:
	            for (; read_it_ != buf_end_; ++read_it_) {
	                auto [proc_ec, forward, negotiation_response] = fsm_.process_byte(*read_it_);
	                if (proc_ec) {
	                    complete(self, proc_ec, 0);
	                    break;
	                }
	                if (forward) {
	                    if (write_it_ >= buf_end_) {
	                        complete(self, std::make_error_code(error::internal_error), 0);
	                        break;
	                    }
	                    *write_it++ = *read_it_;
	                }
	                if (negotiation_response) {
	                	// Pausing processing for async_write_negotiation breaks the loop and prevents the for-loop-based increment.
                        // Do it manually before handing over control.
	                    ++read_it_;
	                    auto [cmd, opt] = *negotiation_response;
	                    parent_socket_.async_write_negotiation(
	                        cmd,
	                        static_cast<option::id_num>(opt),
	                        std::move(self)
	                    );
	                    break; // Wait for negotiation to complete
	                }
	            }
	            if (read_it_ == buf_end_) {
	                complete(self, std::error_code(), std::distance(buf_begin_, write_it_));
	            }
	            break;
	        case State::DONE: [[fallthrough]]
	        default: 
	            break; // Unreachable due to early check
        }
    } //socket::InputProcessor::operator(Self&, std::error_code, std::size_t)

    /**
     * @brief Completes the asynchronous operation with error code and bytes transferred.
     * @tparam Self The type of the coroutine self reference.
     * @param self Reference to the coroutine self.
     * @param ec The error code to report.
     * @param bytes_transferred The number of bytes to report.
     * @remark Sets state to DONE and calls self.complete.
     * @see telnet-socket.cppm for interface
     */
    template<TelnetSocketConcept NLS, ProtocolFSMConfig PC, typename MutableBufferSequence>
    template<typename Self>
    void socket<NLS, PC>::InputProcessor<MutableBufferSequence>::complete(Self& self, const std::error_code& ec, std::size_t bytes_transferred) {
        state_ = State::DONE;
        self.complete(ec, bytes_transferred);
    } //socket::InputProcessor::complete(Self&, const std::error_code&, std::size_t)
} //namespace telnet