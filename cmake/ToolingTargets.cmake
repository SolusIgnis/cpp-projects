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
# Invokes per file for parallelism, incrementalism, and granularity.
add_custom_target(format-check
  COMMENT "Checking clang-format on all sources"
)
foreach(src IN LISTS ALL_TOOLING_SOURCES)
  add_custom_command(
    TARGET format-check
    COMMAND clang-format --dry-run --Werror --Wno-error=unknown ${src}
    COMMENT "Running clang-tidy on ${src}"
    VERBATIM
  )
endforeach()

# Target to run clang-tidy if we're using clang as our compiler.
# Invokes per file for parallelism, incrementalism, and granularity.
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_custom_target(tidy
    COMMENT "Running clang-tidy on all sources"
  )

  foreach(src IN LISTS ALL_TOOLING_SOURCES)
    add_custom_command(
      TARGET tidy
      COMMAND clang-tidy --use-color -p ${CMAKE_BINARY_DIR} ${src}
      COMMENT "Running clang-tidy on ${src}"
      VERBATIM
    )
  endforeach()
endif()
