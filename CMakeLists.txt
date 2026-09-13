# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2025-2026 Jeremy Murphy and any Contributors

# ===========================================================================
# //CMakeLists.txt
# ===========================================================================

cmake_minimum_required(VERSION 4.4.0)
# Will be bumped to 4.5 most likely when "import std;" is no longer experimental

# Experimental "import std;" until it's a first-class feature
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")

project(cpp-projects
  LANGUAGES CXX
)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#For "import std"
set(CMAKE_CXX_MODULE_STD 1)

# ===========================================================================
# Add project CMake modules (./cmake/*) to CMAKE_MODULE_PATH
# ===========================================================================

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CXX_Modules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Discover_Tests")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Tooling_Targets")

# ===========================================================================
# Check for adequate compilers supporting C++23 and "import std;".
# ===========================================================================
include(CompilerValidation)

# ===========================================================================
# Enable any requested sanitizers.
# ===========================================================================
include(SanitizerSetup)

# ===========================================================================
# Test Framework Dependencies
# ===========================================================================
include(FetchCPM)

CPMFindPackage(
  URI "gh:boost-ext/ut#f923e6f@2.3.1"
  OPTIONS "BOOST_UT_DISABLE_MODULE NO"
)

# qlibs/ut
find_package(ut QUIET)

if(NOT TARGET qlibs::ut)
  include(FetchUT)
  fetch_ut()
endif()

# ===========================================================================
# Test Discovery / Target Generation
# ===========================================================================
include(DiscoverTests)

# ===========================================================================
# Tooling Infrastructure (Options, Cache Variables, Registries, and Helper Functions)
# ===========================================================================
include(ToolingInfrastructure)

# ===========================================================================
# Add module utilities.
# ===========================================================================
include(AddNamedModule)
include(AddMetamodule)

# ===========================================================================
# Add module directory tree.
# ===========================================================================
function(trace_me)
  file(RELATIVE_PATH _scoped_path "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")

  message(STATUS "Processing: //${_scoped_path}")
endfunction()


add_subdirectory("modules")

# ===========================================================================
# Aggregate the tooling-registered targets and their sources.
# ===========================================================================
include(ToolingSources)

# ===========================================================================
# Create tooling targets for clang-format (real and check) and clang-tidy.
# ===========================================================================
include(ToolingTargets)
