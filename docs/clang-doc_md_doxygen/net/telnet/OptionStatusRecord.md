# class OptionStatusRecord

*Defined at net/telnet/src/net.telnet-internal.cppm#185*



**brief** Record for tracking the status of a single Telnet option.

**remark** Optimized with bit-fields packed into a single byte for memory efficiency.

**note** Implements RFC 1143 Q Method with 4-state FSMs plus queue bits for local (us) and remote (him) sides.

**remark** Instantiated per-`ProtocolFSM` and used in a single thread/strand.

**see** `:types` for `negotiation_direction`, `:options` for `option::id_num`, `:protocol_fsm` for usage



## Members

private std::uint8_t local_state_

private std::uint8_t remote_state_

private std::uint8_t local_queue_

private std::uint8_t remote_queue_



## Functions

### local_enabled

*public bool local_enabled()*

*Defined at net/telnet/src/net.telnet-internal.cppm#191*

**brief** Checks if the option is enabled locally.

### local_disabled

*public bool local_disabled()*

*Defined at net/telnet/src/net.telnet-internal.cppm#193*

**brief** Checks if the option is fully disabled locally. 

**note** NOT equivalent to !local_enabled()

### local_pending_enable

*public bool local_pending_enable()*

*Defined at net/telnet/src/net.telnet-internal.cppm#195*

**brief** Checks if a local enablement request is pending.

### local_pending_disable

*public bool local_pending_disable()*

*Defined at net/telnet/src/net.telnet-internal.cppm#197*

**brief** Checks if a local disablement request is pending.

### local_pending

*public bool local_pending()*

*Defined at net/telnet/src/net.telnet-internal.cppm#199*

**brief** Checks if local negotiation is pending (WANTNO or WANTYES).

### remote_enabled

*public bool remote_enabled()*

*Defined at net/telnet/src/net.telnet-internal.cppm#203*

**brief** Checks if the option is enabled remotely.

### remote_disabled

*public bool remote_disabled()*

*Defined at net/telnet/src/net.telnet-internal.cppm#205*

**brief** Checks if the option is fully disabled remotely. 

**note** NOT equivalent to !remote_enabled()

### remote_pending_enable

*public bool remote_pending_enable()*

*Defined at net/telnet/src/net.telnet-internal.cppm#207*

**brief** Checks if a remote enablement request is pending.

### remote_pending_disable

*public bool remote_pending_disable()*

*Defined at net/telnet/src/net.telnet-internal.cppm#209*

**brief** Checks if a remote disablement request is pending.

### remote_pending

*public bool remote_pending()*

*Defined at net/telnet/src/net.telnet-internal.cppm#211*

**brief** Checks if remote negotiation is pending (WANTNO or WANTYES).

### enabled

*public bool enabled(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#215*

**brief** Checks if the option is enabled in the designated direction.

### disabled

*public bool disabled(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#217*

**brief** Checks if the option is disabled in the designated direction. 

**note** NOT equivalent to !enabled(direction)

### pending_enable

*public bool pending_enable(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#219*

**brief** Checks if an enablement request is pending in the designated direction.

### pending_disable

*public bool pending_disable(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#221*

**brief** Checks if a disablement request is pending in the designated direction.

### pending

*public bool pending(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#223*

**brief** Checks if a negotiation is pending (WANTNO or WANTYES) in the designated direction.

### is_enabled

*public bool is_enabled()*

*Defined at net/telnet/src/net.telnet-internal.cppm#227*

**brief** Checks if the option is enabled locally or remotely.

### local_queued

*public bool local_queued()*

*Defined at net/telnet/src/net.telnet-internal.cppm#231*

**brief** Checks if a local user request is queued (OPPOSITE state).

### remote_queued

*public bool remote_queued()*

*Defined at net/telnet/src/net.telnet-internal.cppm#233*

**brief** Checks if a remote user request is queued (OPPOSITE state).

### queued

*public bool queued(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#235*

**brief** Checks if a user request is queued (OPPOSITE state) in the designated direction.

### has_queued_request

*public bool has_queued_request()*

*Defined at net/telnet/src/net.telnet-internal.cppm#237*

**brief** Checks if any user request is queued (local or remote). [Optional]

### enable_local

*public void enable_local()*

*Defined at net/telnet/src/net.telnet-internal.cppm#241*

**brief** Enables the option locally.

### disable_local

*public void disable_local()*

*Defined at net/telnet/src/net.telnet-internal.cppm#243*

**brief** Disables the option locally.

### pend_enable_local

*public void pend_enable_local()*

*Defined at net/telnet/src/net.telnet-internal.cppm#245*

**brief** Marks a local enablement request as pending.

### pend_disable_local

*public void pend_disable_local()*

*Defined at net/telnet/src/net.telnet-internal.cppm#247*

**brief** Marks a local disablement request as pending.

### enable_remote

*public void enable_remote()*

*Defined at net/telnet/src/net.telnet-internal.cppm#251*

**brief** Enables the option remotely.

### disable_remote

*public void disable_remote()*

*Defined at net/telnet/src/net.telnet-internal.cppm#253*

**brief** Disables the option remotely.

### pend_enable_remote

*public void pend_enable_remote()*

*Defined at net/telnet/src/net.telnet-internal.cppm#255*

**brief** Marks a remote enablement request as pending.

### pend_disable_remote

*public void pend_disable_remote()*

*Defined at net/telnet/src/net.telnet-internal.cppm#257*

**brief** Marks a remote disablement request as pending.

### enable

*public void enable(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#261*

**brief** Enables the option in the designated direction.

### disable

*public void disable(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#263*

**brief** Disables the option in the designated direction.

### pend_enable

*public void pend_enable(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#265*

**brief** Marks an enablement request as pending in the designated direction.

### pend_disable

*public void pend_disable(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#267*

**brief** Marks a disablement request as pending in the designated direction.

### enqueue_local

*public std::error_code enqueue_local()*

*Defined at net/telnet/src/net.telnet-internal.cppm#271*

**brief** Sets a local user request as queued (OPPOSITE state).

### enqueue_remote

*public std::error_code enqueue_remote()*

*Defined at net/telnet/src/net.telnet-internal.cppm#273*

**brief** Sets a remote user request as queued (OPPOSITE state).

### enqueue

*public std::error_code enqueue(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#275*

**brief** Sets a user request as queued (OPPOSITE state) in the designated direction.

### dequeue_local

*public void dequeue_local()*

*Defined at net/telnet/src/net.telnet-internal.cppm#277*

**brief** Clears a queued local user request.

### dequeue_remote

*public void dequeue_remote()*

*Defined at net/telnet/src/net.telnet-internal.cppm#279*

**brief** Clears a queued remote user request.

### dequeue

*public void dequeue(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#281*

**brief** Clears a queued user request in the designated direction.

### clear_queued_requests

*public void clear_queued_requests()*

*Defined at net/telnet/src/net.telnet-internal.cppm#283*

**brief** Clears all queued requests.

### reset

*public void reset()*

*Defined at net/telnet/src/net.telnet-internal.cppm#287*

**brief** Resets all state to initial values (NO, no queued requests).

### is_negotiating

*public bool is_negotiating()*

*Defined at net/telnet/src/net.telnet-internal.cppm#293*

**brief** Checks if either local or remote negotiation is pending. [Optional]

### is_valid

*public bool is_valid()*

*Defined at net/telnet/src/net.telnet-internal.cppm#295*

**brief** Validates state consistency (e.g., queue flags false when not pending). [Optional]



## Enums

| enum class NegotiationState |

--

| NO |
| YES |
| WANTNO |
| WANTYES |


*Defined at net/telnet/src/net.telnet-internal.cppm#187*



