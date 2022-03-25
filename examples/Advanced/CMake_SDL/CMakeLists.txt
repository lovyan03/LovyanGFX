cmake_minimum_required (VERSION 3.8)
project(LGFXFB)

file(GLOB Target_Files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS 
    *.cpp
    LovyanGFX/src/lgfx/Fonts/efont/*.c
    LovyanGFX/src/lgfx/Fonts/IPA/*.c
    LovyanGFX/src/lgfx/utility/*.c
    LovyanGFX/src/lgfx/v1/*.cpp
    LovyanGFX/src/lgfx/v1/misc/*.cpp
    LovyanGFX/src/lgfx/v1/panel/*.cpp
    LovyanGFX/src/lgfx/v1/platforms/sdl/*.cpp
    )

find_package(SDL2 REQUIRED SDL2)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable (LGFXFB ${Target_Files})
target_include_directories(LGFXFB PUBLIC "LovyanGFX/src/")
target_compile_features(LGFXFB PUBLIC cxx_std_17)
target_link_libraries(LGFXFB -lpthread ${SDL2_LIBRARIES})
