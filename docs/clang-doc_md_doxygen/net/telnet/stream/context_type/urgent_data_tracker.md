# class urgent_data_tracker

*Defined at net/telnet/src/net.telnet-stream.cppm#246*

## Members

private std::atomic<urgent_data_state> state_



## Functions

### saw_urgent

*public void saw_urgent()*

*Defined at net/telnet/src/impl/net.telnet-stream-impl.cpp#180*



 Use `compare_exchange_strong` to update `state_`.

 If `state_` was `no_urgent_data`, it becomes `has_urgent_data`.

 If `state_` was `unexpected_data_mark`, it resets to `no_urgent_data`.

 It can't already be `has_urgent_data` without a major bug.





**brief** Updates the state when the OOB notification arrives.

### saw_data_mark

*public void saw_data_mark()*

*Defined at net/telnet/src/impl/net.telnet-stream-impl.cpp#228*



 Use `compare_exchange_strong` to update `state_`.

 If `state_` was `no_urgent_data`, it becomes `unexpected_data_mark`.

 If `state_` was `has_urgent_data`, it resets to `no_urgent_data`.





**brief** Updates the state when the `telnet::command::dm` arrives.

### has_urgent_data

*public bool has_urgent_data()*

*Defined at net/telnet/src/net.telnet-stream.cppm#265*

### operator bool

*public bool operator bool()*

*Defined at net/telnet/src/net.telnet-stream.cppm#273*



## Enums

| enum class urgent_data_state |

--

| no_urgent_data |
| has_urgent_data |
| unexpected_data_mark |


*Defined at net/telnet/src/net.telnet-stream.cppm#252*



