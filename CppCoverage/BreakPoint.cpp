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

#include "stdafx.h"
#include "BreakPoint.hpp"

#include <cstring>

#include "CppCoverageException.hpp"
#include "Address.hpp"

#include "Tools/Log.hpp"
#include "Tools/ProcessMemory.hpp"

namespace CppCoverage
{
	using Addresses = std::vector<DWORD64>;
	using AddressesIt = Addresses::const_iterator;

	//-------------------------------------------------------------------------
	void SetBreakPointsRange(HANDLE hProcess,
	                         AddressesIt begin,
	                         AddressesIt end,
	                         BreakPoint::InstructionCollection& oldInstructions)
	{
		if (begin == end)
			return;

		auto firstValue = *begin;
		auto memorySpaceSize =
		    *(end - 1) - firstValue + sizeof(BreakPoint::breakPointInstruction);
		auto firstAddress = reinterpret_cast<void*>(firstValue);
		auto buffer = Tools::ReadProcessMemory(
		    hProcess, firstAddress, static_cast<size_t>(memorySpaceSize));

		for (auto it = begin; it < end; ++it)
		{
			auto index = static_cast<size_t>(*it - firstValue);
			BreakPoint::OpCodeValue oldInstruction;
			std::memcpy(&oldInstruction, &buffer[index], sizeof(oldInstruction));
			std::memcpy(&buffer[index],
			            &BreakPoint::breakPointInstruction,
			            sizeof(BreakPoint::breakPointInstruction));
			oldInstructions.emplace_back(oldInstruction, *it);
		}
		Tools::WriteProcessMemory(
		    hProcess, firstAddress, &buffer[0], buffer.size());
	}

#if defined(_M_ARM64) || defined(_M_ARM64EC)
	// BRK #0xF000: the same software-breakpoint encoding used by
	// ntdll!DbgBreakPoint on Windows ARM64 (in-memory little-endian bytes:
	// 00 F0 3E D4), so it is recognized consistently by the OS and other
	// debugging tools.
	const BreakPoint::OpCodeValue BreakPoint::breakPointInstruction = 0xD43EF000;
#else
	const BreakPoint::OpCodeValue BreakPoint::breakPointInstruction = 0xCC;
#endif

	//-------------------------------------------------------------------------
	BreakPoint::InstructionCollection
	BreakPoint::SetBreakPoints(HANDLE hProcess, Addresses&& addresses) const
	{
		InstructionCollection oldInstructions;

		std::sort(addresses.begin(), addresses.end());
		auto beginRange = addresses.cbegin();

		for (auto it = beginRange; it < addresses.cend(); ++it)
		{
			if (*it - *beginRange > 4096)
			{
				SetBreakPointsRange(hProcess, beginRange, it, oldInstructions);
				beginRange = it;
			}
		}
		SetBreakPointsRange(
		    hProcess, beginRange, addresses.end(), oldInstructions);

		return oldInstructions;
	}

	//-------------------------------------------------------------------------
	void BreakPoint::RemoveBreakPoint(const Address& address,
	                                  OpCodeValue oldInstruction) const
	{
		Tools::WriteProcessMemory(address.GetProcessHandle(),
		                          address.GetValue(),
		                          &oldInstruction,
		                          sizeof(oldInstruction));
	}

	//-------------------------------------------------------------------------
	void BreakPoint::AdjustEipAfterBreakPointRemoval(HANDLE hThread) const
	{
#if defined(_M_ARM64) || defined(_M_ARM64EC)
		// On ARM64, the PC captured at a BRK trap already points at the BRK
		// instruction itself (unlike x86, where the CPU leaves Rip/Eip one
		// byte *past* the 1-byte INT3). RemoveBreakPoint has already
		// restored the original 4-byte instruction at that same address, so
		// execution can resume from the unmodified PC: no register rewind
		// is needed here.
		(void)hThread;
#else
		CONTEXT lcContext;
		lcContext.ContextFlags = CONTEXT_ALL;
		if (!GetThreadContext(hThread, &lcContext))
			THROW_LAST_ERROR("Error in GetThreadContext", GetLastError());

#ifdef _WIN64
		--lcContext.Rip; // Move back one byte
#else
		--lcContext.Eip; // Move back one byte
#endif
		if (!SetThreadContext(hThread, &lcContext))
			THROW_LAST_ERROR("Error in SetThreadContext", GetLastError());
#endif
	}
}
