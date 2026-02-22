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
    GIT_TAG main
    GIT_SHALLOW TRUE
  )

  FetchContent_MakeAvailable(ut)

  # ----------------------------------------------------------
  # Verify module exists
  # ----------------------------------------------------------
  if(NOT DEFINED ut_SOURCE_DIR)
    message(FATAL_ERROR "FetchContent failed: ut_SOURCE_DIR undefined")
  endif()
  
  set(UT_CPPM "${ut_SOURCE_DIR}/ut.cppm")
  set(UT_HEADER "${ut_SOURCE_DIR}/ut")

  if(NOT EXISTS "${UT_CPPM}")
    message(FATAL_ERROR
      "qlibs/ut module file ut.cppm not found at expected location:\n"
      "  ${UT_CPPM}\n"
      "Repository layout may have changed."
    )
  endif()
  if(NOT EXISTS "${UT_HEADER}")
    message(FATAL_ERROR
      "qlibs/ut header file ut not found at expected location:\n"
      "  ${UT_HEADER}\n"
      "Repository layout may have changed."
    )
  endif()
  
  # ----------------------------------------------------------
  # Patch module to fix #include <iostream> bug
  # ----------------------------------------------------------
  
  function(_patch filepath old_code new_code)
    file(READ "${filepath}" FILE_CONTENTS)
    string(REPLACE
      "${old_code}"
      "${new_code}"
      FILE_CONTENTS
      "${FILE_CONTENTS}"
    )
    file(WRITE "${filepath}" "${FILE_CONTENTS}")
  endfunction()

  # Insert #include <iostream> after 'module;' and before '#include "ut"'
  _patch("${UT_CPPM}"
    "module;\n#include \"ut\"\n"
    "module;\n#include <iostream>\n#include \"ut\"\n"
  )

  # Patch out the ambiguous forward declarations in ut.cppm
  _patch("${UT_HEADER}"
    "namespace std { // iosfwd\ntemplate<class> struct char_traits;\ntemplate<class, class> class basic_ostream;\nextern basic_ostream<char, char_traits<char>> clog; // only used if defined\n} // namespace std"
     "#if 0\nnamespace std { // iosfwd\ntemplate<class> struct char_traits;\ntemplate<class, class> class basic_ostream;\nextern basic_ostream<char, char_traits<char>> clog; // only used if defined\n} // namespace std\n#endif"
  )

  file(READ "${UT_CPPM}" UT_M_CONTENTS)
  message(STATUS "${UT_M_CONTENTS}")
  file(READ "${UT_HEADER}" UT_H_CONTENTS)
  message(STATUS "${UT_H_CONTENTS}")

  # ----------------------------------------------------------
  # Create module target
  # ----------------------------------------------------------
  add_library(qlibs.ut OBJECT)

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
