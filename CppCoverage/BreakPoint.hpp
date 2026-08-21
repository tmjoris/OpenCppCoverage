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

#include <cstdint>
#include <Windows.h>
#include "CppCoverageExport.hpp"

namespace CppCoverage
{
	class Address;

	class CPPCOVERAGE_DLL BreakPoint
	{
	  public:
		BreakPoint() = default;

		// Native width of the in-memory software breakpoint trap
		// instruction for the current target architecture: a single INT3
		// (0xCC) byte on x86/x64, or a full 4-byte BRK instruction on
		// ARM64 (AArch64 has no sub-word instructions).
#if defined(_M_ARM64) || defined(_M_ARM64EC)
		using OpCodeValue = std::uint32_t;
#else
		using OpCodeValue = unsigned char;
#endif

		static const OpCodeValue breakPointInstruction;

		void RemoveBreakPoint(const Address&,
		                      OpCodeValue oldInstruction) const;

		using InstructionCollection =
		    std::vector<std::pair<OpCodeValue, DWORD64>>;

		InstructionCollection
		SetBreakPoints(HANDLE hProcess, std::vector<DWORD64>&& addresses) const;

		void AdjustEipAfterBreakPointRemoval(HANDLE hThread) const;

	  private:
		BreakPoint(const BreakPoint&) = delete;
		BreakPoint& operator=(const BreakPoint&) = delete;
	};
}
