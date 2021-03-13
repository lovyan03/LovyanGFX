#ifndef LGFX_PANEL_COMMON_HPP_
#define LGFX_PANEL_COMMON_HPP_

#include <cstdint>
#include <type_traits>
#include "../lgfx_common.hpp"

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  class LGFX_Device;
  class LGFX_Sprite;

  static constexpr std::uint8_t CMD_INIT_DELAY = 0x80;

  struct PanelCommon
  {
    virtual ~PanelCommon() = default;
    std::uint32_t freq_write = 2700000;    // SPI freq (write pixel)
    std::uint32_t freq_read  = 1600000;    // SPI freq (read pixel )
    std::uint32_t freq_fill  = 4000000;    // SPI freq (fill pixel )
    std::int_fast16_t spi_cs    = -1;      // SPI CS pin number
    std::int_fast16_t spi_dc    = -1;      // SPI dc pin number
    std::int_fast16_t gpio_rst  = -1;      // hardware reset pin number
    std::int_fast16_t gpio_bl   = -1;      // backlight pin number
    std::int_fast16_t gpio_busy = -1;      // busy check pin number
    std::int_fast8_t spi_mode  = 0;        // SPI mode (0~3)
    std::int_fast8_t spi_mode_read = 0;    // SPI mode (0~3) when read pixel
    std::int_fast8_t spi_dlen  = 0;        // SPI transfer length (8 or 16)
    std::int_fast8_t rotation  = 0;        // default rotation (0~3)
    std::int_fast8_t offset_rotation = 0;  // rotation offset (0~3)
    std::int_fast8_t pwm_ch_bl = -1;       // backlight PWM channel number
    std::uint_fast16_t pwm_freq = 12000;   // backlight PWM freq Hz
    bool reverse_invert = false;           // Reverse the effect of the invert command.

    bool backlight_level = true;      // turn ON back-light backlight level (true = high / false = low)
    bool spi_read  = true;            // Use SPI read
    bool spi_3wire = false;           // SPI 3wire mode (read from MOSI)
    bool invert    = false;           // panel invert
    bool rgb_order = false;           // true:RGB / false: BGR

    std::int_fast16_t memory_width  = 240; // width of driver VRAM
    std::int_fast16_t memory_height = 240; // height of driver VRAM
    std::int_fast16_t panel_width   = 240; // width of panel
    std::int_fast16_t panel_height  = 240; // height of panel
    std::int_fast16_t offset_x      = 0;   // panel offset x axis
    std::int_fast16_t offset_y      = 0;   // panel offset y axis

    std::uint_fast8_t brightness    = 127;

//
    color_depth_t write_depth = rgb565_2Byte;
    color_depth_t read_depth  = rgb888_3Byte;

    std::uint8_t len_setwindow = 32;
    std::uint8_t len_dummy_read_pixel = 8;
    std::uint8_t len_dummy_read_rddid = 1;

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

    virtual void setBrightness(std::uint8_t brightness)
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

    std::int_fast16_t getWidth(void) const {
      return (_internal_rotation & 1) ? panel_height : panel_width;
    }

    std::int_fast16_t getHeight(void) const {
      return (_internal_rotation & 1) ? panel_width : panel_height;
    }

    std::int_fast16_t getColStart(void) const {
      switch (_internal_rotation & 3) {
      default:  return offset_x;
      case 1:   return offset_y;
      case 2:   return memory_width  - (panel_width  + offset_x);
      case 3:   return memory_height - (panel_height + offset_y);
      }
    }

    std::int_fast16_t getRowStart(void) const {
      switch ((_internal_rotation & 3) | (_internal_rotation & 4)) {
      default:          return offset_y;
      case 1: case 7:   return memory_width  - (panel_width  + offset_x);
      case 2: case 4:   return memory_height - (panel_height + offset_y);
      case 3: case 5:   return offset_x;
      }
    }

    void (*fp_begin)(PanelCommon*, LGFX_Device*) = nullptr;
    void (*fp_end)(PanelCommon*, LGFX_Device*) = nullptr;
    void (*fp_display)(PanelCommon*, LGFX_Device*, std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h) = nullptr;
    void (*fp_waitDisplay)(PanelCommon*, LGFX_Device*) = nullptr;
    bool (*fp_displayBusy)(PanelCommon*, LGFX_Device*) = nullptr;
    void (*fp_pushImage)(PanelCommon*, LGFX_Device*, std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, pixelcopy_t* param) = nullptr;
    void (*fp_fillRect)(PanelCommon*, LGFX_Device*, std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::uint32_t rawcolor) = nullptr;
    void (*fp_pushBlock)(PanelCommon*, LGFX_Device*, std::int32_t len, std::uint32_t rawcolor) = nullptr;
    void (*fp_writePixels)(PanelCommon*, LGFX_Device*, std::int32_t len, pixelcopy_t* param) = nullptr;
    void (*fp_readRect)(PanelCommon*, LGFX_Device*, std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, void* dst, pixelcopy_t* param) = nullptr;

    virtual void post_init(LGFX_Device*, bool use_reset) { (void)use_reset; }

    virtual const std::uint8_t* getColorDepthCommands(std::uint8_t* buf, color_depth_t depth) {
      write_depth = getAdjustBpp(depth);
      return buf;
    }

    virtual bool isEPD(void) const { return false; }
    virtual const std::uint8_t* getInitCommands(std::uint8_t listno = 0) const { (void)listno; return nullptr; }
    virtual const std::uint8_t* getWindowCommands1(std::uint8_t* buf, std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye) = 0;
    virtual const std::uint8_t* getWindowCommands2(std::uint8_t* buf, std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye) = 0;
    virtual const std::uint8_t* getSleepInCommands(std::uint8_t* buf) = 0;
    virtual const std::uint8_t* getSleepOutCommands(std::uint8_t* buf) = 0;
    virtual const std::uint8_t* getPowerSaveOnCommands(std::uint8_t* buf) { (void)buf; return nullptr; }
    virtual const std::uint8_t* getPowerSaveOffCommands(std::uint8_t* buf) { (void)buf; return nullptr; }
    virtual const std::uint8_t* getInvertDisplayCommands(std::uint8_t* buf, bool invert) = 0;
    virtual const std::uint8_t* getRotationCommands(std::uint8_t* buf, std::int_fast8_t r)
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

    std::uint8_t getCmdCaset(void) const { return cmd_caset; }
    std::uint8_t getCmdRaset(void) const { return cmd_raset; }
    std::uint8_t getCmdRamwr(void) const { return cmd_ramwr; }
    std::uint8_t getCmdRamrd(void) const { return cmd_ramrd; }
    std::uint8_t getCmdRddid(void) const { return cmd_rddid; }
    //std::uint8_t getCmdSlpin(void) const { return cmd_slpin; }
    //std::uint8_t getCmdSlpout(void)const { return cmd_slpout; }

    static std::uint32_t getWindowAddr32(std::uint_fast16_t H, std::uint_fast16_t L) { return (((std::uint8_t)H)<<8 | (H)>>8) | ((std::uint32_t)((L)<<8 | (L)>>8))<<16; }
    static std::uint32_t getWindowAddr16(std::uint_fast16_t H, std::uint_fast16_t L) { return H | L<<8; }

  protected:
    virtual color_depth_t getAdjustBpp(color_depth_t bpp) const { return (bpp > 16) ? rgb888_3Byte : rgb565_2Byte; }

    std::uint8_t cmd_caset = 0;
    std::uint8_t cmd_raset = 0;
    std::uint8_t cmd_ramwr = 0;
    std::uint8_t cmd_ramrd = 0;
    std::uint8_t cmd_rddid = 0;
    //std::uint8_t cmd_slpin = 0;
    //std::uint8_t cmd_slpout= 0;
    std::uint8_t _internal_rotation = 0;
    std::uint_fast16_t _colstart = 0;
    std::uint_fast16_t _rowstart = 0;
    std::uint_fast16_t _xs = ~0;
    std::uint_fast16_t _xe = ~0;
    std::uint_fast16_t _ys = ~0;
    std::uint_fast16_t _ye = ~0;
  };

//----------------------------------------------------------------------------
 }
}
#endif
