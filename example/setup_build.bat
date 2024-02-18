@echo OFF
setlocal EnableDelayedExpansion

rem Find CMake
where /q cmake.exe
if ERRORLEVEL 1 (
	echo Cannot find cmake.exe! 
	echo Make sure to add [CMake install dir]\bin to PATH
	goto END
) else (
	for /f "tokens=*" %%i in ('where cmake.exe') do set "CMAKE_PATH=%%i"
)

rem Find VS Install with C++ workload installed
if exist "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" (
	for /f "tokens=*" %%i in ('"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -requires Microsoft.VisualStudio.Component.VC.CoreIde -property installationPath') do set "VS_PATH=%%i"
) else (
	rem vswhere not found, guessing Visual Studio install path.
	set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community"
	if not exist !VS_PATH!\* set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community"
)
set "VCVARS64_PATH=%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%VCVARS64_PATH%" (
	echo x64 Native Tools Command Prompt for VS ^(vcvars64.bat^) could not be found!
	echo Make sure Visual Studio ^(2019 or 2022^) including the "Desktop development with C++" workload is installed
	echo https://learn.microsoft.com/en-us/cpp/build/vscpp-step-0-installation?view=msvc-170
	echo ^(DEBUG^) VCVARS64_PATH is "%VCVARS64_PATH%"
	goto END
) else (
	set "VCVARS64_PATH=%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat"
)

echo.
echo Make sure to run this from the main dir of your Projekt (i.e. where CMakeLists.txt) is!
echo Playdate SDK install path: "%PLAYDATE_SDK_PATH%"
echo CMake was found at: "%CMAKE_PATH%"
echo Visual Studio x64 Native Tools Command Prompt path: "%VCVARS64_PATH%"
echo.

call "%VCVARS64_PATH%"
echo.

echo Creating build directories build_sim and build_device...
mkdir build_sim
mkdir build_device
echo Done.
echo.

echo Running CMake setup for simulator build...
echo.
cd build_sim
cmake ..
echo.

echo Running CMake setup for device build...
echo.
cd ..\build_device
cmake .. -G "NMake Makefiles" --toolchain=%PLAYDATE_SDK_PATH%/C_API/buildsupport/arm.cmake
echo.

echo All done!

:END
endlocal
pause