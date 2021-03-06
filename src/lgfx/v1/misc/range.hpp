/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [BSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)

Contributors:
 [ciniml](https://github.com/ciniml)
 [mongonta0716](https://github.com/mongonta0716)
 [tobozo](https://github.com/tobozo)
/----------------------------------------------------------------------------*/
#pragma once

#include <cstdint>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

#pragma pack(push)
#pragma pack(1)
  struct range16_t
  {
    std::int16_t first;
    std::int16_t last;

    std::int_fast16_t length(void) const { return last - first + 1; }
    bool empty(void) const { return last < first; }
    bool intersectsWith(const range16_t& r) const { return (r.first <= last) && (first <= r.last); }
    bool intersectsWith(std::int_fast16_t f, std::int_fast16_t l) const { return (f <= last) && (first <= l); }
  };

  struct rect16_t
  {
    union
    {
      struct
      {
        range16_t horizon;
        range16_t vertical;
      };
      struct
      {
        std::int16_t left;
        std::int16_t right;
        std::int16_t top;
        std::int16_t bottom;
      };
      struct
      {
        std::int16_t l;
        std::int16_t r;
        std::int16_t t;
        std::int16_t b;
      };
    };

    __attribute__ ((always_inline)) inline std::int_fast16_t width(void) const { return right - left + 1; }
    __attribute__ ((always_inline)) inline std::int_fast16_t height(void) const { return bottom - top + 1; }
    __attribute__ ((always_inline)) inline bool empty(void) const { return left > right || top > bottom; }
    __attribute__ ((always_inline)) inline bool contain(std::int_fast16_t x, std::int_fast16_t y) const { return left <= x && x <= right && top <= y && y <= bottom; }
  };
#pragma pack(pop)

//----------------------------------------------------------------------------
 }
}


