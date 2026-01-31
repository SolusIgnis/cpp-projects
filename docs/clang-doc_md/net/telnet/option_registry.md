# class option_registry

*Defined at net/telnet/src/net.telnet-options.cppm#337*

## Members

private set registry_

private shared_mutex mutex_



## Functions

### option_registry

*public void option_registry(std::initializer_list<option> init)*

*Defined at net/telnet/src/net.telnet-options.cppm#340*

### option_registry

*public void option_registry(std::set<option, std::less<>> && init)*

*Defined at net/telnet/src/net.telnet-options.cppm#351*

### get

*public std::optional<option> get(option::id_num opt_id)*

*Defined at net/telnet/src/net.telnet-options.cppm#354*

### has

*public bool has(option::id_num opt_id)*

*Defined at net/telnet/src/net.telnet-options.cppm#366*

### upsert

*public const option & upsert(const option & opt)*

*Defined at net/telnet/src/net.telnet-options.cppm#373*

### upsert

*public void upsert(const option & opt, std::error_code & ec)*

*Defined at net/telnet/src/net.telnet-options.cppm#385*

### upsert

*public const option & upsert(option::id_num opt_id, Args &&... args)*

*Defined at net/telnet/src/net.telnet-options.cppm#400*



