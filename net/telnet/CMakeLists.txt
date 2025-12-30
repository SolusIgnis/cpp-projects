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

add_library(net.telnet STATIC)
add_library(net::telnet ALIAS net.telnet)

# C++23 required for "import std;"
target_compile_features(net.telnet
  PUBLIC cxx_std_23
)

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

# Implementation Units
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
