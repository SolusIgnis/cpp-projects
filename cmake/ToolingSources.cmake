# ===========================================================================
# Aggregate the tooling-registered targets and their sources.
# ===========================================================================

# Start with tooling targets.
set(ALL_TOOLING_TARGETS ${TOOLING_TARGETS})

# Add tooling "example" targets if selected.
if (ENABLE_TOOLING_EXAMPLES)
  list(APPEND ALL_TOOLING_TARGETS ${TOOLING_EXAMPLE_TARGETS})
endif()

# Add tooling "test" targets if selected.
if (ENABLE_TOOLING_TESTS)
  list(APPEND ALL_TOOLING_TARGETS ${TOOLING_TEST_TARGETS})
endif()

# Tooling sources registry 
set(ALL_TOOLING_SOURCES "")

# Convert list into a regex string like: (c|cc|cpp|cxx|cppm...)
string(REPLACE ";" "|" TOOLING_EXT_REGEX "${TOOLING_EXTENSIONS}")

# Aggregate sources from the tooling targets.
foreach(tgt IN LISTS ALL_TOOLING_TARGETS)
  get_target_property(srcs ${tgt} SOURCES)
  foreach(src IN LISTS srcs)
    if (src MATCHES "(?i)\\.(${TOOLING_EXT_REGEX})$")
      list(APPEND ALL_TOOLING_SOURCES "${src}")
    endif()
  endforeach()
endforeach()

# Deduplicate source files.
list(REMOVE_DUPLICATES ALL_TOOLING_SOURCES)

option(SHOW_TOOLING_SOURCES "Display a list of sources passed to clang-format or clang-tidy." OFF)
# Display tooling sources.
if (SHOW_TOOLING_SOURCES)
  message(STATUS "Tooling sources as passed to clang-format or clang-tidy: ${ALL_TOOLING_SOURCES}")
endif()
