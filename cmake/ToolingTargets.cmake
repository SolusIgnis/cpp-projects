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
)

# Target to run clang-format in checking mode.
add_custom_target(format-check
  COMMAND clang-format --dry-run --Werror --Wno-error=unknown ${ALL_TOOLING_SOURCES}
  COMMENT "Checking code formatting compliance with clang-format"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Target to run clang-tidy if we're using clang as our compiler.
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_custom_target(tidy-check
    COMMAND clang-tidy
      --use-color -p ${CMAKE_BINARY_DIR} ${ALL_TOOLING_SOURCES}
    COMMENT "Checking code with clang-tidy"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )
  
  add_custom_target(tidy-fix
    COMMAND clang-tidy
      -fix --use-color -p ${CMAKE_BINARY_DIR} ${ALL_TOOLING_SOURCES}
    COMMENT "Checking code with clang-tidy in \"fix\" mode"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )
  
  if(NOT DEFINED TIDY_JOBS)
    set(TIDY_JOBS 0) # 0 means use all available cores for run-clang-tidy
  endif()
  
  add_custom_target(tidy-check-fast
    COMMAND run-clang-tidy --use-color -p ${CMAKE_BINARY_DIR} -j${TIDY_JOBS}
    COMMENT "Checking code with clang-tidy (parallelized by run-clang-tidy script)"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )
  
  add_custom_target(tidy-fix-fast
    COMMAND run-clang-tidy -fix --use-color -p ${CMAKE_BINARY_DIR} -j${TIDY_JOBS}
    COMMENT "Checking code with clang-tidy in \"fix\" mode (parallelized by run-clang-tidy script)"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )

  set(TIDY_STAMPS "")
  set(TIDY_LOGS "")
  set(COMPILE_COMMANDS "${CMAKE_BINARY_DIR}/compile_commands.json")
  set(TIDY_CONFIG_FILE "${CMAKE_SOURCE_DIR}/.clang-tidy")
  set(TIDY_DIR "${CMAKE_BINARY_DIR}/tidy")
  file(MAKE_DIRECTORY ${TIDY_DIR})
  set(TIDY_LOG_DIR "${TIDY_DIR}/logs")
  file(MAKE_DIRECTORY ${TIDY_LOG_DIR})
  set(TIDY_STAMP_DIR "${TIDY_DIR}/stamps")
  file(MAKE_DIRECTORY ${TIDY_STAMP_DIR})

  foreach(src IN LISTS ALL_TOOLING_SOURCES)
#  foreach (i RANGE 0 3)
#    list(GET ALL_TOOLING_SOURCES ${i} src)
    file(RELATIVE_PATH src_name ${CMAKE_SOURCE_DIR} ${src})
    string(REPLACE "/" "_" src_name ${src_name})
    string(REPLACE "\\" "_" src_name ${src_name})  # Windows path
    string(SHA1 src_hash "${src}")
    set(base_name "${src_name}-${src_hash}")
    set(log_file "${TIDY_LOG_DIR}/${base_name}.log")
    list(APPEND TIDY_LOGS ${log_file}) 
    set(stamp_file "${TIDY_STAMP_DIR}/${base_name}.stamp")
    list(APPEND TIDY_STAMPS ${stamp_file})
      
    add_custom_command(
      OUTPUT ${stamp_file}
      BYPRODUCTS ${log_file}
      COMMAND ${CMAKE_COMMAND}
          -DBUILD_DIR=${CMAKE_BINARY_DIR}
          -DSOURCE=${src}
          -DLOG_FILE=${log_file}
          -DSTAMP_FILE=${stamp_file}
          -P ${CMAKE_SOURCE_DIR}/cmake/RunClangTidy.cmake
      DEPENDS
        ${src}
        ${COMPILE_COMMANDS}
        ${TIDY_CONFIG_FILE}
      VERBATIM
      COMMENT "clang-tidy ${src}"
    )
  endforeach()

  message(STATUS "${TIDY_STAMPS}
  
  ${TIDY_LOGS}")

  add_custom_target(tidy-fast
    DEPENDS ${TIDY_STAMPS}
    COMMAND ${CMAKE_COMMAND}
        -DBUILD_DIR=${CMAKE_BINARY_DIR}
        -DSTAMPS=${TIDY_STAMPS}
        -DLOGS=${TIDY_LOGS}
        -P ${CMAKE_SOURCE_DIR}/cmake/AggregateClangTidyResults.cmake
    COMMENT "All clang-tidy checks completed"
  )
endif()
