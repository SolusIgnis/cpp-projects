execute_process(
  COMMAND clang-tidy --use-color -p "${BUILD_DIR}" "${SOURCE}"
  RESULT_VARIABLE tidy_result
  OUTPUT_FILE "${LOG_FILE}"
  ERROR_FILE  "${LOG_FILE}"
)

file(WRITE "${STAMP_FILE}.tmp" "${SOURCE};${tidy_result}")
execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${STAMP_FILE}.tmp" "${STAMP_FILE}"
)
file(REMOVE "${STAMP_FILE}.tmp")

if (NOT tidy_result EQUAL 0)
  message(FATAL_ERROR "clang-tidy failed for ${SOURCE}")
endif()