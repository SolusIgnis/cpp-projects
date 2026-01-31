# class DefaultProtocolFSMConfig

*Defined at net/telnet/src/net.telnet-protocol_config.cppm#36*



**brief** Default configuration class for `ProtocolFSM`, encapsulating options and handlers.

**remark** Provides thread-safe access to static members via `mutex_`.

**see** RFC 854 for Telnet protocol, RFC 855 for option negotiation, :options for `option` and `option::id_num`, :errors for error codes, :internal for implementation classes



## Members

public static option_registry registered_options

private static function unknown_option_handler_

private static function error_logger_

private static basic_string ayt_response_

private static shared_mutex mutex_

private static once_flag initialization_flag_



## Functions

### initialize

*public static void initialize()*

*Defined at net/telnet/src/net.telnet-protocol_config.cppm#54*

**brief** Initializes the configuration once.

### set_unknown_option_handler

*public static void set_unknown_option_handler(UnknownOptionHandler handler)*

*Defined at net/telnet/src/net.telnet-protocol_config.cppm#59*

**brief** Sets the handler for unknown option negotiation attempts.

### set_error_logger

*public static void set_error_logger(ErrorLogger handler)*

*Defined at net/telnet/src/net.telnet-protocol_config.cppm#65*

**brief** Sets the error logging handler.

### get_unknown_option_handler

*public static const UnknownOptionHandler & get_unknown_option_handler()*

*Defined at net/telnet/src/net.telnet-protocol_config.cppm#71*

**brief** Gets the handler for unknown option negotiation attempts.

### log_error

*public static void log_error(const std::error_code & ec, std::format_string<std::remove_cvref_t<Args>...> fmt, Args &&... args)*

*Defined at net/telnet/src/net.telnet-protocol_config.cppm#78*

**brief** Logs an error with the registered error logger using a formatted string.

### get_ayt_response

*public static std::string_view get_ayt_response()*

*Defined at net/telnet/src/net.telnet-protocol_config.cppm#86*

**brief** Gets the AYT response string.

### set_ayt_response

*public static void set_ayt_response(std::string response)*

*Defined at net/telnet/src/net.telnet-protocol_config.cppm#92*

**brief** Sets the AYT response string.

### initialize_option_registry

*private static option_registry initialize_option_registry()*

*Defined at net/telnet/src/net.telnet-protocol_config.cppm#99*

**brief** Initializes the option registry with default options.

### init

*private static void init()*

*Defined at net/telnet/src/net.telnet-protocol_config.cppm#112*

**brief** Performs initialization for `initialize`.



