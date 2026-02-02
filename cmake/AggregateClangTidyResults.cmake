execute_process(
  COMMAND ${CMAKE_COMMAND} -E cat ${STAMPS}
)
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