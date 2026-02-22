# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
#
# FetchUT.cmake
#
# Fetches qlibs/ut and creates canonical ut::ut module target.
#
# Provides:
#
#   Target:
#     ut::ut
#
# Usage:
#
#   include(cmake/FetchUT.cmake)
#   fetch_ut()
#

include_guard(GLOBAL)

include(FetchContent)

function(fetch_ut)

  # ----------------------------------------------------------
  # Fast exit if already available
  # ----------------------------------------------------------

  if(TARGET ut::ut)
    return()
  endif()

  message(STATUS "Fetching qlibs/ut via FetchContent")

  # ----------------------------------------------------------
  # Declare dependency (pinned commit with module support)
  # ----------------------------------------------------------

  FetchContent_Declare(
    ut
    GIT_REPOSITORY https://github.com/qlibs/ut.git
    GIT_TAG c6752919724ad5e33199751b0b224efb40647539
    GIT_SHALLOW TRUE
  )

  FetchContent_MakeAvailable(ut)

  # ----------------------------------------------------------
  # Verify module exists
  # ----------------------------------------------------------

  if(NOT DEFINED ut_SOURCE_DIR)
    message(FATAL_ERROR "FetchContent failed: ut_SOURCE_DIR undefined")
  endif()

  if(NOT EXISTS "${ut_SOURCE_DIR}/ut.cppm")
    message(FATAL_ERROR
      "qlibs/ut module file ut.cppm not found at expected location:\n"
      "  ${ut_SOURCE_DIR}/ut.cppm\n"
      "Repository layout may have changed."
    )
  endif()

  # ----------------------------------------------------------
  # Create module target
  # ----------------------------------------------------------

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

  # ----------------------------------------------------------
  # Create canonical alias
  # ----------------------------------------------------------

  add_library(ut::ut ALIAS qlibs.ut)

  # ----------------------------------------------------------
  # Export status
  # ----------------------------------------------------------

  message(STATUS "qlibs/ut fetched and ut::ut target created")

endfunction()
