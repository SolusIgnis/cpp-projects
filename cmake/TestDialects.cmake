# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
#
# TestDialects.cmake
#
# Registration of project-supported test-framework dialects.
#
# ============================================================

include_guard(GLOBAL)

# ============================================================
# Ensure we have our dialect registration function.
# ============================================================
include(DiscoverTests)

# ============================================================
# Boost-Ext.UT
# ============================================================
register_test_dialect(
  boost-ut
  CPM_NAME    "ut"
  GH_REPO     "boost-ext/ut"
  VERSION     "2.3.1"
  GIT_TAG     "59a9beba0763dbb45b3cc68e4cf484c659319a97"
  CPM_OPTIONS "BOOST_UT_DISABLE_MODULE NO"
  LINK_TARGET "Boost::ut_module"
)

# ============================================================
# qlibs/ut
# ============================================================
register_test_dialect(
  qlibs-ut
  CPM_NAME    "qlibs.ut"
  GH_REPO     "qlibs/ut"
  VERSION     "2.1.6"
  GIT_TAG     "1a2d76bb0d22e9d9e02c9726f00f6f8e632c21da"
  PATCHES     "${CMAKE_SOURCE_DIR}/cmake/patches/qlibs-ut.patch"
  LINK_TARGET "qlibs.ut::ut"
)

# ============================================================
# Catch2
# ============================================================
register_test_dialect(
  catch2
  CPM_NAME    "Catch2"
  GH_REPO     "catchorg/Catch2"
  VERSION     "3.16.0"
  GIT_TAG     "317ac1ed4c0bb6e6b91eafc817e05c488feffcb3"
  LINK_TARGET "Catch2::Catch2WithMain"
  DISCOVERY   "Catch2"
)

# ============================================================
# GTest
# ============================================================
register_test_dialect(
  gtest
  CPM_NAME    "gtest"
  GH_REPO     "google/googletest"
  VERSION     "1.18.0"
  GIT_TAG     "063de7e9578f82b369302001269680b4b1553359"
  CPM_OPTIONS "INSTALL_GTEST OFF" "gtest_force_shared_crt ON"
  LINK_TARGET "GTest::gtest_main"
  DISCOVERY   "GTest"
)
