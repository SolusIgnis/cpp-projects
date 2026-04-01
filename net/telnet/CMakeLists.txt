# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2025-2026 Jeremy Murphy and any Contributors

# net/telnet/CMakeLists.txt

cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

# Read Version
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" NET_TELNET_VERSION)
string(STRIP "${NET_TELNET_VERSION}" NET_TELNET_VERSION)

# Handle Embed/Standalone
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # Experimental "import std;" until it's a first-class feature
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
  project(net-telnet
    VERSION   ${NET_TELNET_VERSION}
    LANGUAGES CXX
  )
endif()

# Add the net.telnet.test_support library
add_subdirectory(test_support)

add_named_module(net.telnet STATIC
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
      net.asio_concepts
    TEST_DEPENDENCIES
      net.telnet.test_support
)
