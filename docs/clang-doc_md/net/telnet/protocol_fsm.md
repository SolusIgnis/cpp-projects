# class protocol_fsm

*Defined at net/telnet/src/net.telnet-protocol_fsm.cppm#51*

## Members

private option_handler_registry<protocol_config_type, option_enablement_handler_type, option_disablement_handler_type, subnegotiation_handler_type> option_handler_registry_

private option_status_db option_status_

private protocol_state current_state_

private optional current_command_

private optional current_option_

private vector subnegotiation_buffer_



## Functions

### make_negotiation_command

*public static telnet::command make_negotiation_command(negotiation_direction direction, bool enable)*

*Defined at net/telnet/src/impl/net.telnet-protocol_fsm-impl.cpp#31*



 Selects the `telnet::command` corresponding to a given `negotiation_direction` and enablement state





**brief** Makes a negotiation response command

### register_option_handlers

*public void register_option_handlers(option::id_num opt, std::optional<option_enablement_handler_type> enable_handler, std::optional<option_disablement_handler_type> disable_handler, std::optional<subnegotiation_handler_type> subneg_handler)*

*Defined at net/telnet/src/net.telnet-protocol_fsm.cppm#132*

### unregister_option_handlers

*public void unregister_option_handlers(option::id_num opt)*

*Defined at net/telnet/src/net.telnet-protocol_fsm.cppm#148*

### is_enabled

*public bool is_enabled(option::id_num opt)*

*Defined at net/telnet/src/net.telnet-protocol_fsm.cppm#154*

### is_enabled

*public bool is_enabled(option::id_num opt, negotiation_direction dir)*

*Defined at net/telnet/src/net.telnet-protocol_fsm.cppm#157*

### process_byte

*public std::tuple<std::error_code, bool, std::optional<processing_return_variant>> process_byte(byte_t byte)*

*Defined at net/telnet/src/impl/net.telnet-protocol_fsm-impl.cpp#224*



 Dispatches to state-specific handlers based on `current_state_`.

 Handles invalid states (e.g., memory corruption) by logging `error::protocol_violation`, resetting to `protocol_state::normal`, and returning an error tuple.

 Uses `[[unlikely]]` for the default case to optimize for valid states.





**brief** Processes a single byte of Telnet input.

### request_option

*public std::tuple<std::error_code, std::optional<negotiation_response_type>> request_option(option::id_num opt, negotiation_direction direction)*

*Defined at net/telnet/src/impl/net.telnet-protocol_fsm-impl.cpp#50*



 Validates option registration and updates `option_status_db` based on the six states per RFC 1143 Q Method.

 Handles redundant requests (`YES`, `WANTYES/EMPTY`, `WANTNO/OPPOSITE`) as idempotent successes, logging warnings.

 Enqueues opposite requests in `WANTNO/EMPTY` to transition to `WANTNO/OPPOSITE`.

 Transitions to `WANTYES`/`EMPTY` in `NO` state, returning a `negotiation_response_type` to initiate negotiation.

 Does not invoke an enablement handler, as it initiates negotiation without enabling.

 Logs errors for unregistered options (`error::option_not_available`), queue failures (`error::negotiation_queue_error`), or invalid states (`error::protocol_violation`).

 @see RFC 1143 for Q Method, `:options` for `option::id_num`, `:errors` for error codes, `:types` for `negotiation_direction`, `:stream` for usage in `async_request_option`





**brief** Requests an option to be enabled (WILL/DO), synchronously updating option_status_db and returning a negotiation response.

### disable_option

*public std::tuple<std::error_code, std::optional<negotiation_response_type>, std::optional<awaitables::option_disablement_awaitable>> disable_option(option::id_num opt, negotiation_direction direction)*

*Defined at net/telnet/src/impl/net.telnet-protocol_fsm-impl.cpp#128*



 Validates option registration and updates `option_status_db` based on the six states per RFC 1143 Q Method.

 Handles redundant disablements (`NO`, `WANTNO`/`EMPTY`, `WANTYES`/`OPPOSITE`) as idempotent successes, logging warnings.

 Enqueues opposite requests in `WANTYES`/`EMPTY` to transition to `WANTYES`/`OPPOSITE`.

 Transitions to `WANTNO`/`EMPTY` in `YES` state, returning a `negotiation_response_type` and `option_disablement_awaitable` via `option_handler_registry_.handle_disablement`.

 Logs errors for unregistered options (`error::option_not_available`), queue failures (`error::negotiation_queue_error`), or invalid states (`error::protocol_violation`).

 @see RFC 1143 for Q Method, `:options` for `option::id_num`, `:errors` for error codes, `:types` for `negotiation_direction`, `:awaitables` for `option_disablement_awaitable`, `:stream` for usage in `async_disable_option`





**brief** Disables an option (WONT/DONT), synchronously updating option_status_db and returning a negotiation response and optional disablement awaitable.

### protocol_fsm<ConfigT>

*public void protocol_fsm<ConfigT>()*

*Defined at net/telnet/src/net.telnet-protocol_fsm.cppm#129*

### change_state

*private void change_state(protocol_state next_state)*

*Defined at net/telnet/src/impl/net.telnet-protocol_fsm-impl.cpp#207*



 Transitions `protocol_fsm`'s state.

 @note Resets `current_command_`, `current_option_`, and `subnegotiation_buffer_` when transitioning to `protocol_state::normal`.





**brief** Changes the FSM state.

### handle_state_normal

*private std::tuple<std::error_code, bool, std::optional<processing_return_variant>> handle_state_normal(byte_t byte)*

*Defined at net/telnet/src/impl/net.telnet-protocol_fsm-impl.cpp#263*



 Transitions to `protocol_state::has_iac` if `byte` is `IAC` (0xFF), discarding the byte (returns `false` for forward flag).

 Unless in `BINARY` mode, transitions to `protocol_state::has_cr` if `byte` is `'\r'` (0x0D), forwarding the byte.

 For non-`IAC` bytes, retains the byte as data (returns `true` for forward flag) unless it's nul (`'\0'`).





**brief** Handles bytes in the `Normal` state (data or IAC).

### handle_state_has_cr

*private std::tuple<std::error_code, bool, std::optional<processing_return_variant>> handle_state_has_cr(byte_t byte)*

*Defined at net/telnet/src/impl/net.telnet-protocol_fsm-impl.cpp#288*



 For `'\n'`, forwards the byte (CR LF is EOL) and returns `processing_signal::end_of_line`.

 For `'\0'`, discards the byte (CR NUL drops the NUL) and returns `processing_signal::carriage_return` to buffer the CR.

 For `IAC`, logs a protocol violation and transitions to `protocol_state::has_iac`, discarding the byte and returns `processing_signal::carriage_return` to buffer the bare CR.

 For all other bytes, retains the byte as data (returns `true` for forward flag) but logs a protocol violation, transitions to `protocol_state::normal`, and returns `processing_signal::carriage_return` to buffer the bare CR.

 In all cases other than `IAC`, transitions to `protocol_state::normal`.





**brief** Handles bytes immediately after '**<not a builtin command>** ' ('\0', '**n** ', or error)

### handle_state_iac

*private std::tuple<std::error_code, bool, std::optional<processing_return_variant>> handle_state_iac(byte_t byte)*

*Defined at net/telnet/src/impl/net.telnet-protocol_fsm-impl.cpp#339*



 Handles escaped `IAC` (0xFF) by transitioning to `protocol_state::normal` and retaining the byte as data.

 For command bytes, sets `current_command_` and transitions to appropriate states (`OptionNegotiation` for `WILL`/`WONT`/`DO`/`DONT`, `SubnegotiationOption` for `SB`, `Normal` for others).

 Returns `std::error_code` with `telnet::processing_signal` for commands (`GA`, `EOR`, `EC`, `EL`, `AO`, `IP`, `BRK`, `DM`) to signal early completion or special handling.

 Returns `processing_return_variant` for `AYT` with the response from `protocol_config_type::get_ayt_response`.

 Logs `error::invalid_subnegotiation` for `SE` outside subnegotiation, `error::ignored_go_ahead` for `GA` when `SUPPRESS_GO_AHEAD` is enabled, or `error::invalid_command` for unrecognized commands.

 Logs `error::invalid_command` for commands outside of `telnet::command`.

 Discards command bytes (returns `false` for forward flag).

 Uses `[[likely]]` for valid `current_command_` cases.

 @see RFC 854 for command definitions, `:errors` for `processing_signal` and error codes, `:stream` for `input_processor` handling





**brief** Handles bytes after IAC (commands like WILL, DO, SB, etc.).

### handle_state_option_negotiation

*private std::tuple<std::error_code, bool, std::optional<processing_return_variant>> handle_state_option_negotiation(byte_t byte)*

*Defined at net/telnet/src/impl/net.telnet-protocol_fsm-impl.cpp#454*



 Sets `current_option_` by querying `protocol_config_type::registered_options` with the option ID.

 For known options, evaluates enablement state using `option_status_` and updates `local_enabled_` or `remote_enabled_` based on `current_command_` (`WILL`/`WONT` for remote, `DO`/`DONT` for local).

 Generates a response (`WILL`/`WONT`/`DO`/`DONT`) if the request changes the enablement state and is supported.

 For unknown options, invokes `protocol_config_type::get_unknown_option_handler` or logs `error::option_not_available`, and responds with `DONT`/`WONT` for enablement requests.

 Transitions to `protocol_state::normal` and discards the option byte (returns `false` for forward flag).

 Uses `[[likely]]` for valid `current_command_` cases.





NOLINTBEGIN(readability-function-cognitive-complexity)

**brief** Handles bytes in the `OptionNegotiation` state (option ID after WILL/WONT/DO/DONT).

### handle_state_subnegotiation_option

*private std::tuple<std::error_code, bool, std::optional<processing_return_variant>> handle_state_subnegotiation_option(byte_t byte)*

*Defined at net/telnet/src/impl/net.telnet-protocol_fsm-impl.cpp#603*



 Sets `current_option_` by querying `protocol_config_type::registered_options` with the option ID.

 For unknown options, memoizes a default `option` via `registry.upsert` and logs `error::invalid_subnegotiation`.

 Logs `error::invalid_subnegotiation` for options that don’t support subnegotiation or aren’t enabled in `option_status_`.

 Reserves `subnegotiation_buffer_` based on `current_option_->max_subnegotiation_size()`.

 Transitions to `protocol_state::subnegotiation` and discards the option byte (returns `false` for forward flag).





**brief** Handles bytes in the `SubnegotiationOption` state (option ID after IAC SB).

### handle_state_subnegotiation

*private std::tuple<std::error_code, bool, std::optional<processing_return_variant>> handle_state_subnegotiation(byte_t byte)*

*Defined at net/telnet/src/impl/net.telnet-protocol_fsm-impl.cpp#641*



 Logs `error::protocol_violation` and transitions to `protocol_state::normal` if `current_option_` is unset.

 Transitions to `protocol_state::subnegotiation_iac` if `byte` is `IAC`.

 Checks `subnegotiation_buffer_` size against `current_option_->max_subnegotiation_size()` and logs `error::subnegotiation_overflow` if exceeded, transitioning to `protocol_state::normal`.

 Appends non-`IAC` bytes to `subnegotiation_buffer_` and discards them (returns `false` for forward flag).





**brief** Handles bytes in the `Subnegotiation` state (data after option ID).

### handle_state_subnegotiation_iac

*private std::tuple<std::error_code, bool, std::optional<processing_return_variant>> handle_state_subnegotiation_iac(byte_t byte)*

*Defined at net/telnet/src/impl/net.telnet-protocol_fsm-impl.cpp#681*



 Logs `error::protocol_violation` and transitions to `protocol_state::normal` if `current_option_` is unset.

 For `SE`, completes subnegotiation by invoking `option_handler_registry_.handle_subnegotiation` if supported and enabled, then transitions to `protocol_state::normal`.

 @note For `STATUS` subnegotiation, invokes dedicated `handle_status_subnegotiation` helper to yield the `subnegotiation_awaitable` for internal processing.

 For non-`SE`/non-`IAC` bytes, logs `error::invalid_command`, assumes an unescaped IAC, and appends both `IAC` and the byte to `subnegotiation_buffer_`.

 Checks `subnegotiation_buffer_` size against `max_subnegotiation_size()` and logs `error::subnegotiation_overflow` if exceeded.

 Transitions to `protocol_state::subnegotiation` for non-`SE` bytes and discards all bytes (returns `false` for forward flag).





**brief** Handles bytes in the `SubnegotiationIAC` state (IAC in subnegotiation data).

### handle_status_subnegotiation

*private awaitables::subnegotiation_awaitable handle_status_subnegotiation(option opt, std::vector<byte_t> buffer)*

*Defined at net/telnet/src/impl/net.telnet-protocol_fsm-impl.cpp#746*



 Processes an `IAC` `SB` `STATUS` `SEND` or `IS` sequence, validating the input buffer for `SEND` (1) or `IS` (0) and logging `telnet::error::invalid_subnegotiation` if invalid.

 Validates enablement of `STATUS` option (local for `SEND`, remote for `IS`), logging `telnet::error::option_not_available` if not enabled.

 For `IAC` `SB` `STATUS` `IS` ... `IAC` `SE`, delegates to a user-provided subnegotiation handler via `OptionHandlerRegistry`.

 For `IAC` `SB` `STATUS` `SEND` `IAC` `SE`, constructs an `IS` [list] payload using `option_status_db`, `co_return`ing a `subnegotiation_awaitable`.

 Iterates over `option_status_db` to build the `SEND` payload with enabled options, excluding `STATUS`, escaping `IAC` (255) and `SE` (240) by doubling.

 @see RFC 859, `:internal` for `option_status_db`, `:options` for `option`, `:awaitables` for `subnegotiation_awaitable`, `:stream` for `async_write_subnegotiation`





NOLINTBEGIN(readability-function-cognitive-complexity)

**brief** Handles STATUS subnegotiation (RFC 859), returning an awaitable with the IS [list] payload or user-handled result.



## Enums

| enum class protocol_state |

--

| normal |
| has_cr |
| has_iac |
| option_negotiation |
| subnegotiation_option |
| subnegotiation |
| subnegotiation_iac |


*Defined at net/telnet/src/net.telnet-protocol_fsm.cppm#175*



