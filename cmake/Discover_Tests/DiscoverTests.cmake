# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
#
# DiscoverTests.cmake
#
# Fully automatic repository-wide test discovery and target creation based on filename grammar:
#
#   [<group>.]<module>[-<partition>][-impl[-<impl-type>]].test[-<kind>].<dialect>.cpp
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
#   Executables:
#     <module fullname>[-<partition>][-impl[-<impl-type>]].test[-<kind>].<dialect>
#
#   Build targets:
#     <module>.tests
#     <module>.tests.<dialect>
#     <module>.tests-<kind>
#     <module>.tests-<kind>.<dialect>
#
#   Run targets:
#     <module>.tests.run
#     <module>.tests.<dialect>.run
#     <module>.tests-<kind>.run
#     <module>.tests-<kind>.<dialect>.run
#
#   Global targets:
#     tests
#     tests.run
#
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
# 
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
# 
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
