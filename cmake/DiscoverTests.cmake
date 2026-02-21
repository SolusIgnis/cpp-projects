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
#   net.telnet-stream.test-unit.ut.cpp
#   net.telnet-protocol_fsm.test-sequence.catch2.cpp
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

# ============================================================
# Dialect registry (override in root if desired)
# ============================================================

if(NOT DEFINED TEST_DIALECTS)

  set(TEST_DIALECTS
    catch2
    gtest
    ut
  )

  set(TEST_FRAMEWORK.catch2 Catch2::Catch2WithMain)
  set(TEST_FRAMEWORK.gtest  GTest::gtest_main)
  set(TEST_FRAMEWORK.ut     ut::ut)

  set(TEST_DISCOVERY.catch2 Catch2)
  set(TEST_DISCOVERY.gtest  GTest)

endif()

foreach(dialect IN LISTS TEST_DIALECTS)
  if(NOT DEFINED TEST_DISCOVERY.${dialect})
    set(TEST_DISCOVERY.${dialect} CTest)
  endif()
endforeach()

# ============================================================
# Verify framework availability.
# ============================================================
foreach(dialect IN LISTS TEST_DIALECTS)
  set(framework_target ${TEST_FRAMEWORK.${dialect}})
  if(TARGET ${framework_target})
    list(APPEND FILTERED_DIALECTS ${dialect})
  else()
    string(REPLACE "::" ";" target_list "${framework_target}")
    list(GET target_list 0 framework_package)
    find_package(${framework_package} QUIET)
    if(TARGET ${framework_target})
      list(APPEND FILTERED_DIALECTS ${dialect})
    else()
      message(WARNING
        "Framework for dialect '${dialect}' not found. "
        "Target '${framework_target}' is missing. "
        "find_package(${framework_package}) failed to produce it. "
        "'${dialect}' tests are unavailable."
      )
    endif()
  endif()
endforeach()

set(TEST_DIALECTS ${FILTERED_DIALECTS})

# ============================================================
# Include the internal implementation helpers
# ============================================================

include(${CMAKE_CURRENT_LIST_DIR}/DiscoverTests-Internal.cmake)

# ============================================================
# Global aggregation targets (created once)
# ============================================================

_ensure_target(tests)

_create_run_target(
  tests
)

foreach(dialect IN LISTS TEST_DIALECTS)
  _ensure_target(tests.${dialect})
  _create_run_target(
    tests.${dialect}
    LABELS
      ${dialect}
  )
endforeach()

# ============================================================
# Public API
# ============================================================

function(add_tests_for_module module_target)

  if(NOT TARGET ${module_target})
    message(FATAL_ERROR
      "Target does not exist: ${module_target}"
    )
  endif()

  set(test_dir "${CMAKE_CURRENT_SOURCE_DIR}/tests")

  if(NOT EXISTS "${test_dir}")
    return()
  endif()

  file(GLOB test_files CONFIGURE_DEPENDS
    "${test_dir}/*.test*.cpp"
  )

  foreach(test_file IN LISTS test_files)
    _create_test_from_file(${module_target} "${test_file}")
  endforeach()

endfunction()
