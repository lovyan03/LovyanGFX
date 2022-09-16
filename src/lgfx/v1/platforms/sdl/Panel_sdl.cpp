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

Porting for SDL:
 [imliubo](https://github.com/imliubo)
/----------------------------------------------------------------------------*/
#if defined ( LGFX_SDL )

#include "Panel_sdl.hpp"

#include "../common.hpp"
#include "../../Bus.hpp"

#include <list>

namespace lgfx
{
 inline namespace v1
 {

  static std::list<monitor_t*> _list_monitor;

  static monitor_t* const getMonitorByWindowID(uint32_t windowID)
  {
    for (auto& m : _list_monitor)
    {
      if (SDL_GetWindowID(m->window) == windowID) { return m; }
    }
    return nullptr;
  }
//----------------------------------------------------------------------------
  static void memset_multi(uint8_t* buf, uint32_t c, size_t size, size_t length)
  {
    if (size == 1 || ((c & 0xFF) == ((c >> 8) & 0xFF) && (size == 2 || ((c & 0xFF) == ((c >> 16) & 0xFF)))))
    {
      memset(buf, c, size * length);
      return;
    }

    size_t l = length;
    if (l & ~0xF)
    {
      while ((l >>= 1) & ~0xF);
      ++l;
    }
    size_t len = l * size;
    length = (length * size) - len;
    uint8_t* dst = buf;
    if (size == 2) {
      do { // 2byte speed tweak
        *(uint16_t*)dst = c;
        dst += 2;
      } while (--l);
    } else {
      do {
        size_t i = 0;
        do {
          *dst++ = *(((uint8_t*)&c) + i);
        } while (++i != size);
      } while (--l);
    }
    if (!length) return;
    while (length > len) {
      memcpy(dst, buf, len);
      dst += len;
      length -= len;
      len <<= 1;
    }
    if (length) {
      memcpy(dst, buf, length);
    }
  }

  int quit_filter(void * userdata, SDL_Event * event)
  {
    Panel_sdl *sdl = (Panel_sdl *)userdata;

    if(event->type == SDL_WINDOWEVENT) {
      if(event->window.event == SDL_WINDOWEVENT_CLOSE) {
        sdl->sdl_quit();
      }
    }
    else if(event->type == SDL_QUIT) {
      sdl->sdl_quit();
    }

    return 1;
  }

  void Panel_sdl::sdl_update_handler(void)
  {
    SDL_Delay(1);
    for (auto& m : _list_monitor)
    {
      if (m->renderer == nullptr)
      {
        m->panel->sdl_create(m);
      }
      sdl_update(m);
    }
  }

  void Panel_sdl::sdl_event_handler(void)
  {
    sdl_update_handler();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_KEYDOWN)
      {
        switch (event.key.keysym.sym)
        { /// M5StackのBtnA～BtnCのエミュレート;
        case SDLK_LEFT:
          gpio_lo(39);
          break;
        case SDLK_DOWN:
          gpio_lo(38);
          break;
        case SDLK_RIGHT:
          gpio_lo(37);
          break;
        case SDLK_UP:
          gpio_lo(36);
          break;
        }
      }
      else if (event.type == SDL_KEYUP)
        { /// M5StackのBtnA～BtnCのエミュレート;
        switch (event.key.keysym.sym)
        {
        case SDLK_LEFT:
          gpio_hi(39);
          break;
        case SDLK_DOWN:
          gpio_hi(38);
          break;
        case SDLK_RIGHT:
          gpio_hi(37);
          break;
        case SDLK_UP:
          gpio_hi(36);
          break;
        }
      }
      else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP || event.type == SDL_MOUSEMOTION)
      {
        auto mon = getMonitorByWindowID(event.button.windowID);
        if (mon != nullptr)
        {
          SDL_GetMouseState(&mon->touch_x, &mon->touch_y);
          if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
          {
            mon->touched = true;
          }
          if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT)
          {
            mon->touched = false;
          }
        }
      }
      else if (event.type == SDL_WINDOWEVENT) {

        switch((&event)->window.event) {
  #if SDL_VERSION_ATLEAST(2, 0, 5)
          case SDL_WINDOWEVENT_TAKE_FOCUS:
  #endif
          case SDL_WINDOWEVENT_EXPOSED:
          break;
          default:
          break;
        }
      }
    }
  }

  void Panel_sdl::setScaling(uint_fast8_t scaling_x, uint_fast8_t scaling_y)
  {
    monitor.scaling_x = scaling_x;
    monitor.scaling_y = scaling_y;
  }

  Panel_sdl::~Panel_sdl(void)
  {
    _list_monitor.remove(&monitor);
  }

  Panel_sdl::Panel_sdl(void) : Panel_Device()
  {
    monitor.panel = this;
    static bool inited = false;
    if (inited) { return; }
    for (size_t pin = 0; pin < EMULATED_GPIO_MAX; ++pin) { gpio_hi(pin); }

    /*Initialize the SDL*/
    SDL_Init(SDL_INIT_VIDEO);

    SDL_SetEventFilter(quit_filter, this);

    SDL_StartTextInput();
  }

  bool Panel_sdl::init(bool use_reset)
  {
    uint32_t len = _cfg.panel_width * _cfg.panel_height * sizeof(bgr888_t) + 16;
    monitor.tft_fb = (bgr888_t*)malloc(len);
    memset(monitor.tft_fb, 0x44, len);

    _list_monitor.push_back(&monitor);

    return Panel_Device::init(use_reset);
  }

  color_depth_t Panel_sdl::setColorDepth(color_depth_t depth)
  {
    _write_depth = color_depth_t::rgb888_3Byte;
    _read_depth = color_depth_t::rgb888_3Byte;
    return _write_depth;
  }

  void Panel_sdl::setRotation(uint_fast8_t r)
  {
    r &= 7;
    _rotation = r;
    _internal_rotation = ((r + _cfg.offset_rotation) & 3) | ((r & 4) ^ (_cfg.offset_rotation & 4));

    _width  = _cfg.panel_width;
    _height = _cfg.panel_height;
    if (_internal_rotation & 1) std::swap(_width, _height);
  }

  void Panel_sdl::setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye)
  {
    xs = std::max<uint_fast16_t>(0u, std::min<uint_fast16_t>(_width  - 1, xs));
    xe = std::max<uint_fast16_t>(0u, std::min<uint_fast16_t>(_width  - 1, xe));
    ys = std::max<uint_fast16_t>(0u, std::min<uint_fast16_t>(_height - 1, ys));
    ye = std::max<uint_fast16_t>(0u, std::min<uint_fast16_t>(_height - 1, ye));
    _xpos = xs;
    _xs = xs;
    _xe = xe;
    _ypos = ys;
    _ys = ys;
    _ye = ye;
  }

  void Panel_sdl::drawPixelPreclipped(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor)
  {
    uint_fast8_t r = _internal_rotation;
    if (r)
    {
      if ((1u << r) & 0b10010110) { y = _height - (y + 1); }
      if (r & 2)                  { x = _width  - (x + 1); }
      if (r & 1) { std::swap(x, y); }
    }

    size_t bw = _cfg.panel_width;
    size_t index = x + y * bw;
    {
      auto img = &((bgr888_t*)monitor.tft_fb)[index];
      *img = rawcolor;
    }
  }

  void Panel_sdl::writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor)
  {
    uint_fast8_t r = _internal_rotation;
    if (r)
    {
      if ((1u << r) & 0b10010110) { y = _height - (y + h); }
      if (r & 2)                  { x = _width  - (x + w); }
      if (r & 1) { std::swap(x, y);  std::swap(w, h); }
    }

    if (w > 1)
    {
      uint_fast8_t bytes = _write_bits >> 3;
      uint_fast16_t bw = _cfg.panel_width;
      auto* dst = &monitor.tft_fb[(x + y * bw)];
      auto* src = dst;
      uint_fast16_t add_dst = bw;
      uint_fast16_t len = w * bytes;

      if (w != bw)
      {
        dst += add_dst;
      }
      else
      {
        w *= h;
        h = 1;
      }
      memset_multi((uint8_t*)src, rawcolor, bytes, w);
      while (--h)
      {
        memcpy(dst, src, len);
        dst += add_dst;
      }
    }
    else
    {
      size_t bw = _cfg.panel_width;
      size_t index = x + y * bw;
      {
        auto img = &((bgr888_t*)monitor.tft_fb)[index];
        do { *img = rawcolor; img += bw; } while (--h);
      }
    }
  }

  void Panel_sdl::writeBlock(uint32_t rawcolor, uint32_t length)
  {
    do
    {
      uint32_t h = 1;
      auto w = std::min<uint32_t>(length, _xe + 1 - _xpos);
      if (length >= (w << 1) && _xpos == _xs)
      {
        h = std::min<uint32_t>(length / w, _ye + 1 - _ypos);
      }
      writeFillRectPreclipped(_xpos, _ypos, w, h, rawcolor);
      if ((_xpos += w) <= _xe) return;
      _xpos = _xs;
      if (_ye < (_ypos += h)) { _ypos = _ys; }
      length -= w * h;
    } while (length);
  }

  void Panel_sdl::_rotate_pixelcopy(uint_fast16_t& x, uint_fast16_t& y, uint_fast16_t& w, uint_fast16_t& h, pixelcopy_t* param, uint32_t& nextx, uint32_t& nexty)
  {
    uint32_t addx = param->src_x32_add;
    uint32_t addy = param->src_y32_add;
    uint_fast8_t r = _internal_rotation;
    uint_fast8_t bitr = 1u << r;
    if (bitr & 0b10010110) // case 1:2:4:7:
    {
      param->src_y32 += nexty * (h - 1);
      nexty = -(int32_t)nexty;
      y = _height - (y + h);
    }
    if (r & 2)
    {
      param->src_x32 += addx * (w - 1);
      param->src_y32 += addy * (w - 1);
      addx = -(int32_t)addx;
      addy = -(int32_t)addy;
      x = _width  - (x + w);
    }
    if (r & 1)
    {
      std::swap(x, y);
      std::swap(w, h);
      std::swap(nextx, addx);
      std::swap(nexty, addy);
    }
    param->src_x32_add = addx;
    param->src_y32_add = addy;
  }

  void Panel_sdl::writePixels(pixelcopy_t* param, uint32_t length, bool use_dma)
  {
    uint_fast16_t xs = _xs;
    uint_fast16_t xe = _xe;
    uint_fast16_t ys = _ys;
    uint_fast16_t ye = _ye;
    uint_fast16_t x = _xpos;
    uint_fast16_t y = _ypos;
    const size_t bits = _write_bits;
    auto k = _cfg.panel_width * bits >> 3;

    uint_fast8_t r = _internal_rotation;
    if (!r)
    {
      uint_fast16_t linelength;
      do {
        linelength = std::min<uint_fast16_t>(xe - x + 1, length);
        param->fp_copy(&monitor.tft_fb[y * k], x, x + linelength, param);
        if ((x += linelength) > xe)
        {
          x = xs;
          y = (y != ye) ? (y + 1) : ys;
        }
      } while (length -= linelength);
      _xpos = x;
      _ypos = y;
      return;
    }

    int_fast16_t ax = 1;
    int_fast16_t ay = 1;
    if ((1u << r) & 0b10010110) { y = _height - (y + 1); ys = _height - (ys + 1); ye = _height - (ye + 1); ay = -1; }
    if (r & 2)                  { x = _width  - (x + 1); xs = _width  - (xs + 1); xe = _width  - (xe + 1); ax = -1; }
    if (param->no_convert)
    {
      size_t bytes = _write_bits >> 3;
      size_t xw = 1;
      size_t yw = _cfg.panel_width;
      if (r & 1) std::swap(xw, yw);
      size_t idx = y * yw + x * xw;
      auto data = (uint8_t*)param->src_data;
      do
      {
        auto dst = &monitor.tft_fb[idx * bytes];
        size_t b = 0;
        do
        {
          dst[b] = *data++;
        } while (++b < bytes);
        if (x != xe)
        {
          idx += xw * ax;
          x += ax;
        }
        else
        {
          x = xs;
          y = (y != ye) ? (y + ay) : ys;
          idx = y * yw + x * xw;
        }
      } while (--length);
    }
    else
    {
      if (r & 1)
      {
        do
        {
          param->fp_copy(&monitor.tft_fb[x * k], y, y + 1, param);
          if (x != xe)
          {
            x += ax;
          }
          else
          {
            x = xs;
            y = (y != ye) ? (y + ay) : ys;
          }
        } while (--length);
      }
      else
      {
        do
        {
          param->fp_copy(&monitor.tft_fb[y * k], x, x + 1, param);
          if (x != xe)
          {
            x += ax;
          }
          else
          {
            x = xs;
            y = (y != ye) ? (y + ay) : ys;
          }
        } while (--length);
      }
    }
    if ((1u << r) & 0b10010110) { y = _height - (y + 1); }
    if (r & 2)                  { x = _width  - (x + 1); }
    _xpos = x;
    _ypos = y;
  }

  void Panel_sdl::writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool)
  {
    uint_fast8_t r = _internal_rotation;
    if (r == 0 && param->transp == pixelcopy_t::NON_TRANSP && param->no_convert)
    {
      auto sx = param->src_x;
      auto bits = param->src_bits;

      auto bw = _cfg.panel_width * bits >> 3;
      auto dst = (uint8_t*)& monitor.tft_fb[_cfg.panel_width * y];
      auto sw = param->src_bitwidth * bits >> 3;
      auto src = &((uint8_t*)param->src_data)[param->src_y * sw];
      if (sw == bw && this->_cfg.panel_width == w && sx == 0 && x == 0)
      {
        memcpy(dst, src, bw * h);
        return;
      }
      y = 0;
      dst +=  x * bits >> 3;
      src += sx * bits >> 3;
      w    =  w * bits >> 3;
      do
      {
        memcpy(&dst[y * bw], &src[y * sw], w);
      } while (++y != h);
      return;
    }

    uint32_t nextx = 0;
    uint32_t nexty = 1 << pixelcopy_t::FP_SCALE;
    if (r)
    {
      _rotate_pixelcopy(x, y, w, h, param, nextx, nexty);
    }
    uint32_t sx32 = param->src_x32;
    uint32_t sy32 = param->src_y32;

    y *= _cfg.panel_width;
    do
    {
      int32_t pos = x + y;
      int32_t end = pos + w;
      while (end != (pos = param->fp_copy(monitor.tft_fb, pos, end, param))
         &&  end != (pos = param->fp_skip(      pos, end, param)));
      param->src_x32 = (sx32 += nextx);
      param->src_y32 = (sy32 += nexty);
      y += _cfg.panel_width;
    } while (--h);
  }

  void Panel_sdl::writeImageARGB(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param)
  {
    uint32_t nextx = 0;
    uint32_t nexty = 1 << pixelcopy_t::FP_SCALE;
    if (_internal_rotation)
    {
      _rotate_pixelcopy(x, y, w, h, param, nextx, nexty);
    }
    uint32_t sx32 = param->src_x32;
    uint32_t sy32 = param->src_y32;

    uint32_t pos = x + y * _cfg.panel_width;
    uint32_t end = pos + w;
    param->fp_copy(monitor.tft_fb, pos, end, param);
    while (--h)
    {
      pos += _cfg.panel_width;
      end = pos + w;
      param->src_x32 = (sx32 += nextx);
      param->src_y32 = (sy32 += nexty);
      param->fp_copy(monitor.tft_fb, pos, end, param);
    }
  }

  void Panel_sdl::readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param)
  {
    uint_fast8_t r = _internal_rotation;
    if (0 == r && param->no_convert)
    {
      h += y;
      auto bytes = _write_bits >> 3;
      auto bw = _cfg.panel_width;
      auto d = (uint8_t*)dst;
      w *= bytes;
      do {
        memcpy(d, &monitor.tft_fb[(x + y * bw)], w);
        d += w;
      } while (++y != h);
    }
    else
    {
      param->src_bitwidth = _cfg.panel_width;
      param->src_data = monitor.tft_fb;
      uint32_t nextx = 0;
      uint32_t nexty = 1 << pixelcopy_t::FP_SCALE;
      if (r)
      {
        uint32_t addx = param->src_x32_add;
        uint32_t addy = param->src_y32_add;
        uint_fast8_t rb = 1 << r;
        if (rb & 0b10010110) // case 1:2:4:7:
        {
          nexty = -(int32_t)nexty;
          y = _height - (y + 1);
        }
        if (r & 2)
        {
          addx = -(int32_t)addx;
          x = _width - (x + 1);
        }
        if ((r+1) & 2)
        {
          addy  = -(int32_t)addy;
        }
        if (r & 1)
        {
          std::swap(x, y);
          std::swap(addx, addy);
          std::swap(nextx, nexty);
        }
        param->src_x32_add = addx;
        param->src_y32_add = addy;
      }
      size_t dstindex = 0;
      uint32_t x32 = x << pixelcopy_t::FP_SCALE;
      uint32_t y32 = y << pixelcopy_t::FP_SCALE;
      param->src_x32 = x32;
      param->src_y32 = y32;
      do
      {
        param->src_x32 = x32;
        x32 += nextx;
        param->src_y32 = y32;
        y32 += nexty;
        dstindex = param->fp_copy(dst, dstindex, dstindex + w, param);
      } while (--h);
    }
  }

  void Panel_sdl::copyRect(uint_fast16_t dst_x, uint_fast16_t dst_y, uint_fast16_t w, uint_fast16_t h, uint_fast16_t src_x, uint_fast16_t src_y)
  {
    uint_fast8_t r = _internal_rotation;
    if (r)
    {
      if ((1u << r) & 0b10010110) { src_y = _height - (src_y + h); dst_y = _height - (dst_y + h); }
      if (r & 2)                  { src_x = _width  - (src_x + w); dst_x = _width  - (dst_x + w); }
      if (r & 1) { std::swap(src_x, src_y);  std::swap(dst_x, dst_y);  std::swap(w, h); }
    }

    size_t bytes = _write_bits >> 3;
    size_t len = w * bytes;
    int32_t add = _cfg.panel_width * bytes;
    if (src_y < dst_y) add = -add;
    int32_t pos = (src_y < dst_y) ? h - 1 : 0;
    uint8_t* src = (uint8_t*)&monitor.tft_fb[(src_x + (src_y + pos) * _cfg.panel_width)];
    uint8_t* dst = (uint8_t*)&monitor.tft_fb[(dst_x + (dst_y + pos) * _cfg.panel_width)];
    do
    {
      memmove(dst, src, len);
      src += add;
      dst += add;
    } while (--h);
  }

  uint_fast8_t Panel_sdl::getTouchRaw(touch_point_t* tp, uint_fast8_t count)
  {
    tp->x = monitor.touch_x / monitor.scaling_x;
    tp->y = monitor.touch_y / monitor.scaling_y;
    tp->size = monitor.touched ? 1 : 0;
    tp->id = 0;
    return monitor.touched;
  }

  void Panel_sdl::sdl_create(monitor_t * m)
  {
    int flag = 0;
#if SDL_FULLSCREEN
    flag |= SDL_WINDOW_FULLSCREEN;
#endif
    m->panel = this;
    m->window = SDL_CreateWindow("LGFX Simulator",
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              _cfg.panel_width * m->scaling_x, _cfg.panel_height * m->scaling_y, flag);       /*last param. SDL_WINDOW_BORDERLESS to hide borders*/

    m->renderer = SDL_CreateRenderer(m->window, -1, SDL_RENDERER_SOFTWARE);
    m->texture = SDL_CreateTexture(m->renderer,
                                SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, _cfg.panel_width, _cfg.panel_height);
    SDL_SetTextureBlendMode(m->texture, SDL_BLENDMODE_NONE);
  }

  void Panel_sdl::sdl_update(const monitor_t* const m)
  {
    SDL_UpdateTexture(m->texture, NULL, m->tft_fb, m->panel->config().panel_width * sizeof(bgr888_t));

    /*Update the renderer with the texture containing the rendered image*/
    SDL_RenderCopy(m->renderer, m->texture, NULL, NULL);
    SDL_RenderPresent(m->renderer);
  }

  void Panel_sdl::sdl_quit(void)
  {
    SDL_DestroyTexture(monitor.texture);
    SDL_DestroyRenderer(monitor.renderer);
    SDL_DestroyWindow(monitor.window);

    SDL_Quit();
    exit(0);
  }
//----------------------------------------------------------------------------
 }
}

#endif
