cmake_minimum_required (VERSION 3.8)
project(LGFXOpenCV)

# Use this if OpenCV_DIR was not added to the PATH
# set( OpenCV_DIR    "C:/path/to/opencv/build" )

find_package(OpenCV REQUIRED )

# Uncomment this if LovyanGFX_DIR was not added to the PATH
set( LovyanGFX_DIR "../../../LovyanGFX/src" )

message(STATUS "LGFXOpenCV status:")
message(STATUS "    LovyanGFX dir       = ${LovyanGFX_DIR}")
message(STATUS "    OpenCV config       = ${OpenCV_DIR}")
message(STATUS "    OpenCV version      = ${OpenCV_VERSION}")
message(STATUS "    OpenCV libraries    = ${OpenCV_LIBS}")
message(STATUS "    OpenCV include path = ${OpenCV_INCLUDE_DIRS}")

add_definitions(-DLGFX_OPENCV)

file(GLOB Target_Files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS
    *.cpp
    ${LovyanGFX_DIR}/lgfx/Fonts/efont/*.c
    ${LovyanGFX_DIR}/lgfx/Fonts/IPA/*.c
    ${LovyanGFX_DIR}/lgfx/utility/*.c
    ${LovyanGFX_DIR}/lgfx/v1/*.cpp
    ${LovyanGFX_DIR}/lgfx/v1/misc/*.cpp
    ${LovyanGFX_DIR}/lgfx/v1/panel/Panel_Device.cpp
    ${LovyanGFX_DIR}/lgfx/v1/platforms/opencv/*.cpp
    )
add_executable (LGFXOpenCV ${Target_Files})

if(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
endif()

target_include_directories(LGFXOpenCV PUBLIC ${LovyanGFX_DIR} ${OpenCV_INCLUDE_DIRS})
target_link_libraries(LGFXOpenCV PUBLIC ${OpenCV_LIBS})
target_compile_features(LGFXOpenCV PRIVATE cxx_std_17)

