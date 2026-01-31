# class option_status_record

*Defined at net/telnet/src/net.telnet-internal.cppm#195*

## Members

private std::uint8_t local_state_

private std::uint8_t remote_state_

private std::uint8_t local_queue_

private std::uint8_t remote_queue_



## Functions

### local_enabled

*public bool local_enabled()*

*Defined at net/telnet/src/net.telnet-internal.cppm#204*

### local_disabled

*public bool local_disabled()*

*Defined at net/telnet/src/net.telnet-internal.cppm#210*

### local_pending_enable

*public bool local_pending_enable()*

*Defined at net/telnet/src/net.telnet-internal.cppm#216*

### local_pending_disable

*public bool local_pending_disable()*

*Defined at net/telnet/src/net.telnet-internal.cppm#222*

### local_pending

*public bool local_pending()*

*Defined at net/telnet/src/net.telnet-internal.cppm#226*

### remote_enabled

*public bool remote_enabled()*

*Defined at net/telnet/src/net.telnet-internal.cppm#232*

### remote_disabled

*public bool remote_disabled()*

*Defined at net/telnet/src/net.telnet-internal.cppm#236*

### remote_pending_enable

*public bool remote_pending_enable()*

*Defined at net/telnet/src/net.telnet-internal.cppm#242*

### remote_pending_disable

*public bool remote_pending_disable()*

*Defined at net/telnet/src/net.telnet-internal.cppm#248*

### remote_pending

*public bool remote_pending()*

*Defined at net/telnet/src/net.telnet-internal.cppm#253*

### enabled

*public bool enabled(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#260*

### disabled

*public bool disabled(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#266*

### pending_enable

*public bool pending_enable(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#272*

### pending_disable

*public bool pending_disable(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#278*

### pending

*public bool pending(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#284*

### is_enabled

*public bool is_enabled()*

*Defined at net/telnet/src/net.telnet-internal.cppm#291*

### local_queued

*public bool local_queued()*

*Defined at net/telnet/src/net.telnet-internal.cppm#295*

### remote_queued

*public bool remote_queued()*

*Defined at net/telnet/src/net.telnet-internal.cppm#298*

### queued

*public bool queued(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#301*

### has_queued_request

*public bool has_queued_request()*

*Defined at net/telnet/src/net.telnet-internal.cppm#307*

### enable_local

*public void enable_local()*

*Defined at net/telnet/src/net.telnet-internal.cppm#310*

### disable_local

*public void disable_local()*

*Defined at net/telnet/src/net.telnet-internal.cppm#312*

### pend_enable_local

*public void pend_enable_local()*

*Defined at net/telnet/src/net.telnet-internal.cppm#315*

### pend_disable_local

*public void pend_disable_local()*

*Defined at net/telnet/src/net.telnet-internal.cppm#318*

### enable_remote

*public void enable_remote()*

*Defined at net/telnet/src/net.telnet-internal.cppm#323*

### disable_remote

*public void disable_remote()*

*Defined at net/telnet/src/net.telnet-internal.cppm#325*

### pend_enable_remote

*public void pend_enable_remote()*

*Defined at net/telnet/src/net.telnet-internal.cppm#328*

### pend_disable_remote

*public void pend_disable_remote()*

*Defined at net/telnet/src/net.telnet-internal.cppm#331*

### enable

*public void enable(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#336*

### disable

*public void disable(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#345*

### pend_enable

*public void pend_enable(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#355*

### pend_disable

*public void pend_disable(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#365*

### enqueue_local

*public std::error_code enqueue_local()*

*Defined at net/telnet/src/net.telnet-internal.cppm#376*

### enqueue_remote

*public std::error_code enqueue_remote()*

*Defined at net/telnet/src/net.telnet-internal.cppm#386*

### enqueue

*public std::error_code enqueue(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#398*

### dequeue_local

*public void dequeue_local()*

*Defined at net/telnet/src/net.telnet-internal.cppm#406*

### dequeue_remote

*public void dequeue_remote()*

*Defined at net/telnet/src/net.telnet-internal.cppm#410*

### dequeue

*public void dequeue(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-internal.cppm#414*

### clear_queued_requests

*public void clear_queued_requests()*

*Defined at net/telnet/src/net.telnet-internal.cppm#422*

### reset

*public void reset()*

*Defined at net/telnet/src/net.telnet-internal.cppm#427*

### is_negotiating

*public bool is_negotiating()*

*Defined at net/telnet/src/net.telnet-internal.cppm#436*

### is_valid

*public bool is_valid()*

*Defined at net/telnet/src/net.telnet-internal.cppm#440*



## Enums

| enum class negotiation_state |

--

| no |
| yes |
| want_no |
| want_yes |


*Defined at net/telnet/src/net.telnet-internal.cppm#197*



