# Repository-Level CML
cmake_minimum_required(VERSION 4.0)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")

project(cpp-projects
  LANGUAGES CXX
)

set(CMAKE_CXX_MODULE_STD 1)

# Add the "net" module group.
add_subdirectory(net)
