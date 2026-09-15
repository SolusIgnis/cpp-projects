# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
#
# DiscoverTests.cmake
#
# Fully automatic repository-wide test discovery and target creation based on filename grammar.
#
# This file provides the public interface for the test discovery system.
# Test frameworks are registered as dialects with `register_test_dialect()`,
# and module tests are discovered with `add_tests_for_module()`.
#
# Test filenames have the following grammar:
#
#   [<group>.]<module>[-<partition>][-impl[-<impl-type>]].test[-<kind>].<dialect>.cpp
#
# The <group> and <module> components together identify the module target.
# The module suffix identifies a partition, implementation, or implementation
# specialization. The discovery system does not otherwise distinguish these
# forms.
#
# <kind> is optional and defaults to "unit".
#
# <dialect> identifies a registered test-framework dialect.
#
# Examples:
#
#   net.telnet-stream.test.catch2.cpp
#   net.telnet-stream.test-unit.boost-ut.cpp
#   net.telnet-protocol_fsm.test-integration.qlibs-ut.cpp
#   net.telnet-protocol_fsm.test-sequence.gtest.cpp
#
# Creates:
#
#   Test executables:
#     <module fullname>[-<partition>][-impl[-<impl-type>]].test[-<kind>].<dialect>
#
#   Module-scoped build targets:
#     <module fullname>.tests
#     <module fullname>.tests.<dialect>
#     <module fullname>.tests-<kind>
#     <module fullname>.tests-<kind>.<dialect>
#
#   Global build targets:
#     tests
#     tests.<dialect>
#     tests-<kind>
#     tests-<kind>.<dialect>
#
#   Run targets:
#     Every build aggregation target above has a corresponding
#     ".run" target that invokes CTest with the appropriate labels.
#
#   Global run targets:
#     tests.run
#
# Test executables are also registered with CTest. Frameworks
# providing specialized discovery use their CTest discovery
# mechanisms.
# ============================================================

include_guard(GLOBAL)

# ============================================================
# Let our includes see DiscoverTests.cmake was included first.
# ============================================================

set(_DISCOVER_TESTS_INCLUDED TRUE)

# ============================================================
# Enable testing once globally
# ============================================================
include(CTest)

get_property(testing_enabled GLOBAL PROPERTY CMAKE_TESTING_ENABLED)
if(NOT testing_enabled)
  enable_testing()
endif()

# ============================================================
# Initialize the dialect registry to empty.
# Call `register_test_dialect` to populate it.
# ============================================================
set_property(GLOBAL PROPERTY DiscoverTests__DIALECTS "")

# ============================================================
# Include the internal implementation helpers
# ============================================================
include("${CMAKE_CURRENT_LIST_DIR}/DiscoverTests-Internal.cmake")

# ============================================================
# Global aggregation targets (created once)
# ============================================================
DiscoverTests__ensure_target(tests)

DiscoverTests__create_run_target(tests)

# ============================================================
# Public API
# ============================================================

# ============================================================
# register_test_dialect(dialect_name)
# ------------------------------------------------------------
# Register a test-framework dialect for use by test filenames.
#
# Required arguments:
#   dialect_name [positional parameter]
#       Name accepted as <dialect> in the test filename grammar.
#
#   CPM_NAME
#       Name passed to CPMFindPackage().
#
#   LINK_TARGET
#       CMake target produced by the framework and linked into
#       test executables.
#
# Optional framework acquisition arguments:
#
#   GH_REPO
#       GitHub repository passed to CPMFindPackage().
#
#   VERSION
#       Package version passed to CPMFindPackage().
#
#   GIT_TAG
#       Git revision (branch/tag/commit hash) passed to CPMFindPackage().
#
#   CPM_OPTIONS
#       Additional OPTIONS passed to CPMFindPackage().
#
#   PATCHES
#       Patch files passed to CPMFindPackage().
#
# Optional test discovery argument:
#
#   DISCOVERY
#       Selects framework-specific CTest discovery.
#       Supported values are currently:
#
#         GTest
#         Catch2
#
#       When omitted, the test executable itself is registered
#       as a CTest test.
#
# A dialect name must be unique. Attempting to register a dialect
# that has already been registered is a fatal error.
#
# Frameworks are acquired lazily: CPM is invoked only when a test
# using the dialect is discovered and its LINK_TARGET does not
# already exist.
#
# A registered dialect is eligible for discovery, but its framework
# does not have to be available when the dialect is registered.
# If framework acquisition fails while discovering a test, that
# dialect is disabled for the remainder of the configuration and
# subsequent tests using it are skipped.
# ============================================================
function(register_test_dialect dialect_name)
  set(_single_value_parameters
    CPM_NAME
    GH_REPO
    VERSION
    GIT_TAG
    LINK_TARGET
    DISCOVERY
  )
  set(_multi_value_parameters
    CPM_OPTIONS
    PATCHES
  )

  cmake_parse_arguments(
    RTD_ARG
    ""
    "${_single_value_parameters}"
    "${_multi_value_parameters}"
    ${ARGN}
  )

  if (NOT dialect_name)
    message(FATAL_ERROR "register_test_dialect: What dialect is being registered?")
  endif()

  if (NOT DEFINED RTD_ARG_LINK_TARGET)
    message(FATAL_ERROR "register_test_dialect: LINK_TARGET is a required argument")
  endif()

  if (NOT DEFINED RTD_ARG_CPM_NAME)
    message(FATAL_ERROR "register_test_dialect: CPM_NAME is a required argument")
  endif()

  if (RTD_ARG_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "register_test_dialect: unrecognized arguments: ${RTD_ARG_UNPARSED_ARGUMENTS}")
  endif()

  if (RTD_ARG_KEYWORDS_MISSING_VALUES)
    message(FATAL_ERROR "register_test_dialect: arguments missing values: ${RTD_ARG_KEYWORDS_MISSING_VALUES}")
  endif()

  # Prevent duplicate registrations.
  DiscoverTests__get_dialects(registered_dialects)
  list(FIND registered_dialects "${dialect_name}" dialect_index)
  if(NOT dialect_index EQUAL -1)
    message(FATAL_ERROR "register_test_dialect: dialect '${dialect_name}' is already registered")
  endif()

  # Register the dialect.
  DiscoverTests__add_dialect("${dialect_name}")
  
  # Register the metadata.
  foreach(parameter IN LISTS _single_value_parameters _multi_value_parameters)
    if (DEFINED "RTD_ARG_${parameter}")
      set(
        "DiscoverTests__DIALECT.${dialect_name}.${parameter}"
        "${RTD_ARG_${parameter}}"
        PARENT_SCOPE
      )
    endif()
  endforeach()
endfunction()

# ============================================================
# add_tests_for_module(module_target)
# ------------------------------------------------------------
# Discover and create tests belonging to a module.
#
# module_target
#     Existing CMake target representing the module under test.
#     This target must already exist.
#
# DEPENDENCIES
#     Optional list of existing CMake targets to link privately
#     into every test executable created for the module.
#
# Tests are discovered from:
#
#     ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.test*.cpp
#
# Only files matching the test filename pattern are considered
# for discovery. Files with invalid filenames or unavailable
# dialects are skipped with a warning.
#
# The function does nothing when BUILD_TESTING is disabled or
# when the module has no tests directory.
#
# DEPENDENCIES are validated as existing CMake targets; this
# function does not acquire them.
#
# Example:
#
#   add_tests_for_module(base.functional.overload)
#
# or:
#
#   add_tests_for_module(
#     net.telnet
#     DEPENDENCIES
#       tools.test.coroutine_harness
#       framework.coroutines.tagged_awaitable
#   )
# ============================================================
function(add_tests_for_module module_target)
  if(NOT BUILD_TESTING)
    return()
  endif()

  if(NOT TARGET "${module_target}")
    message(FATAL_ERROR
      "Target does not exist: ${module_target}"
    )
  endif()
  
  DiscoverTests__validate_test_dependencies(dependencies "${module_target}" ${ARGN})

  set(test_dir "${CMAKE_CURRENT_SOURCE_DIR}/tests")

  if(NOT EXISTS "${test_dir}")
    return()
  endif()

  file(GLOB test_files CONFIGURE_DEPENDS
    "${test_dir}/*.test*.cpp"
  )

  foreach(test_file IN LISTS test_files)
    DiscoverTests__create_test_from_file("${module_target}" "${test_file}" "${dependencies}")
  endforeach()
endfunction()
