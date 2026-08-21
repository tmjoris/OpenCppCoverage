@echo off
setlocal

REM Builds the arm64-windows third-party dependencies via vcpkg and packages
REM them the same way BuildThirdPartyDependencies.bat does for x64-windows /
REM x86-windows, so the resulting "packages\thirdparty.1.4.0\installed\..."
REM layout is picked up unmodified by every project's existing
REM thirdparty.targets Import and by PropertySheets\GTest-*-ARM64.props /
REM DiaGuids-ARM64.props.
REM
REM Unlike BuildThirdPartyDependencies.bat, this script clones the latest
REM vcpkg (no pinned commit): the ARM64 port is new to this project and has
REM no previously-working baseline to preserve, and a current vcpkg gives
REM the best chance of up-to-date arm64-windows port support and live
REM download mirrors for all dependencies.
REM
REM protobuf:x86-windows is also built: Exporter.vcxproj always invokes an
REM x86-windows-built protoc.exe as a host code-generation tool, even when
REM building other target platforms (x86 emulation is available on Windows
REM ARM64), so protoc.exe must exist even though the ARM64 binaries link
REM against protobuf:arm64-windows.

SET ROOT_FOLDER=%~dp0/Build/ThirdPartyArm64/

IF EXIST "%ROOT_FOLDER%" GOTO THIRD_PARTY_EXISTS
mkdir "%ROOT_FOLDER%"
:THIRD_PARTY_EXISTS

cd Build/ThirdPartyArm64

IF EXIST vcpkg GOTO REPO_EXISTS
git clone https://github.com/Microsoft/vcpkg.git
:REPO_EXISTS

cd vcpkg
git fetch
git checkout master
git pull

IF EXIST vcpkg.exe GOTO VCPKG_EXISTS
	call .\bootstrap-vcpkg.bat
:VCPKG_EXISTS

.\vcpkg install poco:arm64-windows
.\vcpkg install protobuf:arm64-windows protobuf:x86-windows
.\vcpkg install gtest:arm64-windows

REM ctemplate's vcpkg.json declares "supports": "windows & !arm" (dating
REM back to 2020, before this project's Windows ARM64 support matured).
REM Its CMakeLists.txt/sources have no architecture-specific code, so
REM --allow-unsupported is used to force the build past that stale
REM restriction rather than replacing the library.
.\vcpkg install ctemplate:arm64-windows --allow-unsupported
.\vcpkg install boost-optional:arm64-windows
.\vcpkg install boost-filesystem:arm64-windows
.\vcpkg install boost-algorithm:arm64-windows
.\vcpkg install boost-container:arm64-windows
.\vcpkg install boost-program-options:arm64-windows
.\vcpkg install boost-regex:arm64-windows
.\vcpkg install boost-range:arm64-windows
.\vcpkg install boost-log:arm64-windows
.\vcpkg install boost-property-tree:arm64-windows
.\vcpkg install boost-spirit:arm64-windows
.\vcpkg install boost-uuid:arm64-windows
.\vcpkg install boost-locale:arm64-windows
.\vcpkg install boost-iostreams:arm64-windows

.\vcpkg export ^
	poco:arm64-windows ^
	protobuf:arm64-windows protobuf:x86-windows ^
	gtest:arm64-windows ^
	ctemplate:arm64-windows ^
	boost-optional:arm64-windows ^
	boost-filesystem:arm64-windows ^
	boost-algorithm:arm64-windows ^
	boost-container:arm64-windows ^
	boost-program-options:arm64-windows ^
	boost-regex:arm64-windows ^
	boost-range:arm64-windows ^
	boost-log:arm64-windows ^
	boost-property-tree:arm64-windows ^
	boost-spirit:arm64-windows ^
	boost-uuid:arm64-windows ^
	boost-locale:arm64-windows ^
	boost-iostreams:arm64-windows ^
	--nuget --nuget-id=ThirdParty --nuget-version=1.4.0 --allow-unsupported

REM The vcpkg-bundled nuget.exe tool version is not pinned by this script
REM (unlike BuildThirdPartyDependencies.bat, this uses a rolling vcpkg
REM checkout), so locate whatever version was actually downloaded instead of
REM hardcoding its folder name.
for /f "delims=" %%F in ('dir /s /b downloads\tools\nuget.exe') do set NUGET_EXE=%%F
"%NUGET_EXE%" install ThirdParty -Source %ROOT_FOLDER%\vcpkg -OutputDirectory ..\..\..\packages
