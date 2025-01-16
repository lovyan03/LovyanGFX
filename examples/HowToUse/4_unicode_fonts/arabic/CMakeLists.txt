cmake_minimum_required(VERSION 3.16.0)
list(APPEND EXTRA_COMPONENT_DIRS "$(PROJECT_PATH)/../../../../../../LovyanGFX")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(LovyanGFX-build-test)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# spiffs_create_partition_image(data flash FLASH_IN_PROJECT)

