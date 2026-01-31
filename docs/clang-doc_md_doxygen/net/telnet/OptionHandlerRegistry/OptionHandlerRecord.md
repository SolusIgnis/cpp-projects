# struct OptionHandlerRecord

*Defined at net/telnet/src/net.telnet-internal.cppm#52*



**brief** Record for handlers registered to a single Telnet option.

**details** Stores an optional enablement handler, an optional disablement handler, and an optional subnegotiation handler for processing option-specific data.

**see** `:options` for `option::id_num`, `:protocol_fsm` for usage



## Members

public std::optional<OptionEnablementHandler> enablement_handler

public std::optional<OptionDisablementHandler> disablement_handler

public std::optional<SubnegotiationHandler> subnegotiation_handler



