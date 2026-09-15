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

# The build set is fixed: src/lgfx/v1/lgfx_v1.cpp includes every implementation file (*.inl),
# the font tables and the C decoders are separate translation units on purpose.
set(SRCS
     src/lgfx/v1/lgfx_v1.cpp
     src/lgfx/Fonts/efont/lgfx_efont_cn.c
     src/lgfx/Fonts/efont/lgfx_efont_ja.c
     src/lgfx/Fonts/efont/lgfx_efont_kr.c
     src/lgfx/Fonts/efont/lgfx_efont_tw.c
     src/lgfx/Fonts/IPA/lgfx_font_japan.c
     src/lgfx/utility/lgfx_miniz.c
     src/lgfx/utility/lgfx_pngle.c
     src/lgfx/utility/lgfx_qoi.c
     src/lgfx/utility/lgfx_qrcode.c
     src/lgfx/utility/lgfx_tjpgd.c
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
