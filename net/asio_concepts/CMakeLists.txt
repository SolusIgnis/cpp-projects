cmake_minimum_required(VERSION 4.0)

# Read Version
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" NET_ASIO_CONCEPTS_VERSION)
string(STRIP "${NET_ASIO_CONCEPTS_VERSION}" NET_ASIO_CONCEPTS_VERSION)

# Handle Embed/Standalone
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(CMAKE_EXPERIMENTAL_CXX_IMORT_STD ON)
  project(net-asio-concepts
    VERSION   ${NET_ASIO_CONCEPTS_VERSION}
    LANGUAGES CXX
  )
endif()

add_library(net.asio_concepts STATIC)
add_library(net::asio_concepts ALIAS net.asio_concepts)

target_compile_features(net.asio_concepts
  PUBLIC cxx_std_23
)

target_sources(net.asio_concepts
  PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS src
    FILES
      src/net.asio_concepts.cppm
)
