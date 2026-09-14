# CMakeLists for ESP-IDF

set(COMPONENT_ADD_INCLUDEDIRS
    ${LGFX_ROOT}/src
    )
file(GLOB SRCS
     ${LGFX_ROOT}/src/lgfx/Fonts/efont/*.c
     ${LGFX_ROOT}/src/lgfx/Fonts/IPA/*.c
     ${LGFX_ROOT}/src/lgfx/Fonts/lvgl/*.c
     ${LGFX_ROOT}/src/lgfx/utility/*.c
     ${LGFX_ROOT}/src/lgfx/v1/*.cpp
     ${LGFX_ROOT}/src/lgfx/v1/lv_font/*.c
     ${LGFX_ROOT}/src/lgfx/v1/misc/*.cpp
     ${LGFX_ROOT}/src/lgfx/v1/panel/*.cpp
     ${LGFX_ROOT}/src/lgfx/v1/platforms/arduino_default/*.cpp
     ${LGFX_ROOT}/src/lgfx/v1/platforms/esp32/*.cpp
     ${LGFX_ROOT}/src/lgfx/v1/platforms/esp32c3/*.cpp
     ${LGFX_ROOT}/src/lgfx/v1/platforms/esp32s2/*.cpp
     ${LGFX_ROOT}/src/lgfx/v1/platforms/esp32s3/*.cpp
     ${LGFX_ROOT}/src/lgfx/v1/platforms/esp32p4/*.cpp
     ${LGFX_ROOT}/src/lgfx/v1/touch/*.cpp
     )

set(COMPONENT_SRCS ${SRCS})

if(IDF_VERSION_MAJOR GREATER_EQUAL 6)
    set(COMPONENT_REQUIRES nvs_flash efuse esp_lcd driver esp_timer esp_mm esp_driver_ledc esp_driver_i2s hal)
elseif (IDF_VERSION_MAJOR GREATER_EQUAL 5)
    if(IDF_VERSION_MINOR GREATER_EQUAL 1)
        set(COMPONENT_REQUIRES nvs_flash efuse esp_lcd driver esp_timer esp_mm)
    else()
        set(COMPONENT_REQUIRES nvs_flash efuse esp_lcd driver esp_timer)
    endif()
elseif ((IDF_VERSION_MAJOR EQUAL 4) AND (IDF_VERSION_MINOR GREATER 3) OR IDF_VERSION_MAJOR GREATER 4)
    set(COMPONENT_REQUIRES nvs_flash efuse esp_lcd)
else()
    set(COMPONENT_REQUIRES nvs_flash efuse)
endif()




message(STATUS "LovyanGFX use components = ${COMPONENT_REQUIRES}")

register_component()

# Arduino as an ESP-IDF component: arduino-esp32 publishes -DARDUINO... as PUBLIC compile options, so
# they only reach components that link against it, while the public headers of this library change
# class layout with ARDUINO. Link the Arduino component publicly whenever it is part of the build so
# this library is compiled in the same mode as the application. Only components already selected
# for the build are considered; a build without Arduino is unaffected.
#   LGFX_ARDUINO_COMPONENT=<name>  use a differently named Arduino component
#   LGFX_ARDUINO_COMPONENT=OFF     disable the automatic dependency
set(_lgfx_arduino_candidates arduino arduino-esp32 espressif__arduino-esp32)
set(_lgfx_arduino_enabled ON)
if(DEFINED LGFX_ARDUINO_COMPONENT AND NOT "${LGFX_ARDUINO_COMPONENT}" STREQUAL "")
    if(LGFX_ARDUINO_COMPONENT)
        set(_lgfx_arduino_candidates ${LGFX_ARDUINO_COMPONENT})
    else()
        set(_lgfx_arduino_enabled OFF)
    endif()
endif()
if(_lgfx_arduino_enabled)
    idf_build_get_property(_lgfx_arduino_build_components BUILD_COMPONENTS)
    set(_lgfx_arduino_hits)
    foreach(_lgfx_arduino_name ${_lgfx_arduino_candidates})
        if(_lgfx_arduino_name IN_LIST _lgfx_arduino_build_components)
            list(APPEND _lgfx_arduino_hits ${_lgfx_arduino_name})
        endif()
    endforeach()
    list(LENGTH _lgfx_arduino_hits _lgfx_arduino_count)
    if(_lgfx_arduino_count GREATER 1)
        message(FATAL_ERROR "LGFX: several Arduino components are in the build (${_lgfx_arduino_hits}). Set LGFX_ARDUINO_COMPONENT to the one to use.")
    elseif(_lgfx_arduino_count EQUAL 1)
        idf_component_get_property(_lgfx_arduino_lib ${_lgfx_arduino_hits} COMPONENT_LIB)
        target_link_libraries(${COMPONENT_LIB} PUBLIC ${_lgfx_arduino_lib})
    endif()
endif()
