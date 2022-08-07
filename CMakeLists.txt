# CMakeLists for ESP-IDF

set(COMPONENT_ADD_INCLUDEDIRS
    src
    )
file(GLOB SRCS
     src/lgfx/Fonts/efont/*.c
     src/lgfx/Fonts/IPA/*.c
     src/lgfx/utility/*.c
     src/lgfx/v0/*.cpp
     src/lgfx/v0/panel/*.cpp
     src/lgfx/v0/platforms/*.cpp
     src/lgfx/v0/touch/*.cpp
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

set(COMPONENT_REQUIRES nvs_flash efuse)

if(IDF_TARGET MATCHES "esp32s3")
  list(APPEND COMPONENT_REQUIRES esp_lcd)
else()
  if (esp_lcd IN_LIST components)
    list(APPEND COMPONENT_REQUIRES esp_lcd)
  endif()
endif()


### If you use arduino-esp32 components, please activate next comment line.
# list(APPEND COMPONENT_REQUIRES arduino-esp32)


message(STATUS "LovyanGFX use components = ${COMPONENT_REQUIRES}")

register_component()
