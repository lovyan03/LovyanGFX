#ifndef LGFX_PANEL_COMMON_HPP_
#define LGFX_PANEL_COMMON_HPP_

#include <stdint.h>
#include <type_traits>

namespace lgfx
{
  static constexpr uint8_t CMD_INIT_DELAY = 0x80;

  struct PanelCommon
  {
    uint32_t freq_write = 2700000;    // SPI freq (write pixel)
    uint32_t freq_read  = 1600000;    // SPI freq (read pixel )
    uint32_t freq_fill  = 4000000;    // SPI freq (fill pixel )
    int_fast8_t spi_cs    = -1;       // SPI CS pin number
    int_fast8_t spi_dc    = -1;       // SPI dc pin number
    int_fast8_t spi_mode  = 0;        // SPI mode (0~3)
    int_fast8_t spi_mode_read = 0;    // SPI mode (0~3) when read pixel
    int_fast8_t rotation  = 0;        // default rotation (0~3)
    int_fast8_t offset_rotation = 0;  // rotation offset (0~3)
    int_fast8_t gpio_rst  = -1;       // hardware reset pin number
    int_fast8_t gpio_bl   = -1;       // backlight pin number
    int_fast8_t pwm_ch_bl = -1;       // backlight PWM channel number

    bool backlight_level = true;      // turn ON back-light backlight level (true = high / false = low)
    bool spi_read  = true;            // Use SPI read
    bool spi_3wire = false;           // SPI 3wire mode (read from MOSI)
    bool invert    = false;           // panel invert
    bool rgb_order = false;           // true:RGB / false: BGR

    int_fast16_t memory_width  = 240; // width of driver VRAM
    int_fast16_t memory_height = 240; // height of driver VRAM
    int_fast16_t panel_width   = 240; // width of panel
    int_fast16_t panel_height  = 240; // height of panel
    int_fast16_t offset_x      = 0;   // panel offset x axis
    int_fast16_t offset_y      = 0;   // panel offset y axis

    color_depth_t write_depth = rgb565_2Byte;
    color_depth_t read_depth  = rgb888_3Byte;

    uint8_t len_setwindow = 32;
    uint8_t len_dummy_read_pixel = 8;
    uint8_t len_dummy_read_rddid = 1;


    virtual void init(void) {
      if (gpio_rst >= 0) { // RST on
        initGPIO(gpio_rst);
        gpio_lo(gpio_rst);
      }

      if (gpio_bl >= 0) { // Backlight control
        if (pwm_ch_bl < 0) { // nouse PWM

          initGPIO(gpio_bl);

          if (backlight_level) gpio_hi(gpio_bl);
          else                 gpio_lo(gpio_bl);

        } else {  // use PWM

          initPWM(gpio_bl, pwm_ch_bl);

        }
      }

      if (gpio_rst >= 0) { // RST off
        gpio_hi(gpio_rst);
      }

    }

    virtual void setBrightness(uint8_t brightness) {
      if (pwm_ch_bl >= 0) { // PWM
        setPWMDuty(pwm_ch_bl, backlight_level ? brightness : (255 - brightness));
      }
    }

    int_fast16_t getWidth(void) const {
      return ((rotation + offset_rotation) & 1) ? panel_height : panel_width;
    }

    int_fast16_t getHeight(void) const {
      return ((rotation + offset_rotation) & 1) ? panel_width : panel_height;
    }

    int_fast16_t getColStart(void) const {
      switch ((rotation + offset_rotation) & 3) {
      default:  return offset_x;
      case 1:   return offset_y;
      case 2:   return memory_width  - (panel_width  + offset_x);
      case 3:   return memory_height - (panel_height + offset_y);
      }
    }

    int_fast16_t getRowStart(void) const {
      switch (((rotation + offset_rotation) & 3) | (rotation & 4)) {
      default:          return offset_y;
      case 1: case 7:   return memory_width  - (panel_width  + offset_x);
      case 2: case 4:   return memory_height - (panel_height + offset_y);
      case 3: case 5:   return offset_x;
      }
    }

    virtual const uint8_t* getColorDepthCommands(uint8_t* buf, color_depth_t depth) {
      write_depth = getAdjustBpp(depth);
      return buf;
    }

    virtual const uint8_t* getInitCommands(uint8_t listno = 0) const = 0;

    virtual const uint8_t* getInvertDisplayCommands(uint8_t* buf, bool invert) = 0;

    virtual const uint8_t* getRotationCommands(uint8_t* buf, int_fast8_t r) = 0;

    uint8_t getCmdCaset(void) const { return cmd_caset; }
    uint8_t getCmdRaset(void) const { return cmd_raset; }
    uint8_t getCmdRamwr(void) const { return cmd_ramwr; }
    uint8_t getCmdRamrd(void) const { return cmd_ramrd; }
    uint8_t getCmdRddid(void) const { return cmd_rddid; }
    uint8_t getCmdSlpin(void) const { return cmd_slpin; }
    uint8_t getCmdSlpout(void)const { return cmd_slpout; }

    static uint32_t getWindowAddr32(uint_fast16_t H, uint_fast16_t L) { return (((uint8_t)H)<<8 | (H)>>8) | (((L)<<8 | (L)>>8)<<16 ); }
    static uint32_t getWindowAddr16(uint_fast16_t H, uint_fast16_t L) { return H | L<<8; }

  protected:
    virtual color_depth_t getAdjustBpp(color_depth_t bpp) const { return (bpp > 16) ? rgb888_3Byte : rgb565_2Byte; }

    uint8_t cmd_caset = 0;
    uint8_t cmd_raset = 0;
    uint8_t cmd_ramwr = 0;
    uint8_t cmd_ramrd = 0;
    uint8_t cmd_rddid = 0;
    uint8_t cmd_slpin = 0;
    uint8_t cmd_slpout= 0;
  };

//----------------------------------------------------------------------------

}
#endif
