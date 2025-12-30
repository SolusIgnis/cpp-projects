# Repository-Level CML
cmake_minimum_required(VERSION 4.1.2)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")

project(cpp-projects
  LANGUAGES CXX
)

#For "import std"
set(CMAKE_CXX_MODULE_STD 1)

message(STATUS "Configuring with ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 18)
    message(FATAL_ERROR
      "Clang 18 or newer is required for full C++23 module + import std support.\n"
      "Detected: ${CMAKE_CXX_COMPILER_VERSION}")
  endif()

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14)
    message(FATAL_ERROR
      "GCC 14 or newer is required for full C++23 module + import std support.\n"
      "Detected: ${CMAKE_CXX_COMPILER_VERSION}\n"
      "Note: libstdc++ must be built with std module support.")
  endif()

elseif (MSVC)
  # MSVC version numbers are weird; CMake exposes them cleanly
  if (MSVC_VERSION LESS 1940)
    message(FATAL_ERROR
      "MSVC 19.40 (Visual Studio 2022 17.10) or newer is required for import std.\n"
      "Detected MSVC_VERSION: ${MSVC_VERSION}")
  endif()

else()
  message(WARNING
    "Unrecognized compiler ${CMAKE_CXX_COMPILER_ID}. "
        "C++23 module and import std support may not work. Proceeding anyway.")
endif()

# Add the "net" module group.
add_subdirectory(net)
