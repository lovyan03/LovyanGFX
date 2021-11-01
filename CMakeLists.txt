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
     src/lgfx/v1/platforms/esp8266/*.cpp
     src/lgfx/v1/platforms/samd21/*.cpp
     src/lgfx/v1/platforms/samd51/*.cpp
     src/lgfx/v1/platforms/stm32/*.cpp
     src/lgfx/v1/touch/*.cpp
     )
set(COMPONENT_SRCS ${SRCS})
set(COMPONENT_REQUIRES nvs_flash efuse)

register_component()
