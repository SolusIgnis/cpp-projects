# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors

include_guard(GLOBAL)

# ===========================================================================
# Tooling Infrastructure (Options, Cache Variables, Registries, and Helper Functions)
# ===========================================================================

# Tooling-related options: include tests or demos?
option(ENABLE_TOOLING_EXAMPLES "Include examples in clang-tidy/format" OFF)
option(ENABLE_TOOLING_TESTS "Include tests in clang-tidy/format" ON)

# Default set of source extensions
set(TOOLING_EXTENSIONS
  "c;cc;cpp;cxx;cppm;ixx;h;hh;hpp;hxx;inc"
  CACHE STRING "Semicolon-separated list of file extensions to include in tooling targets"
)

# Tooling target registries
set_property(GLOBAL PROPERTY TOOLING_TARGETS "")
set_property(GLOBAL PROPERTY TOOLING_EXAMPLE_TARGETS "")
set_property(GLOBAL PROPERTY TOOLING_TEST_TARGETS "")

# Helper function to register a tooling target.
function(register_tooling_target target)
  set_property(GLOBAL APPEND PROPERTY TOOLING_TARGETS ${target})
  get_property(TOOLING_TARGETS GLOBAL PROPERTY TOOLING_TARGETS)
endfunction()

# Helper function to register a tooling "example" target.
function(register_tooling_demo target)
  set_property(GLOBAL APPEND PROPERTY TOOLING_EXAMPLE_TARGETS ${target})
endfunction()

# Helper function to register a tooling "test" target.
function(register_tooling_test target)
  set_property(GLOBAL APPEND PROPERTY TOOLING_TEST_TARGETS ${target})
endfunction()

# Helper function to escape regex metacharacters in a string.
function(regex_escape out_var input)
  # First, double any \ present in ${input}. Note "\\" is a single \ inside the string.
  string(REPLACE "\\" "\\\\" _escaped "${input}")

  # Then, escape each regex metacharacter by adding a \. These MUST be inserted AFTER the \ doubling of the first step.
  foreach(_char IN ITEMS
    "." "^" "$" "*" "+" "?" "(" ")" "[" "]" "{" "}" "|"
  )
    string(REPLACE "${_char}" "\\${_char}" _escaped "${_escaped}")
  endforeach()

  set(${out_var} "${_escaped}" PARENT_SCOPE)
endfunction()

# Helper function to generate a regex for run-clang-tidy to filter sources from the compilation database.
function(get_tidy_source_filter out_var)
  regex_escape(_tidy_binary_dir_regex "${CMAKE_BINARY_DIR}")

  set(_tidy_excluded_paths
    "${_tidy_binary_dir_regex}/"
    "(.*/)?std\\.cppm$"
    "(.*/)?std\\.compat\\.cppm$"
  )

  list(JOIN _tidy_excluded_paths "|" _tidy_excluded_paths_regex)

  set(${out_var} "^(?!(${_tidy_excluded_paths_regex})).*$" PARENT_SCOPE)
endfunction()
