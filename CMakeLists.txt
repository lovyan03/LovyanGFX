# CMakeLists for ESP-IDF

set(COMPONENT_ADD_INCLUDEDIRS
    src
    )
file(GLOB SRCS
     src/lgfx/Fonts/efont/*.c
     src/lgfx/Fonts/IPA/*.c
     src/lgfx/utility/*.c
     src/lgfx/v1/*.cpp
     src/lgfx/v1/misc/*.cpp
     src/lgfx/v1/panel/*.cpp
     src/lgfx/v1/platforms/arduino_default/*.cpp
     src/lgfx/v1/platforms/esp32/*.cpp
     src/lgfx/v1/platforms/esp32c3/*.cpp
     src/lgfx/v1/platforms/esp32s2/*.cpp
     src/lgfx/v1/platforms/esp32s3/*.cpp
     src/lgfx/v1/touch/*.cpp
     )

set(COMPONENT_SRCS ${SRCS})

if (IDF_VERSION_MAJOR GREATER_EQUAL 5)
    set(COMPONENT_REQUIRES nvs_flash efuse esp_lcd driver esp_timer)
elseif ((IDF_VERSION_MAJOR EQUAL 4) AND (IDF_VERSION_MINOR GREATER 3) OR IDF_VERSION_MAJOR GREATER 4)
    set(COMPONENT_REQUIRES nvs_flash efuse esp_lcd)
else()
    set(COMPONENT_REQUIRES nvs_flash efuse)
endif()


### If you use arduino-esp32 components, please activate next comment line.
# list(APPEND COMPONENT_REQUIRES arduino-esp32)


message(STATUS "LovyanGFX use components = ${COMPONENT_REQUIRES}")

register_component()
