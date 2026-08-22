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

#include <filesystem>
#include <boost/algorithm/string/predicate.hpp >
#include "TestHelper/TemporaryPath.hpp"
#include "CppCoverage/OptionsParser.hpp"
#include "CppCoverage/ProgramOptions.hpp"
#include "OpenCppCoverage/OpenCppCoverage.hpp"
#include "TestCoverageConsole/TestCoverageConsole.hpp"
// TestCoverageSharedLib is built with /clr, which is not supported on ARM64.
#if !defined(_M_ARM64) && !defined(_M_ARM64EC)
#include "TestCoverageSharedLib/TestCoverageSharedLib.hpp"
#endif

#include "OpenCppCoverageTestTools.hpp"

namespace fs = std::filesystem;
namespace cov = CppCoverage;

namespace OpenCppCoverageTest
{
	namespace
	{
		auto testCoverageConsole = TestCoverageConsole::GetOutputBinaryPath();
		// TestCoverageSharedLib is built with /clr, which is not supported on ARM64.
#if !defined(_M_ARM64) && !defined(_M_ARM64EC)
		auto testCoverageSharedLib = TestCoverageSharedLib::GetOutputBinaryPath();
		
		auto testCoverageSharedLibMain = TestCoverageSharedLib::GetMainCppPath();
#endif

		//---------------------------------------------------------------------
		bool FindFilename(const std::wstring& filename, const fs::path& outputDirectory)
		{
			for (fs::recursive_directory_iterator it(outputDirectory);
				it != fs::recursive_directory_iterator(); ++it)
			{
				const auto& path = it->path();

				if (boost::iequals(path.filename().stem().wstring(), filename))
					return true;
			}

			return false;
		}
		
		//---------------------------------------------------------------------
		class CommandLineOptionsTest : public ::testing::Test
		{
		public:
			//-----------------------------------------------------------------
			int RunCoverageOnProgramWithExitCode(
				std::vector<std::pair<std::string, std::string>> coverageArguments,
				bool useSourceInSolutionDir = true)
			{
				if (useSourceInSolutionDir)
					coverageArguments.push_back({ cov::ProgramOptions::SelectedSourcesOption, GetSolutionFolderName()});
				auto thirdParties = { "Build", "packages" };
				for (const auto& thirdParty: thirdParties)
					coverageArguments.push_back({ cov::ProgramOptions::ExcludedSourcesOption, thirdParty });

				AddDefaultHtmlExport(coverageArguments, GetTempPath());
				coverageArguments.emplace_back(cov::ProgramOptions::QuietOption, "");
				std::string ignoreOutput;
				return RunCoverageFor(coverageArguments, testCoverageConsole, {}, &ignoreOutput);
			}

			//-----------------------------------------------------------------
			void RunCoverageOnProgram(
				std::vector<std::pair<std::string, std::string>> coverageArguments,
				bool useSourceInSolutionDir = true)
			{
				int exitCode = RunCoverageOnProgramWithExitCode(coverageArguments,
					useSourceInSolutionDir);

				ASSERT_EQ(0, exitCode);
			}

			//-----------------------------------------------------------------
			void CheckFilenameExistsInOutput(const fs::path& path, bool expectedValue)
			{
				auto filename = path.filename().replace_extension("");
				CheckFilenameWithExtensionExistsInOutput(filename, expectedValue);
			}

			//-----------------------------------------------------------------
			void CheckFilenameWithExtensionExistsInOutput(const fs::path& path, bool expectedValue)
			{
				auto filename = path.filename();
				ASSERT_EQ(expectedValue, FindFilename(filename.wstring(), GetTempPath()));
			}

			//-----------------------------------------------------------------
			const fs::path& GetTempPath() const
			{
				return tempFolder_.GetPath();
			}

		private:
			TestHelper::TemporaryPath tempFolder_;
		};
	}
	
	//-------------------------------------------------------------------------
	// The following tests compare against TestCoverageSharedLib, which is built
	// with /clr and is not supported on ARM64.
#if !defined(_M_ARM64) && !defined(_M_ARM64EC)
	TEST_F(CommandLineOptionsTest, SelectedModulesOption)
	{		
		RunCoverageOnProgram({ { cov::ProgramOptions::SelectedModulesOption, testCoverageConsole.string() } });
		CheckFilenameExistsInOutput(testCoverageConsole, true);
		CheckFilenameExistsInOutput(testCoverageSharedLib, false);
	}

	//-------------------------------------------------------------------------
	TEST_F(CommandLineOptionsTest, ExcludedModulesOption)
	{
		RunCoverageOnProgram({ { cov::ProgramOptions::ExcludedModulesOption, testCoverageConsole.string() } });
		CheckFilenameExistsInOutput(testCoverageConsole, false);
		CheckFilenameExistsInOutput(testCoverageSharedLib, true);
	}

	//-------------------------------------------------------------------------
	TEST_F(CommandLineOptionsTest, SelectedSourcesOption)
	{
		auto testCoverageConsoleMain = TestCoverageConsole::GetMainCppFilename().string();

		RunCoverageOnProgram({ { cov::ProgramOptions::SelectedSourcesOption, testCoverageConsoleMain } }, false);
		CheckFilenameWithExtensionExistsInOutput(testCoverageConsoleMain, true);
		CheckFilenameWithExtensionExistsInOutput(testCoverageSharedLibMain, false);
	}

	//-------------------------------------------------------------------------
	TEST_F(CommandLineOptionsTest, ExcludedSourcesOption)
	{
		auto testCoverageConsoleMain = TestCoverageConsole::GetMainCppFilename().string();

		RunCoverageOnProgram({ { cov::ProgramOptions::ExcludedSourcesOption, testCoverageConsoleMain } });
		CheckFilenameWithExtensionExistsInOutput(testCoverageConsoleMain, false);
		CheckFilenameWithExtensionExistsInOutput(testCoverageSharedLibMain, true);
	}

	//-------------------------------------------------------------------------
	TEST_F(CommandLineOptionsTest, OutputDirectoryOption)
	{
		RunCoverageOnProgram({});
		CheckFilenameExistsInOutput(testCoverageConsole, true);
		CheckFilenameExistsInOutput(testCoverageSharedLib, true);
	}
#endif

	//-------------------------------------------------------------------------
	TEST_F(CommandLineOptionsTest, FailureExitCode)
	{
		std::vector<std::pair<std::string, std::string>> coverageArguments;

		ASSERT_EQ(0, RunCoverageOnProgramWithExitCode(coverageArguments));

		coverageArguments.emplace_back("Invalid option", "Invalid value");
		ASSERT_EQ(OpenCppCoverage::FailureExitCode,
		          RunCoverageOnProgramWithExitCode(coverageArguments));

		// This should failed at runtime
		coverageArguments.clear();
		coverageArguments.emplace_back(cov::ProgramOptions::InputCoverageValue,
		                               __FILE__);
		ASSERT_EQ(OpenCppCoverage::FailureExitCode,
		          RunCoverageOnProgramWithExitCode(coverageArguments));
	}
}
