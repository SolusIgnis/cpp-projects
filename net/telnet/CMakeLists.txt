cmake_minimum_required(VERSION 4.1.2)

# Read Version
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" NET_TELNET_VERSION)
string(STRIP "${NET_TELNET_VERSION}" NET_TELNET_VERSION)

# Handle Embed/Standalone
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
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
set_target_properties(net.telnet PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(net.telnet PROPERTIES CXX_EXTENSIONS OFF)

# Module interface units
target_sources(net.telnet
  PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS src
    FILES
      # Primary Module Interface Unit
      src/net.telnet.cppm
      # Partition Interface Units
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

target_sources(net.telnet
  PRIVATE
      # Implementation Units
      src/impl/net.telnet-protocol_fsm-impl.cpp
      src/impl/net.telnet-stream-impl.cpp
      src/impl/net.telnet-stream-async-impl.cpp
      src/impl/net.telnet-stream-sync-impl.cpp
)
