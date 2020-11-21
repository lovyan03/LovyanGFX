#include "Panel_M5CoreInk.hpp"
#include "../LGFX_Device.hpp"

namespace lgfx
{
  constexpr std::uint8_t Panel_M5CoreInk::Bayer[16];

  void Panel_M5CoreInk::post_init(LGFX_Device* gfx)
  {
    // init DSRAM
    _tr_top = 0;
    _tr_left = 0;
    _tr_right = panel_width - 1;
    _tr_bottom = panel_height - 1;
    gfx->startWrite();
    _tr_top = 0;
    _tr_left = 0;
    _tr_right = panel_width - 1;
    _tr_bottom = panel_height - 1;
    gfx->endWrite();
    _tr_top = panel_height;
    _tr_left = panel_width;
    _tr_right = 0;
    _tr_bottom = 0;
  }

  void Panel_M5CoreInk::_update_transferred_rect(std::int32_t &xs, std::int32_t &ys, std::int32_t &xe, std::int32_t &ye)
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

    _tr_top = std::min(ys, _tr_top);
    _tr_left = std::min(xs, _tr_left);
    _tr_right = std::max(xe, _tr_right);
    _tr_bottom = std::max(ye, _tr_bottom);
  }

  void Panel_M5CoreInk::fillRect(PanelCommon* panel, LGFX_Device*, std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::uint32_t rawcolor)
  {
    auto me = reinterpret_cast<Panel_M5CoreInk*>(panel);
    std::int32_t xs = x, xe = x + w - 1;
    std::int32_t ys = y, ye = y + h - 1;
    me->_update_transferred_rect(xs, ys, xe, ye);

    swap565_t color;
    color.raw = rawcolor;
    std::uint32_t value = (color.R8() + (color.G8() << 1) + color.B8()) >> 2;

    y = ys;
    do
    {
      x = xs;
      std::uint32_t idx = ((me->panel_width + 7) & ~7) * y + x;
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

  void Panel_M5CoreInk::pushImage(PanelCommon* panel, LGFX_Device*, std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, pixelcopy_t* param)
  {
    auto me = reinterpret_cast<Panel_M5CoreInk*>(panel);
    std::int32_t xs = x, xe = x + w - 1;
    std::int32_t ys = y, ye = y + h - 1;
    me->_update_transferred_rect(xs, ys, xe, ye);

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

  void Panel_M5CoreInk::pushBlock(PanelCommon* panel, LGFX_Device*, std::int32_t length, std::uint32_t rawcolor)
  {
    auto me = reinterpret_cast<Panel_M5CoreInk*>(panel);
    std::int32_t xs   = me->_xs  ;
    std::int32_t ys   = me->_ys  ;
    std::int32_t xe   = me->_xe  ;
    std::int32_t ye   = me->_ye  ;
    std::int32_t xpos = me->_xpos;
    std::int32_t ypos = me->_ypos;

    swap565_t color;
    color.raw = rawcolor;
    std::uint32_t value = (color.R8() + (color.G8() << 1) + color.B8()) >> 2;
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
    me->_update_transferred_rect(xs, ys, xe, ye);
  }

  void Panel_M5CoreInk::writePixels(PanelCommon* panel, LGFX_Device*, std::int32_t length, pixelcopy_t* param)
  {
    auto me = reinterpret_cast<Panel_M5CoreInk*>(panel);
    std::int32_t xs   = me->_xs  ;
    std::int32_t ys   = me->_ys  ;
    std::int32_t xe   = me->_xe  ;
    std::int32_t ye   = me->_ye  ;
    std::int32_t xpos = me->_xpos;
    std::int32_t ypos = me->_ypos;

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
      me->_draw_pixel(xpos, ypos, (color.R8() + (color.G8() << 1) + color.B8()) >> 2);
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
    me->_update_transferred_rect(xs, ys, xe, ye);
  }

  void Panel_M5CoreInk::readRect(PanelCommon* panel, LGFX_Device*, std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, void* dst, pixelcopy_t* param)
  {
    auto me = reinterpret_cast<Panel_M5CoreInk*>(panel);

    swap565_t readbuf[w];
    std::int32_t readpos = 0;
    h += y;
    do
    {
      std::int32_t idx = 0;
      do
      {
        readbuf[idx] = me->_read_pixel(x + idx, y) ? ~0u : 0;
      } while (++idx != w);
      param->src_x32 = 0;
      readpos = param->fp_copy(dst, readpos, readpos + w, param);
    } while (++y < h);
  }

  void Panel_M5CoreInk::_exec_transfer(std::uint32_t cmd, LGFX_Device* gfx)
  {
    std::int32_t xs = _tr_left & ~7;
    std::int32_t xe = _tr_right & ~7;

    gfx->writeCommand(0x91);
    gfx->writeCommand(0x90);
    gfx->writeData(xs);
    gfx->writeData(xe);
    gfx->writeData(_tr_top >> 8);
    gfx->writeData(_tr_top);
    gfx->writeData(_tr_bottom >> 8);
    gfx->writeData(_tr_bottom);
    gfx->writeData(1);

    gfx->writeCommand(cmd);
    std::int32_t w = ((xe - xs) >> 3) + 1;
    std::int32_t y = _tr_top;
    do
    {
      gfx->writeBytes(&_buf[(((panel_width + 7) & ~7) * y + xs) >> 3], w);
    } while (++y <= _tr_bottom);
  }

  void Panel_M5CoreInk::beginTransaction(PanelCommon* panel, LGFX_Device* gfx)
  {
    auto me = reinterpret_cast<Panel_M5CoreInk*>(panel);
    if (me->_tr_left > me->_tr_right || me->_tr_top > me->_tr_bottom) return;
    if (me->gpio_busy >= 0) while (!gpio_in(me->gpio_busy)) delay(1);
    me->_exec_transfer(0x10, gfx);
    me->_tr_top = me->panel_height;
    me->_tr_left = me->panel_width;
    me->_tr_right = 0;
    me->_tr_bottom = 0;
    gfx->waitDMA();
  }

  void Panel_M5CoreInk::endTransaction(PanelCommon* panel, LGFX_Device* gfx)
  {
    auto me = reinterpret_cast<Panel_M5CoreInk*>(panel);
    if (me->_tr_left > me->_tr_right || me->_tr_top > me->_tr_bottom) return;
    me->_exec_transfer(0x13, gfx);
    gfx->writeCommand(0x12);
  }

  void Panel_M5CoreInk::flush(PanelCommon* panel, LGFX_Device* gfx)
  {
    endTransaction(panel, gfx);
    beginTransaction(panel, gfx);
  }
}
