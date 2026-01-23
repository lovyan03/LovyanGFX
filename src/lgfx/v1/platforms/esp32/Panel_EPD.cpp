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

#include "Panel_EPD.hpp"
#if SOC_LCD_I80_SUPPORTED && defined (CONFIG_IDF_TARGET_ESP32S3)

#include "lgfx/v1/Bus.hpp"
#include "lgfx/v1/platforms/common.hpp"
#include "lgfx/v1/misc/pixelcopy.hpp"
#include "lgfx/v1/misc/colortype.hpp"

#if __has_include(<esp_cache.h>)
#include <esp_cache.h>
#endif
#if defined (ESP_CACHE_MSYNC_FLAG_DIR_C2M)
__attribute__((weak))
int Cache_WriteBack_Addr(uint32_t addr, uint32_t size)
{
  uintptr_t start = addr & ~127u;
  uintptr_t end = (addr + size + 127u) & ~127u;
  if (start >= end) return 0;
  return esp_cache_msync((void*)start, end - start, ESP_CACHE_MSYNC_FLAG_DIR_C2M | ESP_CACHE_MSYNC_FLAG_TYPE_DATA);
  // auto res = esp_cache_msync((void*)start, end - start, ESP_CACHE_MSYNC_FLAG_DIR_C2M | ESP_CACHE_MSYNC_FLAG_TYPE_DATA);
  // if (res != ESP_OK){
  //   printf("start: %08x, end: %08x\n", start, end);
  // }
  // return res;
}
#define LGFX_USE_CACHE_WRITEBACK_ADDR
#else
#if defined (CONFIG_IDF_TARGET_ESP32S3)
 #if __has_include(<esp32s3/rom/cache.h>)
  #include <esp32s3/rom/cache.h>
  extern int Cache_WriteBack_Addr(uint32_t addr, uint32_t size);
  #define LGFX_USE_CACHE_WRITEBACK_ADDR
 #endif
#endif
#endif

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------
#if defined ( LGFX_USE_CACHE_WRITEBACK_ADDR )
  static void cacheWriteBack(const void* ptr, uint32_t size)
  {
    if (!isEmbeddedMemory(ptr))
    {
      Cache_WriteBack_Addr((uint32_t)ptr, size);
    }
  }
#else
  static inline void cacheWriteBack(const void*, uint32_t) {}
#endif

//----------------------------------------------------------------------------

  static constexpr uint8_t Bayer[16] = { 0, 8, 2,10,12, 4,14, 6, 3,11, 1, 9,15, 7,13, 5 };

#define LUT_MAKE(d0,d1,d2,d3,d4,d5,d6,d7,d8,d9,da,db,dc,dd,de,df) (uint32_t)((d0<< 0)|(d1<< 2)|(d2<< 4)|(d3<< 6)|(d4<< 8)|(d5<<10)|(d6<<12)|(d7<<14)|(d8<<16)|(d9<<18)|(da<<20)|(db<<22)|(dc<<24)|(dd<<26)|(de<<28)|(df<<30))

// LUTの横軸は色の濃さ。左端が 黒、右端が白の16段階のグレースケール。
// LUTの縦軸は時間軸。上から順に下に向かって処理が進んでいく。
// 値の意味は 0 == end of data / 1 == to black / 2 == to white / 3 == no operation
// LUT_MAKE１行あたり 1フレーム分の16階調それぞれの動作が定義される。
  static constexpr const uint32_t lut_quality[] = {
    LUT_MAKE(1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 2, 1, 1, 1, 1, 1),
    LUT_MAKE(2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2),
    LUT_MAKE(2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2),
    LUT_MAKE(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1),
    LUT_MAKE(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1),
    LUT_MAKE(1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1),
    LUT_MAKE(1, 1, 2, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 3),
    LUT_MAKE(1, 1, 1, 1, 1, 2, 1, 1, 2, 2, 1, 2, 1, 2, 2, 2),
    LUT_MAKE(1, 1, 3, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2),
    LUT_MAKE(3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2),
    LUT_MAKE(3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2),
    LUT_MAKE(1, 1, 1, 1, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2),
    LUT_MAKE(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 2, 2, 2, 2),
    LUT_MAKE(3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3),
    LUT_MAKE(3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 3),
    ~0u, ~0u, ~0u, ~0u,
    ~0u, ~0u, ~0u, ~0u,
    ~0u, ~0u, ~0u, ~0u,
    ~0u, ~0u, ~0u, ~0u,
    0u,
  };

  static constexpr const uint32_t lut_text[] = {
    LUT_MAKE(2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1),
    LUT_MAKE(2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1),
    LUT_MAKE(2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1),
    LUT_MAKE(2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 1),
    LUT_MAKE(2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 1),
    LUT_MAKE(1, 2, 2, 1, 1, 1, 1, 1, 3, 3, 1, 1, 3, 3, 1, 2),
    LUT_MAKE(1, 3, 3, 1, 1, 1, 1, 3, 3, 1, 1, 1, 1, 3, 1, 2),
    LUT_MAKE(1, 3, 3, 1, 2, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2),
    LUT_MAKE(3, 1, 3, 2, 2, 2, 1, 1, 1, 2, 2, 1, 1, 1, 2, 3),
    LUT_MAKE(1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2),
    LUT_MAKE(1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2),
    LUT_MAKE(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 2),
    ~0u, ~0u, ~0u,
    ~0u, ~0u, ~0u, ~0u,
    ~0u, ~0u, ~0u, ~0u,
    ~0u, ~0u, ~0u, ~0u,
    ~0u, ~0u, ~0u, ~0u,
    0u,  //  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
  };

  static constexpr const uint32_t lut_fast[] = {
    LUT_MAKE(2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1),
    LUT_MAKE(2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1),
    LUT_MAKE(1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2),
    LUT_MAKE(1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2),
    LUT_MAKE(1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2),
    LUT_MAKE(1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2),
    LUT_MAKE(1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2),
    LUT_MAKE(1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2),
    ~0u,
    0u,
  };

  static constexpr const uint32_t lut_fastest[] = {
    LUT_MAKE(2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1),
    LUT_MAKE(1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2),
    LUT_MAKE(1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2),
    LUT_MAKE(1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2),
    LUT_MAKE(1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2),
    ~0u,
    0u,
  };

  // 消去用LUT 。現在の階調から中間階調付近にシフトさせる。
  // このLUTは単独では使用せず、この後に本来の描画を行う。
  static constexpr const uint32_t lut_eraser[] = {
    LUT_MAKE(2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 1, 1),
    LUT_MAKE(2, 2, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1),
    ~0u,
    0u,
  };

#undef LUT_MAKE
  static constexpr const size_t lut_eraser_step = sizeof(lut_eraser) / sizeof(uint32_t);

  Panel_EPD::Panel_EPD(void)
  {
    _epd_mode = epd_mode_t::epd_quality;
    _auto_display = true;
  }

  Panel_EPD::~Panel_EPD(void)
  {
  }

  color_depth_t Panel_EPD::setColorDepth(color_depth_t depth)
  {
    _write_depth = color_depth_t::grayscale_8bit;
    _read_depth = color_depth_t::grayscale_8bit;
    return depth;
  }

  bool Panel_EPD::init(bool use_reset)
  {
    auto cfg_detail = config_detail();
    if (cfg_detail.lut_quality == nullptr) {
      cfg_detail.lut_quality = lut_quality;
      cfg_detail.lut_quality_step = sizeof(lut_quality) / sizeof(uint32_t);
    }
    if (cfg_detail.lut_text == nullptr) {
      cfg_detail.lut_text = lut_text;
      cfg_detail.lut_text_step = sizeof(lut_text) / sizeof(uint32_t);
    }
    if (cfg_detail.lut_fast == nullptr) {
      cfg_detail.lut_fast = lut_fast;
      cfg_detail.lut_fast_step = sizeof(lut_fast) / sizeof(uint32_t);
    }
    if (cfg_detail.lut_fastest == nullptr) {
      cfg_detail.lut_fastest = lut_fastest;
      cfg_detail.lut_fastest_step = sizeof(lut_fastest) / sizeof(uint32_t);
    }
    config_detail(cfg_detail);

    Panel_HasBuffer::init(use_reset);

    if (init_intenal()) {
      memset(_buf, 0xFF, _cfg.panel_width * _cfg.panel_height / 2);
      _range_mod.top    = INT16_MAX;
      _range_mod.left   = INT16_MAX;
      _range_mod.right  = 0;
      _range_mod.bottom = 0;
      return true;
    }
    return false;
  }


  bool Panel_EPD::init_intenal(void)
  {
    auto memory_w = _cfg.memory_width;
    auto memory_h = _cfg.memory_height;
    auto panel_w = _cfg.panel_width;
    auto panel_h = _cfg.panel_height;
    if (memory_w == 0 || memory_h == 0) { return false; }

    size_t lut_total_step = lut_eraser_step;
    lut_total_step += _config_detail.lut_quality_step;
    lut_total_step += _config_detail.lut_text_step;
    lut_total_step += _config_detail.lut_fast_step;
    lut_total_step += _config_detail.lut_fastest_step;

    // EPD制御用LUT (2ピクセルセット)
    _lut_2pixel = (uint8_t *)heap_caps_malloc(lut_total_step * 256 * sizeof(uint16_t), MALLOC_CAP_DMA);

    // リフレッシュの進行状況付きフレームバッファ (下位8bitはピクセル2個分の16階調値そのまま)
    // 偶数インデクスは処理中のバッファ、奇数インデクスは予約バッファ。処理中のバッファの完了時に予約バッファの値が参照される。
    _step_framebuf = (uint16_t *)heap_caps_aligned_alloc(16, (memory_w * memory_h / 2) * 2 * sizeof(uint16_t), MALLOC_CAP_SPIRAM); // current pixels

    // 面積分のフレームバッファ (1Byte=2pixel)
    _buf = (uint8_t *)heap_caps_aligned_alloc(16, (panel_w * panel_h) / 2, MALLOC_CAP_SPIRAM); // current pixels

    // DMA転送用バッファx2 (1Byte=4pixel)
    const auto dma_len = memory_w / 4 + _config_detail.line_padding;
    _dma_bufs[0] = (uint8_t *)heap_caps_malloc(dma_len, MALLOC_CAP_DMA);
    _dma_bufs[1] = (uint8_t *)heap_caps_malloc(dma_len, MALLOC_CAP_DMA);

    if (!_step_framebuf || !_buf || !_dma_bufs[0] || !_dma_bufs[1] || !_lut_2pixel) {
      if (_buf) { heap_caps_free(_buf); _buf = nullptr; }
      if (_step_framebuf) { heap_caps_free(_step_framebuf); _step_framebuf = nullptr; }
      if (_dma_bufs[0]) { heap_caps_free(_dma_bufs[0]); _dma_bufs[0] = nullptr; }
      if (_dma_bufs[1]) { heap_caps_free(_dma_bufs[1]); _dma_bufs[1] = nullptr; }
      if (_lut_2pixel) { heap_caps_free(_lut_2pixel); _lut_2pixel = nullptr; }

      return false;
    }

    memset(_dma_bufs[0], 0, dma_len);
    memset(_dma_bufs[1], 0, dma_len);

    // グレーの初期値をセット
    memset(_step_framebuf, 0x88, (memory_w * memory_h / 2) * 2 * sizeof(uint16_t));

    auto dst = _lut_2pixel;
    memset(dst, 0x0F, 256);
    size_t lindex = 0;
    for (int epd_mode = 0; epd_mode < 5; ++epd_mode) {
      const uint32_t* lut_src = nullptr;
      size_t lut_step = 0;
      switch (epd_mode) {
        default:                      lut_src = lut_eraser; lut_step = lut_eraser_step; break;
        case epd_mode_t::epd_quality: lut_src = _config_detail.lut_quality; lut_step = _config_detail.lut_quality_step; break;
        case epd_mode_t::epd_text:    lut_src = _config_detail.lut_text;    lut_step = _config_detail.lut_text_step;    break;
        case epd_mode_t::epd_fast:    lut_src = _config_detail.lut_fast;    lut_step = _config_detail.lut_fast_step;    break;
        case epd_mode_t::epd_fastest: lut_src = _config_detail.lut_fastest; lut_step = _config_detail.lut_fastest_step; break;
      }
      if (lut_src == nullptr) { continue; }
      _lut_offset_table[epd_mode] = lindex >> 8;
      _lut_remain_table[epd_mode] = lut_step;
// ESP_LOGV("dbg", "\n\nepd_mode: %d, offset: %d, remain: %d\n", epd_mode, _lut_offset_table[epd_mode], _lut_remain_table[epd_mode]);
// printf("\nepd_mode: %d, offset: %d, remain: %d\n\n", epd_mode, _lut_offset_table[epd_mode], _lut_remain_table[epd_mode]);
      for (int step = 0; step < lut_step; ++step) {
        auto lu = lut_src[0];
        for (int lv = 0; lv < 256; ++lv) {
          dst[lindex] = (((lu >> ((lv >> 4) << 1)) & 3) << 2) + ((lu >> ((lv & 15) << 1)) & 3);
          ++lindex;
        }
        ++lut_src;
      }
    }

    _update_queue_handle = xQueueCreate(8, sizeof(update_data_t));
    auto task_priority = _config_detail.task_priority;
    auto task_pinned_core = _config_detail.task_pinned_core;
    if (task_pinned_core >= portNUM_PROCESSORS)
    {
      task_pinned_core = (xPortGetCoreID() + 1) % portNUM_PROCESSORS;
    }
    xTaskCreatePinnedToCore((TaskFunction_t)task_update, "epd", 4096, this, task_priority, &_task_update_handle, task_pinned_core);
    // タスク側とメイン側の処理CPUコアが異なる場合、PSRAMのキャッシュ同期をしないとフレームバッファが即時反映されない点に注意

    return true;
  }

  void Panel_EPD::beginTransaction(void)
  {
  }

  void Panel_EPD::endTransaction(void)
  {
  }

  void Panel_EPD::waitDisplay(void)
  {
    while (_display_busy) { vTaskDelay(1); }
  }

  bool Panel_EPD::displayBusy(void)
  {
// キュー _update_queue_handle に余裕があるか調べる
    if (_update_queue_handle && uxQueueSpacesAvailable(_update_queue_handle) == 0) {
      return true;
    }
    return false;
  };


  void Panel_EPD::setInvert(bool invert)
  {
    // unimplemented
    _invert = invert;
  }

  void Panel_EPD::setSleep(bool flg)
  {
    getBusEPD()->powerControl(!flg);
  }

  void Panel_EPD::setPowerSave(bool flg)
  {
    getBusEPD()->powerControl(!flg);
  }

  void Panel_EPD::writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor)
  {
    uint_fast16_t xs = x, xe = x + w - 1;
    uint_fast16_t ys = y, ye = y + h - 1;
    _xs = xs;
    _ys = ys;
    _xe = xe;
    _ye = ye;
    _update_transferred_rect(xs, ys, xe, ye);

    uint8_t tile[16];
    bool fast = _epd_mode == epd_mode_t::epd_fast || _epd_mode == epd_mode_t::epd_fastest;
    if (fast) {
      for (int i = 0; i < 16; ++i) {
        tile[i] = (int)(rawcolor + Bayer[i] * 16) < 248 ? 0 : 0xF;
      }
    } else {
      for (int i = 0; i < 16; ++i) {
        tile[i] = std::min(15, std::max(0, ((int)(rawcolor + Bayer[i] - 8)) >> 4));
      }
    }
    y = ys;
    do
    {
      x = xs;
      auto btbl = &tile[(y & 3) << 2];
      auto buf = &_buf[y * ((_cfg.panel_width + 1) >> 1)];
      do
      {
        size_t idx = x >> 1;
        uint_fast8_t shift = (x & 1) ? 0 : 4;
        uint_fast8_t value = btbl[x & 3] << shift;
        buf[idx] = (buf[idx] & (0xF0 >> shift)) | value;
      } while (++x <= xe);
    } while (++y <= ye);
  }

  void Panel_EPD::writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool use_dma)
  {
    uint_fast16_t xs = x, xe = x + w - 1;
    uint_fast16_t ys = y, ye = y + h - 1;
    _update_transferred_rect(xs, ys, xe, ye);

    auto readbuf = (grayscale_t*)alloca(w * sizeof(grayscale_t));
    auto sx = param->src_x32;
    h += y;
    do
    {
      uint32_t prev_pos = 0, new_pos = 0;
      do
      {
        new_pos = param->fp_copy(readbuf, prev_pos, w, param);
        if (new_pos != prev_pos)
        {
          _draw_pixels(x + prev_pos, y, &readbuf[prev_pos], new_pos - prev_pos);
          prev_pos = new_pos;
        }
      } while (w != new_pos && w != (prev_pos = param->fp_skip(new_pos, w, param)));
      param->src_x32 = sx;
      param->src_y++;
    } while (++y < h);
  }

  void Panel_EPD::writePixels(pixelcopy_t* param, uint32_t length, bool use_dma)
  {
    {
      uint_fast16_t xs = _xs;
      uint_fast16_t xe = _xe;
      uint_fast16_t ys = _ys;
      uint_fast16_t ye = _ye;
      _update_transferred_rect(xs, ys, xe, ye);
    }
    uint_fast16_t xs   = _xs  ;
    uint_fast16_t ys   = _ys  ;
    uint_fast16_t xe   = _xe  ;
    uint_fast16_t ye   = _ye  ;
    uint_fast16_t xpos = _xpos;
    uint_fast16_t ypos = _ypos;

    static constexpr uint32_t buflen = 16;
    grayscale_t colors[buflen];
    int bufpos = buflen;

    do
    {
      if (bufpos == buflen) {
        param->fp_copy(colors, 0, std::min(length, buflen), param);
        bufpos = 0;
      }

      uint32_t len = std::min(length, buflen - bufpos);
      len = std::min<uint32_t>(len, 1u + xe - xpos);
      _draw_pixels(xpos, ypos, &colors[bufpos], len);
      bufpos += len;
      length -= len;
      xpos += len;
      if (xpos > xe)
      {
        xpos = xs;
        if (++ypos > ye)
        {
          ypos = ys;
        }
      }
    } while (length);

    _xpos = xpos;
    _ypos = ypos;
  }

  void Panel_EPD::readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* __restrict dst, pixelcopy_t* param)
  {
    auto readbuf = (grayscale_t*)alloca(w * sizeof(grayscale_t));
    param->src_data = readbuf;
    int32_t readpos = 0;
    h += y;
    do
    {
      uint32_t idx = 0;
      do
      {
        readbuf[idx] = _read_pixel(x + idx, y);
      } while (++idx != w);
      param->src_x32 = 0;
      readpos = param->fp_copy(dst, readpos, readpos + w, param);
    } while (++y < h);
  }

  __attribute((optimize("-O3")))
  void Panel_EPD::_draw_pixels(uint_fast16_t x, uint_fast16_t y, const grayscale_t* values, size_t len)
  {
    int ax = 1;
    int ay = 0;
    uint_fast8_t r = _internal_rotation;
    if (r)
    {
      if (r & 1) { std::swap(x, y); std::swap(ax, ay); }
      uint_fast8_t rb = 1 << r;
      if (rb & 0b11000110) { x = _cfg.panel_width  - 1 - x; ax = -ax; } // case 1:2:6:7:
      if (rb & 0b10011100) { y = _cfg.panel_height - 1 - y; ay = -ay; } // case 2:3:4:7:
    }

    const int w = _cfg.panel_width;
    const bool fast = _epd_mode == epd_mode_t::epd_fast || _epd_mode == epd_mode_t::epd_fastest;

    uint8_t* readbuf = (uint8_t*)alloca(len * sizeof(grayscale_t));
    int16_t btbl[4];
    int yy = (y&3)<<2; int xx = x&3;
    for (int i = 0; i < 4; ++i) {
      btbl[i] = Bayer[yy + xx];
      xx = (xx + ax) & 0x03;
      yy = (yy + (ay << 2)) & 0x0C;
    }
    if (fast) {
      for (size_t i = 0; i < len; ++i) {
        int sum = values[i].get();
        int b = btbl[i & 3];
        readbuf[i] = (sum + (b << 4)) < 248 ? 0 : 0xF;
      }
    } else {
      for (size_t i = 0; i < len; ++i) {
        int v = values[i].get();
        int b = btbl[i & 3];
        readbuf[i] = std::min(15, std::max(0, (v + b - 8) >> 4));
      }
    }
    auto buf = _buf;

    if (ax) {
      buf += y * ((w + 1) >> 1);
      uint_fast8_t shift = (x & 1) ? 0 : 4;
      for (size_t i = 0; i < len; ++i) {
        uint_fast8_t value = readbuf[i] << shift;
        buf[x >> 1] = (buf[x >> 1] & (0xF0 >> shift)) | value;
        x += ax;
        shift = 4 - shift;
      }
    } else {
      int add =             (ay * ((w + 1) >> 1));
      int idx = (x  >> 1) + ( y * ((w + 1) >> 1));
      if (x & 1) {
        for (size_t i = 0; i < len; ++i) {
          uint_fast8_t value = readbuf[i];
          buf[idx] = (buf[idx] & 0xF0) | value;
          idx += add;
        }
      } else {
        for (size_t i = 0; i < len; ++i) {
          uint_fast8_t value = readbuf[i];
          buf[idx] = (buf[idx] & 0x0F) | (value << 4);
          idx += add;
        }
      }
    }
  }

  uint8_t Panel_EPD::_read_pixel(uint_fast16_t x, uint_fast16_t y)
  {
    _rotate_pos(x, y);
    size_t idx = (x >> 1) + (y * ((_cfg.panel_width + 1) >> 1));
    return 0x11u * ((x & 1)
         ? (_buf[idx] & 0x0F)
         : (_buf[idx] >> 4))
         ;
  }

  void Panel_EPD::_update_transferred_rect(uint_fast16_t &xs, uint_fast16_t &ys, uint_fast16_t &xe, uint_fast16_t &ye)
  {
    _rotate_pos(xs, ys, xe, ye);
    _range_mod.left   = std::min<int32_t>(xs, _range_mod.left);
    _range_mod.right  = std::max<int32_t>(xe, _range_mod.right);
    _range_mod.top    = std::min<int32_t>(ys, _range_mod.top);
    _range_mod.bottom = std::max<int32_t>(ye, _range_mod.bottom);
  }

  void Panel_EPD::display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h)
  {
    if (0 < w && 0 < h)
    {
      _range_mod.left   = std::min<int16_t>(_range_mod.left  , x        );
      _range_mod.right  = std::max<int16_t>(_range_mod.right , x + w - 1);
      _range_mod.top    = std::min<int16_t>(_range_mod.top   , y        );
      _range_mod.bottom = std::max<int16_t>(_range_mod.bottom, y + h - 1);
    }

    if (_range_mod.empty()) { return; }

    uint_fast16_t xs = (_range_mod.left + _cfg.offset_x) & ~1u;
    uint_fast16_t xe = (_range_mod.right + _cfg.offset_x) & ~1u;
    uint_fast16_t ys = _range_mod.top    + _cfg.offset_y;
    uint_fast16_t ye = _range_mod.bottom + _cfg.offset_y;

    update_data_t upd;
    upd.x = xs;
    upd.w = xe - xs + 2;
    upd.y = ys;
    upd.h = ye - ys + 1;
    upd.mode = _epd_mode;

    _display_busy = true;
    cacheWriteBack(&_buf[y * _cfg.panel_width >> 1], h * _cfg.panel_width >> 1);
    vTaskDelay(1);
    bool res = xQueueSend(_update_queue_handle, &upd, 128 / portTICK_PERIOD_MS) == pdTRUE;
// printf("\nres: %d, xs: %d, xe: %d, ys: %d, ye: %d\n", res, xs, xe, ys, ye);
    if (res)
    {
      _range_mod.top    = INT16_MAX;
      _range_mod.left   = INT16_MAX;
      _range_mod.right  = 0;
      _range_mod.bottom = 0;
    }
  }

#if defined( __XTENSA__ )
  __attribute__((noinline,noclone,optimize("-O3")))
  static bool blit_dmabuf(uint32_t* dst, uint16_t* src, const uint8_t* lut, size_t len)
  {
#define DST "a2"  // a2 == dst
#define SRC "a3"  // a3 == src
#define LUT "a4"  // a4 == lut
                  // a5 == len
#define S_0 "a5"  // pixel section0 value
#define S_1 "a6"  // pixel section1 value
#define S_2 "a7"  // pixel section2 value
#define S_3 "a8"  // pixel section3 value
#define S_4 "a9"  // pixel section4 value
#define S_5 "a10" // pixel section5 value
#define S_6 "a11" // pixel section6 value
#define S_7 "a12" // pixel section7 value
#define X80 "a13" // 0x8000
#define LPX "a14" // lut pixel data
#define BUF "a15" // pixel result (uint32) value
    uint32_t result;
__asm__ __volatile(
    " movi   " LPX ", 0                    \n"  // LPX = 0
    " addmi  " X80 ", " LPX ", -32768      \n"  // X80 = 0x8000
    " loop     a5, BLT_BUFFER_END          \n"  // lenの回数だけループ命令で処理

    " movi   " BUF ", 0                    \n"  // 出力用バッファを0クリア
    " l16si  " S_0 "," SRC ", 0            \n"  // S_0 = src[0]; // 元データを 8セット分 取得
    " l16si  " S_1 "," SRC ", 4            \n"  // S_1 = src[2];
    " l16si  " S_2 "," SRC ", 8            \n"  // S_2 = src[4];
    " l16si  " S_3 "," SRC ", 12           \n"  // S_3 = src[6];
    " l16si  " S_4 "," SRC ", 16           \n"  // S_4 = src[8];
    " l16si  " S_5 "," SRC ", 20           \n"  // S_5 = src[10];
    " l16si  " S_6 "," SRC ", 24           \n"  // S_6 = src[12];
    " l16si  " S_7 "," SRC ", 28           \n"  // S_7 = src[14];

    " bgei   " S_0 ",  0    , BLT_SECTION0 \n"  // データ値が負でない場合は更新処理を行うためジャンプ
    " bgei   " S_1 ",  0    , BLT_SECTION1 \n"
    "BLT_RETURN1:                          \n"
    " bgei   " S_2 ",  0    , BLT_SECTION2 \n"
    "BLT_RETURN2:                          \n"
    " bgei   " S_3 ",  0    , BLT_SECTION3 \n"
    "BLT_RETURN3:                          \n"
    " bgei   " S_4 ",  0    , BLT_SECTION4 \n"
    "BLT_RETURN4:                          \n"
    " bgei   " S_5 ",  0    , BLT_SECTION5 \n"
    "BLT_RETURN5:                          \n"
    " bgei   " S_6 ",  0    , BLT_SECTION6 \n"
    "BLT_RETURN6:                          \n"
    " bgei   " S_7 ",  0    , BLT_SECTION7 \n"
    "BLT_RETURN7:                          \n"
    " s32i   " BUF "," DST ",  0           \n"  // データを出力
    " addi   " SRC "," SRC ",  32          \n"  // 元データのポインタを進める
    " addi   " DST "," DST ",  4           \n"  // 出力先のポインタを進める
    "BLT_BUFFER_END:                       \n"  // ループ終端
    " j        BLT_END                     \n"  // 関数終了

    "BLT_SECTION0:                         \n"
    " add    " LPX "," S_0 "," LUT "       \n"  // LPX = &lut[S_0]
    " l8ui   " LPX "," LPX ", 0            \n"  // LPX = *LPX
    " addmi  " S_0 "," S_0 ", 256          \n"  // S_0 += 256
    " beqz   " LPX ",  BLT_SWITCH0         \n"  // if (LPX == 0) goto BLT_SWITCH0
    " slli   " LPX "," LPX ", 4            \n"  // LPX <<= 4
    " add    " BUF "," BUF "," LPX "       \n"  // buf += LPX
    " s16i   " S_0 "," SRC ", 0            \n"  // src[0] = S_0;
    " blti   " S_1 ",  0    , BLT_RETURN1  \n"

    "BLT_SECTION1:                         \n"
    " add    " LPX "," S_1 "," LUT "       \n"  // LPX = &lut[S_1]
    " l8ui   " LPX "," LPX ", 0            \n"  // LPX = *LPX
    " addmi  " S_1 "," S_1 ", 256          \n"  // S_1 += 256
    " beqz   " LPX ",  BLT_SWITCH1         \n"  // if (LPX == 0) goto BLT_SWITCH1
  //" slli   " LPX "," LPX ", 0            \n"  // LPX <<= 0
    " add    " BUF "," BUF "," LPX "       \n"  // buf += LPX
    " s16i   " S_1 "," SRC ", 4            \n"  // src[2] = S_1;
    " blti   " S_2 ",  0    , BLT_RETURN2  \n"

    "BLT_SECTION2:                         \n"
    " add    " LPX "," S_2 "," LUT "       \n"  // LPX = &lut[S_1]
    " l8ui   " LPX "," LPX ", 0            \n"  // LPX = *LPX
    " addmi  " S_2 "," S_2 ", 256          \n"  // S_2 += 256
    " beqz   " LPX ",  BLT_SWITCH2         \n"  // if (LPX == 0) goto BLT_SWITCH2
    " slli   " LPX "," LPX ", 12           \n"  // LPX <<= 12
    " add    " BUF "," BUF "," LPX "       \n"  // buf += LPX
    " s16i   " S_2 "," SRC ", 8            \n"  // src[4] = S_2;
    " blti   " S_3 ",  0    , BLT_RETURN3  \n"

    "BLT_SECTION3:                         \n"
    " add    " LPX "," S_3 "," LUT "       \n"  // LPX = &lut[S_3]
    " l8ui   " LPX "," LPX ", 0            \n"  // LPX = *LPX
    " addmi  " S_3 "," S_3 ", 256          \n"  // S_3 += 256
    " beqz   " LPX ",  BLT_SWITCH3         \n"  // if (LPX == 0) goto BLT_SWITCH3
    " slli   " LPX "," LPX ", 8            \n"  // LPX <<= 8
    " add    " BUF "," BUF "," LPX "       \n"  // buf += LPX
    " s16i   " S_3 "," SRC ", 12           \n"  // src[6] = S_3;
    " blti   " S_4 ",  0    , BLT_RETURN4  \n"

    "BLT_SECTION4:                         \n"
    " add    " LPX "," S_4 "," LUT "       \n"  // LPX = &lut[S_4]
    " l8ui   " LPX "," LPX ", 0            \n"  // LPX = *LPX
    " addmi  " S_4 "," S_4 ", 256          \n"  // S_4 += 256
    " beqz   " LPX ",  BLT_SWITCH4         \n"  // if (LPX == 0) goto BLT_SWITCH4
    " slli   " LPX "," LPX ", 20           \n"  // LPX <<= 20
    " add    " BUF "," BUF "," LPX "       \n"  // buf += LPX
    " s16i   " S_4 "," SRC ", 16           \n"  // src[8] = S_4;
    " blti   " S_5 ",  0    , BLT_RETURN5  \n"

    "BLT_SECTION5:                         \n"
    " add    " LPX "," S_5 "," LUT "       \n"  // LPX = &lut[S_5]
    " l8ui   " LPX "," LPX ", 0            \n"  // LPX = *LPX
    " addmi  " S_5 "," S_5 ", 256          \n"  // S_5 += 256
    " beqz   " LPX ",  BLT_SWITCH5         \n"  // if (LPX == 0) goto BLT_SWITCH5
    " slli   " LPX "," LPX ", 16           \n"  // LPX <<= 16
    " add    " BUF "," BUF "," LPX "       \n"  // buf += LPX
    " s16i   " S_5 "," SRC ", 20           \n"  // src[10] = S_5;
    " blti   " S_6 ",  0    , BLT_RETURN6  \n"

    "BLT_SECTION6:                         \n"
    " add    " LPX "," S_6 "," LUT "       \n"  // LPX = &lut[S_6]
    " l8ui   " LPX "," LPX ", 0            \n"  // LPX = *LPX
    " addmi  " S_6 "," S_6 ", 256          \n"  // S_6 += 256
    " beqz   " LPX ",  BLT_SWITCH6         \n"  // if (LPX == 0) goto BLT_SWITCH6
    " slli   " LPX "," LPX ", 28           \n"  // LPX <<= 28
    " add    " BUF "," BUF "," LPX "       \n"  // buf += LPX
    " s16i   " S_6 "," SRC ", 24           \n"  // src[12] = S_6;
    " blti   " S_7 ",  0    , BLT_RETURN7  \n"

    "BLT_SECTION7:                         \n"
    " add    " LPX "," S_7 "," LUT "       \n"  // LPX = &lut[S_7]
    " l8ui   " LPX "," LPX ", 0            \n"  // LPX = *LPX
    " addmi  " S_7 "," S_7 ", 256          \n"  // S_7 += 256
    " beqz   " LPX ",  BLT_SWITCH7         \n"  // if (LPX == 0) goto BLT_SWITCH7
    " slli   " LPX "," LPX ", 24           \n"  // LPX <<= 24
    " add    " BUF "," BUF "," LPX "       \n"  // buf += LPX
    " s16i   " S_7 "," SRC ", 28           \n"  // src[14] = S_7;
    " j                       BLT_RETURN7  \n"

    "BLT_SWITCH0:                          \n"
    " l16si  " LPX "," SRC ", 2            \n"  // LPX = src[1];
    " s16i   " LPX "," SRC ", 0            \n"  // src[0] = LPX;
    " or     " LPX "," LPX ", " X80 "      \n"
    " s16i   " LPX "," SRC ", 2            \n"  // src[1] = LPX|0x8000;
    " bgei   " S_1 ",  0    , BLT_SECTION1 \n"
    " j                       BLT_RETURN1  \n"
    "BLT_SWITCH1:                          \n"
    " l16si  " LPX "," SRC ", 6            \n"  // LPX = src[3];
    " s16i   " LPX "," SRC ", 4            \n"  // src[2] = LPX;
    " or     " LPX "," LPX ", " X80 "      \n"
    " s16i   " LPX "," SRC ", 6            \n"  // src[3] = LPX|0x8000;
    " bgei   " S_2 ",  0    , BLT_SECTION2 \n"
    " j                       BLT_RETURN2  \n"
    "BLT_SWITCH2:                          \n"
    " l16si  " LPX "," SRC ", 10           \n"  // LPX = src[5];
    " s16i   " LPX "," SRC ", 8            \n"  // src[4] = LPX;
    " or     " LPX "," LPX ", " X80 "      \n"
    " s16i   " LPX "," SRC ", 10           \n"  // src[5] = LPX|0x8000;
    " bgei   " S_3 ",  0    , BLT_SECTION3 \n"
    " j                       BLT_RETURN3  \n"
    "BLT_SWITCH3:                          \n"
    " l16si  " LPX "," SRC ", 14           \n"  // LPX = src[7];
    " s16i   " LPX "," SRC ", 12           \n"  // src[6] = LPX;
    " or     " LPX "," LPX ", " X80 "      \n"
    " s16i   " LPX "," SRC ", 14           \n"  // src[7] = LPX|0x8000;
    " bgei   " S_4 ",  0    , BLT_SECTION4 \n"
    " j                       BLT_RETURN4  \n"
    "BLT_SWITCH4:                          \n"
    " l16si  " LPX "," SRC ", 18           \n"  // LPX = src[9];
    " s16i   " LPX "," SRC ", 16           \n"  // src[8] = LPX;
    " or     " LPX "," LPX ", " X80 "      \n"
    " s16i   " LPX "," SRC ", 18           \n"  // src[9] = LPX|0x8000;
    " bgei   " S_5 ",  0    , BLT_SECTION5 \n"
    " j                       BLT_RETURN5  \n"
    "BLT_SWITCH5:                          \n"
    " l16si  " LPX "," SRC ", 22           \n"  // LPX = src[11];
    " s16i   " LPX "," SRC ", 20           \n"  // src[10] = LPX;
    " or     " LPX "," LPX ", " X80 "      \n"
    " s16i   " LPX "," SRC ", 22           \n"  // src[11] = LPX|0x8000;
    " bgei   " S_6 ",  0    , BLT_SECTION6 \n"
    " j                       BLT_RETURN6  \n"
    "BLT_SWITCH6:                          \n"
    " l16si  " LPX "," SRC ", 26           \n"  // LPX = src[13];
    " s16i   " LPX "," SRC ", 24           \n"  // src[12] = LPX;
    " or     " LPX "," LPX ", " X80 "      \n"
    " s16i   " LPX "," SRC ", 26           \n"  // src[13] = LPX|0x8000;
    " bgei   " S_7 ",  0    , BLT_SECTION7 \n"
    " j                       BLT_RETURN7  \n"
    "BLT_SWITCH7:                          \n"
    " l16si  " LPX "," SRC ", 30           \n"  // LPX = src[15];
    " s16i   " LPX "," SRC ", 28           \n"  // src[14] = LPX;
    " or     " LPX "," LPX ", " X80 "      \n"
    " s16i   " LPX "," SRC ", 30           \n"  // src[15] = LPX|0x8000;
    " j                       BLT_RETURN7  \n"
  
    "BLT_END:                              \n"
    " mov      %0   ," LPX "               \n"  // 戻り値にLPXを指定する。処理ナシの場合 0 / データ処理が存在した場合 0以外となる
  :"=r"(result)::"a3","a4","a5","a6","a7","a8","a9","a10","a11","a12","a13","a14","a15");
  // ASM側でDSTの値が操作され、EPDに対する更新がある場合は nullptr以外の値になるので、bool化して戻り値とする
  return result;

#undef DST
#undef SRC
#undef LUT
#undef S_0
#undef S_1
#undef S_2
#undef S_3
#undef S_4
#undef S_5
#undef S_6
#undef S_7
#undef X80
#undef LPX
#undef BUF

}
#else
  __attribute((optimize("-O3")))
  static bool blit_dmabuf(uint32_t* dst, uint16_t* src, const uint8_t* lut, size_t len)
  {
    uint32_t tmp = 0;
    while (len--)
    {
      uint32_t buf = 0;

      int_fast16_t s_0 = (int16_t)src[0];
      int_fast16_t s_1 = (int16_t)src[2];
      int_fast16_t s_2 = (int16_t)src[4];
      int_fast16_t s_3 = (int16_t)src[6];
      int_fast16_t s_4 = (int16_t)src[8];
      int_fast16_t s_5 = (int16_t)src[10];
      int_fast16_t s_6 = (int16_t)src[12];
      int_fast16_t s_7 = (int16_t)src[14];
      if (s_0 >= 0) {
        tmp = lut[s_0];
        s_0 += 256;
        buf += tmp << 4;
        if (tmp == 0) { s_0 = src[1]; tmp = s_0 | 0x8000; src[1] = tmp; }
        src[0] = s_0;
      }
      if (s_1 >= 0) {
        tmp = lut[s_1];
        s_1 += 256;
        buf += tmp << 0;
        if (tmp == 0) { s_1 = src[3]; tmp = s_1 | 0x8000; src[3] = tmp; }
        src[2] = s_1;
      }
      if (s_2 >= 0) {
        tmp = lut[s_2];
        s_2 += 256;
        buf += tmp << 12;
        if (tmp == 0) { s_2 = src[5]; tmp = s_2 | 0x8000; src[5] = tmp; }
        src[4] = s_2;
      }
      if (s_3 >= 0) {
        tmp = lut[s_3];
        s_3 += 256;
        buf += tmp << 8;
        if (tmp == 0) { s_3 = src[7]; tmp = s_3 | 0x8000; src[7] = tmp; }
        src[6] = s_3;
      }
      if (s_4 >= 0) {
        tmp = lut[s_4];
        s_4 += 256;
        buf += tmp << 20;
        if (tmp == 0) { s_4 = src[9]; tmp = s_4 | 0x8000; src[9] = tmp; }
        src[8] = s_4;
      }
      if (s_5 >= 0) {
        tmp = lut[s_5];
        s_5 += 256;
        buf += tmp << 16;
        if (tmp == 0) { s_5 = src[11]; tmp = s_5 | 0x8000; src[11] = tmp; }
        src[10] = s_5;
      }
      if (s_6 >= 0) {
        tmp = lut[s_6];
        s_6 += 256;
        buf += tmp << 28;
        if (tmp == 0) { s_6 = src[13]; tmp = s_6 | 0x8000; src[13] = tmp; }
        src[12] = s_6;
      }
      if (s_7 >= 0) {
        tmp = lut[s_7];
        s_7 += 256;
        buf += tmp << 24;
        if (tmp == 0) { s_7 = src[15]; tmp = s_7 | 0x8000; src[15] = tmp; }
        src[14] = s_7;
      }
      dst[0] = buf;
      src += 16;
      dst ++;
    }
    return (tmp != 0);
  }

#endif

  void Panel_EPD::task_update(Panel_EPD* me)
  {
    update_data_t new_data;

    const size_t panel_w = me->_cfg.panel_width;
    const size_t memory_w = me->_cfg.memory_width;
    const size_t write_len = (memory_w / 4) + me->_config_detail.line_padding;

    const int magni_h = (me->_cfg.memory_height > me->_cfg.panel_height)
                      ? (me->_cfg.memory_height / me->_cfg.panel_height)
                      : 1;
    const size_t mh = (me->_cfg.memory_height + magni_h - 1) / magni_h;

    auto bus = me->getBusEPD();

    bool remain = false;

    for (;;) {
      me->_display_busy = remain;
      TickType_t wait_tick = remain ? 0 : portMAX_DELAY;
      if (xQueueReceive(me->_update_queue_handle, &new_data, wait_tick)) {
        me->_display_busy = true;
        uint32_t usec = lgfx::micros();
        for (;;) {
// printf("\n new_data: x:%d y:%d w:%d h:%d \n", new_data.x, new_data.y, new_data.w, new_data.h);
          bool flg_fast = ( new_data.mode == epd_mode_t::epd_fastest)
                       || ( new_data.mode == epd_mode_t::epd_fast   );

          size_t panel_idx = ((new_data.x + new_data.y * panel_w) >> 1);
          size_t memory_idx = ((new_data.x + new_data.y * memory_w) >> 1);
          auto src = &me->_buf[panel_idx];
          auto dst = &me->_step_framebuf[memory_idx*2];
          size_t h = new_data.h;
          uint_fast16_t lut_offset = me->_lut_offset_table[new_data.mode] << 8;

          if (flg_fast) { lut_offset += 0x8000; }

          do {
            size_t w = new_data.w >> 1;
            auto s = src;
            auto d = dst;
            src += panel_w >> 1;
            dst += (memory_w >> 1) * 2;
            if (flg_fast) {
              for (int i = 0; i < w; i += 2) {
                uint_fast16_t s0 = s[0];
                uint_fast16_t d1 = d[1];
                uint_fast16_t s1 = s[1];
                uint_fast16_t d3 = d[3];
                s0 += lut_offset;
                s1 += lut_offset;
                // 既にリクエスト済みの内容と相違がある場合のみ更新
                if (d1 != s0) {
                  // 高速描画の場合は消去処理は行わず直接更新指示する。
                  d[1] = s0;
                  d[0] = s0 - 0x8000;
                }
                if (d3 != s1) {
                  d[3] = s1;
                  d[2] = s1 - 0x8000;
                }
                s += 2;
                d += 4;
              }
            } else
            if (new_data.mode == epd_mode_t::epd_text) {
              uint_fast16_t white = lut_offset | 0x00FF;
              for (int i = 0; i < w; i += 2) {
                uint_fast16_t s0 = s[0];
                uint_fast16_t s1 = s[1];
                uint_fast16_t d1 = d[1];
                uint_fast16_t d3 = d[3];
                s0 += lut_offset;
                s1 += lut_offset;
                d1 &= 0x7FFF;
                d3 &= 0x7FFF;

                // 白以外またはリクエスト済みの内容と相違がある場合に更新
                if (white != d1 || d1 != s0) {
                  uint_fast16_t d0 = d[0];
                  d[1] = s0;
                  // 消去処理を挟んで更新指示する。(元の値の下位8bitのみを使用するとlut_eraser扱いになる)
                  // 既に消去処理動作中の場合は変更しない
                  if (d0 >= (lut_eraser_step << 8)) {
                    d[0] = (uint8_t)d0;
                  }
                }

                // 白以外またはリクエスト済みの内容と相違がある場合に更新
                if (white != d3 || d3 != s1) {
                  uint_fast16_t d2 = d[2];
                  d[3] = s1;
                  if (d2 >= (lut_eraser_step << 8)) {
                    // 消去処理を挟んで更新指示する。(元の値の下位8bitのみを使用するとlut_eraser扱いになる)
                    d[2] = (uint8_t)d2;
                  }
                }
                s += 2;
                d += 4;
              }
            } else {
              for (int i = 0; i < w; i += 2) {
                uint_fast16_t s0 = s[0];
                uint_fast16_t s1 = s[1];
                uint_fast16_t d1 = d[1];
                uint_fast16_t d3 = d[3];
                s0 += lut_offset;
                s1 += lut_offset;

                // 既にリクエスト済みの内容と相違がある場合のみ更新
                if (d1 != s0) {
                  uint_fast16_t d0 = d[0];
                  d[1] = s0;
                  // 消去処理を挟んで更新指示する。(元の値の下位8bitのみを使用するとlut_eraser扱いになる)
                  // 既に消去処理動作中の場合は変更しない
                  if (d0 >= (lut_eraser_step << 8)) {
                    d[0] = (uint8_t)d0;
                  }
                }

                // 既にリクエスト済みの内容と相違がある場合のみ更新
                if (d3 != s1) {
                  uint_fast16_t d2 = d[2];
                  d[3] = s1;
                  if (d2 >= (lut_eraser_step << 8)) {
                    // 消去処理を挟んで更新指示する。(元の値の下位8bitのみを使用するとlut_eraser扱いになる)
                    d[2] = (uint8_t)d2;
                  }
                }
                s += 2;
                d += 4;
              }
            }
          } while (--h);

          if (lgfx::micros() - usec >= 2048) {
            break;
          }

          const update_data_t prev_data = new_data;
          bool result;
          while (true == (result = xQueueReceive(me->_update_queue_handle, &new_data, 0))) {
            if (prev_data != new_data) { break; }
          }
          if (result == false) { break; }
        }
      }

      bus->powerControl(true);

      int w = (memory_w + 15) >> 4;
      remain = false;
      for (uint_fast16_t y = 0; y < mh; y++) {
        auto dma_buf = (uint32_t*)(me->_dma_bufs[y & 1]);
        if (blit_dmabuf(dma_buf, &me->_step_framebuf[(y * memory_w >> 1) * 2], me->_lut_2pixel, w)) {
          remain = true;
        }
        for (int m = 0; m < magni_h; ++m) {
          if (y == 0 && m == 0) {
            bus->beginTransaction();
          } else {
            bus->scanlineDone();
          }
          bus->writeScanLine((const uint8_t*)dma_buf, write_len);
        }
      }

      vTaskDelay(1);
      bus->scanlineDone();
      bus->endTransaction();

      if (remain == false) {
        bus->powerControl(false);
      }
    }
  }

//----------------------------------------------------------------------------
 }
}

#endif
