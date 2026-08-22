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
#include <boost/optional.hpp>

// boost::optional<T> unconditionally declares its own operator<< (in
// optional.hpp), gated by a static_assert requiring optional_io.hpp - and
// even with that included, it requires T itself to be streamable via a
// narrow std::ostream, which is not true for every T used in this test
// suite (e.g. std::filesystem::file_time_type isn't streamable at all;
// CppCoverage::Options only has a std::wostream operator<<). Rather than
// fix this per-T as each one surfaces a static_assert or compile error,
// define a generic PrintTo() for boost::optional<T> here (found via ADL,
// and given priority by GTest/GMock over operator<< for any T since it is
// more specialized than the generic ::testing::internal::PrintTo<T>
// fallback). It delegates to GTest's own UniversalPrinter<T>, which
// already handles arbitrary T gracefully (falling back to a raw byte dump
// if nothing else applies), so this never fails to compile regardless of
// whether T is printable.
namespace boost
{
	template <typename T>
	void PrintTo(const boost::optional<T>& value, std::ostream* os)
	{
		if (value)
			::testing::internal::UniversalPrinter<T>::Print(*value, os);
		else
			*os << "none";
	}
}