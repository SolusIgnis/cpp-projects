# class default_protocol_fsm_config

*Defined at net/telnet/src/net.telnet-protocol_config.cppm#36*

## Members

public static option_registry registered_options

private static function unknown_option_handler

private static function error_logger

private static basic_string ayt_response

private static shared_mutex mutex

private static once_flag initialization_flag



## Functions

### initialize

*public static void initialize()*

*Defined at net/telnet/src/net.telnet-protocol_config.cppm#54*

### set_unknown_option_handler

*public static void set_unknown_option_handler(unknown_option_handler_type handler)*

*Defined at net/telnet/src/net.telnet-protocol_config.cppm#57*

### set_error_logger

*public static void set_error_logger(error_logger_type handler)*

*Defined at net/telnet/src/net.telnet-protocol_config.cppm#64*

### get_unknown_option_handler

*public static const unknown_option_handler_type & get_unknown_option_handler()*

*Defined at net/telnet/src/net.telnet-protocol_config.cppm#70*

### log_error

*public static void log_error(const std::error_code & ec, std::format_string<std::remove_cvref_t<Args>...> fmt, Args &&... args)*

*Defined at net/telnet/src/net.telnet-protocol_config.cppm#78*

### get_ayt_response

*public static std::string_view get_ayt_response()*

*Defined at net/telnet/src/net.telnet-protocol_config.cppm#88*

### set_ayt_response

*public static void set_ayt_response(std::string response)*

*Defined at net/telnet/src/net.telnet-protocol_config.cppm#95*

### initialize_option_registry

*private static option_registry initialize_option_registry()*

*Defined at net/telnet/src/net.telnet-protocol_config.cppm#103*

### init

*private static void init()*

*Defined at net/telnet/src/net.telnet-protocol_config.cppm#121*



