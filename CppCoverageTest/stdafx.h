// OpenCppCoverage is an open source code coverage for C++.
// Copyright (C) 2014 OpenCppCoverage
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Needed so ASSERT_EQ/EXPECT_EQ can stream boost::optional<T> values (e.g.
// boost::optional<std::filesystem::path>) in failure messages; without it,
// boost::optional's own operator<< is only declared, not defined, and any
// use of it hits a static_assert. FileSystemMock.hpp separately defines a
// PrintTo() for boost::optional<std::filesystem::file_time_type>, which is
// not itself streamable and is therefore excluded via GTest/GMock's PrintTo
// > operator<< priority instead of relying on this header.
#include <boost/optional/optional_io.hpp>