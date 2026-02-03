if (NOT DEFINED STAMPS OR STAMPS STREQUAL "")
  message(FATAL_ERROR "STAMPS variable MUST be set to a list of stamp files")
endif()

if (NOT DEFINED BUILD_DIR OR BUILD_DIR STREQUAL "")
  message(FATAL_ERROR "BUILD_DIR variable MUST be set to the build directory")
endif()

if (DEFINED LOGS AND NOT LOGS STREQUAL "")
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E cat ${LOGS}
    OUTPUT_VARIABLE all_logs
  )
  file(WRITE "${BUILD_DIR}/tidy/logs/tidy.log" "${all_logs}")
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E echo "${all_logs}"
  )
  foreach (log IN LISTS LOGS)
    file(REMOVE "${log}")
  endforeach()
endif()

foreach(stamp IN LISTS STAMPS)
  if (NOT EXISTS "${stamp}")
    message(FATAL_ERROR "Missing stamp file: ${stamp}")
  endif()

  file(READ "${stamp}" previous_stamp)
  string(STRIP "${previous_stamp}" previous_stamp)

  list(LENGTH previous_stamp len)
  if (NOT len EQUAL 2)
    message(FATAL_ERROR "Invalid stamp format: ${stamp} -> \"${previous_stamp}\" (expected: \"/path/to/file;0\")")
  endif()

  list(GET previous_stamp 0 src)
  list(GET previous_stamp 1 result)
    
  if (NOT result STREQUAL "0")
    message(FATAL_ERROR "clang-tidy failed for ${src}")
  endif()
endforeach()