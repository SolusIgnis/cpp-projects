# class stream

*Defined at net/telnet/src/net.telnet-stream.cppm#55*

## Members

private next_layer_type next_layer_

private fsm_type fsm_

private context_type context_



## Records

context_type

input_processor



## Functions

### stream<NextLayerT, ProtocolConfigT>

*public void stream<NextLayerT, ProtocolConfigT>(next_layer_type && next_layer_stream)*

*Defined at net/telnet/src/impl/net.telnet-stream-impl.cpp#39*



 Moves the provided `next_layer_stream` into `next_layer_`, default-constructs `fsm_`, and enables SO_OOBINLINE.





**brief** Constructs a stream with a next-layer stream.

### async_request_option

*public auto async_request_option(option::id_num opt, negotiation_direction direction, CompletionToken && token)*

*Defined at net/telnet/src/impl/net.telnet-stream-async-impl.cpp#43*



 Implements asynchronous option request by delegating to FSM and writing negotiation.

 @remark Calls `fsm_.request_option` to check state and generate a response tuple containing an optional `negotiation_response`.

 @remark If no response is needed or an error occurs, invokes `async_report_error` with the error code and 0 bytes.

 @remark Forwards valid responses to `async_write_negotiation` to send IAC WILL/DO sequence.

 @remark Uses `std::forward` for `CompletionToken` to preserve handler efficiency.





**brief** Asynchronously requests or offers an option, sending IAC WILL/DO.

### request_option

*public std::size_t request_option(option::id_num opt, negotiation_direction direction)*

*Defined at net/telnet/src/impl/net.telnet-stream-sync-impl.cpp#44*



 Wraps awaitable `async_request_option` in `sync_await`, forwarding the option and direction.

 @see `async_request_option` in "net.telnet-stream-async-impl.cpp".





**brief** Synchronously requests or offers an option, sending IAC WILL/DO.

### request_option

*public std::size_t request_option(option::id_num opt, negotiation_direction direction, std::error_code & ec)*

*Defined at net/telnet/src/impl/net.telnet-stream-sync-impl.cpp#147*



 Calls the throwing `request_option`, catching exceptions to set `ec` to `std::system_error`’s code or `telnet::error::internal_error`.

 @see `request_option` for throwing version, `async_request_option` in "net.telnet-stream-async-impl.cpp" for async implementation, "net.telnet-stream.cppm" for interface





**brief** Synchronously requests or offers an option, sending IAC WILL/DO.

### async_disable_option

*public auto async_disable_option(option::id_num opt, negotiation_direction direction, CompletionToken && token)*

*Defined at net/telnet/src/impl/net.telnet-stream-async-impl.cpp#69*



 Implements asynchronous option disablement with FSM state management and optional awaitable handling.

 @remark Calls `fsm_.disable_option` to obtain error code, optional `negotiation_response`, and optional awaitable.

 @remark Uses `asio::co_spawn` to manage coroutine lifetime, executing on the stream’s executor.

 @remark If a response is present, writes IAC WONT/DONT via `async_write_negotiation` and accumulates bytes transferred.

 @remark If an awaitable is present, co_awaits it to handle registered disablement callbacks.

 @remark Catches non-system errors and converts to `telnet::error::internal_error` for consistency.

 @remark Returns 0 bytes on error via `async_report_error` if neither response nor awaitable is provided.





**brief** Asynchronously disables an option, sending IAC WONT/DONT.

### disable_option

*public std::size_t disable_option(option::id_num opt, negotiation_direction direction)*

*Defined at net/telnet/src/impl/net.telnet-stream-sync-impl.cpp#59*



 Wraps awaitable `async_disable_option` in `sync_await`, forwarding the option and direction.

 @see `async_disable_option` in "net.telnet-stream-async-impl.cpp".





**brief** Synchronously disables an option, sending IAC WONT/DONT.

### lowest_layer

*public lowest_layer_type & lowest_layer()*

*Defined at net/telnet/src/net.telnet-stream.cppm#106*

### next_layer

*public next_layer_type & next_layer()*

*Defined at net/telnet/src/net.telnet-stream.cppm#109*

### register_option_handlers

*public void register_option_handlers(option::id_num opt, std::optional<typename fsm_type::option_enablement_handler_type> enable_handler, std::optional<typename fsm_type::option_disablement_handler_type> disable_handler, std::optional<typename fsm_type::subnegotiation_handler_type> subneg_handler)*

*Defined at net/telnet/src/net.telnet-stream.cppm#112*

### unregister_option_handlers

*public void unregister_option_handlers(option::id_num opt)*

*Defined at net/telnet/src/net.telnet-stream.cppm#125*

### disable_option

*public std::size_t disable_option(option::id_num opt, negotiation_direction direction, std::error_code & ec)*

*Defined at net/telnet/src/impl/net.telnet-stream-sync-impl.cpp#170*



 Calls the throwing `disable_option`, catching exceptions to set `ec` to `std::system_error`’s code or `telnet::error::internal_error`.

 @see `disable_option` for throwing version, `async_disable_option` in "net.telnet-stream-async-impl.cpp" for async implementation, "net.telnet-stream.cppm" for interface





**brief** Synchronously disables an option, sending IAC WONT/DONT.

### async_read_some

*public auto async_read_some(const MBufSeq & buffers, CompletionToken && token)*

*Defined at net/telnet/src/impl/net.telnet-stream-async-impl.cpp#112*



 Uses `asio::async_compose` to initiate asynchronous read with `input_processor`.

 @remark Binds the operation to the stream’s executor using `get_executor()`.





**brief** Asynchronously reads some data with Telnet processing.

### read_some

*public std::size_t read_some(MBufSeq && buffers)*

*Defined at net/telnet/src/impl/net.telnet-stream-sync-impl.cpp#74*



 Wraps awaitable `async_read_some` in `sync_await` with perfect forwarding of the buffer sequence.

 @see `async_read_some` in "net.telnet-stream-async-impl.cpp".





**brief** Synchronously reads some data with Telnet processing.

### read_some

*public std::size_t read_some(MBufSeq && buffers, std::error_code & ec)*

*Defined at net/telnet/src/impl/net.telnet-stream-sync-impl.cpp#193*



 Calls the throwing `read_some`, catching exceptions to set `ec` to `std::system_error`’s code, `std::errc::not_enough_memory`, or `telnet::error::internal_error`.

 @see `read_some` for throwing version, `async_read_some` in "net.telnet-stream-async-impl.cpp" for async implementation, "net.telnet-stream.cppm" for interface





**brief** Synchronously reads some data with Telnet processing.

### async_write_some

*public auto async_write_some(const CBufSeq & data, CompletionToken && token)*

*Defined at net/telnet/src/impl/net.telnet-stream-async-impl.cpp#128*



 Escapes input data using `escape_telnet_output` and delegates to `async_write_temp_buffer`.

 @remark Returns via `async_report_error` if `escape_telnet_output` fails.





**brief** Asynchronously writes some data with Telnet-specific IAC escaping.

### write_some

*public std::size_t write_some(const CBufSeq & data)*

*Defined at net/telnet/src/impl/net.telnet-stream-sync-impl.cpp#86*



 Wraps awaitable `async_write_some` in `sync_await`, forwarding the buffer sequence.

 @see `async_write_some` in "net.telnet-stream-async-impl.cpp".





**brief** Synchronously writes some data with Telnet-specific IAC escaping.

### write_some

*public std::size_t write_some(const CBufSeq & data, std::error_code & ec)*

*Defined at net/telnet/src/impl/net.telnet-stream-sync-impl.cpp#216*



 Calls the throwing `write_some`, catching exceptions to set `ec` to `std::system_error`’s code, `std::errc::not_enough_memory`, or `telnet::error::internal_error`.

 @see `write_some` for throwing version, `async_write_some` in "net.telnet-stream-async-impl.cpp" for async implementation, "net.telnet-stream.cppm" for interface





**brief** Synchronously writes some data with Telnet-specific IAC escaping.

### async_write_raw

*public auto async_write_raw(const CBufSeq & data, CompletionToken && token)*

*Defined at net/telnet/src/impl/net.telnet-stream-async-impl.cpp#144*



 Uses `asio::async_write` to write the raw buffer directly to `next_layer_`.

 @remark Binds the operation to the stream’s executor using `asio::bind_executor`.





**brief** Asynchronously writes a pre-escaped buffer containing data and raw Telnet commands.

### write_raw

*public std::size_t write_raw(const CBufSeq & data)*

*Defined at net/telnet/src/impl/net.telnet-stream-sync-impl.cpp#98*



 Wraps awaitable `async_write_raw` in `sync_await`, forwarding the buffer sequence.

 @see `async_write_raw` in "net.telnet-stream-async-impl.cpp".





**brief** Synchronously writes a pre-escaped buffer containing data and raw Telnet commands.

### get_executor

*public executor_type get_executor()*

*Defined at net/telnet/src/net.telnet-stream.cppm#103*

### write_raw

*public std::size_t write_raw(const CBufSeq & data, std::error_code & ec)*

*Defined at net/telnet/src/impl/net.telnet-stream-sync-impl.cpp#239*



 Calls the throwing `write_raw`, catching exceptions to set `ec` to `std::system_error`’s code, `std::errc::not_enough_memory`, or `telnet::error::internal_error`.

 @see `write_raw` for throwing version, `async_write_raw` in "net.telnet-stream-async-impl.cpp" for async implementation, "net.telnet-stream.cppm" for interface





**brief** Synchronously writes a pre-escaped buffer containing data and raw Telnet commands.

### async_write_command

*public auto async_write_command(telnet::command cmd, CompletionToken && token)*

*Defined at net/telnet/src/impl/net.telnet-stream-async-impl.cpp#158*



 Constructs a 2-byte `std::array` with `{IAC, cmd}` and writes it using `asio::async_write`.

 @remark Binds the operation to the stream’s executor using `asio::bind_executor`.





**brief** Asynchronously writes a Telnet command.

### write_command

*public std::size_t write_command(telnet::command cmd)*

*Defined at net/telnet/src/impl/net.telnet-stream-sync-impl.cpp#110*



 Wraps awaitable `async_write_command` in `sync_await`, forwarding the command.

 @see `async_write_command` in "net.telnet-stream-async-impl.cpp".





**brief** Synchronously writes a Telnet command.

### write_command

*public std::size_t write_command(telnet::command cmd, std::error_code & ec)*

*Defined at net/telnet/src/impl/net.telnet-stream-sync-impl.cpp#262*



 Calls the throwing `write_command`, catching exceptions to set `ec` to `std::system_error`’s code, `std::errc::not_enough_memory`, or `telnet::error::internal_error`.

 @see `write_command` for throwing version, `async_write_command` in "net.telnet-stream-async-impl.cpp" for async implementation, "net.telnet-stream.cppm" for interface





**brief** Synchronously writes a Telnet command.

### async_write_negotiation

*public auto async_write_negotiation(typename fsm_type::negotiation_response response, CompletionToken && token)*

*Defined at net/telnet/src/impl/net.telnet-stream-async-impl.cpp#315*



 @remark Constructs a 3-byte `std::array` with `{IAC, cmd, opt}` and writes it using `asio::async_write` if valid.

 @remark Binds the operation to the stream’s executor using `asio::bind_executor`.





**todo** Future Development: make this private

**brief** Asynchronously writes a Telnet negotiation command with an option.

### write_negotiation

*public std::size_t write_negotiation(typename fsm_type::negotiation_response response)*

**brief** Synchronously writes a Telnet negotiation command with an option.

### write_negotiation

*public std::size_t write_negotiation(typename fsm_type::negotiation_response response, std::error_code & ec)*

**brief** Synchronously writes a Telnet negotiation command with an option.

### async_write_subnegotiation

*public auto async_write_subnegotiation(option opt, const std::vector<byte_t> & subnegotiation_buffer, CompletionToken && token)*

*Defined at net/telnet/src/impl/net.telnet-stream-async-impl.cpp#180*



 Validates `opt` using `opt.supports_subnegotiation()` and `fsm_.is_enabled(opt)`.

 @remark Reserves space in `escaped_buffer` for subnegotiation data plus 5 bytes (IAC SB, opt, IAC SE).

 @remark Appends IAC SB, `opt`, escaped subnegotiation data via `escape_telnet_output`, and IAC SE.

 @remark Returns `telnet::error::invalid_subnegotiation` or `telnet::error::option_not_available` via `async_report_error` if validation fails.

 @remark Catches `std::bad_alloc` to return `std::errc::not_enough_memory` and other exceptions to return `telnet::error::internal_error` via `async_report_error`.

 @remark Delegates to `async_write_temp_buffer` for writing the constructed buffer.





**brief** Asynchronously writes a Telnet subnegotiation command.

### write_subnegotiation

*public std::size_t write_subnegotiation(option opt, const std::vector<byte_t> & subnegotiation_buffer)*

*Defined at net/telnet/src/impl/net.telnet-stream-sync-impl.cpp#121*



 Wraps awaitable `async_write_subnegotiation` in `sync_await`, forwarding the option and buffer.

 @see `async_write_subnegotiation` in "net.telnet-stream-async-impl.cpp".





**brief** Synchronously writes a Telnet subnegotiation command.

### write_subnegotiation

*public std::size_t write_subnegotiation(option opt, const std::vector<byte_t> & subnegotiation_buffer, std::error_code & ec)*

*Defined at net/telnet/src/impl/net.telnet-stream-sync-impl.cpp#284*



 Calls the throwing `write_subnegotiation`, catching exceptions to set `ec` to `std::system_error`’s code, `std::errc::not_enough_memory`, or `telnet::error::internal_error`.

 @see `write_subnegotiation` for throwing version, `async_write_subnegotiation` in "net.telnet-stream-async-impl.cpp" for async implementation, "net.telnet-stream.cppm" for interface





**brief** Synchronously writes a Telnet subnegotiation command.

### async_send_synch

*public auto async_send_synch(CompletionToken && token)*

*Defined at net/telnet/src/impl/net.telnet-stream-async-impl.cpp#241*



 Send 3 NULs, middle urgent, then IAC DM.

 Ensures correct Synch behavior regardless of URG pointer semantics.





**brief** Asynchronously sends Telnet Synch sequence (NUL bytes and IAC DM).

### send_synch

*public std::size_t send_synch()*

*Defined at net/telnet/src/impl/net.telnet-stream-sync-impl.cpp#132*



 Wraps awaitable `async_send_synch` in `sync_await`, forwarding the command and option.

 @see `async_send_synch` in "net.telnet-stream-async-impl.cpp".





**brief** Synchronously sends Telnet Synch sequence (NUL bytes and IAC DM).

### send_synch

*public std::size_t send_synch(std::error_code & ec)*

*Defined at net/telnet/src/impl/net.telnet-stream-sync-impl.cpp#310*



 Calls the throwing `send_synch`, catching exceptions to set `ec` to `std::system_error`’s code, `std::errc::not_enough_memory`, or `telnet::error::internal_error`.

 @see `send_synch` for throwing version, `async_send_synch` in "net.telnet-stream-async-impl.cpp" for async implementation, "net.telnet-stream.cppm" for interface





**brief** Synchronously sends Telnet Synch sequence (NUL bytes and IAC DM).

### launch_wait_for_urgent_data

*public void launch_wait_for_urgent_data()*

*Defined at net/telnet/src/impl/net.telnet-stream-impl.cpp#148*



 Launches an asynchronous wait (via 0-byte async_receive) for notification of urgent (i.e. "out-of-band") data.

 @remark Only launches the async_receive if another one is not already in progress and there is not already urgent data in the byte stream.





**brief** Asynchronously waits (via 0-byte receive) for notification that OOB data is in the stream.

### async_send_nul

*private auto async_send_nul(bool urgent, CompletionToken && token)*

*Defined at net/telnet/src/impl/net.telnet-stream-async-impl.cpp#295*



 Holds a static NUL byte and `async_send`s it (with `message_out_of_band` set if urgent).





**brief** Asynchronously sends a single NUL byte, optionally as OOB.

### sync_await

*private static auto sync_await(Awaitable && awaitable)*

*Defined at net/telnet/src/impl/net.telnet-stream-impl.cpp#56*



 Uses a temporary `asio::io_context` and `std::jthread` to run the awaitable via `asio::co_spawn`.

 @remark Captures the result or exception using `std::promise` and `std::future`.

 @remark Deduces `result_type` with `asio::awaitable_traits` to handle `void` and non-`void` return types.

 @remark Uses a lambda to set the promise’s value or exception, handling multiple return values via `std::make_tuple`.





**brief** Synchronously awaits the completion of an awaitable operation.

### escape_telnet_output

*private std::tuple<std::error_code, std::vector<byte_t> &> escape_telnet_output(std::vector<byte_t> & escaped_data, const CBufSeq & data)*

*Defined at net/telnet/src/impl/net.telnet-stream-impl.cpp#90*



 Iterates over `data` using `asio::buffers_begin` and `asio::buffers_end`.

 @remark Appends each byte to `escaped_data`, duplicating 0xFF (IAC) bytes plus transforming LF to CR LF and CR to CR NUL when not transmitting in BINARY mode.

 @remark Catches `std::bad_alloc` to clear `escaped_data` and return `std::errc::not_enough_memory`.

 @remark Catches other exceptions to clear `escaped_data` and return `telnet::error::internal_error`.





**brief** Escapes Telnet output data by duplicating 0xFF (IAC) bytes into a provided vector.

### escape_telnet_output

*private std::tuple<std::error_code, std::vector<byte_t>> escape_telnet_output(const CBufSeq & data)*

*Defined at net/telnet/src/impl/net.telnet-stream-impl.cpp#126*



 Creates a new `std::vector<byte_t>` with 10% extra capacity based on `asio::buffer_size(data)`.

 @remark Delegates to the overload with a provided vector.

 @remark Catches `std::bad_alloc` to return `std::errc::not_enough_memory` with an empty vector.

 @remark Catches other exceptions to return `telnet::error::internal_error` with an empty vector.





**brief** Escapes Telnet output data by duplicating 0xFF (IAC) bytes.

### async_write_temp_buffer

*private auto async_write_temp_buffer(std::vector<byte_t> && temp_buffer, CompletionToken && token)*

*Defined at net/telnet/src/impl/net.telnet-stream-async-impl.cpp#341*



 Uses `asio::async_initiate` to write `temp_buffer` via `asio::async_write` to `next_layer_`.

 @remark Moves `temp_buffer` into a lambda to manage its lifetime.

 @remark Binds the operation to the stream’s executor using `asio::bind_executor`.

 @remark Forwards the handler to receive `ec` and `bytes_written`.





**brief** Asynchronously writes a temporary buffer to the next layer.

### async_report_error

*private auto async_report_error(std::error_code ec, CompletionToken && token)*

*Defined at net/telnet/src/impl/net.telnet-stream-async-impl.cpp#365*



 Uses `asio::async_initiate` to invoke the handler with `ec` and 0 bytes transferred.





**brief** Asynchronously reports an error via the completion token.



