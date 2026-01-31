# class InputProcessor

*Defined at net/telnet/src/net.telnet-stream.cppm#258*



**brief** A private nested class template for processing Telnet input asynchronously.

**remark** Manages the stateful processing of Telnet input, handling reads and negotiation writes using `async_compose`.

**see** :protocol_fsm for `ProtocolFSM`, :errors for error codes



## Members

private stream<NextLayerT, ProtocolConfigT> & parent_stream_

private int & fsm_

private context_type & context_

private MBufSeq buffers_

private iterator_type user_buf_begin_

private iterator_type user_buf_end_

private iterator_type write_it_

private State state_

private static const std::size_t READ_BLOCK_SIZE



## Functions

### InputProcessor<MBufSeq>

*public void InputProcessor<MBufSeq>(stream<NextLayerT, ProtocolConfigT> & parent_stream, int & fsm, stream<NextLayerT, ProtocolConfigT>::context_type & context, MBufSeq buffers)*

**brief** Constructs an `InputProcessor` with the parent stream, FSM, and buffers.

### operator()

*public void operator()(Self & self, std::error_code ec, std::size_t bytes_transferred)*

**brief** Asynchronous operation handler for processing Telnet input.

### handle_processor_state_initializing

*private void handle_processor_state_initializing(Self & self)*

**brief** Handles processing of the INTIALIZING state.

### handle_processor_state_reading

*private void handle_processor_state_reading(Self & self, std::error_code ec_in, std::size_t bytes_transferred)*

**brief** Handles processing of the READING state.

### handle_processor_state_processing

*private void handle_processor_state_processing(Self & self, std::error_code ec_in)*

**brief** Handles processing of the PROCESSING state.

### complete

*private void complete(Self & self, const std::error_code & ec, std::size_t bytes_transferred)*

**brief** Completes the asynchronous operation with error code and bytes transferred.

### process_write_error

*private void process_write_error(std::error_code ec)*

**brief** Defers write errors for later reporting and logs multi-error pile-ups.

### process_fsm_signals

*private void process_fsm_signals(std::error_code & signal_ec)*

**brief** Handles processing_signal error codes from fsm_.process_byte, modifying the user buffer or urgent data state.

### do_response

*private void do_response(int response, Self && self)*

**brief** Handle a `NegotiationResponse` by initiating an async write of the negotiation.

### do_response

*private void do_response(std::string response, Self && self)*

**brief** Handle a `std::string` by initiating an async write of raw data.

### do_response

*private void do_response(awaitables::subnegotiation_awaitable awaitable, Self && self)*

**brief** Handle a `SubnegotiationAwaitable` by spawning a coroutine to process it.

### do_response

*private void do_response(int response, Self && self)*

**brief** Handle any `TaggedAwaitable` with optional `NegotiationResponse`.



## Enums

| enum class State |

--

| INITIALIZING |
| READING |
| PROCESSING |
| DONE |


*Defined at net/telnet/src/net.telnet-stream.cppm#318*



