# See how to install sdl2: https://wiki.libsdl.org/SDL2/Installation
cmake_minimum_required (VERSION 3.8)
project(LGFX_SDL)

add_definitions(-DLGFX_SDL)

# ビルド対象にするファイルを指定する;
# LovyanGFXのあるパスと位置関係を変えた場合は相対パス記述を環境に合わせて調整すること;
file(GLOB Target_Files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS 
    *.cpp
    ../../../LovyanGFX/src/lgfx/Fonts/efont/*.c
    ../../../LovyanGFX/src/lgfx/Fonts/IPA/*.c
    ../../../LovyanGFX/src/lgfx/utility/*.c
    ../../../LovyanGFX/src/lgfx/v1/*.cpp
    ../../../LovyanGFX/src/lgfx/v1/misc/*.cpp
    ../../../LovyanGFX/src/lgfx/v1/panel/Panel_Device.cpp
    ../../../LovyanGFX/src/lgfx/v1/panel/Panel_FrameBufferBase.cpp
    ../../../LovyanGFX/src/lgfx/v1/platforms/sdl/*.cpp
    )
add_executable (LGFX_SDL ${Target_Files})

# includeするファイルのパスを指定する;
# LovyanGFXのあるパスと位置関係を変えた場合は相対パス記述を環境に合わせて調整すること;
target_include_directories(LGFX_SDL PUBLIC "../../../LovyanGFX/src/")

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")

# Linux環境の場合;
# SDL2_INCLUDE_DIRS と SDL2_LIBRARIES を事前に設定しておく;
    find_package(SDL2 REQUIRED SDL2)
    include_directories(${SDL2_INCLUDE_DIRS})
    target_link_libraries(LGFX_SDL -lpthread ${SDL2_LIBRARIES})

ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")

# Windows環境の場合;
# SDL2を配置したパス内のincludeのパスを指定する;
    target_include_directories(LGFX_SDL PUBLIC "C:/SDL2/include")

# SDL2を配置したパス内の.libファイルのパスを指定する;
    target_link_libraries(LGFX_SDL PUBLIC "C:/SDL2/lib/x64/SDL2.lib")


    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>")

ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")

# MacOS(Darwin)環境の場合;
    find_package(SDL2 REQUIRED COMPONENTS SDL2)
    target_link_libraries(LGFX_SDL PRIVATE SDL2::SDL2)

ENDIF ()

target_compile_features(LGFX_SDL PUBLIC cxx_std_17)
