# class input_processor

*Defined at net/telnet/src/net.telnet-stream.cppm#289*

## Members

private stream<NextLayerT, ProtocolConfigT> & parent_stream_

private fsm_type & fsm_

private context_type & context_

private MBufSeq buffers_

private iterator_type user_buf_begin_

private iterator_type user_buf_end_

private iterator_type write_it_

private state state_

private static const std::size_t read_block_size



## Functions

### input_processor<MBufSeq>

*public void input_processor<MBufSeq>(stream<NextLayerT, ProtocolConfigT> & parent_stream, stream<NextLayerT, ProtocolConfigT>::fsm_type & fsm, stream<NextLayerT, ProtocolConfigT>::context_type & context, MBufSeq buffers)*

*Defined at net/telnet/src/impl/net.telnet-stream-impl.cpp#270*



 Initializes member variables `parent_stream_`, `fsm_`, `buffers_`, and sets `state_` to `initializing`.





**brief** Constructs an `input_processor` with the parent stream, FSM, and buffers.

### operator()

*public void operator()(Self & self, std::error_code ec_in, std::size_t bytes_transferred)*

*Defined at net/telnet/src/impl/net.telnet-stream-impl.cpp#288*



 Manages state transitions using a switch statement (`initializing`, `reading`, `processing`, `done`).

 Dispatches to "handle_processor_state_*" helpers for each valid state.

 @remark Uses `[[unlikely]]` for `done` state and default case, and `[[fallthrough]]` for `reading` to `processing`.

 @remark Checks `done` state early to prevent reentrancy.





**brief** Asynchronous operation handler for processing Telnet input.

### handle_processor_state_initializing

*private void handle_processor_state_initializing(Self & self)*

*Defined at net/telnet/src/impl/net.telnet-stream-impl.cpp#320*



 In `initializing`, calls `next_layer_.async_read_some` with `buffers_` unless `context_.input_side_buffer` already has data. Transitions to `reading`.

 @remark Directly calls `handle_processor_state_reading` if there is data in the buffer already waiting to be processed.





**brief** Handles processing of the `initializing` state.

### handle_processor_state_reading

*private void handle_processor_state_reading(Self & self, std::error_code ec_in, std::size_t bytes_transferred)*

*Defined at net/telnet/src/impl/net.telnet-stream-impl.cpp#349*



 In `reading`, sets up iterators (`user_buf_begin_`, `user_buf_end_`, `write_it_`) and transitions to `processing`.

 @remark Directly calls `handle_processor_state_processing` to immediately begin processing.





**brief** Handles processing of the `reading` state.

### handle_processor_state_processing

*private void handle_processor_state_processing(Self & self, std::error_code ec_in)*

*Defined at net/telnet/src/impl/net.telnet-stream-impl.cpp#390*



 In `processing`, iterates over the buffer, calling `fsm_.process_byte`, handling forward flags (outside of urgent/Synch mode), and dispatching to `do_response` overloads that handle `ProcessingReturnVariant` responses via `std::visit`

 Consumes bytes from the `context_.input_side_buffer` manually before async operations to avoid re-processing.

 Completes with `std::distance(user_buf_begin_, write_it_)` bytes when `read_it == buf_end` or `write_it_ == user_buf_end_` or `process_byte` returns an error code. [std::distance should be linear time in the number of buffers in the sequence rather than the number of bytes]

 Delegates to `process_fsm_signal` to handle `processing_signal`s returned from `process_byte`.

 @remark Handles AO by clearing `context_.output_side_buffer` but propagates the signal to the caller for higher-level notification.

 @note Unhandled `processing_signal`s and other `error_code`s propagate to the caller for higher-level notification.





**brief** Handles processing of the `processing` state.

### complete

*private void complete(Self & self, const std::error_code & ec, std::size_t bytes_transferred)*

*Defined at net/telnet/src/impl/net.telnet-stream-impl.cpp#474*



 Sets `state_` to `done` and calls `self.complete` with `ec` and `bytes_transferred`.





**brief** Completes the asynchronous operation with error code and bytes transferred.

### process_write_error

*private void process_write_error(std::error_code ec)*

*Defined at net/telnet/src/impl/net.telnet-stream-impl.cpp#491*



 Defers write errors for later reporting and logs multi-error pile-ups.





**brief** Defers write errors for later reporting and logs multi-error pile-ups.

### process_fsm_signals

*private void process_fsm_signals(std::error_code & signal_ec)*

*Defined at net/telnet/src/impl/net.telnet-stream-impl.cpp#514*

**brief** Handles processing_signal error codes from fsm_.process_byte, modifying the user buffer or urgent data state.

### do_response

*private void do_response(typename stream<NextLayerT, ProtocolConfigT>::fsm_type::negotiation_response response, Self && self)*

*Defined at net/telnet/src/impl/net.telnet-stream-impl.cpp#545*



 Initiates an asynchronous write of a `negotiation_response` using `async_write_negotiation`.

 @see "net.telnet-stream.cppm" for interface, `:protocol_fsm` for `negotiation_response`, `:errors` for error codes, RFC 855 for negotiation





**brief** Handle a `negotiation_response` by initiating an async write of the negotiation.

### do_response

*private void do_response(std::string response, Self && self)*

*Defined at net/telnet/src/impl/net.telnet-stream-impl.cpp#562*



 Initiates an asynchronous write of raw data using `async_write_raw`.

 @remark Wraps the `std::string` in a `std::shared_ptr<std::string>` for lifetime management and forwards it to `async_write_raw`.

 @see "net.telnet-stream.cppm" for interface, `:errors` for error codes, RFC 854 for IAC escaping





**brief** Handle a `std::string` by initiating an async write of raw data.

### do_response

*private void do_response(awaitables::subnegotiation_awaitable awaitable, Self && self)*

*Defined at net/telnet/src/impl/net.telnet-stream-impl.cpp#577*



 Spawns a coroutine to process a `subnegotiation_awaitable`, writing the result via `async_write_subnegotiation` if non-empty.

 @remark Uses `asio::co_spawn` to execute the awaitable, handling potential exceptions by throwing `std::system_error` with `telnet::error::internal_error` for non-system errors.

 @see "net.telnet-stream.cppm" for interface, `:awaitables` for `subnegotiation_awaitable`, `:errors` for error codes, RFC 855 for subnegotiation





**brief** Handle a `subnegotiation_awaitable` by spawning a coroutine to process it.

### do_response

*private void do_response(std::tuple<awaitables::tagged_awaitable<Tag, T, Awaitable>, std::optional<typename stream<NextLayerT, ProtocolConfigT>::fsm_type::negotiation_response>> response, Self && self)*

*Defined at net/telnet/src/impl/net.telnet-stream-impl.cpp#609*



 Spawns a coroutine to process a `tagged_awaitable`, optionally writing a `negotiation_response` via `async_write_negotiation`.

 @remark Uses `asio::co_spawn` to execute the awaitable, followed by an optional negotiation write, handling exceptions by throwing `std::system_error` with `telnet::error::internal_error` for non-system errors.

 @see "net.telnet-stream.cppm" for interface, `:awaitables` for `tagged_awaitable`, `:protocol_fsm` for `negotiation_response`, `:errors` for error codes, RFC 855 for negotiation





**brief** Handle any `tagged_awaitable` with optional `negotiation_response`.



## Enums

| enum class state |

--

| initializing |
| reading |
| processing |
| done |


*Defined at net/telnet/src/net.telnet-stream.cppm#368*



