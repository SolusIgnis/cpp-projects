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

  if(NOT EXISTS "${UT_CPPM}")
    message(FATAL_ERROR
      "qlibs/ut module file ut.cppm not found at expected location:\n"
      "  ${UT_CPPM}\n"
      "Repository layout may have changed."
    )
  endif()
  
  # ----------------------------------------------------------
  # Patch module to fix #include <iostream> bug
  # ----------------------------------------------------------
  
  # Read the file
  file(READ "${UT_CPPM}" UT_CONTENTS)

  # Insert #include <iostream> after 'module;' and before '#include "ut"'
#  string(REPLACE
#    "module;\n#include \"ut\"\n"
#    "module;\n#include <iostream>\n#include \"ut\"\n"
#    UT_CONTENTS
#    "${UT_CONTENTS}"
#  )
  string(REPLACE
    "module;\n#include \"ut\"\nexport module ut;\n"
    "module;\n#include \"ut\"\nexport module ut;\nimport std;\n"
    UT_CONTENTS
    "${UT_CONTENTS}"
  )

  # Patch out the ambiguous forward declarations in ut.cppm
#  string(REGEX REPLACE
#    "namespace std \\{[ \t\n]*template<class> struct char_traits;[ \t\n]*template<class, class> class basic_ostream;[ \t\n]*extern basic_ostream<char, char_traits<char>> clog;[ \t\n]*\\}"
#    "#if 0\nnamespace std {\ntemplate<class> struct char_traits;\ntemplate<class, class> class basic_ostream;\nextern basic_ostream<char, char_traits<char>> clog;\n}\n#endif"
#    UT_CONTENTS
#    "${UT_CONTENTS}"
#  )

#message(STATUS "${UT_CONTENTS}")


  # Write it back
  file(WRITE "${UT_CPPM}" "${UT_CONTENTS}")

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
