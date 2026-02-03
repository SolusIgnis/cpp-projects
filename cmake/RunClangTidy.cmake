# Sanity check the required variables.
if (NOT DEFINED BUILD_DIR OR BUILD_DIR STREQUAL "")
  message(FATAL_ERROR "BUILD_DIR must be set to the build directory")
endif()
if (NOT DEFINED SOURCE OR SOURCE STREQUAL "")
  message(FATAL_ERROR "SOURCE must be set to the source file to check")
endif()
if (NOT DEFINED LOG_FILE OR LOG_FILE STREQUAL "")
  message(FATAL_ERROR "LOG_FILE must be set to the log file path")
endif()
if (NOT DEFINED STAMP_FILE OR STAMP_FILE STREQUAL "")
  message(FATAL_ERROR "STAMP_FILE must be set to the stamp file path")
endif()

# The actual work happens here. Run clang-tidy on the source file, and capture its output in a temporary log.
execute_process(
  COMMAND clang-tidy --use-color -p "${BUILD_DIR}" "${SOURCE}"
  RESULT_VARIABLE tidy_result
  OUTPUT_FILE "${LOG_FILE}.tmp"
  ERROR_FILE  "${LOG_FILE}.tmp"
)

# Generate a header to separate sources in the aggregate log
file(WRITE
  "${LOG_FILE}.header.tmp" "===== clang-tidy output for ${SOURCE} =====
" #This newline is intentional to provide a literal newline after the header.
)

# Combine header and temporary log into final log file
execute_process(
  COMMAND ${CMAKE_COMMAND} -E cat "${LOG_FILE}.header.tmp" "${LOG_FILE}.tmp"
  OUTPUT_FILE "${LOG_FILE}"
)

# Clean up temporary files
file(REMOVE "${LOG_FILE}.tmp")
file(REMOVE "${LOG_FILE}.header.tmp")

# Write out the stamp file indicating success or failure.
file(WRITE "${STAMP_FILE}" "${SOURCE};${tidy_result}")
