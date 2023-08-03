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
#include "Panel_sdl.hpp"

#if defined ( SDL_h_ )

#include "../common.hpp"
#include "../../misc/common_function.hpp"
#include "../../Bus.hpp"

#include <list>

namespace lgfx
{
 inline namespace v1
 {
  static SDL_semaphore *_update_in_semaphore = nullptr;
  static SDL_semaphore *_update_out_semaphore = nullptr;
  volatile static uint32_t _in_step_exec = 0;
  volatile static uint32_t _msec_step_exec = 512;
  static bool _inited = false;
  static bool _all_close = false;

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

  void Panel_sdl::_event_proc(void)
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      if ((event.type == SDL_KEYDOWN) || (event.type == SDL_KEYUP))
      {
        int gpio = -1;
        switch (event.key.keysym.sym)
        { /// M5StackのBtnA～BtnCのエミュレート;
        case SDLK_LEFT:  gpio = 39; break;
        case SDLK_DOWN:  gpio = 38; break;
        case SDLK_RIGHT: gpio = 37; break;
        case SDLK_UP:    gpio = 36; break;
        default: continue;
        }
        if (event.type == SDL_KEYDOWN) {
          gpio_lo(gpio);
        } else {
          gpio_hi(gpio);
        }
      }
      else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP || event.type == SDL_MOUSEMOTION)
      {
        auto mon = getMonitorByWindowID(event.button.windowID);
        if (mon != nullptr)
        {
          int x, y, w, h;
          SDL_GetWindowSize(mon->window, &w, &h);
          SDL_GetMouseState(&x, &y);
          mon->touch_x = x * mon->panel->config().panel_width / w;
          mon->touch_y = y * mon->panel->config().panel_height / h;
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
      else
      if (event.type == SDL_WINDOWEVENT
      || event.type == SDL_QUIT) {
        auto monitor = getMonitorByWindowID(event.window.windowID);
        if (monitor) {
          if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
            monitor->panel->sdl_invalidate();
          }
          else
          if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
            monitor->closing = true;
          }
        }
      }
    }
  }

  /// デバッガでステップ実行されていることを検出するスレッド用関数。
  static int detectDebugger(bool* running)
  {
    uint32_t prev_ms = SDL_GetTicks();
    do {
      SDL_Delay(1);
      uint32_t ms = SDL_GetTicks();
      /// 時間間隔が広すぎる場合はステップ実行中 (ブレークポイントで止まった)と判断する。
      /// また、解除されたと判断した後も1023msecほど状態を維持する。
      if (ms - prev_ms > 64) { _in_step_exec = _msec_step_exec; }
      else if (_in_step_exec) { --_in_step_exec; }
      prev_ms = ms;
    } while (*running);
    return 0;
  }

  void Panel_sdl::_update_proc(void)
  {
    for (auto it = _list_monitor.begin(); it != _list_monitor.end(); )
    {
      if ((*it)->closing) {
        SDL_DestroyTexture((*it)->texture);
        SDL_DestroyRenderer((*it)->renderer);
        SDL_DestroyWindow((*it)->window);
        _list_monitor.erase(it++);
        if (_list_monitor.empty()) {
          _all_close = true;
          return;
        }
        continue;
      }
      (*it)->panel->sdl_update();
      ++it;
    }
  }

  int Panel_sdl::setup(void)
  {
    if (_inited) return 1;
    _inited = true;

    SDL_CreateThread((SDL_ThreadFunction)detectDebugger, "dbg", &_inited);

    _update_in_semaphore = SDL_CreateSemaphore(0);
    _update_out_semaphore = SDL_CreateSemaphore(0);
    for (size_t pin = 0; pin < EMULATED_GPIO_MAX; ++pin) { gpio_hi(pin); }
    /*Initialize the SDL*/
    SDL_Init(SDL_INIT_VIDEO);
    SDL_StartTextInput();

    // SDL_SetThreadPriority(SDL_ThreadPriority::SDL_THREAD_PRIORITY_HIGH);
    return 0;
  }

  int Panel_sdl::loop(void)
  {
    if (!_inited) return 1;

    _event_proc();
    SDL_SemWaitTimeout(_update_in_semaphore, 1);
    _update_proc();
    _event_proc();
    if (SDL_SemValue(_update_out_semaphore) == 0)
    {
      SDL_SemPost(_update_out_semaphore);
    }

    return _all_close;
  }

  int Panel_sdl::close(void)
  {
    if (!_inited) return 1;
    _inited = false;

    SDL_StopTextInput();
    SDL_DestroySemaphore(_update_in_semaphore);
    SDL_DestroySemaphore(_update_out_semaphore);
    SDL_Quit();
    return 0;
  }

  int Panel_sdl::main(int(*fn)(bool*), uint32_t msec_step_exec)
  {
    _msec_step_exec = msec_step_exec;

    /// SDLの準備
    if (0 != Panel_sdl::setup()) { return 1; }

    /// ユーザコード関数の動作・停止フラグ
    bool running = true;

    /// ユーザコード関数を起動する
    auto thread = SDL_CreateThread((SDL_ThreadFunction)fn, "fn", &running);

    /// 全部のウィンドウが閉じられるまでSDLのイベント・描画処理を継続
    while (0 == Panel_sdl::loop()) {};

    /// ユーザコード関数を終了する
    running = false;
    SDL_WaitThread(thread, nullptr);

    /// SDLを終了する
    return Panel_sdl::close();
  }

  void Panel_sdl::setScaling(uint_fast8_t scaling_x, uint_fast8_t scaling_y)
  {
    monitor.scaling_x = scaling_x;
    monitor.scaling_y = scaling_y;
  }

  Panel_sdl::~Panel_sdl(void)
  {
    _list_monitor.remove(&monitor);
    SDL_DestroyMutex(_sdl_mutex);
  }

  Panel_sdl::Panel_sdl(void) : Panel_FrameBufferBase()
  {
    _sdl_mutex = SDL_CreateMutex();
    _auto_display = true;
    monitor.panel = this;
  }

  bool Panel_sdl::init(bool use_reset)
  {
    initFrameBuffer(_cfg.panel_width * 4, _cfg.panel_height);
    bool res = Panel_FrameBufferBase::init(use_reset);

    _list_monitor.push_back(&monitor);

    return res;
  }

  color_depth_t Panel_sdl::setColorDepth(color_depth_t depth)
  {
    auto bits = depth & color_depth_t::bit_mask;
    if (bits >= 16) {
      depth = (bits > 16)
            ? rgb888_3Byte
            : rgb565_2Byte;
    } else {
      depth = (depth == color_depth_t::grayscale_8bit)
            ? grayscale_8bit
            : rgb332_1Byte;
    }
    _write_depth = depth;
    _read_depth = depth;

    return depth;
  }

  Panel_sdl::lock_t::lock_t(Panel_sdl* parent)
  : _parent { parent }
  {
    SDL_LockMutex(parent->_sdl_mutex);
  };

  Panel_sdl::lock_t::~lock_t(void)
  {
    ++_parent->_modified_counter;
    SDL_UnlockMutex(_parent->_sdl_mutex);
    if (SDL_SemValue(_update_in_semaphore) < 2)
    {
      SDL_SemPost(_update_in_semaphore);
      if (!_in_step_exec) {
        SDL_SemWaitTimeout(_update_out_semaphore, 1);
      }
    }
  };

  void Panel_sdl::drawPixelPreclipped(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor)
  {
    lock_t lock(this);
    Panel_FrameBufferBase::drawPixelPreclipped(x, y, rawcolor);
  }

  void Panel_sdl::writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor)
  {
    lock_t lock(this);
    Panel_FrameBufferBase::writeFillRectPreclipped(x, y, w, h, rawcolor);
  }

  void Panel_sdl::writeBlock(uint32_t rawcolor, uint32_t length)
  {
//    lock_t lock(this);
    Panel_FrameBufferBase::writeBlock(rawcolor, length);
  }

  void Panel_sdl::writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool use_dma)
  {
    lock_t lock(this);
    Panel_FrameBufferBase::writeImage(x, y, w, h, param, use_dma);
  }

  void Panel_sdl::writeImageARGB(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param)
  {
    lock_t lock(this);
    Panel_FrameBufferBase::writeImageARGB(x, y, w, h, param);
  }

  void Panel_sdl::writePixels(pixelcopy_t* param, uint32_t len, bool use_dma)
  {
    lock_t lock(this);
    Panel_FrameBufferBase::writePixels(param, len, use_dma);
  }

  void Panel_sdl::display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h)
  {
    if (_in_step_exec)
    {
      if (_display_counter != _modified_counter) {
        do {
          SDL_SemPost(_update_in_semaphore);
          SDL_SemWaitTimeout(_update_out_semaphore, 1);
        } while (_display_counter != _modified_counter);
        SDL_Delay(1);
      }
    }
  }

  uint_fast8_t Panel_sdl::getTouchRaw(touch_point_t* tp, uint_fast8_t count)
  {
    tp->x = monitor.touch_x;
    tp->y = monitor.touch_y;
    tp->size = monitor.touched ? 1 : 0;
    tp->id = 0;
    return monitor.touched;
  }

  void Panel_sdl::setWindowTitle(const char* title)
  {
    _window_title = title;
    if (monitor.window) {
      SDL_SetWindowTitle(monitor.window, _window_title);
    }
  }

  void Panel_sdl::sdl_create(monitor_t * m)
  {
    int flag = SDL_WINDOW_RESIZABLE;
#if SDL_FULLSCREEN
    flag |= SDL_WINDOW_FULLSCREEN;
#endif
    m->window = SDL_CreateWindow(_window_title,
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              _cfg.panel_width * m->scaling_x, _cfg.panel_height * m->scaling_y, flag);       /*last param. SDL_WINDOW_BORDERLESS to hide borders*/

    m->renderer = SDL_CreateRenderer(m->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    m->texture = SDL_CreateTexture(m->renderer, SDL_PIXELFORMAT_RGB24,
                     SDL_TEXTUREACCESS_STATIC, _cfg.panel_width, _cfg.panel_height);
    SDL_SetTextureBlendMode(m->texture, SDL_BLENDMODE_NONE);
  }

  void Panel_sdl::sdl_update(void)
  {
    if (monitor.renderer == nullptr)
    {
      sdl_create(&monitor);
    }

    bool step_exec = _in_step_exec;

    if (_texupdate_counter != _modified_counter) {
      pixelcopy_t pc(nullptr, color_depth_t::rgb888_3Byte, _write_depth, false);
      if (_write_depth == rgb565_2Byte) {
        pc.fp_copy = pixelcopy_t::copy_rgb_fast<bgr888_t, swap565_t>;
      } else if (_write_depth == rgb888_3Byte) {
        pc.fp_copy = pixelcopy_t::copy_rgb_fast<bgr888_t, bgr888_t>;
      } else if (_write_depth == rgb332_1Byte) {
        pc.fp_copy = pixelcopy_t::copy_rgb_fast<bgr888_t, rgb332_t>;
      } else if (_write_depth == grayscale_8bit) {
        pc.fp_copy = pixelcopy_t::copy_rgb_fast<bgr888_t, grayscale_t>;
      }

      if (0 == SDL_LockMutex(_sdl_mutex))
      {
        _texupdate_counter = _modified_counter;
        for (int y = 0; y < _cfg.panel_height; ++y)
        {
          pc.src_x32 = 0;
          pc.src_data = _lines_buffer[y];
          pc.fp_copy(&_texturebuf[y * _cfg.panel_width], 0, _cfg.panel_width, &pc);
        }
        SDL_UnlockMutex(_sdl_mutex);
        SDL_UpdateTexture(monitor.texture, nullptr, _texturebuf, _cfg.panel_width * sizeof(rgb888_t));
      }
    }

    if (_invalidated || (_display_counter != _texupdate_counter))
    {
      SDL_RendererInfo info;
      if (0 == SDL_GetRendererInfo(monitor.renderer, &info)) {
        // ステップ実行中はVSYNCを待機しない
        if (((bool)(info.flags & SDL_RENDERER_PRESENTVSYNC)) == step_exec)
        {
          SDL_RenderSetVSync(monitor.renderer, !step_exec);
        }
      }
      SDL_RenderCopy(monitor.renderer, monitor.texture, nullptr, nullptr);
      SDL_RenderPresent(monitor.renderer);
      _display_counter = _texupdate_counter;
      if (_invalidated) {
        _invalidated = false;
        SDL_RenderCopy(monitor.renderer, monitor.texture, nullptr, nullptr);
        SDL_RenderPresent(monitor.renderer);
      }
    }
  }

  bool Panel_sdl::initFrameBuffer(size_t width, size_t height)
  {
    uint8_t** lineArray = (uint8_t**)heap_alloc_dma(height * sizeof(uint8_t*));
    if ( nullptr == lineArray ) { return false; }

    _texturebuf = (rgb888_t*)heap_alloc_dma(width * height * sizeof(rgb888_t));

    /// 8byte alignment;
    width = (width + 7) & ~7u;

    _lines_buffer = lineArray;
    memset(lineArray, 0, height * sizeof(uint8_t*));

    uint8_t* framebuffer = (uint8_t*)heap_alloc_dma(width * height + 16);

    auto fb = framebuffer;
    {
      for (int y = 0; y < height; ++y)
      {
        lineArray[y] = fb;
        fb += width;
      }
    }
    return true;
  }

  void Panel_sdl::deinitFrameBuffer(void)
  {
    auto lines = _lines_buffer;
    _lines_buffer = nullptr;
    if (lines != nullptr)
    {
      heap_free(lines[0]);
      heap_free(lines);
    }
    if (_texturebuf) {
      heap_free(_texturebuf);
      _texturebuf = nullptr;
    }
  }

//----------------------------------------------------------------------------
 }
}

#endif
