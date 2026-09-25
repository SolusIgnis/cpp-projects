// SPDX-License-Identifier: Apache-2.0
// Common fixtures for unit tests for base.vocab.ptr

#pragma once

#include "base.vocab.ptr-common_fixtures.boost-ut.hpp"

//NOTE: This header is only included in single-TU test runners.
namespace {
    template<typename Lambda>
    //NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward): Forwarding is not needed to call the lambda.
    constexpr void test_each_pointer_type_with(Lambda&& test_impl)
    {
        test_impl.template operator()<base::vocab::ptr::dependency_ptr>();
        test_impl.template operator()<base::vocab::ptr::required_ptr>();
        test_impl.template operator()<base::vocab::ptr::alias_ptr>();
        test_impl.template operator()<base::vocab::ptr::cursor_ptr>();
        test_impl.template operator()<base::vocab::ptr::iterator_ptr>();
    }
} //namespace
