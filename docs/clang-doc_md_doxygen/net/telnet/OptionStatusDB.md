# class OptionStatusDB

*Defined at net/telnet/src/net.telnet-internal.cppm#548*



**brief** Collection of `OptionStatusRecord` objects for tracking Telnet option statuses.

**remark** Provides array-based access to option statuses by `option::id_num`.

**remark** Used by `:protocol_fsm` to manage the state of Telnet options.

**remark** Instantiated per-`ProtocolFSM` and used in a single thread/strand.

**see** `:options` for `option::id_num`, `:protocol_fsm` for usage, `OptionStatusRecord` for status details



## Members

private array status_records_

public static const size_t MAX_OPTION_COUNT



## Functions

### operator[]

*public OptionStatusRecord & operator[](option::id_num opt)*

*Defined at net/telnet/src/net.telnet-internal.cppm#551*

**brief** Accesses or creates an `OptionStatusRecord` for a Telnet option.

### operator[]

*public const OptionStatusRecord & operator[](option::id_num opt)*

*Defined at net/telnet/src/net.telnet-internal.cppm#554*

**brief** Retrieves an `OptionStatusRecord` for a Telnet option.



