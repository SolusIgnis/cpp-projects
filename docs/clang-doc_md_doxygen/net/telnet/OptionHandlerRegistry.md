# class OptionHandlerRegistry

*Defined at net/telnet/src/net.telnet-internal.cppm#45*



**brief** Registry for managing Telnet option handlers.

**ProtocolConfig** Configuration class providing logging and error handling.

**OptionEnablementHandler** Handler type for option enablement.

**OptionDisablementHandler** Handler type for option disablement.

**SubnegotiationHandler** Handler type for processing subnegotiation data.

**remark** Used by `:protocol_fsm` to manage handlers for Telnet options.

**remark** Instantiated per-`ProtocolFSM` and used in a single thread/strand.

**see** `:protocol_fsm` for handler usage, `:options` for `option::id_num`, `:errors` for error codes



## Members

private std::map<option::id_num, OptionHandlerRecord> handlers_



## Records

OptionHandlerRecord



## Functions

### register_handlers

*public void register_handlers(option::id_num opt, std::optional<OptionEnablementHandler> enablement_handler, std::optional<OptionDisablementHandler> disablement_handler, std::optional<SubnegotiationHandler> subnegotiation_handler)*

*Defined at net/telnet/src/net.telnet-internal.cppm#68*



**brief** Registers handlers for a Telnet option.

**opt** The `option::id_num` to register handlers for.

**enablement_handler** Optional handler for option enablement.

**disablement_handler** Optional handler for option disablement.

**subnegotiation_handler** Optional handler for subnegotiation (defaults to `std::nullopt`).

**remark** Overwrites existing handlers for the specified option.

**see** `:options` for `option::id_num`, `:awaitables` for handler return types

### unregister_handlers

*public void unregister_handlers(option::id_num opt)*

*Defined at net/telnet/src/net.telnet-internal.cppm#85*



**brief** Unregisters all handlers for a Telnet option.

**opt** The `option::id_num` to unregister handlers for.

**remark** Removes the handler record from the registry.

**see** `:options` for `option::id_num`

### handle_enablement

*public awaitables::option_enablement_awaitable handle_enablement(const option opt, negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#90*

**brief** Handles enablement for a Telnet option.

### handle_disablement

*public awaitables::option_disablement_awaitable handle_disablement(const option opt, negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#100*

**brief** Handles disablement for a Telnet option.

### handle_subnegotiation

*public awaitables::subnegotiation_awaitable handle_subnegotiation(const option opt, std::vector<byte_t> data)*

*Defined at net/telnet/src/net.telnet-internal.cppm#110*

**brief** Handles subnegotiation for a Telnet option.

### undefined_subnegotiation_handler

*private awaitables::subnegotiation_awaitable undefined_subnegotiation_handler(option opt, std::vector<byte_t> )*

*Defined at net/telnet/src/net.telnet-internal.cppm#121*

**brief** Default handler for undefined subnegotiation.



