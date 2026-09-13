# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
#
# FetchCPM.cmake
#
# Fetches CPM and includes it. 

include_guard(GLOBAL)

# Current as of 2026-09-12
set(CPM_VERSION "0.43.1")
# SHA-256 Hash per GitHub Asset Metadata
set(CPM_HASH "1c40fc102ce9625d7de7eb14f541cab30cc3138dca627f0b0ec40293ce6c2934")

set(CPM_URL "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_VERSION}/CPM.cmake")
set(CPM_PATH "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_VERSION}.cmake")

if(NOT EXISTS "${CPM_PATH}")
  file(DOWNLOAD
       "${CPM_URL}"
       "${CPM_PATH}"
       EXPECTED_HASH "SHA256=${CPM_HASH}"
       TLS_VERIFY ON
  )
endif()

include("${CPM_PATH}")
