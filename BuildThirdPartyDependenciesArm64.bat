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
REM
REM vcpkg-overlays\ctemplate overrides the upstream ctemplate port: its
REM src/base/macros.h only ever defines UNALIGNED_LOAD32 for x86/x64 (or via
REM POSIX endian.h, which doesn't exist on Windows), so it hard #errors on
REM MSVC/ARM64. The overlay applies a small patch adding an ARM64/ARM64EC
REM branch (unaligned little-endian loads are safe there too) and corrects
REM the port's stale vcpkg.json "supports": "windows & !arm" declaration.

SET ROOT_FOLDER=%~dp0/Build/ThirdPartyArm64/
SET VCPKG_OVERLAY_PORTS=%~dp0vcpkg-overlays

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

REM ctemplate's vcpkg.json (upstream) declares "supports": "windows & !arm"
REM and its source has no genuine ARM64 branch in UNALIGNED_LOAD32 either;
REM vcpkg-overlays\ctemplate (see VCPKG_OVERLAY_PORTS above) fixes both.
.\vcpkg install ctemplate:arm64-windows
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
	--nuget --nuget-id=ThirdParty --nuget-version=1.4.0

REM Current vcpkg packs the --nuget export directly (no separate
REM downloads\tools\nuget.exe helper is fetched anymore, unlike the pinned
REM vcpkg commit BuildThirdPartyDependencies.bat still relies on), so fetch
REM nuget.exe the same way InstallThirdPartyLibraries.ps1 does for the
REM prebuilt x64/x86 package, then install our freshly built
REM ThirdParty.1.4.0.nupkg from its own output directory (which doubles as
REM a valid local NuGet feed) into packages\.
powershell -NoProfile -Command "Invoke-WebRequest -OutFile nuget.exe https://dist.nuget.org/win-x86-commandline/latest/nuget.exe"
.\nuget.exe install ThirdParty -Source %ROOT_FOLDER%\vcpkg -OutputDirectory ..\..\..\packages
