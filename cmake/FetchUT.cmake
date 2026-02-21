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

  FetchContent_GetProperties(ut)

  if(NOT ut_POPULATED)
    FetchContent_Populate(ut)
  endif()

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
