# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors

# ===========================================================================
# Create tooling targets for clang-format (real and check) and clang-tidy.
# ===========================================================================

get_property(ALL_TOOLING_SOURCES GLOBAL PROPERTY ALL_TOOLING_SOURCES)

# Target to run clang-format in place on all sources.
add_custom_target(format-fix
  COMMAND clang-format -i ${ALL_TOOLING_SOURCES}
  COMMENT "Fixing code formatting with clang-format"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMAND_EXPAND_LISTS
)

# Target to run clang-format in checking mode.
add_custom_target(format-check
  COMMAND clang-format --dry-run --Werror --Wno-error=unknown ${ALL_TOOLING_SOURCES}
  COMMENT "Checking code formatting compliance with clang-format"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Target to run clang-tidy if we're using clang as our compiler.
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if(NOT DEFINED TIDY_JOBS)
    set(TIDY_JOBS 0) # 0 means use all available cores for run-clang-tidy
  endif()
  
  if(NOT DEFINED TIDY_SOURCE_FILTER_REGEX)
    get_tidy_source_filter(TIDY_SOURCE_FILTER_REGEX)
  endif()  

  add_custom_target(tidy-check
    COMMAND run-clang-tidy -use-color -p ${CMAKE_BINARY_DIR} -j${TIDY_JOBS} -source-filter "${TIDY_SOURCE_FILTER_REGEX}" -allow-no-checks
    COMMENT "Checking code with clang-tidy (parallelized by run-clang-tidy script)"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    VERBATIM
  )
  
  add_custom_target(tidy-fix
    COMMAND run-clang-tidy -fix -use-color -p ${CMAKE_BINARY_DIR} -j${TIDY_JOBS} -source-filter "${TIDY_SOURCE_FILTER_REGEX}" -allow-no-checks
    COMMENT "Checking code with clang-tidy in \"fix\" mode (parallelized by run-clang-tidy script)"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    VERBATIM
  )
endif()
