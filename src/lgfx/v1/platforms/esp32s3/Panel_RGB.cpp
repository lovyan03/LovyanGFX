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

#include "Panel_RGB.hpp"
#include "../../Bus.hpp"
#include "../common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  Panel_RGB::Panel_RGB(void)
  {
    _write_depth = color_depth_t::rgb565_2Byte;
    _read_depth = color_depth_t::rgb565_2Byte;
    // _write_depth = color_depth_t::rgb332_1Byte;
    // _read_depth = color_depth_t::rgb332_1Byte;
  }

  Panel_RGB::~Panel_RGB(void)
  {
    deinitFrameBuffer();
  }

  bool Panel_RGB::init(bool use_reset)
  {
    if (!Panel_FrameBufferBase::init(use_reset)) { return false; }

    auto h = _cfg.panel_height;

    auto frame_buffer_ = _bus->getDMABuffer(0);
    size_t lineArray_size = h * sizeof(void*);
    uint8_t** lineArray = (uint8_t**)heap_alloc_dma(lineArray_size);

    if (lineArray)
    {
      _lines_buffer = lineArray;
      memset(lineArray, 0, lineArray_size);

      uint8_t bits = _write_bits;
      int w = (_cfg.panel_width + 3) & ~3;
      if (frame_buffer_) {
        auto fb = frame_buffer_;
        for (int i = 0; i < h; ++i) {
          lineArray[i] = fb;
          fb += w * bits >> 3;
        }
        return true;
      }
      heap_free(lineArray);
    }

    return true;
  }
/*
  static inline uint8_t* sub_heap_alloc(bool flg_psram, size_t size)
  {
    uint8_t* res = nullptr;
    if (flg_psram) { res = (uint8_t*)heap_alloc_psram(size); }
    if (res == nullptr)
    {
      res = (uint8_t*)heap_alloc_dma(size);
    }
    if (res) { memset(res, 0, size); }
    return (uint8_t*)res;
  }
*/
  bool Panel_RGB::initFrameBuffer(uint_fast16_t w, uint_fast16_t h, color_depth_t depth, uint8_t chunk_lines, uint8_t use_psram)
  {
    size_t lineArray_size = h * sizeof(void*);
// ESP_LOGE("DEBUG","height:%d", h);
    uint8_t** lineArray = (uint8_t**)heap_alloc_dma(lineArray_size);
    if (lineArray)
    {
      memset(lineArray, 0, lineArray_size);

      uint8_t bits = (depth & color_depth_t::bit_mask);
      w = (w + 3) & ~3;
// 暫定実装。画面全体のバッファを一括で確保する。
// ToDo : 分割確保
      _frame_buffer = (uint8_t*)heap_alloc_psram((w * bits >> 3) * h);
      if (_frame_buffer) {
        _lines_buffer = lineArray;
        auto fb = _frame_buffer;
        for (int i = 0; i < h; ++i) {
          lineArray[i] = fb;
          fb += w * bits >> 3;
        }
        return true;
      }
      heap_free(lineArray);
    }
    return false;
  }

  void Panel_RGB::deinitFrameBuffer(void)
  {
    if (_frame_buffer)
    {
      heap_free(_frame_buffer);
      _frame_buffer = nullptr;
    }

    if (_lines_buffer)
    {
      heap_free(_lines_buffer);
      _lines_buffer = nullptr;
    }
  }

//----------------------------------------------------------------------------
 }
}
