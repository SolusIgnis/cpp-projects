# ===========================================================================
# Create tooling targets for clang-format (real and check) and clang-tidy.
# ===========================================================================

get_property(ALL_TOOLING_SOURCES GLOBAL PROPERTY ALL_TOOLING_SOURCES)

# Target to run clang-format (for real).
add_custom_target(format
  COMMAND clang-format -i
          ${ALL_TOOLING_SOURCES}
  COMMENT "Running clang-format"
)

# Target to run clang-format in checking mode.
add_custom_target(format-check
  COMMAND clang-format --dry-run --Werror
          ${ALL_TOOLING_SOURCES}
  COMMENT "Checking clang-format"
)

# Target to run clang-tidy if we're using clang as our compiler.
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_custom_target(tidy
    COMMAND clang-tidy
      -p ${CMAKE_BINARY_DIR}
      ${ALL_TOOLING_SOURCES}
    COMMENT "Running clang-tidy"
  )
endif()
