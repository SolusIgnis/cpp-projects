# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2025-2026 Jeremy Murphy and any Contributors
# ===========================================================================
# //modules/net/telnet/CMakeLists.txt
# ===========================================================================

cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

trace_me()

add_named_module(net.telnet
    LIB_TYPE STATIC
    PARTITIONS
      src/net.telnet-awaitables.cppm
      src/net.telnet-concepts.cppm
      src/net.telnet-errors.cppm
      src/net.telnet-internal.cppm
      src/net.telnet-options.cppm
      src/net.telnet-protocol_config.cppm
      src/net.telnet-protocol_fsm.cppm
      src/net.telnet-stream.cppm
      src/net.telnet-types.cppm
    IMPLEMENTATIONS 
      src/impl/net.telnet-protocol_fsm-impl.cpp
      src/impl/net.telnet-stream-impl.cpp
      src/impl/net.telnet-stream-async-impl.cpp
      src/impl/net.telnet-stream-sync-impl.cpp
    IMPORTS
      framework.coroutines.tagged_awaitable
      net.asio_concepts
    TEST_DEPENDENCIES
      tools.test.coroutine_harness
)
