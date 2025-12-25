# Repository-Level CML
cmake_minimum_required(VERSION 4.1.2)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")

project(cpp-projects
  LANGUAGES CXX
)

set(CMAKE_CXX_MODULE_STD 1)

# Add the "net" module group.
add_subdirectory(net)
