# net/CMakeLists.txt

cmake_minimum_required(VERSION 4.0)

# Handle Embed/Standalone
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(net-bundle
    LANGUAGES CXX
  )
endif()

# Add the net.asio_concepts library
add_subdirectory(asio_concepts)

# Add the net.telnet library
add_subdirectory(telnet)
