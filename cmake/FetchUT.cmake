include_guard(GLOBAL)

include(FetchContent)

function(fetch_ut)
  if(TARGET ut::ut)
    return()
  endif()

  message(STATUS "Fetching qlibs/ut via FetchContent")
  FetchContent_Declare(
    ut
    GIT_REPOSITORY https://github.com/qlibs/ut.git
    GIT_TAG v2.1.6
  )

  FetchContent_MakeAvailable(ut)

message(STATUS "ut_SOURCE_DIR = ${ut_SOURCE_DIR}")

file(GLOB_RECURSE ut_contents
  RELATIVE "${ut_SOURCE_DIR}"
  "${ut_SOURCE_DIR}/*"
)

foreach(item IN LISTS ut_contents)
  if(IS_DIRECTORY "${ut_SOURCE_DIR}/${item}")
    message(STATUS "[DIR ] ${item}")
  else()
    message(STATUS "[FILE] ${item}")
  endif()
endforeach()


  # Create module target
  add_library(qlibs.ut STATIC)

  target_sources(qlibs.ut
    PUBLIC
      FILE_SET CXX_MODULES
      BASE_DIRS ${ut_SOURCE_DIR}
      FILES
        ${ut_SOURCE_DIR}/ut.cppm
  )
  
  target_include_directories(qlibs.ut
    PUBLIC
      ${ut_SOURCE_DIR}/include
  )

  target_compile_features(qlibs.ut
    PUBLIC
      cxx_std_23
  )

  # Create canonical alias
  add_library(ut::ut ALIAS qlibs.ut)
endfunction()
