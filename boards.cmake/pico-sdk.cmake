# CMakeLists for PICO SDK

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

pico_sdk_init()

add_compile_definitions(
        USE_PICO_SDK=1
        )

# pico_add_library()
add_library(LovyanGFX_headers INTERFACE)
add_library(LovyanGFX INTERFACE)

target_include_directories(
    LovyanGFX_headers INTERFACE
    src
)

#add_compile_definitions
target_compile_definitions(LovyanGFX INTERFACE
     USE_PICO_SDK
    )

file(GLOB SRCS
     src/lgfx/Fonts/efont/*.c
     src/lgfx/Fonts/IPA/*.c
     src/lgfx/utility/*.c
     src/lgfx/v1/*.cpp
     src/lgfx/v1/misc/*.cpp
     src/lgfx/v1/panel/*.cpp
     src/lgfx/v1/platforms/arduino_default/*.cpp
     src/lgfx/v1/platforms/rp2040/*.cpp
     src/lgfx/v1/touch/*.cpp
    )
target_sources(LovyanGFX INTERFACE ${SRCS})
target_link_libraries(LovyanGFX INTERFACE
     LovyanGFX_headers
     pico_stdlib
     pico_float
     pico_double
     hardware_dma
     hardware_gpio
     hardware_spi
     hardware_i2c
     hardware_pwm
    )

pico_enable_stdio_usb(LovyanGFX 0)
pico_enable_stdio_uart(LovyanGFX 1)
