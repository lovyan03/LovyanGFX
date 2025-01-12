#
# Usage:
#
# 1. Copy this directory outside "LovyanGFX" directory
#
# 2. Set variables below (in environment variables or CMakeLists.txt)
#    PICO_SDK_PATH
#    PICO_PLATFORM (rp2040,rp2350-arm-s)
#    PICO_BOARD (optional, "pico", "pico2",...)
#
# 3. Copy "LovyanGFX" directory to this directory)
#
# 4. mkdir and cd build directory
#
# 5. run cmake
#

set (PROJ_NAME i2c_test)

cmake_minimum_required(VERSION 3.13)

if (NOT (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/LovyanGFX))
    message(FATAL_ERROR "Please copy \"LovyanGFX\" directory to \"${CMAKE_CURRENT_SOURCE_DIR}\".")
endif()


include(../pico_sdk_import.cmake)
include(../pico_extras_import_optional.cmake)

project(${PROJ_NAME}
    C
    CXX
    ASM
)

set(CMAKE_C_STNDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_subdirectory(
    LovyanGFX
)

add_executable(${PROJ_NAME}
  main.cpp
)

target_include_directories(${PROJ_NAME} PRIVATE
    .
)

pico_enable_stdio_usb(${PROJ_NAME} 0)
pico_enable_stdio_uart(${PROJ_NAME} 1)

pico_add_extra_outputs(${PROJ_NAME})

target_link_libraries(${PROJ_NAME}
    pico_stdlib
    LovyanGFX
)
