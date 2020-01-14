#ifndef LGFX_TFT_ESPI_HPP_
#define LGFX_TFT_ESPI_HPP_

#include <LovyanGFX.hpp>

#define TFT_BLACK       0x0000      /*   0,   0,   0 */
#define TFT_NAVY        0x000F      /*   0,   0, 128 */
#define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define TFT_MAROON      0x7800      /* 128,   0,   0 */
#define TFT_PURPLE      0x780F      /* 128,   0, 128 */
#define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
#define TFT_LIGHTGREY   0xC618      /* 192, 192, 192 */
#define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define TFT_BLUE        0x001F      /*   0,   0, 255 */
#define TFT_GREEN       0x07E0      /*   0, 255,   0 */
#define TFT_CYAN        0x07FF      /*   0, 255, 255 */
#define TFT_RED         0xF800      /* 255,   0,   0 */
#define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
#define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
#define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
#define TFT_ORANGE      0xFDA0      /* 255, 180,   0 */
#define TFT_GREENYELLOW 0xB7E0      /* 180, 255,   0 */
#define TFT_PINK        0xFC9F

namespace lgfx {

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE) // M5Stack

  struct LGFX_Config {
    static const Panel_M5Stack panel;
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int dma_ch   = 1;
    static constexpr int spi_mosi = 23;
    static constexpr int spi_miso = 19;
    static constexpr int spi_sclk = 18;
    static constexpr int panel_rst = 33;
  };

#elif defined(ARDUINO_M5Stick_C) // M5Stick C

  struct LGFX_Config {
    static const Panel_M5StickC panel;
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int spi_mosi = 15;
    static constexpr int spi_miso = 14;
    static constexpr int spi_sclk = 13;
    static constexpr int panel_rst = 18;
  };

#elif defined(ARDUINO_ODROID_ESP32) // ODROID-GO

  struct LGFX_Config {
    static const Panel_ODROID_GO panel;
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int spi_mosi = 23;
    static constexpr int spi_miso = 19;
    static constexpr int spi_sclk = 18;
    static constexpr int panel_rst = -1;
    static constexpr int panel_bl  = 14;
  };

#elif defined(ARDUINO_T) // TTGO T-Watch

  struct LGFX_Config {
    static const Panel_TTGO_TWatch panel;
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int spi_mosi = 19;
    static constexpr int spi_miso = -1;
    static constexpr int spi_sclk = 18;
    static constexpr int panel_rst = -1;
    static constexpr int panel_bl  = 12;
    static lgfx::Panel_ST7789_240x240 panel;
  };

#elif defined ( ARDUINO_ESP32_DEV ) // ESP-WROVER-KIT

  struct LGFX_Config {
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr bool spi_half_duplex = false;
    static constexpr int spi_mosi = 23;
    static constexpr int spi_miso = 25;
    static constexpr int spi_sclk = 19;
    static constexpr int spi_cs   = 22;
    static constexpr int spi_dc   = 21;
    static constexpr int panel_rst = 18;
    static constexpr int panel_bl  = 5;
    static constexpr int freq_write = 40000000;
    static constexpr int freq_read  = 20000000;
    static constexpr int freq_fill  = 40000000;
    static lgfx::Panel_ILI9341_240x320 panel;
  };

#endif

}

class TFT_eSPI : public lgfx::LGFX_SPI<lgfx::LGFX_Config> {};
class TFT_eSprite : public lgfx::LGFXSprite {};

#endif
