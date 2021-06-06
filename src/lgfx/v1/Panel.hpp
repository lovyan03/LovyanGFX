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

#include <cstdint>
#include <memory>

#include "misc/enum.hpp"
#include "misc/pixelcopy.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  class pixelcopy_t;

  struct IPanel
  {
  protected:
    std::uint32_t _start_count = 0;
    std::uint16_t _xs = ~0;
    std::uint16_t _xe = ~0;
    std::uint16_t _ys = ~0;
    std::uint16_t _ye = ~0;
    std::uint16_t _width = 0;
    std::uint16_t _height = 0;
    color_depth_t _write_depth = color_depth_t::rgb565_2Byte;
    color_depth_t _read_depth  = color_depth_t::rgb565_2Byte;
    std::uint8_t _write_bits = 16;
    std::uint8_t _read_bits = 16;
    std::uint8_t _rotation = 0;
    epd_mode_t _epd_mode = (epd_mode_t)0;  // EPDでない場合は0。それ以外の場合はEPD描画モード
    bool _invert = false;
    bool _auto_display = false;

  public:
    constexpr IPanel(void) = default;
    virtual ~IPanel(void) = default;

    void startWrite(bool transaction = true) { if (1 == ++_start_count && transaction) { beginTransaction(); } }
    void endWrite(void) { if (_start_count) {  if (0 == --_start_count) { if (_auto_display) { display(0,0,0,0); } endTransaction(); } } }
    std::uint32_t getStartCount(void) const { return _start_count; }
    color_depth_t getWriteDepth(void) const { return _write_depth; }
    color_depth_t getReadDepth(void) const { return _read_depth; }
    std::uint8_t getRotation(void) const { return _rotation; }
    bool getInvert(void) const { return _invert; }

    std::uint16_t width(void) const { return _width; }
    std::uint16_t height(void) const { return _height; }
    epd_mode_t getEpdMode(void) const { return _epd_mode; }
    void setEpdMode(epd_mode_t epd_mode) { if (_epd_mode && epd_mode) _epd_mode = epd_mode; }
    bool isEpd(void) const { return _epd_mode; }
    bool getAutoDisplay(void) const { return _auto_display; }
    void setAutoDisplay(bool auto_display) { _auto_display = auto_display; }

    virtual void beginTransaction(void) = 0;
    virtual void endTransaction(void) = 0;

    virtual void setBrightness(std::uint8_t brightness) {};

    virtual color_depth_t setColorDepth(color_depth_t depth) = 0;

    virtual void setInvert(bool invert) = 0;
    virtual void setRotation(std::uint_fast8_t r) = 0;
    virtual void setSleep(bool flg_sleep) = 0;
    virtual void setPowerSave(bool flg_idle) = 0;

    virtual void writeCommand(std::uint32_t cmd, std::uint_fast8_t length) = 0;
    virtual void writeData(std::uint32_t data, std::uint_fast8_t length) = 0;

    virtual void initDMA(void) = 0;
    virtual void waitDMA(void) = 0;
    virtual bool dmaBusy(void) = 0;
    virtual void waitDisplay(void) = 0;
    virtual bool displayBusy(void) = 0;
    virtual void display(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h) = 0;
    virtual bool isReadable(void) const = 0;
    virtual bool isBusShared(void) const = 0;

    virtual void writeBlock(std::uint32_t rawcolor, std::uint32_t len) = 0;
    virtual void setWindow(std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye) = 0;
    virtual void drawPixelPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint32_t rawcolor) = 0;
    virtual void writeFillRectPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, std::uint32_t rawcolor) = 0;
    virtual void writeImage(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, pixelcopy_t* param, bool use_dma) = 0;
    virtual void writeImageARGB(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, pixelcopy_t* param) = 0;
    virtual void writePixels(pixelcopy_t* param, std::uint32_t len) = 0;

    virtual std::uint32_t readCommand(std::uint_fast8_t cmd, std::uint_fast8_t index = 0, std::uint_fast8_t length = 4) = 0;
    virtual std::uint32_t readData(std::uint_fast8_t index = 0, std::uint_fast8_t length = 4) = 0;
    virtual void readRect(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, void* dst, pixelcopy_t* param) = 0;
    virtual void copyRect(std::uint_fast16_t dst_x, std::uint_fast16_t dst_y, std::uint_fast16_t w, std::uint_fast16_t h, std::uint_fast16_t src_x, std::uint_fast16_t src_y) = 0;

    virtual void writeFillRectAlphaPreclipped(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, std::uint32_t argb8888)
    {
      effect(x, y, w, h, effect_fill_alpha ( argb8888_t { argb8888 } ) );
    }

    template<typename TFunc>
    void effect(std::uint_fast16_t x, std::uint_fast16_t y, std::uint_fast16_t w, std::uint_fast16_t h, TFunc&& effector)
    {
      auto ye = y + h;
      RGBColor buf[w];
      pixelcopy_t pc_write(buf    ,_write_depth, RGBColor::depth, false);
      pixelcopy_t pc_read( nullptr, RGBColor::depth, _read_depth, false);
      startWrite();
      do
      {
        readRect(x, y, w, 1, buf, &pc_read);
        std::size_t i = 0;
        do
        {
          effector(x + i, y, buf[i]);
        } while (++i < w);
        writeImage(x, y, w, 1, &pc_write, true);
        pc_write.src_x32 = 0;
      } while (++y < ye);
      endWrite();
    }
 };

//----------------------------------------------------------------------------
 }
}
