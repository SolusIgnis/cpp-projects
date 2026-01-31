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
set(TOOLING_TARGETS "")
set(TOOLING_EXAMPLE_TARGETS "")
set(TOOLING_TEST_TARGETS "")

# Helper function to register a tooling target.
function(register_tooling_target target)
  list(APPEND TOOLING_TARGETS ${target})
  set(TOOLING_TARGETS "${TOOLING_TARGETS}" PARENT_SCOPE)
endfunction()

# Helper function to register a tooling "example" target.
function(register_tooling_demo target)
  list(APPEND TOOLING_EXAMPLE_TARGETS ${target})
  set(TOOLING_EXAMPLE_TARGETS "${TOOLING_EXAMPLE_TARGETS}" PARENT_SCOPE)
endfunction()

# Helper function to register a tooling "test" target.
function(register_tooling_test target)
  list(APPEND TOOLING_TEST_TARGETS ${target})
  set(TOOLING_TEST_TARGETS "${TOOLING_TEST_TARGETS}" PARENT_SCOPE)
endfunction()
