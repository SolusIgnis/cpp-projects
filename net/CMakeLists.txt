# net/CMakeLists.txt

# Embed/Standalone Shim
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  cmake_minimum_required(VERSION 4.0)
  project(net-bundle VERSION 0.0.0 LANGUAGES CXX)
endif()

# Add the net.asio_concepts library
add_subdirectory(asio_concepts)

# Add the net.telnet library
add_subdirectory(telnet)
