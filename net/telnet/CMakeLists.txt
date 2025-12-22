cmake_minimum_required(VERSION 4.0)

# Read Version
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" NET_TELNET_VERSION)
string(STRIP "${NET_TELNET_VERSION}" NET_TELNET_VERSION)

# Handle Embed/Standalone
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(net-telnet
    VERSION   ${NET_TELNET_VERSION}
    LANGUAGES CXX
  )
endif()

add_library(net.telnet)
add_library(net::telnet ALIAS net.telnet)

target_compile_features(net.telnet
  PUBLIC cxx_std_23
)

# Module interface units
target_sources(net.telnet
  PRIVATE
    FILE_SET CXX_MODULES
    BASE_DIRS src
    FILES
      src/net.telnet.cppm
      src/net.telnet-awaitables.cppm
      src/net.telnet-concepts.cppm
      src/net.telnet-errors.cppm
      src/net.telnet-internal.cppm
      src/net.telnet-options.cppm
      src/net.telnet-protocol_config.cppm
      src/net.telnet-protocol_fsm.cppm
      src/net.telnet-stream.cppm
      src/net.telnet-types.cppm
)

# Implementation units (normal TUs)
target_sources(net.telnet
  PRIVATE
    src/impl/net.telnet-protocol_fsm-impl.cpp
    src/impl/net.telnet-stream-impl.cpp
    src/impl/net.telnet-stream-async-impl.cpp
    src/impl/net.telnet-stream-sync-impl.cpp
)

target_link_libraries(net.telnet
  PUBLIC
    net::asio_concepts
)
