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
if(IDF_TARGET MATCHES "esp32s3")
  set(COMPONENT_REQUIRES nvs_flash efuse esp_lcd)
# message(STATUS "LGFX TARGET is esp32s3")
else()
  find_package(LCD COMPONENTS esp_lcd QUIET)
  if (NOT LCD_FOUND)
    set(COMPONENT_REQUIRES nvs_flash efuse)
# message(STATUS "LGFX TARGET is NOT esp32s3")
  else()
    set(COMPONENT_REQUIRES nvs_flash efuse esp_lcd)
# message(STATUS "LGFX TARGET is NOT esp32s3 but has esp_lcd")
  endif()
endif()

register_component()
