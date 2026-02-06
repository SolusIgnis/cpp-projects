# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors

# ===========================================================================
# Aggregate the tooling-registered targets and their sources.
# ===========================================================================

# Start with tooling targets.
get_property(TOOLING_TARGETS GLOBAL PROPERTY TOOLING_TARGETS)
set(ALL_TOOLING_TARGETS ${TOOLING_TARGETS})

# Add tooling "example" targets if selected.
if (ENABLE_TOOLING_EXAMPLES)
  get_property(TOOLING_EXAMPLE_TARGETS GLOBAL PROPERTY TOOLING_EXAMPLE_TARGETS)
  list(APPEND ALL_TOOLING_TARGETS ${TOOLING_EXAMPLE_TARGETS})
endif()

# Add tooling "test" targets if selected.
if (ENABLE_TOOLING_TESTS)
  get_property(TOOLING_TEST_TARGETS GLOBAL PROPERTY TOOLING_TEST_TARGETS)
  list(APPEND ALL_TOOLING_TARGETS ${TOOLING_TEST_TARGETS})
endif()

# Tooling sources registry 
unset(ALL_TOOLING_SOURCES)

# Convert list into a regex string like: (c|cc|cpp|cxx|cppm...)
string(REPLACE ";" "|" TOOLING_EXT_REGEX "${TOOLING_EXTENSIONS}")

# Aggregate sources from the tooling targets.
foreach(tgt IN LISTS ALL_TOOLING_TARGETS)
  get_target_property(modules ${tgt} CXX_MODULE_SET)
  get_target_property(srcs ${tgt} SOURCES)
  get_target_property(src_dir ${tgt} SOURCE_DIR)
  foreach(src IN LISTS modules srcs)
    string(TOLOWER "${src}" src_lower)
    string(TOLOWER "${TOOLING_EXT_REGEX}" ext_regex_lower)
    if (src_lower MATCHES "\\.(${ext_regex_lower})$")
      if (IS_ABSOLUTE "${src}")
        list(APPEND ALL_TOOLING_SOURCES "${src}")
      else()
       list(APPEND ALL_TOOLING_SOURCES "${src_dir}/${src}")
      endif()
    endif()
  endforeach()
endforeach()

# Deduplicate source files.
list(REMOVE_DUPLICATES ALL_TOOLING_SOURCES)

# Store the aggregated tooling sources in a global property.
set_property(GLOBAL PROPERTY ALL_TOOLING_SOURCES "${ALL_TOOLING_SOURCES}")
