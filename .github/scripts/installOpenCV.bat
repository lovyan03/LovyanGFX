::@echo off
setlocal enabledelayedexpansion
::DO_NOT_CHANGE::
:: ============================================::
set cvVersion=4.5.5
echo Installing OpenCV-%cvVersion%
:: ============================================::
mkdir opencv-%cvVersion%
cd opencv-%cvVersion%
mkdir Installation
set count=1
set cwd=%cd%
:: ============================================::
echo Downloading opencv from github
:: download latest opencv from git
git clone https://github.com/opencv/opencv.git -b %cvVersion% --depth=1
cd opencv
:: checkout appropriate cv version
:: git checkout %cvVersion%
:: free some space on the disk
rmdir -Force -Recurse .git

:: ============================================::
echo Compiling using cmake

set cwd=%cd%

echo ============================================
echo Building Debug version
echo ============================================

mkdir build
cd build
:: xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx::
:: Configure OpenCV
cmake -G "Visual Studio 17 2022" -T host=x64 -DCMAKE_INSTALL_PREFIX=%cwd%/Installation -D BUILD_opencv_java=OFF -D BUILD_opencv_python=OFF -D BUILD_EXAMPLES=OFF -D INSTALL_C_EXAMPLES=OFF -D INSTALL_PYTHON_EXAMPLES=OFF ..
::DO_NOT_CHANGE::
::============================================::
:: Compile OpenCV in release mode
cmake.exe --build . --config Debug --target INSTALL
cd ..
cd ..
cd ..
