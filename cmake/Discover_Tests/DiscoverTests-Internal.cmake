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
# DiscoverTests__parse_test_filename(out_prefix filename module_name)
# ------------------------------------------------------------
# Internal: Parse test filename metadata
#
# Test filenames have the following grammar:
#
#   [<group>.]<module>[-<partition>][-impl[-<impl-type>]].test[-<kind>].<dialect>.cpp
#
# where:
#
#   The <group> and <module> components are supplied together as
#   module_name by the caller. Consequently, module_name is treated
#   as a literal prefix by this parser.
#
#   The module suffix may identify a partition, an implementation,
#   or an implementation specialization. The parser intentionally
#   does not distinguish these cases; it only validates the filename
#   structure and extracts the resulting base name.
#
#   <kind>
#       An identifier optionally followed by additional
#       "-<identifier>" components.
#
#   <dialect>
#       An identifier optionally followed by additional
#       "-<identifier>" components.
#
# Examples:
#
#   net.telnet-stream.test.catch2.cpp
#   net.telnet-stream.test-unit.boost-ut.cpp
#   net.telnet-protocol_fsm.test-integration.qlibs-ut.cpp
#   net.telnet-protocol_fsm.test-sequence.gtest.cpp
#
# The parser deliberately validates only the filename grammar.
# Whether the extracted dialect is registered is checked separately.
#
# Outputs:
#
#   <out_prefix>_TEST_NAME
#       Complete test target name without the ".cpp" suffix.
#
#   <out_prefix>_TEST_BASE_NAME
#       Module name plus any partition and implementation suffixes.
#
#   <out_prefix>_TEST_KIND
#       Explicit test kind, or empty when the filename omits it.
#
#   <out_prefix>_TEST_DIALECT
#       Test framework dialect encoded in the filename.
#
# On failure, <out_prefix>_TEST_NAME is unset and a warning is emitted.
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
# DiscoverTests__get_dialects(out_var)
# ------------------------------------------------------------
# Internal: Read the dialects registry.
# ============================================================
function(DiscoverTests__get_dialects out_var)
  get_property(
    registered_dialects
    GLOBAL PROPERTY
    DiscoverTests__DIALECTS
  )
  set(${out_var} "${registered_dialects}" PARENT_SCOPE)
endfunction()

# ============================================================
# DiscoverTests__set_dialects(registered_dialects)
# ------------------------------------------------------------
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
# DiscoverTests__add_dialect(dialect_name)
# ------------------------------------------------------------
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
# DiscoverTests__remove_dialect(dialect_name)
# ------------------------------------------------------------
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
# DiscoverTests__validate_test_dialect(out_var dialect filename)
# ------------------------------------------------------------
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
# DiscoverTests__verify_framework_availability(out_var dialect)
# ------------------------------------------------------------
# Internal: Verify the framework target for a dialect exists.
#
# If the configured LINK_TARGET already exists, the framework is
# considered available and no package-manager operation occurs.
#
# Otherwise, the dialect's CPM metadata is passed to
# CPMFindPackage(). The framework is considered successfully
# acquired only if that operation produces the configured
# LINK_TARGET.
#
# If acquisition fails:
#
#   - a warning is emitted;
#   - the dialect is removed from the active dialect registry;
#   - <out_var> is set to FALSE.
#
# Removing the dialect prevents every subsequent test file using
# that dialect from retrying the same failed acquisition.
#
# On success, <out_var> is set to TRUE.
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
# DiscoverTests__ensure_target(target)
# ------------------------------------------------------------
# Internal: Ensure target exists
# ============================================================
function(DiscoverTests__ensure_target target)
  if(NOT TARGET "${target}")
    add_custom_target("${target}")
  endif()
endfunction()

# ============================================================
# DiscoverTests__bind_aggregate_dependency(aggregate target)
# ------------------------------------------------------------
# Internal: Bind a target as a dependency of an aggregate
# ============================================================
function(DiscoverTests__bind_aggregate_dependency aggregate target)
  DiscoverTests__ensure_target("${aggregate}")
  add_dependencies("${aggregate}" "${target}")
endfunction()

# ============================================================
# DiscoverTests__validate_test_dependencies(out_var module_target)
# ------------------------------------------------------------
# Internal: Validate test dependencies as linkable targets.
#
# DEPENDENCIES is an optional list of CMake targets that will be
# linked privately into every test executable created for the
# module.
#
# Every dependency must already exist as a CMake target. This
# function intentionally does not create or acquire dependencies;
# dependency acquisition belongs to the caller/project configuration.
#
# The validated dependency list is returned through <out_var>.
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
  
  foreach(dependency IN LISTS VTD_ARG_DEPENDENCIES)
    if(NOT TARGET "${dependency}")
      message(FATAL_ERROR "Test dependency '${dependency}' of module '${module_target}' does not exist as a target. Unable to link test targets against it.")
    endif()
  endforeach()

  set(${out_var} "${VTD_ARG_DEPENDENCIES}" PARENT_SCOPE)
endfunction()

# ============================================================
# DiscoverTests__create_run_target(build_target)
# ------------------------------------------------------------
# Internal: Create a run target associated with a build target.
#
# The generated target is named:
#
#   <build_target>.run
#
# It invokes CTest with:
#
#   --output-on-failure
#   -V
#
# and, when LABELS are supplied, restricts execution to tests
# matching each supplied CTest label.
#
# The run target depends on <build_target>, ensuring that the
# corresponding tests are built before CTest is invoked.
#
# Run targets are created only once. Subsequent calls for the
# same build target are therefore harmless.
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

  if (CRT_ARG_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "DiscoverTests__create_run_target: unrecognized arguments: ${CRT_ARG_UNPARSED_ARGUMENTS}")
  endif()

  if (CRT_ARG_KEYWORDS_MISSING_VALUES)
    message(FATAL_ERROR "DiscoverTests__create_run_target: arguments missing values: ${CRT_ARG_KEYWORDS_MISSING_VALUES}")
  endif()

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
# DiscoverTests__create_test_from_file(module_target test_file dependencies)
# ------------------------------------------------------------
# Internal: Create and register one test executable.
#
# The test file is processed through the following stages:
#
#   1. Parse the filename to extract test metadata.
#   2. Validate that its dialect is registered and active.
#   3. Lazily acquire the dialect's test framework if necessary.
#   4. Apply the default test kind "unit" when none is specified.
#   5. Create and link the test executable.
#   6. Register the executable with the project's tooling.
#   7. Assign CTest labels for module, dialect, test, and kind.
#   8. Register individual tests using the dialect's discovery
#      mechanism, or add the executable directly when no specialized
#      discovery mechanism is configured.
#   9. Bind the executable to the appropriate build aggregates.
#  10. Create the corresponding filtered run targets.
#
# The executable target name is the parsed test name, which also
# makes it the natural CTest name for frameworks using direct
# executable registration.
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
  #
  # Each test executable contributes to four global aggregates
  # and four module-scoped aggregates:
  #
  #   tests
  #   tests.<dialect>
  #   tests-<kind>
  #   tests-<kind>.<dialect>
  #
  #   <module>.tests
  #   <module>.tests.<dialect>
  #   <module>.tests-<kind>
  #   <module>.tests-<kind>.<dialect>
  #
  # The same naming dimensions are used by the corresponding
  # ".run" targets below.
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
