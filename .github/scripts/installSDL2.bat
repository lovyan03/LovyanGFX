::@echo off
setlocal enabledelayedexpansion
::============================================::
set sdlVersion=2.0.22
echo Installing SDL2-%sdlVersion%
::============================================::
::set count=1
::============================================::

mkdir contrib

rem  echo ============================================
rem  echo Downloading SDL2 for VisualC++
rem  echo ============================================
rem
rem  curl https://libsdl.org/release/SDL2-devel-%sdlVersion%-VC.zip -o contrib/SDL2-VC.zip
rem  7z x contrib/SDL2-VC.zip -ocontrib/MSVC/
rem  cp ".github/scripts/sdl2-config-vc.cmake" contrib/MSVC/SDL2-%sdlVersion%/sdl2-config.cmake
rem
rem  echo ============================================
rem  echo Downloading SDL2 for mingw
rem  echo ============================================
rem
rem  curl https://libsdl.org/release/SDL2-devel-%sdlVersion%-mingw.tar.gz -o contrib/SDL2-mingw.tar.gz
rem  7z x contrib/SDL2-mingw.tar.gz -ocontrib/
rem  7z x contrib/SDL2-mingw.tar -ocontrib/mingw/
rem  cp ".github/scripts/sdl2-config-mingw.cmake" contrib/mingw/SDL2-%sdlVersion%/sdl2-config.cmake
rem
rem  ls contrib/src/
rem  ls contrib/MSVC/
rem  ls contrib/mingw/

echo ============================================
echo Downloading SDL2 Source
echo ============================================

curl https://www.libsdl.org/release/SDL2-%sdlVersion%.zip -o contrib/SDL2-SRC.zip
7z x contrib/SDL2-SRC.zip -ocontrib/src/


cd contrib/src/SDL2-%sdlVersion%

set cwd=%cd%

echo ============================================
echo Building Debug version
echo ============================================

:: build debug version
cmake -S . -B build/debug -G "Visual Studio 17 2022" -T host=x64 -DCMAKE_INSTALL_PREFIX=%cwd%/install -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug --target install

echo ============================================
echo Building Release version
echo ============================================

:: build release verion
cmake -S . -B build/release -G "Visual Studio 17 2022" -T host=x64 -DCMAKE_INSTALL_PREFIX=%cwd%/install -DCMAKE_BUILD_TYPE=Release
cmake --build build/release --target install

