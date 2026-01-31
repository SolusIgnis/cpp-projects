# namespace telnet



## Namespaces

* [awaitables](awaitables/index.md)
* [concepts](concepts/index.md)


## Records

* [default_protocol_fsm_config](default_protocol_fsm_config.md)
* [option](option.md)
* [option_handler_registry](option_handler_registry.md)
* [option_registry](option_registry.md)
* [option_status_db](option_status_db.md)
* [option_status_record](option_status_record.md)
* [protocol_fsm](protocol_fsm.md)
* [stream](stream.md)
* [telnet_error_category](telnet_error_category.md)
* [telnet_processing_signal_category](telnet_processing_signal_category.md)


## Functions

### make_error_code

*std::error_code make_error_code(error ec)*

*Defined at net/telnet/src/net.telnet-errors.cppm#222*

### make_error_code

*std::error_code make_error_code(processing_signal ec)*

*Defined at net/telnet/src/net.telnet-errors.cppm#233*



## Enums

| enum class error |

--

| protocol_violation |
| internal_error |
| invalid_command |
| invalid_negotiation |
| option_not_available |
| invalid_subnegotiation |
| subnegotiation_overflow |
| ignored_go_ahead |
| user_handler_forbidden |
| user_handler_not_found |
| negotiation_queue_error |


*Defined at net/telnet/src/net.telnet-errors.cppm#27*

| enum class command |

--

| eor |
| se |
| nop |
| dm |
| brk |
| ip |
| ao |
| ayt |
| ec |
| el |
| ga |
| sb |
| will_opt |
| wont_opt |
| do_opt |
| dont_opt |
| iac |


*Defined at net/telnet/src/net.telnet-types.cppm#38*

| enum class processing_signal |

--

| end_of_line |
| carriage_return |
| end_of_record |
| go_ahead |
| erase_character |
| erase_line |
| abort_output |
| interrupt_process |
| telnet_break |
| data_mark |


*Defined at net/telnet/src/net.telnet-errors.cppm#46*

| enum class negotiation_direction |

--

| local |
| remote |


*Defined at net/telnet/src/net.telnet-types.cppm#63*



