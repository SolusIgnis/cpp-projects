# class option_handler_registry

*Defined at net/telnet/src/net.telnet-internal.cppm#50*

## Members

private std::map<option::id_num, option_handler_record> handlers_



## Records

option_handler_record



## Functions

### register_handlers

*public void register_handlers(option::id_num opt, std::optional<OptionEnablementHandler> enablement_handler, std::optional<OptionDisablementHandler> disablement_handler, std::optional<SubnegotiationHandler> subnegotiation_handler)*

*Defined at net/telnet/src/net.telnet-internal.cppm#73*

### unregister_handlers

*public void unregister_handlers(option::id_num opt)*

*Defined at net/telnet/src/net.telnet-internal.cppm#91*

### handle_enablement

*public awaitables::option_enablement_awaitable handle_enablement(const option opt, negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#94*

### handle_disablement

*public awaitables::option_disablement_awaitable handle_disablement(const option opt, negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#105*

### handle_subnegotiation

*public awaitables::subnegotiation_awaitable handle_subnegotiation(const option opt, std::vector<byte_t> data)*

*Defined at net/telnet/src/net.telnet-internal.cppm#116*

### undefined_subnegotiation_handler

*private awaitables::subnegotiation_awaitable undefined_subnegotiation_handler(option opt, std::vector<byte_t> )*

*Defined at net/telnet/src/net.telnet-internal.cppm#129*



