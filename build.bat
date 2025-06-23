@echo off
REM Build script for NotationChordHelper VST3 Plugin (Windows)

echo Building NotationChordHelper VST3 Plugin...

REM Try to find and set up Visual Studio environment
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
) else (
    echo Error: Visual Studio 2022 not found. Please install Visual Studio 2022 with C++ development tools.
    exit /b 1
)

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo Configuring with CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release

REM Build the project
echo Building project...
cmake --build . --config Release

REM Check if the VST3 file was created
if exist "VST3\Release\NotationChordHelper.vst3" (
    echo ✅ Build successful! VST3 file created:
    echo    %CD%\VST3\Release\NotationChordHelper.vst3
) else (
    echo ❌ Build failed! VST3 file not found.
    exit /b 1
)

echo Build completed successfully! 