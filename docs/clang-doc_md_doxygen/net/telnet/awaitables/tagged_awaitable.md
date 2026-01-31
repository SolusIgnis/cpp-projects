# class tagged_awaitable

*Defined at net/telnet/src/net.telnet-awaitables.cppm#37*



**brief** Wrapper for an awaitable with a semantic tag for type safety.

**Tag** The semantic tag type.

**T** The awaitable's value type (i.e. the "return type" of `co_await`ing it) (e.g., `void`).

**Awaitable** The underlying awaitable type (default: `boost::asio::awaitable<T>`).

**remark** Provides implicit conversion to/from the underlying awaitable and supports direct `co_await`.

**see** `tags` namespace for semantic tag types, `:protocol_fsm`, `:internal`



## Members

private awaitable_type awaitable_



## Functions

### tagged_awaitable<Tag, T, Awaitable>

*public void tagged_awaitable<Tag, T, Awaitable>()*

*Defined at net/telnet/src/net.telnet-awaitables.cppm#47*

**brief** Default constructor.

### tagged_awaitable<Tag, T, Awaitable>

*public void tagged_awaitable<Tag, T, Awaitable>(awaitable_type awaitable)*

*Defined at net/telnet/src/net.telnet-awaitables.cppm#50*

**brief** Constructs from an awaitable.

### operator type-parameter-0-2 &

*public awaitable_type & operator type-parameter-0-2 &()*

*Defined at net/telnet/src/net.telnet-awaitables.cppm#53*

**brief** Implicit conversion to underlying awaitable (lvalue).

### operator const type-parameter-0-2 &

*public const awaitable_type & operator const type-parameter-0-2 &()*

*Defined at net/telnet/src/net.telnet-awaitables.cppm#56*

**brief** Implicit conversion to underlying awaitable (const lvalue).

### operator type-parameter-0-2 &&

*public awaitable_type && operator type-parameter-0-2 &&()*

*Defined at net/telnet/src/net.telnet-awaitables.cppm#59*

**brief** Implicit conversion to underlying awaitable (rvalue).

### operator co_await

*public auto operator co_await()*

*Defined at net/telnet/src/net.telnet-awaitables.cppm#64*

**brief** Supports co_await for lvalue.

### operator co_await

*public auto operator co_await()*

*Defined at net/telnet/src/net.telnet-awaitables.cppm#67*

**brief** Supports co_await for const lvalue.

### operator co_await

*public auto operator co_await()*

*Defined at net/telnet/src/net.telnet-awaitables.cppm#70*

**brief** Supports co_await for rvalue.



