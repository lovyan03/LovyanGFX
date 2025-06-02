/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)

Contributors:
 [ciniml](https://github.com/ciniml)
 [mongonta0716](https://github.com/mongonta0716)
 [tobozo](https://github.com/tobozo)
/----------------------------------------------------------------------------*/
#pragma once

#include "Panel_Device.hpp"
#include "Panel_SSD1306.hpp"

#include "../misc/range.hpp"
#include "../platforms/common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Panel_SharpLCD : public Panel_HasBuffer
  {

    struct config_detail_t
    {
      // Some SharpLCD breakouts can optionally drive those pins
      int16_t pin_dispon = -1; // DISPON
      int16_t pin_extmod = -1; // EXTMOD

      // number of bytes containing update command and the line number to update
      // should be 1 when the display height is under 255 pixels
      uint8_t prefix_bytes = 1;

      // number of trailing (dummy) bytes at the end of a line
      uint8_t suffix_bytes = 1;

      bool enable_dithering = false;  // enable if working with lgfx colors, but expect low performances
      bool enable_autodisplay = true;
    };

    const config_detail_t& config_detail(void) const { return _config_detail; }
    void config_detail(const config_detail_t& config_detail) { _config_detail = config_detail; }


    Panel_SharpLCD() : Panel_HasBuffer()
    {
      _cfg.bus_shared = false;
      _epd_mode = epd_quality;
      _write_depth = color_depth_t::rgb565_2Byte;
      _read_depth = color_depth_t::rgb565_2Byte;
    }


    uint8_t *getBuffer() { return _buf; }
    uint32_t getBufferSize() { return _get_buffer_length(); }

    void setTilePattern(uint_fast8_t i);
    void clearDisplay();

    void waitDisplay(void) override {}
    bool displayBusy(void) override { return false; }
    void setSleep(bool) override {}
    void setPowerSave(bool) override {}
    void setBrightness(uint8_t) override {}
    uint32_t readCommand(uint_fast16_t, uint_fast8_t, uint_fast8_t) override { return 0; }
    uint32_t readData(uint_fast8_t, uint_fast8_t) override { return 0; }


    bool init(bool use_reset) override;
    color_depth_t setColorDepth(color_depth_t depth) override;
    void setInvert(bool invert) override;
    void display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h) override;
    void display(uint8_t* dst, uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h);


    void init_cs(void) override;
    void cs_control(bool level) override;
    void writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor) override;

    // TODO: implement those
    void writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool use_dma) override /*{ ESP_LOGE("LGFX", "TODO"); }*/;
    void writePixels(pixelcopy_t* param, uint32_t len, bool use_dma) override /*{ ESP_LOGE("LGFX", "TODO"); }*/;
    void readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param) override  /*{ ESP_LOGE("LGFX", "TODO"); }*/;



  protected:

    config_detail_t _config_detail;

    static constexpr uint8_t CMD_MAINTAIN           = 0x00;  // 0xff in LSB format
    static constexpr uint8_t CMD_UPDATE             = 0x01;  // 0x80 in LSB format
    static constexpr uint8_t BIT_VCOM               = 0x02;  // 0x40 in LSB format
    static constexpr uint8_t CMD_CLEAR              = 0x04;  // 0x20 in LSB format

    static constexpr uint8_t VAL_TRAILER            = 0x00;

    uint8_t _bayer_offset = 0;
    uint8_t _vcom = 0;

    uint16_t _pitch = 0;

    size_t _get_buffer_length(void) const override;
    void _draw_pixel(uint8_t* dst, uint_fast16_t x, uint_fast16_t y, uint32_t value);
    bool _read_pixel(uint_fast16_t x, uint_fast16_t y);
    void _update_transferred_rect(uint_fast16_t &xs, uint_fast16_t &ys, uint_fast16_t &xe, uint_fast16_t &ye);

  };


//----------------------------------------------------------------------------
 }
}
