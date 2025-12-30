cmake_minimum_required(VERSION 4.1.2)
# Will be bumped to 4.3 most likely when "import std;" is no longer experimental

# Read Version
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" NET_ASIO_CONCEPTS_VERSION)
string(STRIP "${NET_ASIO_CONCEPTS_VERSION}" NET_ASIO_CONCEPTS_VERSION)

# Handle Embed/Standalone
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # Experimental "import std;" until it's a first-class feature
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
  project(net-asio-concepts
    VERSION   ${NET_ASIO_CONCEPTS_VERSION}
    LANGUAGES CXX
  )
endif()

add_library(net.asio_concepts STATIC)
add_library(net::asio_concepts ALIAS net.asio_concepts)

# C++23 required for "import std;"
target_compile_features(net.asio_concepts
  PUBLIC cxx_std_23
)

# Module interface units
target_sources(net.asio_concepts
  PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS src
    FILES
      # Primary Module Interface Unit
      src/net.asio_concepts.cppm
)
