cmake_minimum_required (VERSION 3.8)
project(LGFXOpenCV)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>")

file(GLOB Target_Files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS 
    *.cpp
    ../../../../LovyanGFX/src/lgfx/Fonts/efont/*.c
    ../../../../LovyanGFX/src/lgfx/Fonts/IPA/*.c
    ../../../../LovyanGFX/src/lgfx/utility/*.c
    ../../../../LovyanGFX/src/lgfx/v1/*.cpp
    ../../../../LovyanGFX/src/lgfx/v1/misc/*.cpp
    ../../../../LovyanGFX/src/lgfx/v1/panel/*.cpp
    ../../../../LovyanGFX/src/lgfx/v1/platforms/opencv/*.cpp
    )
add_executable (LGFXOpenCV ${Target_Files})

target_compile_features(LGFXOpenCV PUBLIC cxx_std_17)
target_include_directories(LGFXOpenCV PUBLIC "../../../../LovyanGFX/src/")
target_include_directories(LGFXOpenCV PUBLIC "C:/opencv/build/include")
target_link_libraries(LGFXOpenCV PUBLIC "C:/opencv/build/x64/vc15/lib/opencv_world452d.lib" )
