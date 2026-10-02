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

# Functional hubs share header parsing; large implementations and data tables stay
# in separate translation units so unused groups can be discarded by the linker.
set(SRCS
     src/lgfx/v1/lgfx_v1.cpp
     src/lgfx/v1/lgfx_v1_panel.cpp
     src/lgfx/v1/lgfx_v1_touch.cpp
     src/lgfx/v1/lgfx_v1_platforms.cpp
     src/lgfx/v1/LGFXBase.cpp
     src/lgfx/v1/lgfx_fonts.cpp
     src/lgfx/v1/panel/Panel_M5HDMI.cpp
     src/lgfx/v1/touch/Touch_GSLx680.cpp
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
