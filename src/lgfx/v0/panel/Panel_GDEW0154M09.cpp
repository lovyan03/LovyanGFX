#include "../lgfx_common.hpp"
#if defined (LGFX_ENABLE_V0)

#include "Panel_GDEW0154M09.hpp"
#include "../LGFX_Device.hpp"

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  constexpr uint8_t Panel_GDEW0154M09::Bayer[16];

  void Panel_GDEW0154M09::post_init(LGFX_Device* gfx, bool use_reset)
  {
    (void)use_reset;
    // init DSRAM
    _range_old.top = 0;
    _range_old.left = 0;
    _range_old.right = panel_width - 1;
    _range_old.bottom = panel_height - 1;
    gfx->startWrite();
    _exec_transfer(0x13, gfx, _range_old);
    _close_transfer(gfx);
    _range_new = _range_old;
    gfx->setBaseColor(TFT_WHITE);
    gfx->setTextColor(TFT_BLACK, TFT_WHITE);

    gfx->endWrite();
  }

  void Panel_GDEW0154M09::_update_transferred_rect(LGFX_Device* gfx, int32_t &xs, int32_t &ys, int32_t &xe, int32_t &ye)
  {
    auto r = _internal_rotation;
    if (r & 1) { std::swap(xs, ys); std::swap(xe, ye); }
    switch (r) {
    default: break;
    case 1:  case 2:  case 6:  case 7:
      std::swap(xs, xe);
      xs = panel_width - 1 - xs;
      xe = panel_width - 1 - xe;
      break;
    }
    switch (r) {
    default: break;
    case 2: case 3: case 4: case 7:
      std::swap(ys, ye);
      ys = panel_height - 1 - ys;
      ye = panel_height - 1 - ye;
      break;
    }
    int32_t x1 = xs & ~7;
    int32_t x2 = (xe & ~7) + 7;

    if (_range_old.horizon.intersectsWith(x1, x2) && _range_old.vertical.intersectsWith(ys, ye))
    {
      _close_transfer(gfx);
    }
    _range_new.top = std::min(ys, _range_new.top);
    _range_new.left = std::min(x1, _range_new.left);
    _range_new.right = std::max(x2, _range_new.right);
    _range_new.bottom = std::max(ye, _range_new.bottom);
  }

  void Panel_GDEW0154M09::fillRect(PanelCommon* panel, LGFX_Device* gfx, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t rawcolor)
  {
    auto me = reinterpret_cast<Panel_GDEW0154M09*>(panel);
    int32_t xs = x, xe = x + w - 1;
    int32_t ys = y, ye = y + h - 1;
    me->_update_transferred_rect(gfx, xs, ys, xe, ye);

    swap565_t color;
    color.raw = rawcolor;
    uint32_t value = (color.R8() + (color.G8() << 1) + color.B8() + 3) >> 2;

    y = ys;
    do
    {
      x = xs;
      uint32_t idx = ((me->panel_width + 7) & ~7) * y + x;
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

  void Panel_GDEW0154M09::pushImage(PanelCommon* panel, LGFX_Device* gfx, int32_t x, int32_t y, int32_t w, int32_t h, pixelcopy_t* param)
  {
    auto me = reinterpret_cast<Panel_GDEW0154M09*>(panel);
    int32_t xs = x, xe = x + w - 1;
    int32_t ys = y, ye = y + h - 1;
    me->_update_transferred_rect(gfx, xs, ys, xe, ye);

    swap565_t readbuf[w];
    auto sx = param->src_x32;
    h += y;
    do
    {
      int32_t prev_pos = 0, new_pos = 0;
      do
      {
        new_pos = param->fp_copy(readbuf, prev_pos, w, param);
        if (new_pos != prev_pos)
        {
          do
          {
            auto color = readbuf[prev_pos];
            me->_draw_pixel(x + prev_pos, y, (color.R8() + (color.G8() << 1) + color.B8() + 3) >> 2);
          } while (new_pos != ++prev_pos);
        }
      } while (w != new_pos && w != (prev_pos = param->fp_skip(new_pos, w, param)));
      param->src_x32 = sx;
      param->src_y++;
    } while (++y < h);
  }

  void Panel_GDEW0154M09::pushBlock(PanelCommon* panel, LGFX_Device* gfx, int32_t length, uint32_t rawcolor)
  {
    auto me = reinterpret_cast<Panel_GDEW0154M09*>(panel);
    {
      int32_t xs = me->_xs;
      int32_t xe = me->_xe;
      int32_t ys = me->_ys;
      int32_t ye = me->_ye;
      me->_update_transferred_rect(gfx, xs, ys, xe, ye);
    }
    int32_t xs = me->_xs;
    int32_t ys = me->_ys;
    int32_t xe = me->_xe;
    int32_t ye = me->_ye;
    int32_t xpos = me->_xpos;
    int32_t ypos = me->_ypos;

    swap565_t color;
    color.raw = rawcolor;
    uint32_t value = (color.R8() + (color.G8() << 1) + color.B8() + 3) >> 2;
    do
    {
      me->_draw_pixel(xpos, ypos, value);
      if (++xpos > xe)
      {
        xpos = xs;
        if (++ypos > ye)
        {
          ypos = ys;
        }
      }
    } while (--length);
    me->_xpos = xpos;
    me->_ypos = ypos;
//    me->_update_transferred_rect(xs, ys, xe, ye);
  }

  void Panel_GDEW0154M09::writePixels(PanelCommon* panel, LGFX_Device* gfx, int32_t length, pixelcopy_t* param)
  {
    auto me = reinterpret_cast<Panel_GDEW0154M09*>(panel);
    {
      int32_t xs = me->_xs;
      int32_t xe = me->_xe;
      int32_t ys = me->_ys;
      int32_t ye = me->_ye;
      me->_update_transferred_rect(gfx, xs, ys, xe, ye);
    }
    int32_t xs   = me->_xs  ;
    int32_t ys   = me->_ys  ;
    int32_t xe   = me->_xe  ;
    int32_t ye   = me->_ye  ;
    int32_t xpos = me->_xpos;
    int32_t ypos = me->_ypos;

    static constexpr int32_t buflen = 16;
    swap565_t colors[buflen];
    int bufpos = buflen;
    do
    {
      if (bufpos == buflen) {
        param->fp_copy(colors, 0, std::min(length, buflen), param);
        bufpos = 0;
      }
      auto color = colors[bufpos++];
      me->_draw_pixel(xpos, ypos, (color.R8() + (color.G8() << 1) + color.B8() + 3) >> 2);
      if (++xpos > xe)
      {
        xpos = xs;
        if (++ypos > ye)
        {
          ypos = ys;
        }
      }
    } while (--length);
    me->_xpos = xpos;
    me->_ypos = ypos;
//    me->_update_transferred_rect(xs, ys, xe, ye);
  }

  void Panel_GDEW0154M09::readRect(PanelCommon* panel, LGFX_Device*, int32_t x, int32_t y, int32_t w, int32_t h, void* dst, pixelcopy_t* param)
  {
    auto me = reinterpret_cast<Panel_GDEW0154M09*>(panel);

    swap565_t readbuf[w];
    param->src_data = readbuf;
    int32_t readpos = 0;
    h += y;
    do
    {
      int32_t idx = 0;
      do
      {
        readbuf[idx] = me->_read_pixel(x + idx, y) ? ~0u : 0;
      } while (++idx != w);
      param->src_x32 = 0;
      readpos = param->fp_copy(dst, readpos, readpos + w, param);
    } while (++y < h);
  }

  void Panel_GDEW0154M09::_exec_transfer(uint32_t cmd, LGFX_Device* gfx, const range_rect_t& range, bool invert)
  {
    int32_t xs = range.left & ~7;
    int32_t xe = range.right & ~7;

    _wait_busy();

    gfx->writeCommand(0x91);
    gfx->writeCommand(0x90);
    gfx->writeData16(xs << 8 | xe);
    gfx->writeData16(range.top);
    gfx->writeData16(range.bottom);
    gfx->writeData(1);

    _wait_busy();

    gfx->writeCommand(cmd);
    int32_t w = ((xe - xs) >> 3) + 1;
    int32_t y = range.top;
    int32_t add = ((panel_width + 7) & ~7) >> 3;
    auto b = &_buf[xs >> 3];
    if (invert)
    {
      b += y * add;
      do
      {
        int32_t i = 0;
        do
        {
          gfx->writeData(~b[i]);
        } while (++i != w);
        b += add;
      } while (++y <= range.bottom);
    }
    else
    {
      do
      {
        gfx->writeBytes(&b[add * y], w);
      } while (++y <= range.bottom);
    }
    /*
    range->top = INT_MAX;
    range->left = INT_MAX;
    range->right = 0;
    range->bottom = 0;
    //*/
  }

  void Panel_GDEW0154M09::_close_transfer(LGFX_Device* gfx)
  {
    if (_range_old.empty()) { return; }
    while (millis() - _send_msec < 320) delay(1);
    _exec_transfer(0x10, gfx, _range_old);
    _range_old.top = INT_MAX;
    _range_old.left = INT_MAX;
    _range_old.right = 0;
    _range_old.bottom = 0;
    
    gfx->waitDMA();
  }

  bool Panel_GDEW0154M09::displayBusy(PanelCommon* panel, LGFX_Device* gfx)
  {
    auto me = reinterpret_cast<Panel_GDEW0154M09*>(panel);
    return gfx->dmaBusy() || (me->gpio_busy >= 0 && !gpio_in(me->gpio_busy));
  }

  void Panel_GDEW0154M09::display(PanelCommon* panel, LGFX_Device* gfx, int32_t x, int32_t y, int32_t w, int32_t h)
  {
    auto me = reinterpret_cast<Panel_GDEW0154M09*>(panel);
    if (0 < w && 0 < h)
    {
      me->_range_new.left   = std::min(me->_range_new.left  , x        );
      me->_range_new.right  = std::max(me->_range_new.right , x + w - 1);
      me->_range_new.top    = std::min(me->_range_new.top   , y        );
      me->_range_new.bottom = std::max(me->_range_new.bottom, y + h - 1);
    }
    if (me->_range_new.empty()) { return; }
    me->_close_transfer(gfx);
    me->_range_old = me->_range_new;
    while (millis() - me->_send_msec < _refresh_msec) delay(1);
    if (gfx->getEpdMode() == epd_mode_t::epd_quality)
    {
      me->_exec_transfer(0x13, gfx, me->_range_new, true);
      me->_wait_busy();
      gfx->writeCommand(0x12);
      auto send_msec = millis();
      delay(300);
      while (millis() - send_msec < _refresh_msec) delay(1);
      me->_exec_transfer(0x10, gfx, me->_range_new, true);
    }
    me->_exec_transfer(0x13, gfx, me->_range_new);
    me->_range_new.top = INT_MAX;
    me->_range_new.left = INT_MAX;
    me->_range_new.right = 0;
    me->_range_new.bottom = 0;

    me->_wait_busy();
    gfx->writeCommand(0x12);
    me->_send_msec = millis();
  }

  bool Panel_GDEW0154M09::_wait_busy(uint32_t timeout)
  {
    if (gpio_busy >= 0 && !gpio_in(gpio_busy))
    {
      uint32_t start_time = millis();
      while (!gpio_in(gpio_busy))
      {
        if (millis() - start_time > timeout) return false;
        delay(1);
      }
    }
    return true;
  }

  void Panel_GDEW0154M09::waitDisplay(PanelCommon* panel, LGFX_Device* gfx)
  {
    auto me = reinterpret_cast<Panel_GDEW0154M09*>(panel);
    gfx->waitDMA();
    me->_wait_busy();
  }

  void Panel_GDEW0154M09::sleep(LGFX_Device* gfx)
  {
    _wait_busy();
    gfx->startWrite();
    gfx->writeCommand(0x07);
    gfx->writeData(0xA5);
    gfx->endWrite();
  }

  void Panel_GDEW0154M09::wakeup(LGFX_Device* gfx)
  {
    if (gpio_rst >= 0)
    {
      lgfx::gpio_lo(gpio_rst);
      auto time = millis();
      do {
        delay(1);
      } while (millis() - time < 2);
      lgfx::gpio_hi(gpio_rst);
      time = millis();
      do {
        delay(1);
      } while (millis() - time < 10);
    }
    gfx->initPanel(false);
  }
  /*
  void Panel_GDEW0154M09::beginTransaction(PanelCommon* panel, LGFX_Device* gfx)
  {
    auto me = reinterpret_cast<Panel_GDEW0154M09*>(panel);
    me->_close_transfer(gfx);
  }

  void Panel_GDEW0154M09::endTransaction(PanelCommon* panel, LGFX_Device* gfx)
  {
    display(panel, gfx);
  }
  //*/

//----------------------------------------------------------------------------
 }
}
#endif
