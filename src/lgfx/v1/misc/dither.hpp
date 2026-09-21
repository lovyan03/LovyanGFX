/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)
/----------------------------------------------------------------------------*/
#pragma once

#include <stdint.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  // 4x4 ordered dither thresholds, row-major: index = (x & 3) | (y & 3) << 2.
  // An 8-bit gray value v is set when 256 <= v + bayer_4x4[index].
  static constexpr uint8_t bayer_4x4[16] = { 8, 136, 40, 168, 200, 72, 232, 104, 56, 184, 24, 152, 248, 120, 216, 88 };

  // Same thresholds in a different arrangement (mid gray becomes columns instead of a
  // checkerboard). The GDEW0154 e-paper panels showed display defects with the
  // checkerboard arrangement, so the panels that use this one must keep it.
  static constexpr uint8_t bayer_4x4_alt[16] = { 8, 200, 40, 232, 72, 136, 104, 168, 56, 248, 24, 216, 120, 184, 88, 152 };

  // The same two tables as (threshold - 128) / 4, added to an 8-bit gray value before
  // it is reduced to 2 or 4 bits.
  static constexpr int8_t bayer_4x4_signed[16]     = { -30, 2, -22, 10, 18, -14, 26, -6, -18, 14, -26, 6, 30, -2, 22, -10 };
  static constexpr int8_t bayer_4x4_alt_signed[16] = { -30, 18, -22, 26, -14, 2, -6, 10, -18, 30, -26, 22, -2, 14, -10, 6 };

  // 8-bit gray from 8-bit RGB: gamma 2.0 and ITU-R BT.601 luma.
  inline uint32_t to_gray8(uint8_t r, uint8_t g, uint8_t b)
  {
    return (uint32_t)
          ( (r * r * 19749)    // R 0.299
          + (g * g * 38771)    // G 0.587
          + (b * b *  7530)    // B 0.114
          ) >> 24;
  }

//----------------------------------------------------------------------------
 }
}
