# class ProtocolFSM

*Defined at net/telnet/src/net.telnet-protocol_fsm.cppm#54*



**brief** Finite State Machine for Telnet protocol processing.

**ConfigT** Configuration class defining options and handlers (defaults to `DefaultProtocolFSMConfig`).

**remark** Initialized to `ProtocolState::Normal` per RFC 854.

**remark** Thread-safe for `ProtocolConfig` access via `mutex_`.

**see** RFC 854 for Telnet protocol, RFC 855 for option negotiation, `:options` for `option` and `option::id_num`, `:types` for `telnet::command`, `:errors` for error codes, `:stream` for FSM usage, `:internal` for implementation classes



## Members

private int option_handler_registry_

private int option_status_

private ProtocolState current_state_

private std::optional<telnet::command> current_command_

private optional current_option_

private int subnegotiation_buffer_



## Functions

### ProtocolFSM<ConfigT>

*public void ProtocolFSM<ConfigT>()*

*Defined at net/telnet/src/net.telnet-protocol_fsm.cppm#126*

**brief** Constructs the FSM, initializing `ProtocolConfig` once.

### process_byte

*public int process_byte(byte_t byte)*

**brief** Processes a single byte of Telnet input.

### make_negotiation_command

*public static telnet::command make_negotiation_command(negotiation_direction direction, bool enable)*

**brief** Makes a negotiation response command

### request_option

*public std::tuple<std::error_code, std::optional<NegotiationResponse>> request_option(option::id_num opt, negotiation_direction direction)*

**brief** Requests an option to be enabled (WILL/DO), synchronously updating OptionStatusDB and returning a negotiation response.

### disable_option

*public int disable_option(option::id_num opt, negotiation_direction direction)*

**brief** Disables an option (WONT/DONT), synchronously updating OptionStatusDB and returning a negotiation response and optional disablement awaitable.

### change_state

*private void change_state(ProtocolState next_state)*

**brief** Changes the FSM state.

### handle_state_normal

*private int handle_state_normal(byte_t byte)*

**brief** Handles bytes in the `Normal` state (data or IAC).

### handle_state_has_cr

*private int handle_state_has_cr(byte_t byte)*

**brief** Handles bytes immediately after '**<not a builtin command>** ' ('\0', '**n** ', or error)

### handle_state_iac

*private int handle_state_iac(byte_t byte)*

**brief** Handles bytes after IAC (commands like WILL, DO, SB, etc.).

### handle_state_option_negotiation

*private int handle_state_option_negotiation(byte_t byte)*

**brief** Handles bytes in the `OptionNegotiation` state (option ID after WILL/WONT/DO/DONT).

### handle_state_subnegotiation_option

*private int handle_state_subnegotiation_option(byte_t byte)*

**brief** Handles bytes in the `SubnegotiationOption` state (option ID after IAC SB).

### handle_state_subnegotiation

*private int handle_state_subnegotiation(byte_t byte)*

**brief** Handles bytes in the `Subnegotiation` state (data after option ID).

### handle_state_subnegotiation_iac

*private int handle_state_subnegotiation_iac(byte_t byte)*

**brief** Handles bytes in the `SubnegotiationIAC` state (IAC in subnegotiation data).

### handle_status_subnegotiation

*private int handle_status_subnegotiation(option opt, int buffer)*

**brief** Handles STATUS subnegotiation (RFC 859), returning an awaitable with the IS [list] payload or user-handled result.

### register_option_handlers

*public void register_option_handlers(option::id_num opt, int enable_handler, int disable_handler, int subneg_handler)*

*Defined at net/telnet/src/net.telnet-protocol_fsm.cppm#131*

**brief** Registers handlers for option enablement, disablement, and subnegotiation.

### unregister_option_handlers

*public void unregister_option_handlers(option::id_num opt)*

*Defined at net/telnet/src/net.telnet-protocol_fsm.cppm#134*

**brief** Unregisters handlers for an option.

### is_enabled

*public bool is_enabled(option::id_num opt)*

*Defined at net/telnet/src/net.telnet-protocol_fsm.cppm#141*

**brief** Checks if an option is enabled locally or remotely.

### is_enabled

*public bool is_enabled(option::id_num opt, negotiation_direction dir)*

*Defined at net/telnet/src/net.telnet-protocol_fsm.cppm#144*

**brief** Checks if an option is enabled in a specified direction.



## Enums

| enum class ProtocolState |

--

| Normal |
| HasCR |
| HasIAC |
| OptionNegotiation |
| SubnegotiationOption |
| Subnegotiation |
| SubnegotiationIAC |


*Defined at net/telnet/src/net.telnet-protocol_fsm.cppm#156*



