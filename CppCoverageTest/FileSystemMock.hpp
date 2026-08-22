// OpenCppCoverage is an open source code coverage for C++.
// Copyright (C) 2018 OpenCppCoverage
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

#include "CppCoverage/IFileSystem.hpp"

namespace boost
{
	// GMock's verbose output needs to print GetLastWriteTime's return type
	// below. boost::optional<T> unconditionally declares its own operator<<
	// (in optional.hpp), so GMock's printer-detection picks it over any
	// fallback, but that operator<< requires T itself to be streamable -
	// which std::filesystem::file_time_type is not - and fails to compile
	// once instantiated. Defining PrintTo here (found via ADL, and given
	// priority by GTest/GMock over operator<<) avoids that entirely.
	inline void PrintTo(const boost::optional<std::filesystem::file_time_type>& value, std::ostream* os)
	{
		if (value)
			*os << "file_time_type(present)";
		else
			*os << "none";
	}
}

namespace CppCoverageTest
{
	class FileSystemMock : public CppCoverage::IFileSystem
	{
	  public:
		MOCK_CONST_METHOD1(
		    GetLastWriteTime,
		    boost::optional<std::filesystem::file_time_type>(const std::filesystem::path&));
	};
}