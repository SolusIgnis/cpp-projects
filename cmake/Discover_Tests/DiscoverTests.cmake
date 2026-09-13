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

get_property(testing_enabled GLOBAL PROPERTY CMAKE_TESTING_ENABLED)
if(NOT testing_enabled)
  enable_testing()
endif()

# ============================================================
# Dialect registry (override in root if desired)
# ============================================================

if(NOT DEFINED TEST_DIALECTS)

  set(TEST_DIALECTS
    catch2
    gtest
    ut
    boost-ut
  )

  set(TEST_FRAMEWORK.catch2.CPM_NAME    "Catch2")
  set(TEST_FRAMEWORK.catch2.GH_REPO     "catchorg/Catch2")
  set(TEST_FRAMEWORK.catch2.VERSION     "3.16.0")
  set(TEST_FRAMEWORK.catch2.GIT_TAG     "317ac1ed4c0bb6e6b91eafc817e05c488feffcb3")
  set(TEST_FRAMEWORK.catch2.LINK_TARGET "Catch2::Catch2WithMain")

  set(TEST_FRAMEWORK.gtest.CPM_NAME    "gtest")
  set(TEST_FRAMEWORK.gtest.GH_REPO     "google/googletest")
  set(TEST_FRAMEWORK.gtest.VERSION     "1.18.0")
  set(TEST_FRAMEWORK.gtest.GIT_TAG     "063de7e9578f82b369302001269680b4b1553359")
  set(TEST_FRAMEWORK.gtest.CPM_OPTIONS "INSTALL_GTEST OFF" "gtest_force_shared_crt ON")
  set(TEST_FRAMEWORK.gtest.LINK_TARGET "GTest::gtest_main")

  set(TEST_FRAMEWORK.ut.LINK_TARGET       qlibs.ut::ut)

  set(TEST_FRAMEWORK.boost-ut.CPM_NAME    "ut")
  set(TEST_FRAMEWORK.boost-ut.GH_REPO     "boost-ext/ut")
  set(TEST_FRAMEWORK.boost-ut.VERSION     "2.3.1")
  set(TEST_FRAMEWORK.boost-ut.GIT_TAG     "59a9beba0763dbb45b3cc68e4cf484c659319a97")
  set(TEST_FRAMEWORK.boost-ut.CPM_OPTIONS "BOOST_UT_DISABLE_MODULE NO")
  set(TEST_FRAMEWORK.boost-ut.LINK_TARGET "Boost::ut_module")

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
  set(framework_target ${TEST_FRAMEWORK.${dialect}.LINK_TARGET})
  if(NOT TARGET ${framework_target})
    string(REPLACE "::" ";" target_list "${framework_target}")
    list(GET target_list 0 framework_package)
    if(NOT TARGET ${framework_target})
      find_package(${framework_package} QUIET)
    endif()
    if(NOT TARGET ${framework_target})
      message(WARNING
        "Framework for dialect '${dialect}' not found. "
        "Target '${framework_target}' is missing. "
        "find_package(${framework_package}) failed to produce it. "
        "'${dialect}' tests are unavailable."
      )
      list(REMOVE_ITEM TEST_DIALECTS ${dialect})
    endif()
  endif()
endforeach()

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
  if(NOT BUILD_TESTING)
    return()
  endif()

  if(NOT TARGET ${module_target})
    message(FATAL_ERROR
      "Target does not exist: ${module_target}"
    )
  endif()
  
  _validate_test_dependencies("${module_target}" ${ARGN})

  set(test_dir "${CMAKE_CURRENT_SOURCE_DIR}/tests")

  if(NOT EXISTS "${test_dir}")
    return()
  endif()

  file(GLOB test_files CONFIGURE_DEPENDS
    "${test_dir}/*.test*.cpp"
  )

  foreach(test_file IN LISTS test_files)
    _create_test_from_file("${module_target}" "${test_file}" "${VALIDATED_DEPENDENCIES}")
  endforeach()
endfunction()
