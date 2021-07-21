#ifndef LGFX_PANEL_COMMON_HPP_
#define LGFX_PANEL_COMMON_HPP_

#include <stdint.h>
#include <type_traits>
#include "../lgfx_common.hpp"

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  class LGFX_Device;
  class LGFX_Sprite;

  static constexpr uint8_t CMD_INIT_DELAY = 0x80;

  struct PanelCommon
  {
    virtual ~PanelCommon() = default;
    uint32_t freq_write = 2700000;    // SPI freq (write pixel)
    uint32_t freq_read  = 1600000;    // SPI freq (read pixel )
    uint32_t freq_fill  = 4000000;    // SPI freq (fill pixel )
    int_fast16_t spi_cs    = -1;      // SPI CS pin number
    int_fast16_t spi_dc    = -1;      // SPI dc pin number
    int_fast16_t gpio_rst  = -1;      // hardware reset pin number
    int_fast16_t gpio_bl   = -1;      // backlight pin number
    int_fast16_t gpio_busy = -1;      // busy check pin number
    int_fast8_t spi_mode  = 0;        // SPI mode (0~3)
    int_fast8_t spi_mode_read = 0;    // SPI mode (0~3) when read pixel
    int_fast8_t spi_dlen  = 0;        // SPI transfer length (8 or 16)
    int_fast8_t rotation  = 0;        // default rotation (0~3)
    int_fast8_t offset_rotation = 0;  // rotation offset (0~3)
    int_fast8_t pwm_ch_bl = -1;       // backlight PWM channel number
    uint_fast16_t pwm_freq = 12000;   // backlight PWM freq Hz
    bool reverse_invert = false;           // Reverse the effect of the invert command.

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

    uint_fast8_t brightness    = 127;

//
    color_depth_t write_depth = rgb565_2Byte;
    color_depth_t read_depth  = rgb888_3Byte;

    uint8_t len_setwindow = 32;
    uint8_t len_dummy_read_pixel = 8;
    uint8_t len_dummy_read_rddid = 1;

    virtual void init(bool use_reset)
    {
      if (gpio_rst >= 0) { // RST on
        gpio_hi(gpio_rst);
        lgfxPinMode(gpio_rst, pin_mode_t::output);
        if (use_reset)
        {
          gpio_lo(gpio_rst);
          delay(1);
        }
      }

      if (gpio_bl >= 0) { // Backlight control
        if (pwm_ch_bl < 0) { // nouse PWM

          lgfxPinMode(gpio_bl, pin_mode_t::output);

          if (backlight_level) gpio_hi(gpio_bl);
          else                 gpio_lo(gpio_bl);

        } else {  // use PWM

          initPWM(gpio_bl, pwm_ch_bl, pwm_freq);

        }
      }

      if (gpio_rst >= 0) { // RST off
        gpio_hi(gpio_rst);
      }
    }

    virtual void setBrightness(uint8_t brightness)
    {
      this->brightness = brightness;
      if (pwm_ch_bl >= 0) { // PWM
        setPWMDuty(pwm_ch_bl, backlight_level ? brightness : (255 - brightness));
      }
    }

    virtual void sleep(LGFX_Device*)
    {
      if (pwm_ch_bl >= 0) { // PWM
        setPWMDuty(pwm_ch_bl, backlight_level ? 0 : 255);
      }
    }

    virtual void wakeup(LGFX_Device*)
    {
      if (pwm_ch_bl >= 0) { // PWM
        setPWMDuty(pwm_ch_bl, backlight_level ? brightness : (255 - brightness));
      }
    }

    int_fast16_t getWidth(void) const {
      return (_internal_rotation & 1) ? panel_height : panel_width;
    }

    int_fast16_t getHeight(void) const {
      return (_internal_rotation & 1) ? panel_width : panel_height;
    }

    int_fast16_t getColStart(void) const {
      switch (_internal_rotation & 3) {
      default:  return offset_x;
      case 1:   return offset_y;
      case 2:   return memory_width  - (panel_width  + offset_x);
      case 3:   return memory_height - (panel_height + offset_y);
      }
    }

    int_fast16_t getRowStart(void) const {
      switch ((_internal_rotation & 3) | (_internal_rotation & 4)) {
      default:          return offset_y;
      case 1: case 7:   return memory_width  - (panel_width  + offset_x);
      case 2: case 4:   return memory_height - (panel_height + offset_y);
      case 3: case 5:   return offset_x;
      }
    }

    void (*fp_begin)(PanelCommon*, LGFX_Device*) = nullptr;
    void (*fp_end)(PanelCommon*, LGFX_Device*) = nullptr;
    void (*fp_display)(PanelCommon*, LGFX_Device*, int32_t x, int32_t y, int32_t w, int32_t h) = nullptr;
    void (*fp_waitDisplay)(PanelCommon*, LGFX_Device*) = nullptr;
    bool (*fp_displayBusy)(PanelCommon*, LGFX_Device*) = nullptr;
    void (*fp_pushImage)(PanelCommon*, LGFX_Device*, int32_t x, int32_t y, int32_t w, int32_t h, pixelcopy_t* param) = nullptr;
    void (*fp_fillRect)(PanelCommon*, LGFX_Device*, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t rawcolor) = nullptr;
    void (*fp_pushBlock)(PanelCommon*, LGFX_Device*, int32_t len, uint32_t rawcolor) = nullptr;
    void (*fp_writePixels)(PanelCommon*, LGFX_Device*, int32_t len, pixelcopy_t* param) = nullptr;
    void (*fp_readRect)(PanelCommon*, LGFX_Device*, int32_t x, int32_t y, int32_t w, int32_t h, void* dst, pixelcopy_t* param) = nullptr;

    virtual void post_init(LGFX_Device*, bool use_reset) { (void)use_reset; }

    virtual const uint8_t* getColorDepthCommands(uint8_t* buf, color_depth_t depth) {
      write_depth = getAdjustBpp(depth);
      return buf;
    }

    virtual bool isEPD(void) const { return false; }
    virtual const uint8_t* getInitCommands(uint8_t listno = 0) const { (void)listno; return nullptr; }
    virtual const uint8_t* getWindowCommands1(uint8_t* buf, uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye) = 0;
    virtual const uint8_t* getWindowCommands2(uint8_t* buf, uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye) = 0;
    virtual const uint8_t* getSleepInCommands(uint8_t* buf) = 0;
    virtual const uint8_t* getSleepOutCommands(uint8_t* buf) = 0;
    virtual const uint8_t* getPowerSaveOnCommands(uint8_t* buf) { (void)buf; return nullptr; }
    virtual const uint8_t* getPowerSaveOffCommands(uint8_t* buf) { (void)buf; return nullptr; }
    virtual const uint8_t* getInvertDisplayCommands(uint8_t* buf, bool invert) = 0;
    virtual const uint8_t* getRotationCommands(uint8_t* buf, int_fast8_t r)
    {
      (void)buf;
      r &= 7;
      rotation = r;
      _internal_rotation = ((r + offset_rotation) & 3) | ((r & 4) ^ (offset_rotation & 4));
      _xs = _xe = _ys = _ye = ~0;
      _colstart = getColStart();
      _rowstart = getRowStart();
      return buf;
    }

    uint8_t getCmdCaset(void) const { return cmd_caset; }
    uint8_t getCmdRaset(void) const { return cmd_raset; }
    uint8_t getCmdRamwr(void) const { return cmd_ramwr; }
    uint8_t getCmdRamrd(void) const { return cmd_ramrd; }
    uint8_t getCmdRddid(void) const { return cmd_rddid; }
    //uint8_t getCmdSlpin(void) const { return cmd_slpin; }
    //uint8_t getCmdSlpout(void)const { return cmd_slpout; }

    static uint32_t getWindowAddr32(uint_fast16_t H, uint_fast16_t L) { return (((uint8_t)H)<<8 | (H)>>8) | ((uint32_t)((L)<<8 | (L)>>8))<<16; }
    static uint32_t getWindowAddr16(uint_fast16_t H, uint_fast16_t L) { return H | L<<8; }

  protected:
    virtual color_depth_t getAdjustBpp(color_depth_t bpp) const { return (bpp > 16) ? rgb888_3Byte : rgb565_2Byte; }

    uint8_t cmd_caset = 0;
    uint8_t cmd_raset = 0;
    uint8_t cmd_ramwr = 0;
    uint8_t cmd_ramrd = 0;
    uint8_t cmd_rddid = 0;
    //uint8_t cmd_slpin = 0;
    //uint8_t cmd_slpout= 0;
    uint8_t _internal_rotation = 0;
    uint_fast16_t _colstart = 0;
    uint_fast16_t _rowstart = 0;
    uint_fast16_t _xs = ~0;
    uint_fast16_t _xe = ~0;
    uint_fast16_t _ys = ~0;
    uint_fast16_t _ye = ~0;
  };

//----------------------------------------------------------------------------
 }
}
#endif
