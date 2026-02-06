# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors

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
