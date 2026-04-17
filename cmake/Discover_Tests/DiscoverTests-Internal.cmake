# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
#
# DiscoverTests-Internal.cmake
# ============================================================

if(NOT _DISCOVER_TESTS_INCLUDED)
  message(FATAL_ERROR "DiscoverTests-Internal.cmake is internal and must not be included directly.")
endif()

include_guard(GLOBAL)

# ============================================================
# Ensure TEST_DIALECTS was defined before including this file.
# ============================================================

if(NOT DEFINED TEST_DIALECTS)
  message(FATAL_ERROR "TEST_DIALECTS must be configured before including DiscoverTests-Internal.cmake")
endif()

# ============================================================
# Make sure we have our tooling registration function.
# ============================================================

include(ToolingInfrastructure)

# ============================================================
# Internal: Parse filename metadata
#
# Outputs:
#
#   TEST_BASE_NAME
#   TEST_DIALECT
#   TEST_KIND
#
# ============================================================

function(_parse_test_filename filename module_name)

  set(identifier "[a-zA-Z0-9_]+")
  
  set(module_part_id "(-${identifier})")
  set(base_name_id "(${module_name}${module_part_id}*)")

  set(kind_id "(${identifier}(-${identifier})*)")
  set(dialect_id "(${identifier}(-${identifier})*)")

  set(test_name_id "(${base_name_id}\.test(-${kind_id})?\.${dialect_id})")
  set(regex_id "^${test_name_id}\.cpp$")

  string(REGEX MATCH
    "${regex_id}"
    match
    "${filename}"
  )

  if(NOT match)
    message(WARNING
      "Skipping invalid test filename (does not match grammar): ${filename}"
    )
    unset(TEST_NAME PARENT_SCOPE)
    return()
  endif()

  # Groups:
  #
  # 1 = test name
  # 2 = base name
  # 3 = (ignored but required by POSIX-ERE)
  # 4 = (ignored but required by POSIX-ERE)
  # 5 = kind (optional)
  # 6 = (ignored but required by POSIX-ERE)
  # 7 = dialect
  # 8 = (ignored but required by POSIX-ERE)

  set(TEST_NAME      "${CMAKE_MATCH_1}" PARENT_SCOPE)
  set(TEST_BASE_NAME "${CMAKE_MATCH_2}" PARENT_SCOPE)
  set(TEST_KIND      "${CMAKE_MATCH_5}" PARENT_SCOPE)
  set(TEST_DIALECT   "${CMAKE_MATCH_7}" PARENT_SCOPE)

endfunction()

# ============================================================
# Internal: Validate dialect
# ============================================================

function(_validate_test_dialect dialect filename)
  list(FIND TEST_DIALECTS ${dialect} dialect_index)
  if(dialect_index EQUAL -1)
    message(WARNING
      "Unknown test dialect '${dialect}' in file: ${filename}\n"
      "Registered dialects: ${TEST_DIALECTS}"
    )
    set(TEST_DIALECT_VALID False PARENT_SCOPE)
  else()
    set(TEST_DIALECT_VALID True PARENT_SCOPE)
  endif()
endfunction()

# ============================================================
# Internal: Ensure target exists
# ============================================================

function(_ensure_target target)
  if(NOT TARGET "${target}")
    add_custom_target("${target}")
  endif()
endfunction()

# ============================================================
# Internal: Bind a target as a dependency of an aggregate
# ============================================================

function(_bind_aggregate_dependency aggregate target)
  _ensure_target("${aggregate}")
  add_dependencies("${aggregate}" "${target}")
endfunction()

# ============================================================
# Internal: Validate test dependencies as linkable targets
# ============================================================

function(_validate_test_dependencies module_target)
  set(multiValueArgs DEPENDENCIES)

  cmake_parse_arguments(
    ARG
    ""
    ""
    "${multiValueArgs}"
    ${ARGN}
  )
  
  set(dependencies_block "")
  foreach(dependency IN LISTS ARG_DEPENDENCIES)
    if(NOT TARGET "${dependency}")
      message(WARNING "Test dependency '${dependency}' of module '${module_target}' does not exist as a target. Unable to link test targets against it.")
    else()
      list(APPEND dependencies_block "${dependency}")
    endif()
  endforeach()

  set(VALIDATED_DEPENDENCIES ${dependencies_block} PARENT_SCOPE)
endfunction()

# ============================================================
# Internal: Create run target with labels if it doesn't exist
# ============================================================

function(_create_run_target build_target)
  set(target "${build_target}.run")
  
  set(multiValueArgs LABELS)

  cmake_parse_arguments(
    ARG
    ""
    ""
    "${multiValueArgs}"
    ${ARGN}
  )
  
  if(NOT TARGET "${target}")
    set(ctest_args "--output-on-failure")
    list(APPEND ctest_args "-V")
    foreach(label IN LISTS ARG_LABELS)
      list(APPEND ctest_args "-L" "${label}")
    endforeach()

    add_custom_target("${target}"
      COMMAND ${CMAKE_CTEST_COMMAND} ${ctest_args}
      USES_TERMINAL
    )
    add_dependencies("${target}" "${build_target}")
  endif()
endfunction()

# ============================================================
# Internal: Create executable from test file
# ============================================================

function(_create_test_from_file module_target test_file dependencies)
  get_target_property(module_name "${module_target}" NAME)

  get_filename_component(filename "${test_file}" NAME)

  _parse_test_filename("${filename}" "${module_name}")
  if(NOT TEST_NAME)
    return()
  endif()
  
  _validate_test_dialect("${TEST_DIALECT}" "${filename}")
  if(NOT TEST_DIALECT_VALID)
    return()
  endif()

  if(NOT TEST_KIND)
    set(TEST_KIND unit)
  endif()

  set(target "${TEST_NAME}")

  if(DEFINED DEBUG_TEST_REGEX)
    message(STATUS "Module Name: ${module_name}")
    message(STATUS "Module Target: ${module_target}")
    message(STATUS "Test File: ${filename}")
    message(STATUS "Test Name: ${TEST_NAME}")
    message(STATUS "Test Base Name: ${TEST_BASE_NAME}")
    message(STATUS "Test Kind: ${TEST_KIND}")
    message(STATUS "Test Dialect: ${TEST_DIALECT}")
  endif()
  
  # ----------------------------------------------------------
  # Executable
  # ----------------------------------------------------------

  add_executable("${target}")

  target_sources("${target}"
    PRIVATE
      "${test_file}"
  )

  target_link_libraries("${target}"
    PRIVATE
      "${module_target}"
      "${TEST_FRAMEWORK.${TEST_DIALECT}}"
      ${dependencies}
  )
  
  target_link_options("${target}"
    PRIVATE
      "$<$<OR:$<LINKER_ID:GNU>,$<LINKER_ID:LLD>>:-Wl,--no-gc-sections>" #This prevents a linker gc bug that gets triggered by ut.
  )

  target_compile_features("${target}"
    PRIVATE
      $<TARGET_PROPERTY:${module_target},COMPILE_FEATURES>
  )

  set_target_properties("${target}"
    PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY
        "${CMAKE_BINARY_DIR}/tests"
  )
  
  register_tooling_test("${target}")

  # ----------------------------------------------------------
  # Labels
  # ----------------------------------------------------------

  set(labels
    "${module_name}"
    "${TEST_DIALECT}"
    "test"
    "${TEST_KIND}"
  )

  # ----------------------------------------------------------
  # Register with CTest
  # ----------------------------------------------------------

  if(TEST_DISCOVERY.${TEST_DIALECT} STREQUAL "GTest")
    include(GoogleTest)

    gtest_discover_tests("${target}"
      PROPERTIES LABELS ${labels}
    )
  elseif(TEST_DISCOVERY.${TEST_DIALECT} STREQUAL "Catch2")
    include(Catch)

    set(labels_block)
    foreach(label IN LISTS labels)
      list(APPEND labels_block LABELS ${label})
    endforeach()

    catch_discover_tests("${target}"
      PROPERTIES
        ${labels_block}
    )
  else()
    add_test(NAME "${target}" COMMAND ${target})

    set_tests_properties("${target}"
      PROPERTIES LABELS "${labels}"
    )
  endif()

  # ----------------------------------------------------------
  # Build aggregation targets
  # ----------------------------------------------------------

  _bind_aggregate_dependency(
    tests
    "${target}"
  )
  _bind_aggregate_dependency(
    tests.${TEST_DIALECT}
    "${target}"
  )
  _bind_aggregate_dependency(
    tests-${TEST_KIND}
    "${target}"
  )
  _bind_aggregate_dependency(
    tests-${TEST_KIND}.${TEST_DIALECT}
    "${target}"
  )
    
  _bind_aggregate_dependency(
    ${module_name}.tests
    "${target}"
  )
  _bind_aggregate_dependency(
    ${module_name}.tests.${TEST_DIALECT}
    "${target}"
  )
  _bind_aggregate_dependency(
    ${module_name}.tests-${TEST_KIND}
    "${target}"
  )
  _bind_aggregate_dependency(
    ${module_name}.tests-${TEST_KIND}.${TEST_DIALECT}
    "${target}"
  )

  # ----------------------------------------------------------
  # Run aggregation targets
  # ----------------------------------------------------------

  _create_run_target(
    tests-${TEST_KIND}
    LABELS
      "${TEST_KIND}"
  )
    
  _create_run_target(
    tests-${TEST_KIND}.${TEST_DIALECT}
    LABELS
      "${TEST_KIND}"
      "${TEST_DIALECT}"
  )

  _create_run_target(
    ${module_name}.tests
    LABELS
      "${module_name}"
  )

  _create_run_target(
    ${module_name}.tests.${TEST_DIALECT}
    LABELS
      "${module_name}"
      "${TEST_DIALECT}"
  )

  _create_run_target(
    ${module_name}.tests-${TEST_KIND}
    LABELS
      "${module_name}"
      "${TEST_KIND}"
  )
    
  _create_run_target(
    ${module_name}.tests-${TEST_KIND}.${TEST_DIALECT}
    LABELS
      "${module_name}"
      "${TEST_KIND}"
      "${TEST_DIALECT}"
  )

endfunction()
