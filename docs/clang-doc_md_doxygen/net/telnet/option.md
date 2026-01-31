# class option

*Defined at net/telnet/src/net.telnet-options.cppm#39*

## Members

private id_num id_

private basic_string name_

private function local_predicate_

private function remote_predicate_

private bool supports_subnegotiation_

private size_t max_subneg_size_

private static const size_t max_subnegotiation_size_



## Functions

### option

*public void option(id_num id, std::string name, enable_predicate_type local_pred, enable_predicate_type remote_pred, bool subneg_supported, size_t max_subneg_size)*

*Defined at net/telnet/src/net.telnet-options.cppm#58*

### make_option

*public static option make_option(id_num id, std::string name, bool local_supported, bool remote_supported, bool subneg_supported, size_t max_subneg_size)*

*Defined at net/telnet/src/net.telnet-options.cppm#74*

### operator<=>

*public std::__1::strong_ordering operator<=>(const option & other)*

*Defined at net/telnet/src/net.telnet-options.cppm#94*

### operator<=>

*public std::__1::strong_ordering operator<=>(option::id_num other_id)*

*Defined at net/telnet/src/net.telnet-options.cppm#97*

### operator net::telnet::option::id_num

*public id_num operator net::telnet::option::id_num()*

*Defined at net/telnet/src/net.telnet-options.cppm#100*

### get_id

*public id_num get_id()*

*Defined at net/telnet/src/net.telnet-options.cppm#104*

### get_name

*public const std::string & get_name()*

*Defined at net/telnet/src/net.telnet-options.cppm#106*

### supports_local

*public bool supports_local()*

*Defined at net/telnet/src/net.telnet-options.cppm#109*

### supports_remote

*public bool supports_remote()*

*Defined at net/telnet/src/net.telnet-options.cppm#112*

### supports

*public bool supports(negotiation_direction direction)*

*Defined at net/telnet/src/net.telnet-options.cppm#115*

### max_subnegotiation_size

*public size_t max_subnegotiation_size()*

*Defined at net/telnet/src/net.telnet-options.cppm#121*

### supports_subnegotiation

*public bool supports_subnegotiation()*

*Defined at net/telnet/src/net.telnet-options.cppm#124*

### always_accept

*public static bool always_accept(id_num )*

*Defined at net/telnet/src/net.telnet-options.cppm#127*

### always_reject

*public static bool always_reject(id_num )*

*Defined at net/telnet/src/net.telnet-options.cppm#131*



## Enums

| enum class id_num |

--

| binary |
| echo |
| reconnection |
| suppress_go_ahead |
| approx_message_size_negotiation |
| status |
| timing_mark |
| remote_controlled_trans_and_echo |
| output_line_width |
| output_page_size |
| output_carriage_return_disposition |
| output_horizontal_tab_stops |
| output_horizontal_tab_disposition |
| output_formfeed_disposition |
| output_vertical_tabstops |
| output_vertical_tab_disposition |
| output_linefeed_disposition |
| extended_ascii |
| logout |
| byte_macro |
| data_entry_terminal |
| supdup |
| supdup_output |
| send_location |
| terminal_type |
| end_of_record |
| tacacs_user_identification |
| output_marking |
| terminal_location_number |
| telnet_3270_regime |
| x_3_pad |
| negotiate_about_window_size |
| terminal_speed |
| remote_flow_control |
| linemode |
| x_display_location |
| environment_option |
| authentication |
| encrypt_deprecated |
| new_environment_option |
| tn3270e |
| xauth |
| charset |
| telnet_remote_serial_port |
| com_port_control_option |
| telnet_suppress_local_echo |
| telnet_start_tls |
| kermit |
| send_url |
| forward_x |
| msdp |
| mssp |
| gssapi |
| mccp1 |
| mccp2 |
| mccp3 |
| msp |
| mxp |
| zmp |
| pueblo |
| aardwolf_102 |
| telopt_pragma_logon |
| telopt_sspi_logon |
| telopt_pragma_heartbeat |
| ssl_deprecated |
| mcp |
| atcp |
| gmcp |
| extended_options_list |


*Defined at net/telnet/src/net.telnet-options.cppm#242*



