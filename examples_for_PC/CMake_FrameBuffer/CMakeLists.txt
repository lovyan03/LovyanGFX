cmake_minimum_required (VERSION 3.8)
project(LGFX_LINUX_FB)

add_definitions(-DLGFX_LINUX_FB)

file(GLOB Target_Files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS 
    *.cpp
    LovyanGFX/src/lgfx/Fonts/efont/*.c
    LovyanGFX/src/lgfx/Fonts/IPA/*.c
    LovyanGFX/src/lgfx/utility/*.c
    LovyanGFX/src/lgfx/v1/*.cpp
    LovyanGFX/src/lgfx/v1/misc/*.cpp
    LovyanGFX/src/lgfx/v1/panel/Panel_Device.cpp
    LovyanGFX/src/lgfx/v1/platforms/framebuffer/*.cpp
    )

add_executable (LGFX_LINUX_FB ${Target_Files})
target_include_directories(LGFX_LINUX_FB PUBLIC "LovyanGFX/src/")
target_compile_features(LGFX_LINUX_FB PUBLIC cxx_std_17)
target_link_libraries(LGFX_LINUX_FB -lpthread)
