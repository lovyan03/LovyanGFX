#ifndef LGFX_PANEL_M5COREINK_HPP_
#define LGFX_PANEL_M5COREINK_HPP_

#include "PanelCommon.hpp"
#include "../LGFX_Device.hpp"
#include "../LGFX_Sprite.hpp"

namespace lgfx
{
  struct Panel_M5CoreInk : public PanelCommon
  {
    Panel_M5CoreInk()
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
      cmd_slpin  = 0x02;
      cmd_slpout = 0x04;
    }

  protected:

    void init(void) override
    {
      PanelCommon::init();
      if (gpio_busy >= 0) {
        lgfxPinMode(gpio_busy, pin_mode_t::input);
        delay(10);
        int retry = 1000;
        while (!gpio_in(gpio_busy) && --retry) delay(1);
      }
    }

    bool makeWindowCommands1(std::uint8_t* buf, std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye) override
    {
      reinterpret_cast<std::uint16_t*>(buf)[0] = 0x91;
      reinterpret_cast<std::uint16_t*>(buf)[1] = 0xFFFF;
      if (this->gpio_busy >= 0) while (!gpio_in(this->gpio_busy)) delay(1);
      return true;
    }

    bool makeWindowCommands2(std::uint8_t* buf, std::uint_fast16_t xs, std::uint_fast16_t ys, std::uint_fast16_t xe, std::uint_fast16_t ye) override
    {
      buf[0] = 0x90; buf[1] = 7;  buf[2] = xs;
                                  buf[3] = xe;
                                  buf[4] = ys >> 8;
                                  buf[5] = ys;
                                  buf[6] = ye >> 8;
                                  buf[7] = ye;
                                  buf[8] = 1;
      *reinterpret_cast<std::uint16_t*>(&buf[9]) = 0xFFFF;
      return true;
    }

    const std::uint8_t* getInvertDisplayCommands(std::uint8_t* buf, bool invert) override
    {
      buf[0] = buf[1] = 0xFF;
      return buf;
    }

    const std::uint8_t* getRotationCommands(std::uint8_t* buf, std::int_fast8_t r) override
    {
      buf[0] = buf[1] = 0xFF;
      return PanelCommon::getRotationCommands(buf, rotation);
    }

    const std::uint8_t* getColorDepthCommands(std::uint8_t* buf, color_depth_t depth) override
    {
      _xs = _xe = _ys = _ye = ~0;
      buf[0] = buf[1] = 0xFF;
      return buf;
    }

    color_depth_t getAdjustBpp(color_depth_t bpp) const override { return rgb565_2Byte; }

    const std::uint8_t* getInitCommands(std::uint8_t listno) const override {
      static constexpr std::uint8_t list0[] = {
          0x00,2,0xff,0x0e,       //panel setting
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
//          0x00,2,0xff,0x0e,       //panel setting
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

    bool hasPush(void) const override { return true; }

    void push(LGFX_Device* gfx, LGFX_Sprite* sprite, std::int_fast16_t x = 0, std::int_fast16_t y = 0) override
    {
      static constexpr std::uint8_t Bayer[16] = { 8, 136, 40, 168, 200, 72, 232, 104, 56, 184, 24, 152, 248, 120, 216, 88 };

      std::int_fast16_t xoffset = x & 7;
      std::size_t bitwidth = (xoffset + sprite->width() + 7) & ~7;
      x &= ~7;
      std::uint8_t buf[(bitwidth >> 3) * sprite->height()];
      RGBColor readbuf_raw[bitwidth];
      RGBColor* readbuf = &readbuf_raw[xoffset];
      auto bc = gfx->getBaseColor();
      for (int i = 0; i < 8; ++i) 
      {
        readbuf_raw[i] = bc;
        readbuf_raw[bitwidth - 8 + i] = bc;
      }
      for (int i = 0; i < sprite->height(); ++i)
      {
        auto btbl = &Bayer[((y + i) & 3) << 2];
        auto d = &buf[i * (bitwidth >> 3)];
        sprite->readRectRGB(0, i, sprite->width(), 1, readbuf);
        for (int j = -xoffset; j < sprite->width(); j += 8)
        {
          std::size_t bytebuf = 0;
          for (int k = 0; k < 8; ++k)
          {
            auto color = readbuf[j + k];
            if (256 <= (int)((color.r + (color.g << 1) + color.b) >> 2) + btbl[k & 3])
            {
              bytebuf |= 0x80 >> k;
            }
          }
          *d++ = bytebuf;
        }
      }

      std::int32_t clip_l, clip_t, clip_w, clip_h;
      gfx->getClipRect(&clip_l, &clip_t, &clip_w, &clip_h);
      if (clip_l & 7) {
        clip_w += clip_l & 7;
        clip_l &= ~7;
      }
      if (clip_w & 7) {
        clip_w = (clip_w + 7) & ~7;
      }

      std::int32_t dx=0, dw=bitwidth;
      if (0 < clip_l - x) { dx = clip_l - x; dw -= dx; x = clip_l; }

      if (_adjust_width(x, dx, dw, clip_l, clip_w)) return;

      std::int32_t dy=0, dh=sprite->height();
      if (0 < clip_t - y) { dy = clip_t - y; dh -= dy; y = clip_t; }
      if (_adjust_width(y, dy, dh, clip_t, clip_h)) return;

      {
        gfx->startWrite();
        gfx->setAddrWindow(x, y, dw, dh);
        gfx->writeCommand(0x13);
        int i = 0, add = bitwidth >> 3, len = dw >> 3;
        auto buffer = &buf[(dx + dy * bitwidth) >> 3];
        do {
          gfx->writeBytes(&buffer[i * add], len);
        } while (++i != dh);

        gfx->writeCommand(0x12);

        delay(250);
        while (!lgfx::gpio_in(gpio_busy)) delay(1);
        gfx->writeCommand(0x10);
        i = 0;
        do {
          gfx->writeBytes(&buffer[i * add], len);
        } while (++i != dh);
        gfx->endWrite();
      }
    }

    void push(LGFX_Device* gfx, std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, pixelcopy_t* param, bool use_dma) override
    {
      (void)use_dma;
      gfx->startWrite();
      gfx->setAddrWindow(x, y, w, h);
      gfx->writeCommand(0x13);
      _push_internal(gfx, x, y, w, h, param);
      gfx->writeCommand(0x12);
      delay(250);
      while (!lgfx::gpio_in(gpio_busy)) delay(1);
      gfx->writeCommand(0x10);
      _push_internal(gfx, x, y, w, h, param);
      gfx->endWrite();
    }

private:

    void _push_internal(LGFX_Device* gfx, std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, pixelcopy_t* param)
    {
      static constexpr std::uint8_t Bayer[16] = { 8, 136, 40, 168, 200, 72, 232, 104, 56, 184, 24, 152, 248, 120, 216, 88 };

      std::int_fast16_t xoffset = x & 7;
      std::size_t bitwidth = (xoffset + w + 7) & ~7;
      x &= ~7;
      std::uint8_t buf[2][(bitwidth >> 3)];
      swap565_t  readbuf_raw[bitwidth];
      swap565_t* readbuf = &readbuf_raw[xoffset];
      auto bc = gfx->getBaseColor();
      for (int i = 0; i < 8; ++i) 
      {
        readbuf_raw[i] = bc;
        readbuf_raw[bitwidth - 8 + i] = bc;
      }
      auto sx = param->src_x32;
      auto sy = param->src_y32;
      for (int i = 0; i < h; ++i)
      {
        auto btbl = &Bayer[((y + i) & 3) << 2];
        std::int32_t pos = 0;
        while (w != (pos = param->fp_copy(readbuf, pos, w, param))) {
          if ( w == (pos = param->fp_skip(         pos, w, param))) break;
        }
        param->src_x32 = sx;
        param->src_y++;
        auto d = buf[i & 1];
        for (int j = -xoffset; j < w; j += 8)
        {
          std::size_t bytebuf = 0;
          for (int k = 0; k < 8; ++k)
          {
            auto color = readbuf[j + k];
            if (256 <= (int)((color.R8() + (color.G8() << 1) + color.B8()) >> 2) + btbl[k & 3])
            {
              bytebuf |= 0x80 >> k;
            }
          }
          *d++ = bytebuf;
        }
        gfx->writeBytes(buf[i & 1], bitwidth >> 3, true);
      }
      param->src_y32 = sy;
      gfx->waitDMA();
    }

    static bool _adjust_width(std::int32_t& x, std::int32_t& dx, std::int32_t& dw, std::int32_t left, std::int32_t width)
    {
      if (x < left) { dx = -x; dw += x; x = left; }
      if (dw > left + width - x) dw = left + width  - x;
      return (dw <= 0);
    }

  };
}

#endif
