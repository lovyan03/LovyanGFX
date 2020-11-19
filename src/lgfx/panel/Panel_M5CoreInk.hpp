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

      fp_begin     = beginTransaction;
      fp_end       = endTransaction;
      fp_pushImage = pushImage;
      fp_fillRect  = fillRect;
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
      int len = panel_width * panel_height;
      _buf = static_cast<std::uint8_t*>(heap_alloc(len));
      memset(_buf, 0, len);
      //_framebuffer.setColorDepth(1);
      //_framebuffer.createSprite(panel_width, panel_height);
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
      return PanelCommon::getRotationCommands(buf, r);
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

  private:
    const std::uint8_t Bayer[16] = { 8, 136, 40, 168, 200, 72, 232, 104, 56, 184, 24, 152, 248, 120, 216, 88 };

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
      std::uint32_t idx = panel_width * y + x;
      bool flg = 256 <= value + Bayer[(x & 3) | (y & 3) << 2];
      if (flg) _buf[idx >> 3] |=   0x80 >> (idx & 7);
      else     _buf[idx >> 3] &= ~(0x80 >> (idx & 7));
    }

    static void fillRect(PanelCommon* panel, LGFX_Device*, std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::uint32_t rawcolor)
    {
      auto me = reinterpret_cast<Panel_M5CoreInk*>(panel);
      std::int32_t xs = x, xe = x + w - 1;
      std::int32_t ys = y, ye = y + h - 1;
      auto r = me->_internal_rotation;
      if (r & 1) { std::swap(xs, ys); std::swap(xe, ye); }
      switch (r) {
      default: break;
      case 1:  case 2:  case 6:  case 7:
        std::swap(xs, xe);
        xs = me->panel_width - 1 - xs;
        xe = me->panel_width - 1 - xe;
        break;
      }
      switch (r) {
      default: break;
      case 2: case 3: case 4: case 7:
        std::swap(ys, ye);
        ys = me->panel_height - 1 - ys;
        ye = me->panel_height - 1 - ye;
        break;
      }

      rgb565_t rgb565 = rawcolor;
      std::uint32_t value = (rgb565.R8() + (rgb565.G8() << 1) + rgb565.B8()) >> 2;

      y = ys;
      do
      {
        x = xs;
        std::uint32_t idx = me->panel_width * y + x;
        auto btbl = &me->Bayer[(y & 3) << 2];
        do
        {
          bool flg = 256 <= value + btbl[x & 3];
          if (flg) me->_buf[idx >> 3] |=   0x80 >> (idx & 7);
          else     me->_buf[idx >> 3] &= ~(0x80 >> (idx & 7));
          ++idx;
        } while (++x <= xe);
      } while (++y <= ye);
    }

    static void pushImage(PanelCommon* panel, LGFX_Device* gfx, std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, pixelcopy_t* param)
    {
      auto me = reinterpret_cast<Panel_M5CoreInk*>(panel);
      swap565_t readbuf[w];
      auto sx = param->src_x32;
      h += y;
      do
      {
        std::int32_t prev_pos = 0, new_pos = 0;
        do
        {
          new_pos = param->fp_copy(readbuf, prev_pos, w, param);
          if (new_pos != prev_pos)
          {
            do
            {
              auto color = readbuf[prev_pos];
              me->_draw_pixel(x + prev_pos, y, (color.R8() + (color.G8() << 1) + color.B8()) >> 2);
            } while (new_pos != ++prev_pos);
          }
        } while (w != new_pos && w != (prev_pos = param->fp_skip(new_pos, w, param)));
        param->src_x32 = sx;
        param->src_y++;
      } while (++y < h);
    }

    static void beginTransaction(PanelCommon* panel, LGFX_Device* gfx)
    {
      auto me = reinterpret_cast<Panel_M5CoreInk*>(panel);
      while (!lgfx::gpio_in(me->gpio_busy)) delay(1);
      gfx->setWindow(0, 0, me->panel_width - 1, me->panel_height - 1);
      gfx->writeCommand(0x10);
      gfx->writeBytes(me->_buf, me->panel_width * me->panel_height >> 3);
      gfx->waitDMA();
    }

    static void endTransaction(PanelCommon* panel, LGFX_Device* gfx)
    {
      auto me = reinterpret_cast<Panel_M5CoreInk*>(panel);
      gfx->setWindow(0, 0, me->panel_width - 1, me->panel_height - 1);
      gfx->writeCommand(0x13);
      gfx->writeBytes(me->_buf, me->panel_width * me->panel_height >> 3);
      gfx->writeCommand(0x12);
    }

    //LGFX_Sprite _framebuffer;
    std::uint8_t* _buf;
/*
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
//*/
  };
}

#endif
