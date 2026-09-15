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
# Ensure we have our tooling registration function.
# ============================================================
include(ToolingInfrastructure)

# ============================================================
# Internal: Parse filename metadata
#
# Outputs:
#
#   <out_prefix>_TEST_NAME
#   <out_prefix>_TEST_BASE_NAME
#   <out_prefix>_TEST_DIALECT
#   <out_prefix>_TEST_KIND
#
# ============================================================
function(DiscoverTests__parse_test_filename out_prefix filename module_name)
  string(REPLACE "." "\\." module_name_esc "${module_name}")

  set(identifier "[a-zA-Z0-9_]+")
  
  set(module_part_id "(-${identifier})")
  set(base_name_id "(${module_name_esc}${module_part_id}*)")

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
    unset("${out_prefix}_TEST_NAME" PARENT_SCOPE)
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

  set("${out_prefix}_TEST_NAME"      "${CMAKE_MATCH_1}" PARENT_SCOPE)
  set("${out_prefix}_TEST_BASE_NAME" "${CMAKE_MATCH_2}" PARENT_SCOPE)
  set("${out_prefix}_TEST_KIND"      "${CMAKE_MATCH_5}" PARENT_SCOPE)
  set("${out_prefix}_TEST_DIALECT"   "${CMAKE_MATCH_7}" PARENT_SCOPE)
endfunction()

# ============================================================
# Internal: Read the dialects registry.
# ============================================================
function(DiscoverTests__get_dialects out_var)
  get_property(
    ${out_var}
    GLOBAL PROPERTY
    DiscoverTests__DIALECTS
  )
endfunction()

# ============================================================
# Internal: Replace the dialects registry.
# ============================================================
function(DiscoverTests__set_dialects registered_dialects)
  set_property(
    GLOBAL PROPERTY
    DiscoverTests__DIALECTS
    "${registered_dialects}"
  )
endfunction()

# ============================================================
# Internal: Add a dialect to the registry.
# ============================================================
function(DiscoverTests__add_dialect dialect_name)
  DiscoverTests__get_dialects(registered_dialects)
  list(APPEND
    registered_dialects
    "${dialect_name}"
  )
  list(REMOVE_DUPLICATES
    registered_dialects
  )
  DiscoverTests__set_dialects("${registered_dialects}")
endfunction()

# ============================================================
# Internal: Remove a dialect from the registry.
# ============================================================
function(DiscoverTests__remove_dialect dialect_name)
  DiscoverTests__get_dialects(registered_dialects)
  list(REMOVE_ITEM
    registered_dialects
    "${dialect_name}"
  )
  DiscoverTests__set_dialects("${registered_dialects}")
endfunction()

# ============================================================
# Internal: Validate dialect
# ============================================================
function(DiscoverTests__validate_test_dialect out_var dialect filename)
  DiscoverTests__get_dialects(registered_dialects)
  list(FIND registered_dialects "${dialect}" dialect_index)
  if(dialect_index EQUAL -1)
    message(WARNING
      "Unknown test dialect '${dialect}' in file: ${filename}\n"
      "Registered dialects: ${registered_dialects}"
    )
    set(${out_var}  False PARENT_SCOPE)
  else()
    set(${out_var}  True PARENT_SCOPE)
  endif()
endfunction()

# ============================================================
# Internal: Verify framework availability
# ============================================================
function(DiscoverTests__verify_framework_availability out_var dialect)
  set(framework_target "${DiscoverTests__DIALECT.${dialect}.LINK_TARGET}")
  if(NOT TARGET "${framework_target}")
    set(cpm_args "NAME" "${DiscoverTests__DIALECT.${dialect}.CPM_NAME}")
        
    if(DEFINED "DiscoverTests__DIALECT.${dialect}.VERSION")
      list(APPEND cpm_args "VERSION" "${DiscoverTests__DIALECT.${dialect}.VERSION}")
    endif()
        
    if(DEFINED "DiscoverTests__DIALECT.${dialect}.GH_REPO")
      list(APPEND cpm_args "GITHUB_REPOSITORY" "${DiscoverTests__DIALECT.${dialect}.GH_REPO}")
    endif()
        
    if(DEFINED "DiscoverTests__DIALECT.${dialect}.GIT_TAG")
      list(APPEND cpm_args "GIT_TAG" "${DiscoverTests__DIALECT.${dialect}.GIT_TAG}")
    endif()
        
    if(DEFINED "DiscoverTests__DIALECT.${dialect}.CPM_OPTIONS")
      list(APPEND cpm_args OPTIONS ${DiscoverTests__DIALECT.${dialect}.CPM_OPTIONS})
    endif()

    if(DEFINED "DiscoverTests__DIALECT.${dialect}.PATCHES")
      list(APPEND cpm_args PATCHES ${DiscoverTests__DIALECT.${dialect}.PATCHES})
    endif()
    
    CPMFindPackage(
      ${cpm_args}
      SYSTEM YES
      EXCLUDE_FROM_ALL YES
    )

    if(NOT TARGET "${framework_target}")
      message(WARNING
        "Framework for dialect '${dialect}' not found. "
        "Target '${framework_target}' is missing. "
        "CPMFindPackage failed to produce it. "
        "'${dialect}' tests are unavailable."
      )

      # A dialect whose framework cannot be acquired is removed from the active
      # DIALECTS set so that subsequent test files do not repeatedly invoke CPM.
      DiscoverTests__remove_dialect("${dialect}")
      set(${out_var} FALSE PARENT_SCOPE)
      return()
    endif()
  endif()
  set(${out_var} TRUE PARENT_SCOPE)
endfunction()

# ============================================================
# Internal: Ensure target exists
# ============================================================
function(DiscoverTests__ensure_target target)
  if(NOT TARGET "${target}")
    add_custom_target("${target}")
  endif()
endfunction()

# ============================================================
# Internal: Bind a target as a dependency of an aggregate
# ============================================================
function(DiscoverTests__bind_aggregate_dependency aggregate target)
  DiscoverTests__ensure_target("${aggregate}")
  add_dependencies("${aggregate}" "${target}")
endfunction()

# ============================================================
# Internal: Validate test dependencies as linkable targets
# ============================================================
function(DiscoverTests__validate_test_dependencies out_var module_target)
  cmake_parse_arguments(
    VTD_ARG
    ""
    ""
    "DEPENDENCIES"
    ${ARGN}
  )
  
  if (VTD_ARG_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "DiscoverTests__validate_test_dependencies: unrecognized arguments: ${VTD_ARG_UNPARSED_ARGUMENTS}")
  endif()

  if (VTD_ARG_KEYWORDS_MISSING_VALUES)
    message(FATAL_ERROR "DiscoverTests__validate_test_dependencies: arguments missing values: ${VTD_ARG_KEYWORDS_MISSING_VALUES}")
  endif()
  
  set(dependencies_block "")
  foreach(dependency IN LISTS VTD_ARG_DEPENDENCIES)
    if(NOT TARGET "${dependency}")
      message(FATAL_ERROR "Test dependency '${dependency}' of module '${module_target}' does not exist as a target. Unable to link test targets against it.")
    endif()

    list(APPEND dependencies_block "${dependency}")
  endforeach()

  set(${out_var} "${dependencies_block}" PARENT_SCOPE)
endfunction()

# ============================================================
# Internal: Create run target with labels if it doesn't exist
# ============================================================
function(DiscoverTests__create_run_target build_target)
  set(target "${build_target}.run")

  cmake_parse_arguments(
    CRT_ARG
    ""
    ""
    "LABELS"
    ${ARGN}
  )
  
  if(NOT TARGET "${target}")
    set(ctest_args "--output-on-failure")
    list(APPEND ctest_args "-V")
    foreach(label IN LISTS CRT_ARG_LABELS)
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
function(DiscoverTests__create_test_from_file module_target test_file dependencies)
  get_target_property(module_name "${module_target}" NAME)

  get_filename_component(filename "${test_file}" NAME)

  DiscoverTests__parse_test_filename(PARSED "${filename}" "${module_name}")
  if(NOT PARSED_TEST_NAME)
    return()
  endif()
  
  DiscoverTests__validate_test_dialect(test_dialect_valid "${PARSED_TEST_DIALECT}" "${filename}")
  if(NOT test_dialect_valid)
    return()
  endif()

  DiscoverTests__verify_framework_availability(framework_available "${PARSED_TEST_DIALECT}")
  if(NOT framework_available)
    return()
  endif()

  if(NOT PARSED_TEST_KIND)
    set(PARSED_TEST_KIND unit)
  endif()

  set(target "${PARSED_TEST_NAME}")

  if(DEFINED DEBUG_TEST_REGEX)
    message(STATUS "Module Name: ${module_name}")
    message(STATUS "Module Target: ${module_target}")
    message(STATUS "Test File: ${filename}")
    message(STATUS "Test Name: ${PARSED_TEST_NAME}")
    message(STATUS "Test Base Name: ${PARSED_TEST_BASE_NAME}")
    message(STATUS "Test Kind: ${PARSED_TEST_KIND}")
    message(STATUS "Test Dialect: ${PARSED_TEST_DIALECT}")
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
      "${DiscoverTests__DIALECT.${PARSED_TEST_DIALECT}.LINK_TARGET}"
      ${dependencies}
  )
  
  target_link_options("${target}"
    PRIVATE
      #This prevents a linker gc bug that gets triggered by ut.
      "$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wl,--no-gc-sections>"
      # This could be better in the future, but it's not yet reliable.
      #"$<$<OR:$<LINKER_ID:GNU>,$<LINKER_ID:LLD>>:-Wl,--no-gc-sections>"
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
    "${PARSED_TEST_DIALECT}"
    "test"
    "${PARSED_TEST_KIND}"
  )

  # ----------------------------------------------------------
  # Register with CTest
  # ----------------------------------------------------------

  set(discovery_method "${DiscoverTests__DIALECT.${PARSED_TEST_DIALECT}.DISCOVERY}")
  if(discovery_method STREQUAL "GTest")
    include(GoogleTest)

    gtest_discover_tests("${target}"
      PROPERTIES LABELS ${labels}
    )
  elseif(discovery_method STREQUAL "Catch2")
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

  set(aggregates
    "tests"
    "tests.${PARSED_TEST_DIALECT}"
    "tests-${PARSED_TEST_KIND}"
    "tests-${PARSED_TEST_KIND}.${PARSED_TEST_DIALECT}"

    "${module_name}.tests"
    "${module_name}.tests.${PARSED_TEST_DIALECT}"
    "${module_name}.tests-${PARSED_TEST_KIND}"
    "${module_name}.tests-${PARSED_TEST_KIND}.${PARSED_TEST_DIALECT}"
  )
  
  foreach(aggregate IN LISTS aggregates)
    DiscoverTests__bind_aggregate_dependency("${aggregate}" "${target}")
  endforeach()

  # ----------------------------------------------------------
  # Run aggregation targets
  # ----------------------------------------------------------

  DiscoverTests__create_run_target(
    "tests.${PARSED_TEST_DIALECT}"
    LABELS
      "${PARSED_TEST_DIALECT}"
  )

  DiscoverTests__create_run_target(
    "tests-${PARSED_TEST_KIND}"
    LABELS
      "${PARSED_TEST_KIND}"
  )
    
  DiscoverTests__create_run_target(
    "tests-${PARSED_TEST_KIND}.${PARSED_TEST_DIALECT}"
    LABELS
      "${PARSED_TEST_KIND}"
      "${PARSED_TEST_DIALECT}"
  )

  DiscoverTests__create_run_target(
    "${module_name}.tests"
    LABELS
      "${module_name}"
  )

  DiscoverTests__create_run_target(
    "${module_name}.tests.${PARSED_TEST_DIALECT}"
    LABELS
      "${module_name}"
      "${PARSED_TEST_DIALECT}"
  )

  DiscoverTests__create_run_target(
    "${module_name}.tests-${PARSED_TEST_KIND}"
    LABELS
      "${module_name}"
      "${PARSED_TEST_KIND}"
  )
    
  DiscoverTests__create_run_target(
    "${module_name}.tests-${PARSED_TEST_KIND}.${PARSED_TEST_DIALECT}"
    LABELS
      "${module_name}"
      "${PARSED_TEST_KIND}"
      "${PARSED_TEST_DIALECT}"
  )
endfunction()
