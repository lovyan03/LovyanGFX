#ifndef LGFX_PANEL_GDEW0154M09_HPP_
#define LGFX_PANEL_GDEW0154M09_HPP_

#include "PanelCommon.hpp"

namespace lgfx
{
  struct Panel_GDEW0154M09 : public PanelCommon
  {
    Panel_GDEW0154M09()
    {
      panel_width  = memory_width  = 200;
      panel_height = memory_height = 200;

      freq_write = 27000000;
      freq_read  = 16000000;
      freq_fill  = 27000000;

      write_depth = rgb565_2Byte;
      read_depth = rgb565_2Byte;
      spi_read = false;
      len_dummy_read_pixel = 8;
      len_dummy_read_rddid = 0;
      len_setwindow = 16;

      //cmd_caset  = 1;
      //cmd_raset  = 2;
      //cmd_ramwr  = 0x13;
      cmd_ramwr  = 0x11; // dummy setting (data stop)
      //cmd_ramrd  = 4;
      cmd_rddid  = 0x70;

      fp_begin       = beginTransaction;
      fp_end         = endTransaction;
      fp_display     = display;
      fp_waitDisplay = waitDisplay;
      fp_fillRect    = fillRect;
      fp_pushImage   = pushImage;
      fp_pushBlock   = pushBlock;
      fp_writePixels = writePixels;
      fp_readRect    = readRect;
    }

  protected:

    void init(bool use_reset) override
    {
      PanelCommon::init(use_reset);
      if (gpio_busy >= 0) {
        lgfxPinMode(gpio_busy, pin_mode_t::input);
        delay(10);
        int retry = 1000;
        while (!gpio_in(gpio_busy) && --retry) delay(1);
      }
      int len = ((panel_width + 7) & ~7) * panel_height >> 3;
      if (_buf) heap_free(_buf);
      _buf = static_cast<std::uint8_t*>(heap_alloc_dma(len));
      memset(_buf, 255, len);
    }

    void post_init(LGFX_Device* gfx, bool use_reset) override;

    const std::uint8_t* getWindowCommands1(std::uint8_t* buf, std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye) override
    {
      (void)buf;
      _xpos = xs;
      _xs = xs;
      _ypos = ys;
      _ys = ys;
      _xe = xe;
      _ye = ye;
      return nullptr;
    }
    const std::uint8_t* getWindowCommands2(std::uint8_t*, std::uint_fast16_t, std::uint_fast16_t, std::uint_fast16_t, std::uint_fast16_t) override
    {
      return nullptr;
    }

    const std::uint8_t* getSleepInCommands(std::uint8_t* buf) override
    {
      buf[0] = 0x50; buf[1] = 1; buf[2] = 0xF7;
      buf[3] = 0x02; buf[4] = 0;
      buf[5] = buf[6] = 0xFF;
      return buf;
    }

    const std::uint8_t* getSleepOutCommands(std::uint8_t* buf) override
    {
      buf[0] = 0x50; buf[1] = 1; buf[2] = 0xD7;
      buf[3] = 0x04; buf[4] = 0;
      buf[5] = buf[6] = 0xFF;
      return buf;
    }

    const std::uint8_t* getPartialOnCommands(std::uint8_t* buf) override
    {
      buf[0] = 0x50; buf[1] = 1; buf[2] = 0xF7;
      buf[3] = 0x02; buf[4] = 0;
      buf[5] = buf[6] = 0xFF;
      return buf;
    }

    const std::uint8_t* getPartialOffCommands(std::uint8_t* buf) override
    {
      buf[0] = 0x50; buf[1] = 1; buf[2] = 0xD7;
      buf[3] = 0x04; buf[4] = 0;
      buf[5] = buf[6] = 0xFF;
      return buf;
    }

    const std::uint8_t* getInvertDisplayCommands(std::uint8_t* buf, bool invert) override
    {
      (void)invert;
      reinterpret_cast<std::uint16_t*>(buf)[0] = 0xFFFF;
      return buf;
    }

    const std::uint8_t* getRotationCommands(std::uint8_t* buf, std::int_fast8_t r) override
    {
      reinterpret_cast<std::uint16_t*>(buf)[0] = 0xFFFF;
      return PanelCommon::getRotationCommands(buf, r);
    }

    const std::uint8_t* getColorDepthCommands(std::uint8_t* buf, color_depth_t depth) override
    {
      (void)depth;
      reinterpret_cast<std::uint16_t*>(buf)[0] = 0xFFFF;
      return buf;
    }

    color_depth_t getAdjustBpp(color_depth_t bpp) const override { (void)bpp; return rgb565_2Byte; }

    const std::uint8_t* getInitCommands(std::uint8_t listno) const override {
      static constexpr std::uint8_t list0[] = {
          0x00,2,0xdf,0x0e,       //panel setting
          0x4D,1,0x55,            //FITIinternal code
          0xaa,1,0x0f,
          0xe9,1,0x02,
          0xb6,1,0x11,
          0xf3,1,0x0a,
          0x61,3,0xc8,0x00,0xc8,  //resolution setting
          0x60,1,0x00,            //Tcon setting
          0x50,1,0xd7,
          0xe3,1,0x00,
          0x04,0,                 //Power on
          0x00,2,0xff,0x0e,       //panel setting
          0xFF,0xFF, // end
      };
      static constexpr std::uint8_t list1[] = {
          0x20,56,0x01, 0x04, 0x04, 0x03, 0x01, 0x01, 0x01,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x21,42,0x01, 0x04, 0x04, 0x03, 0x01, 0x01, 0x01,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x22,56,0x01, 0x84, 0x84, 0x83, 0x01, 0x01, 0x01,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x23,56,0x01, 0x44, 0x44, 0x43, 0x01, 0x01, 0x01,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x24,56,0x01, 0x04, 0x04, 0x03, 0x01, 0x01, 0x01,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0xFF,0xFF, // end
      };
      switch (listno) {
      case 0: return list0;
      case 1: return list1;
      default: return nullptr;
      }
    }

  private:
    static constexpr std::uint8_t Bayer[16] = { 8, 136, 40, 168, 200, 72, 232, 104, 56, 184, 24, 152, 248, 120, 216, 88 };
    std::uint8_t* _buf = nullptr;

    range_rect_t _range_new;
    range_rect_t _range_old;

    //std::int32_t _tr_top = INT32_MAX;
    //std::int32_t _tr_left = INT32_MAX;
    //std::int32_t _tr_right = 0;
    //std::int32_t _tr_bottom = 0;
    std::int32_t _xpos = 0;
    std::int32_t _ypos = 0;

    __attribute__ ((always_inline)) inline 
    void _draw_pixel(std::int32_t x, std::int32_t y, std::uint32_t value)
    {
      if (_internal_rotation & 1) { std::swap(x, y); }
      switch (_internal_rotation) {
      case 1: case 2: case 6: case 7:  x = panel_width - x - 1; break;
      default: break;
      }
      switch (_internal_rotation) {
      case 2: case 3: case 4: case 7:  y = panel_height - y - 1; break;
      default: break;
      }
      std::uint32_t idx = ((panel_width + 7) & ~7) * y + x;
      bool flg = 256 <= value + Bayer[(x & 3) | (y & 3) << 2];
      if (flg) _buf[idx >> 3] |=   0x80 >> (idx & 7);
      else     _buf[idx >> 3] &= ~(0x80 >> (idx & 7));
    }

    __attribute__ ((always_inline)) inline 
    bool _read_pixel(std::int32_t x, std::int32_t y)
    {
      if (_internal_rotation & 1) { std::swap(x, y); }
      switch (_internal_rotation) {
      case 1: case 2: case 6: case 7:  x = panel_width - x - 1; break;
      default: break;
      }
      switch (_internal_rotation) {
      case 2: case 3: case 4: case 7:  y = panel_height - y - 1; break;
      default: break;
      }
      std::uint32_t idx = ((panel_width + 7) & ~7) * y + x;
      return _buf[idx >> 3] & (0x80 >> (idx & 7));
    }

    void _update_transferred_rect(LGFX_Device* gfx, std::int32_t &xs, std::int32_t &ys, std::int32_t &xe, std::int32_t &ye);
    void _exec_transfer(std::uint32_t cmd, LGFX_Device* gfx, range_rect_t* range, bool invert = false);
    void _close_transfer(LGFX_Device* gfx);

    static void beginTransaction(PanelCommon* panel, LGFX_Device* gfx);
    static void endTransaction(PanelCommon* panel, LGFX_Device* gfx);
    static void display(PanelCommon* panel, LGFX_Device* gfx);
    static void waitDisplay(PanelCommon* panel, LGFX_Device* gfx);
    static void fillRect(PanelCommon* panel, LGFX_Device* gfx, std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::uint32_t rawcolor);
    static void pushImage(PanelCommon* panel, LGFX_Device* gfx, std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, pixelcopy_t* param);
    static void pushBlock(PanelCommon* panel, LGFX_Device* gfx, std::int32_t length, std::uint32_t rawcolor);
    static void writePixels(PanelCommon* panel, LGFX_Device* gfx, std::int32_t len, pixelcopy_t* param);
    static void readRect(PanelCommon* panel, LGFX_Device* gfx, std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, void* dst, pixelcopy_t* param);
  };
}

#endif
