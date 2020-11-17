#ifndef LGFX_PANEL_M5COREINK_HPP_
#define LGFX_PANEL_M5COREINK_HPP_

#include "PanelCommon.hpp"
#include "../LGFX_Device.hpp"

namespace lgfx
{
  struct Panel_M5CoreInk : public PanelCommon
  {
    Panel_M5CoreInk()
    {
      panel_width  = memory_width  = 200;
      panel_height = memory_height = 200;

      freq_write = 40000000;
      freq_read  = 16000000;
      freq_fill  = 40000000;

      write_depth = palette_1bit;
      read_depth = palette_1bit;
      len_dummy_read_pixel = 8;
      len_dummy_read_rddid = 0;
      len_setwindow = 16;

      //cmd_caset  = 1;
      //cmd_raset  = 2;
      cmd_ramwr  = 0x13;
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

    color_depth_t getAdjustBpp(color_depth_t bpp) const override { return palette_1bit; }

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
      /*
      if (sprite->getColorDepth() == color_depth_t::palette_1bit)
      {
        auto buf = static_cast<const uint8_t*>(sprite->getBuffer());
        gfx->startWrite();
        sprite->pushSprite(gfx, x, y);
        gfx->writeCommand(0x12);
        delay(10);
        while (!lgfx::gpio_in(gpio_busy)) delay(1);
        gfx->writeCommand(0x10);
        std::size_t len = sprite->width() * sprite->height() >> 3;
        for (std::size_t i = 0; i < len; ++i) { gfx->writeData(buf[i]); }
        gfx->endWrite();
      }
      else
      //*/
      {
        static int count = 0;
        count = (count + 1) & 3;
        static constexpr int8_t Bayer[16] = { -8, 120, 24, -104, -72, 56, -40, 88, 40, -88, 8, -120, -24, 104, -56, 72 };

        std::size_t bitwidth = (sprite->width()+7)&~7;
        std::size_t len = (bitwidth >> 3) * sprite->height();
        std::uint8_t buf[len];
        RGBColor readbuf[bitwidth];
        for (int i = 0; i < sprite->height(); ++i)
        {
          auto btbl = &Bayer[((y+i+count)&3)<<2];
          auto d = &buf[i * (bitwidth >> 3)];
          sprite->readRectRGB(0, i, sprite->width(), 1, readbuf);
          for (int j = 0; j < sprite->width(); j+=8)
          {
            std::size_t bytebuf = 0;
            for (int k = 0; k < 8; ++k)
            {
              auto color = readbuf[j + k];
              if (128 <= (int)((color.r + (color.g<<1) + color.b)>>2) + btbl[k & 3])
              {
                bytebuf |= 0x80 >> k;
              }
            }
            *d++ = bytebuf;
          }
        }
        gfx->startWrite();
        gfx->setAddrWindow(x, y, bitwidth, sprite->height());
        gfx->writeCommand(0x13);
        for (std::size_t i = 0; i < len; ++i) { gfx->writeData(buf[i]); }
        gfx->writeCommand(0x12);
        delay(100);
        while (!lgfx::gpio_in(gpio_busy)) delay(1);
        gfx->writeCommand(0x10);
        for (std::size_t i = 0; i < len; ++i) { gfx->writeData(buf[i]); }
        gfx->endWrite();
      }
    }
  };
}

#endif
