cmake_minimum_required(VERSION 3.12)
project (lv_wasm)

add_definitions(-DLGFX_SDL)
set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -g -s USE_SDL=2")

include_directories(../../../LovyanGFX/src/)

file(GLOB MY_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS 
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
set(SOURCES ${MY_SOURCES})

add_executable(index ${SOURCES} ${INCLUDES})

set(CMAKE_EXECUTABLE_SUFFIX ".html")
target_link_libraries(index -lembind)
set_target_properties(index PROPERTIES LINK_FLAGS "--shell-file ${PROJECT_SOURCE_DIR}/lgfx_wasm.html -s SINGLE_FILE=1")
