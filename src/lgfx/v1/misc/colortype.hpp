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

#include <stdint.h>
#include <type_traits>

#if defined ( ARDUINO ) && !defined ( pgm_read_byte )
 #if __has_include(<pgmspace.h>)
  #include <pgmspace.h>
 #elif __has_include(<avr/pgmspace.h>) || defined(__AVR__)
  #include <avr/pgmspace.h>
 #else
  #include <Arduino.h>
 #endif
#endif
#if !defined ( pgm_read_byte )
 #define pgm_read_byte(addr)  (*(const uint8_t  *)((uintptr_t)addr))
 #define pgm_read_word(addr)  (*(const uint16_t *)((uintptr_t)addr))
 #define pgm_read_dword(addr) (*(const uint32_t *)((uintptr_t)addr))
#endif

/// for  not ESP8266
#if !defined ( pgm_read_dword_with_offset )
 #if defined (__SAMD21__)
  #define pgm_read_dword_unaligned(addr) (uint32_t) \
  ( *(const uint8_t *)((uintptr_t)addr) \
  | *(const uint8_t *)((uintptr_t)addr+1) << 8  \
  | *(const uint8_t *)((uintptr_t)addr+2) << 16 \
  | *(const uint8_t *)((uintptr_t)addr+3) << 24 )
 #else
  #define pgm_read_dword_unaligned(addr) (*(const uint32_t *)((uintptr_t)addr))
 #endif
#endif

#include "enum.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

#define LGFX_INLINE __attribute__((used)) __attribute__ ((always_inline)) inline static constexpr

  LGFX_INLINE uint8_t  color332(uint8_t r, uint8_t g, uint8_t b) { return (r >> 5) << 5 | (g >> 5) << 2 | b >> 6; }
  LGFX_INLINE uint16_t color565(uint8_t r, uint8_t g, uint8_t b) { return (r >> 3) <<11 | (g >> 2) << 5 | b >> 3; }
  LGFX_INLINE uint32_t color888(uint8_t r, uint8_t g, uint8_t b) { return  r << 16 | g << 8 | b; }
  LGFX_INLINE uint16_t swap565( uint8_t r, uint8_t g, uint8_t b) { return (((r >> 3) << 3) + (g >> 5)) | (((g >> 2) << 5) | (b >> 3)) << 8; }
  LGFX_INLINE uint32_t swap888( uint8_t r, uint8_t g, uint8_t b) { return b << 16 | g << 8 | r; }
  LGFX_INLINE uint16_t getSwap16(uint16_t c) { return __builtin_bswap16(c); }
  LGFX_INLINE uint32_t getSwap24(uint32_t c) { return ((uint8_t)c)<<16 | ((uint8_t)(c>>8))<<8 | (uint8_t)(c>>16); }

  LGFX_INLINE uint32_t convert_bgr888_to_bgra8888(uint32_t c) { return c << 8 | 0xFF; }
  LGFX_INLINE uint32_t convert_bgr888_to_argb8888(uint32_t c) { return getSwap24(c) | 0xFF << 24;  }
  LGFX_INLINE uint32_t convert_bgr888_to_rgb888( uint32_t c) { return getSwap24(c);  }
  LGFX_INLINE uint32_t convert_bgr888_to_rgb565( uint32_t c) { return  (((uint8_t)c) >> 3) << 11 | (((uint16_t)c)>>10)<<5 | c>>19; }
  LGFX_INLINE uint32_t convert_bgr888_to_swap565(uint32_t c) { return  (((uint8_t)c) >> 3) << 3 |  ((uint16_t)c) >> 13 | (c & 0x1C00) << 3 | (c>>19) << 8; }
  LGFX_INLINE uint32_t convert_bgr888_to_bgr666 (uint32_t c) { return (c>>2) & 0x3F3F3F;  }
  LGFX_INLINE uint32_t convert_bgr888_to_rgb332 (uint32_t c) { return ((uint8_t)c >> 5) << 5 | (((uint16_t)c)>>13) << 2 | c>>22; }

  LGFX_INLINE uint32_t convert_rgb888_to_bgra8888(uint32_t c) { return __builtin_bswap32(c) | 0xFF; }
  LGFX_INLINE uint32_t convert_rgb888_to_argb8888(uint32_t c) { return c | 0xFF << 24; }
  LGFX_INLINE uint32_t convert_rgb888_to_bgr666 (uint32_t c) { return ((c>>2) & 0x3F) << 16 | ((c >> 10) & 0x3F) << 8 | ((c>>18)&0x3F);  }
  LGFX_INLINE uint32_t convert_rgb888_to_rgb565 (uint32_t c) { return  (c>>19) << 11 | (((uint16_t)c)>>10)<<5 | ((uint8_t)c) >> 3;   }
  LGFX_INLINE uint32_t convert_rgb888_to_bgr888 (uint32_t c) { return getSwap24(c);  }
  LGFX_INLINE uint32_t convert_rgb888_to_swap565(uint32_t c) { return  (c>>19) << 3 |  ((uint16_t)c) >> 13 | (c & 0x1C00) << 3 | (((uint8_t)c) >> 3) << 8; }
  LGFX_INLINE uint32_t convert_rgb888_to_rgb332 (uint32_t c) { return ((c>>21) << 5) | ((((uint16_t)c)>>13) << 2) | ((c>>6) & 3); }

  LGFX_INLINE uint32_t convert_rgb565_to_bgra8888(uint32_t c) { return (((((c&0x1F)*0x21)>>2)<<8 | ((((c>>5)&0x3F)*0x41)>>4))<<8 | (((c>>11)*0x21)>>2)) << 8 | 0xFF; }
  LGFX_INLINE uint32_t convert_rgb565_to_argb8888(uint32_t c) { return ((((c>>11)*0x21)>>2)<<8 | ((((c>>5)&0x3F)*0x41)>>4))<<8 | (((c&0x1F)*0x21)>>2) | 0xFF << 24; }
  LGFX_INLINE uint32_t convert_rgb565_to_rgb888( uint32_t c) { return ((((c>>11)*0x21)>>2)<<8 | ((((c>>5)&0x3F)*0x41)>>4))<<8 | (((c&0x1F)*0x21)>>2); }
  LGFX_INLINE uint32_t convert_rgb565_to_bgr888 (uint32_t c) { return ((((c&0x1F)*0x21)>>2)<<8 | ((((c>>5)&0x3F)*0x41)>>4))<<8 | (((c>>11)*0x21)>>2); }
  LGFX_INLINE uint32_t convert_rgb565_to_bgr666 (uint32_t c) { return ((c&0x1F)<<17) | ((c&0x10)<<12) | ((c&0x7E0)<<3) | ((c>>10)&0xF8) | (c>>15); }
  LGFX_INLINE uint32_t convert_rgb565_to_swap565(uint32_t c) { return (0xFF & c)<<8|c>>8; }
  LGFX_INLINE uint32_t convert_rgb565_to_rgb332 (uint32_t c) { return ((c>>13) <<5) | ((c>>6) & 0x1C) | ((c>>3) & 3); }

  LGFX_INLINE uint32_t convert_rgb332_to_bgra8888(uint32_t c) { return ((((c&3)*0x55)<<8 | ((c&0x1C)*0x49)>>3)<<8 | (((c>>5)*0x49) >> 1)) << 8 | 0xFF; }
  LGFX_INLINE uint32_t convert_rgb332_to_argb8888(uint32_t c) { return ((((c>>5)*0x49) >> 1)<<8 | ((c&0x1C)*0x49)>>3)<<8 | ((c&3)*0x55) | 0xFF << 24; }
  LGFX_INLINE uint32_t convert_rgb332_to_rgb888 (uint32_t c) { return ((((c>>5)*0x49) >> 1)<<8 | ((c&0x1C)*0x49)>>3)<<8 | ((c&3)*0x55); }
  LGFX_INLINE uint32_t convert_rgb332_to_bgr888 (uint32_t c) { return (((c&3)*0x55)<<8 | ((c&0x1C)*0x49)>>3)<<8 | (((c>>5)*0x49) >> 1); }
  LGFX_INLINE uint32_t convert_rgb332_to_bgr666 (uint32_t c) { return (((c&0xE0)*9)>>5) | ((c&0x1C)*0x240) | ((c&3)*0x15)<<16; }
  LGFX_INLINE uint32_t convert_rgb332_to_swap565(uint32_t c) { return (((c&3)*0x15)>>1)<<8 | ((c&0x1C)<<11) | ((c&0x1C)>>2) | (((c>>5)*0x24)&0xF8); }
  LGFX_INLINE uint32_t convert_rgb332_to_rgb565 (uint32_t c) { return (((c&3)*0x15)>>1) | ((c&0x1C)*0x48) | ((c&0xE0)|(c>>6)<<3)<<8; }

  LGFX_INLINE uint32_t convert_bgra8888_to_bgr888(uint32_t c) { return c >> 8; }
  LGFX_INLINE uint32_t convert_bgra8888_to_bgr666(uint32_t c) { return (c >> 10) & 0x3F3F3F; }
  LGFX_INLINE uint32_t convert_bgra8888_to_swap565(uint32_t c){ return convert_bgr888_to_swap565(c>>8); }
  LGFX_INLINE uint32_t convert_bgra8888_to_rgb332(uint32_t c) { return convert_bgr888_to_rgb332(c>>8); }
  LGFX_INLINE uint32_t convert_bgra8888_to_argb8888(uint32_t c) { return __builtin_bswap32(c); }
  LGFX_INLINE uint32_t convert_argb8888_to_bgra8888(uint32_t c) { return __builtin_bswap32(c); }

  LGFX_INLINE uint32_t convert_uint32_to_palette8(uint32_t c) { return  c & 0xFF; }
  LGFX_INLINE uint32_t convert_uint32_to_palette4(uint32_t c) { return (c & 0x0F) * 0x11; }
  LGFX_INLINE uint32_t convert_uint32_to_palette2(uint32_t c) { return (c & 0x03) * 0x55; }
  LGFX_INLINE uint32_t convert_uint32_to_palette1(uint32_t c) { return (c & 1) ? 0xFF : 0; }
  LGFX_INLINE uint32_t no_convert(uint32_t c)                 { return c; }

#undef LGFX_INLINE


#pragma pack(push)
#pragma pack(1)
  struct rgb332_t;    //  8bpp
  struct rgb565_t;    // 16bpp
  struct rgb888_t;    // 24bpp
  struct argb8888_t;  // 32bpp
  struct swap565_t;   // 16bpp
  struct bgr666_t;    // 18bpp (24bpp __RRRRRR__GGGGGG__BBBBBB (for OLED SSD1351)
  struct bgr888_t;    // 24bpp
  struct bgra8888_t;  // 32bpp
  struct grayscale_t; //  8bpp grayscale

  struct rgb332_t
  {
    union
    {
      struct
      {
        uint8_t b2: 2;
        uint8_t g3: 3;
        uint8_t r3: 3;
      };
      uint8_t raw;
    };
    static constexpr uint8_t bits = 8;
    static constexpr bool swapped = true;
    static constexpr color_depth_t depth = rgb332_1Byte;
    constexpr rgb332_t(void) : raw(0) {}
    constexpr rgb332_t(const rgb332_t&) = default;
    constexpr rgb332_t(uint8_t r8, uint8_t g8, uint8_t b8) : raw(color332(r8, g8, b8)) {}
    constexpr rgb332_t(uint8_t rgb332) : raw(rgb332) {}
    inline rgb332_t& operator=(const rgb565_t&);
    inline rgb332_t& operator=(const rgb888_t&);
    inline rgb332_t& operator=(const argb8888_t&);
    inline rgb332_t& operator=(const swap565_t&);
    inline rgb332_t& operator=(const bgr666_t&);
    inline rgb332_t& operator=(const bgr888_t&);
    inline rgb332_t& operator=(const bgra8888_t&);
    inline rgb332_t& operator=(const grayscale_t&);
    inline rgb332_t& operator=(uint8_t rgb332) { *reinterpret_cast<uint8_t*>(this) = rgb332; return *this; }
    explicit inline constexpr operator uint8_t(void) const { return raw; }
    explicit inline constexpr operator bool(void) const { return raw; }
    static constexpr uint8_t A8(void) { return 255; }
    inline constexpr uint8_t R8(void) const { return (r3 * 0x49) >> 1; } // (r<<5)|(r<<2)|(r>>1);
    inline constexpr uint8_t G8(void) const { return (g3 * 0x49) >> 1; } // (g<<5)|(g<<2)|(g>>1);
    inline constexpr uint8_t B8(void) const { return  b2 * 0x55; }       // (b<<6)|(b<<4)|(b<<2)|b;
    inline constexpr uint8_t R6(void) const { return  r3 | r3 << 3; }
    inline constexpr uint8_t G6(void) const { return  g3 | g3 << 3; }
    inline constexpr uint8_t B6(void) const { return  b2 * 0x15; }
    inline void R8(uint8_t r8) { r3 = r8 >> 5; }
    inline void G8(uint8_t g8) { g3 = g8 >> 5; }
    inline void B8(uint8_t b8) { b2 = b8 >> 6; }
    inline void set(uint8_t r8, uint8_t g8, uint8_t b8) { *reinterpret_cast<uint8_t*>(this) = color332(r8, g8, b8); }
  };

  struct swap565_t
  {
    union
    {
      struct
      {
        uint16_t gh:3;
        uint16_t r5:5;
        uint16_t b5:5;
        uint16_t gl:3;
      };
      uint16_t raw;
    };
    static constexpr uint8_t bits = 16;
    static constexpr bool swapped = true;
    static constexpr color_depth_t depth = rgb565_2Byte;
    constexpr swap565_t(void) : raw(0) {}
    constexpr swap565_t(const swap565_t&) = default;
    constexpr swap565_t(uint8_t r8, uint8_t g8, uint8_t b8) : raw(swap565(r8, g8, b8)) {}
    constexpr swap565_t(uint16_t swap565) : raw(swap565) {}
    inline swap565_t& operator=(const rgb332_t&);
    inline swap565_t& operator=(const rgb565_t&);
    inline swap565_t& operator=(const rgb888_t&);
    inline swap565_t& operator=(const argb8888_t&);
    inline swap565_t& operator=(const bgr666_t&);
    inline swap565_t& operator=(const bgr888_t&);
    inline swap565_t& operator=(const bgra8888_t&);
    inline swap565_t& operator=(const grayscale_t&);
    inline swap565_t& operator=(uint16_t swap565) { *reinterpret_cast<uint16_t*>(this) = swap565; return *this; }
    explicit inline constexpr operator uint16_t(void) const { return raw; }
    explicit inline constexpr operator bool(void) const { return raw; }
    static constexpr uint8_t A8(void) { return 255; }
    inline constexpr uint8_t R8(void) const { return (r5 << 3) + (r5 >> 5); }
    inline constexpr uint8_t G8(void) const { return (gl << 2) + ((gh << 5) | (gh >> 1)); }
    inline constexpr uint8_t B8(void) const { return (b5 << 3) + (b5 >> 5); }
    inline constexpr uint8_t R6(void) const { return (r5 * 0x21) >> 4; }
    inline constexpr uint8_t G6(void) const { return  gh << 3 | gl; }
    inline constexpr uint8_t B6(void) const { return (b5 * 0x21) >> 4; }
    inline void R8(uint8_t r8) { r5 = r8 >> 3; }
    inline void G8(uint8_t g8) { gh = g8 >> 5; gl = g8 >> 2;}
    inline void B8(uint8_t b8) { b5 = b8 >> 3; }
    inline void set(uint8_t r8, uint8_t g8, uint8_t b8) { *reinterpret_cast<uint16_t*>(this) = swap565(r8, g8, b8); }
  };

  struct rgb565_t
  {
    union
    {
      struct
      {
        uint16_t b5: 5;
        uint16_t g6: 6;
        uint16_t r5: 5;
      };
      uint16_t raw;
    };
    static constexpr uint8_t bits = 16;
    static constexpr bool swapped = false;
    static constexpr color_depth_t depth = rgb565_nonswapped;
    constexpr rgb565_t(void) : raw(0) {}
    constexpr rgb565_t(const rgb565_t&) = default;
    constexpr rgb565_t(uint8_t r8, uint8_t g8, uint8_t b8) : raw(color565(r8, g8, b8)) {}
    constexpr rgb565_t(uint16_t rgb565) : raw(rgb565) {}
    inline rgb565_t& operator=(const rgb332_t&);
    inline rgb565_t& operator=(const rgb888_t&);
    inline rgb565_t& operator=(const argb8888_t&);
    inline rgb565_t& operator=(const swap565_t&);
    inline rgb565_t& operator=(const bgr666_t&);
    inline rgb565_t& operator=(const bgr888_t&);
    inline rgb565_t& operator=(const bgra8888_t&);
    inline rgb565_t& operator=(const grayscale_t&);
    inline rgb565_t& operator=(uint16_t rgb565) { *reinterpret_cast<uint16_t*>(this) = rgb565; return *this; }
    explicit inline constexpr operator uint16_t(void) const { return raw; }
    explicit inline constexpr operator bool(void) const { return raw; }
    static constexpr uint8_t A8(void) { return 255; }
    inline constexpr uint8_t R8(void) const { return (r5 * 0x21) >> 2; } // (r << 3) | (r >> 2);
    inline constexpr uint8_t G8(void) const { return (g6 * 0x41) >> 4; } // (g << 2) | (g >> 4);
    inline constexpr uint8_t B8(void) const { return (b5 * 0x21) >> 2; } // (b << 3) | (b >> 2);
    inline constexpr uint8_t R6(void) const { return (r5 * 0x21) >> 4; }
    inline constexpr uint8_t G6(void) const { return  g6; }
    inline constexpr uint8_t B6(void) const { return (b5 * 0x21) >> 4; }
    inline void R8(uint8_t r8) { r5 = r8 >> 3; }
    inline void G8(uint8_t g8) { g6 = g8 >> 2; }
    inline void B8(uint8_t b8) { b5 = b8 >> 3; }
    inline void set(uint8_t r8, uint8_t g8, uint8_t b8) { *reinterpret_cast<uint16_t*>(this) = color565(r8, g8, b8); }
  };

  struct bgr666_t
  {
    uint8_t r6;
    uint8_t g6;
    uint8_t b6;
    static constexpr uint8_t bits = 24;
    static constexpr bool swapped = true;
    static constexpr color_depth_t depth = rgb666_3Byte;
    constexpr bgr666_t(void) : r6(0), g6(0), b6(0) {};
    constexpr bgr666_t(const bgr666_t&) = default;
    constexpr bgr666_t(uint8_t r8, uint8_t g8, uint8_t b8) : r6(r8>>2), g6(g8>>2), b6(b8>>2) {}
    constexpr bgr666_t(uint32_t raw) : r6(raw), g6(raw>>8), b6(raw>>16) {}
    inline bgr666_t& operator=(uint32_t rhs) { r6 = rhs; g6 = rhs>>8 ; b6 = rhs>>16; return *this; }
    inline bgr666_t& operator=(const rgb332_t&);
    inline bgr666_t& operator=(const rgb565_t&);
    inline bgr666_t& operator=(const rgb888_t&);
    inline bgr666_t& operator=(const argb8888_t&);
    inline bgr666_t& operator=(const swap565_t&);
    inline bgr666_t& operator=(const bgr888_t&);
    inline bgr666_t& operator=(const bgra8888_t&);
    inline bgr666_t& operator=(const grayscale_t&);
    explicit inline constexpr operator uint32_t(void) const { return *reinterpret_cast<const uint32_t*>(this) & ((1ul << 24) - 1); }
    explicit inline constexpr operator bool(void) const { return r6 || g6 || b6; }
    static constexpr uint8_t A8(void) { return 255; }
    inline constexpr uint8_t R8(void) const { return r6<<2; }
    inline constexpr uint8_t G8(void) const { return g6<<2; }
    inline constexpr uint8_t B8(void) const { return b6<<2; }
    inline constexpr uint8_t R6(void) const { return r6; }
    inline constexpr uint8_t G6(void) const { return g6; }
    inline constexpr uint8_t B6(void) const { return b6; }
    inline void R8(uint8_t r8) { r6 = r8 >> 2; }
    inline void G8(uint8_t g8) { g6 = g8 >> 2; }
    inline void B8(uint8_t b8) { b6 = b8 >> 2; }
    inline void set(uint8_t r8, uint8_t g8, uint8_t b8) { r6 = r8>>2; g6 = g8>>2; b6 = b8>>2; }
  };

  struct bgr888_t
  {
    union
    {
      struct
      {
        uint8_t r;
        uint8_t g;
        uint8_t b;
      };
      uint16_t rg;
    };
    static constexpr uint8_t bits = 24;
    static constexpr bool swapped = true;
    static constexpr color_depth_t depth = rgb888_3Byte;
    constexpr bgr888_t(void) : r{0}, g{0}, b{0} {};
    constexpr bgr888_t(const bgr888_t&) = default;
    constexpr bgr888_t(uint8_t r8, uint8_t g8, uint8_t b8) : r(r8), g(g8), b(b8) {}
    constexpr bgr888_t(uint32_t bgr888) : r(bgr888), g(bgr888>>8), b(bgr888>>16) {}
    inline bgr888_t& operator=(uint32_t rhs) { r = rhs; g = rhs>>8 ; b = rhs>>16; return *this; }
    inline bgr888_t& operator=(const rgb332_t&);
    inline bgr888_t& operator=(const rgb565_t&);
    inline bgr888_t& operator=(const rgb888_t&);
    inline bgr888_t& operator=(const argb8888_t&);
    inline bgr888_t& operator=(const swap565_t&);
    inline bgr888_t& operator=(const bgr666_t&);
    inline bgr888_t& operator=(const bgra8888_t&);
    inline bgr888_t& operator=(const grayscale_t&);
    explicit inline constexpr operator uint32_t(void) const { return *reinterpret_cast<const uint32_t*>(this) & ((1ul << 24) - 1); }
    explicit inline constexpr operator bool(void) const { return rg || b; }
    static constexpr uint8_t A8() { return 255; }
    inline constexpr uint8_t R8() const { return r; }
    inline constexpr uint8_t G8() const { return g; }
    inline constexpr uint8_t B8() const { return b; }
    inline constexpr uint8_t R6() const { return r >> 2; }
    inline constexpr uint8_t G6() const { return g >> 2; }
    inline constexpr uint8_t B6() const { return b >> 2; }
    inline void R8(uint8_t r8) { r = r8; }
    inline void G8(uint8_t g8) { g = g8; }
    inline void B8(uint8_t b8) { b = b8; }
    inline void set(uint8_t r8, uint8_t g8, uint8_t b8) { r = r8; g = g8; b = b8; }
  };

  struct rgb888_t
  {
    union
    {
      struct
      {
        uint8_t b;
        uint8_t g;
        uint8_t r;
      };
      uint16_t bg;
    };
    static constexpr uint8_t bits = 24;
    static constexpr bool swapped = false;
    static constexpr color_depth_t depth = rgb888_nonswapped;
    constexpr rgb888_t(void) : b(0), g(0), r(0) {}
    constexpr rgb888_t(const rgb888_t&) = default;
    constexpr rgb888_t(uint8_t r8, uint8_t g8, uint8_t b8) : b(b8), g(g8), r(r8) {}
    constexpr rgb888_t(uint32_t rgb888) : b(rgb888), g(rgb888>>8), r(rgb888>>16) {}
    inline rgb888_t& operator=(const rgb332_t&);
    inline rgb888_t& operator=(const rgb565_t&);
    inline rgb888_t& operator=(const argb8888_t&);
    inline rgb888_t& operator=(const swap565_t&);
    inline rgb888_t& operator=(const bgr666_t&);
    inline rgb888_t& operator=(const bgr888_t&);
    inline rgb888_t& operator=(const bgra8888_t&);
    inline rgb888_t& operator=(const grayscale_t&);
    inline rgb888_t& operator=(uint32_t rgb888) { r = rgb888>>16; g = rgb888>>8; b = rgb888; return *this; }
    explicit inline constexpr operator uint32_t(void) const { return *reinterpret_cast<const uint32_t*>(this) & ((1ul << 24) - 1); }
    explicit inline constexpr operator bool(void) const { return bg || r; }
    static constexpr uint8_t A8(void) { return 255; }
    inline constexpr uint8_t R8(void) const { return r; }
    inline constexpr uint8_t G8(void) const { return g; }
    inline constexpr uint8_t B8(void) const { return b; }
    inline constexpr uint8_t R6(void) const { return r >> 2; }
    inline constexpr uint8_t G6(void) const { return g >> 2; }
    inline constexpr uint8_t B6(void) const { return b >> 2; }
    inline void R8(uint8_t r8) { r = r8; }
    inline void G8(uint8_t g8) { g = g8; }
    inline void B8(uint8_t b8) { b = b8; }
    inline void set(uint8_t r8, uint8_t g8, uint8_t b8) { r = r8; g = g8; b = b8; }
  };

  struct bgra8888_t
  {
    union
    {
      struct
      {
        uint8_t a;
        uint8_t r;
        uint8_t g;
        uint8_t b;
      };
      uint32_t raw;
    };
    static constexpr uint8_t bits = 32;
    static constexpr bool swapped = true;
    static constexpr color_depth_t depth = argb8888_4Byte;
    constexpr bgra8888_t(void) : raw{0} {};
    constexpr bgra8888_t(const bgra8888_t&) = default;
    constexpr bgra8888_t(uint8_t r8, uint8_t g8, uint8_t b8) : r(r8), g(g8), b(b8) {}
    constexpr bgra8888_t(uint8_t a8, uint8_t r8, uint8_t g8, uint8_t b8) : a(a8), r(r8), g(g8), b(b8) {}
    constexpr bgra8888_t(uint32_t bgra8888) : raw(bgra8888) {}
    inline bgra8888_t& operator=(uint32_t rhs) { raw = rhs; return *this; }
    inline bgra8888_t& operator=(const rgb332_t&);
    inline bgra8888_t& operator=(const rgb565_t&);
    inline bgra8888_t& operator=(const rgb888_t&);
    inline bgra8888_t& operator=(const argb8888_t&);
    inline bgra8888_t& operator=(const swap565_t&);
    inline bgra8888_t& operator=(const bgr666_t&);
    inline bgra8888_t& operator=(const bgr888_t&);
    inline bgra8888_t& operator=(const grayscale_t&);
    explicit inline constexpr operator uint32_t(void) const { return raw; }
    explicit inline constexpr operator bool(void) const { return raw; }
    inline constexpr uint8_t A8(void) const { return a; }
    inline constexpr uint8_t R8(void) const { return r; }
    inline constexpr uint8_t G8(void) const { return g; }
    inline constexpr uint8_t B8(void) const { return b; }
    inline constexpr uint8_t R6(void) const { return r >> 2; }
    inline constexpr uint8_t G6(void) const { return g >> 2; }
    inline constexpr uint8_t B6(void) const { return b >> 2; }
    inline void A8(uint8_t a8) { a = a8; }
    inline void R8(uint8_t r8) { r = r8; }
    inline void G8(uint8_t g8) { g = g8; }
    inline void B8(uint8_t b8) { b = b8; }
    inline void set(uint8_t r8, uint8_t g8, uint8_t b8) { a = 255; r = r8; g = g8; b = b8; }
    inline void set(uint8_t a8, uint8_t r8, uint8_t g8, uint8_t b8) { a = a8; r = r8; g = g8; b = b8; }
  };

  struct argb8888_t
  {
    union
    {
      struct
      {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;
      };
      uint32_t raw;
    };
    static constexpr uint8_t bits = 32;
    static constexpr bool swapped = false;
    static constexpr color_depth_t depth = argb8888_nonswapped;
    constexpr argb8888_t(void) : raw(0) {}
    constexpr argb8888_t(const argb8888_t&) = default;
    constexpr argb8888_t(uint8_t r8, uint8_t g8, uint8_t b8) : b(b8), g(g8), r(r8), a(255) {}
    constexpr argb8888_t(uint8_t a8, uint8_t r8, uint8_t g8, uint8_t b8) : b(b8), g(g8), r(r8), a(a8) {}
    constexpr argb8888_t(uint32_t argb8888) : raw(argb8888) {}
    inline argb8888_t& operator=(const rgb332_t&);
    inline argb8888_t& operator=(const rgb565_t&);
    inline argb8888_t& operator=(const rgb888_t&);
    inline argb8888_t& operator=(const swap565_t&);
    inline argb8888_t& operator=(const bgr666_t&);
    inline argb8888_t& operator=(const bgr888_t&);
    inline argb8888_t& operator=(const bgra8888_t&);
    inline argb8888_t& operator=(const grayscale_t&);
    inline argb8888_t& operator=(uint32_t argb8888) { *reinterpret_cast<uint32_t*>(this) = argb8888; return *this; }
    explicit inline constexpr operator uint32_t(void) const { return raw; }
    explicit inline constexpr operator bool(void) const { return raw; }
    inline constexpr uint8_t A8(void) const { return a; }
    inline constexpr uint8_t R8(void) const { return r; }
    inline constexpr uint8_t G8(void) const { return g; }
    inline constexpr uint8_t B8(void) const { return b; }
    inline constexpr uint8_t R6(void) const { return r >> 2; }
    inline constexpr uint8_t G6(void) const { return g >> 2; }
    inline constexpr uint8_t B6(void) const { return b >> 2; }
    inline void A8(uint8_t a8) { a = a8; }
    inline void R8(uint8_t r8) { r = r8; }
    inline void G8(uint8_t g8) { g = g8; }
    inline void B8(uint8_t b8) { b = b8; }
    inline void set(uint8_t r8, uint8_t g8, uint8_t b8) { a = 255; r = r8; g = g8; b = b8; }
    inline void set(uint8_t a8, uint8_t r8, uint8_t g8, uint8_t b8) { a = a8; r = r8; g = g8; b = b8; }
  };

  struct grayscale_t
  {
    union
    {
      uint8_t raw;
      uint8_t r;
      uint8_t g;
      uint8_t b;
    };
    static constexpr uint8_t bits = 8;
    static constexpr bool swapped = true;
    static constexpr color_depth_t depth = grayscale_8bit;
    constexpr grayscale_t(void) : raw{0} {};
    constexpr grayscale_t(const grayscale_t&) = default;
    constexpr grayscale_t(uint8_t r8, uint8_t g8, uint8_t b8) : raw((r8 + (g8 << 1) + b8) >> 2) {}
    constexpr grayscale_t(uint8_t gray8) : raw(gray8) {}
    inline grayscale_t& operator=(uint8_t gray8) { *reinterpret_cast<uint8_t*>(this) = gray8; return *this; }
    inline grayscale_t& operator=(const rgb332_t&);
    inline grayscale_t& operator=(const rgb565_t&);
    inline grayscale_t& operator=(const rgb888_t&);
    inline grayscale_t& operator=(const argb8888_t&);
    inline grayscale_t& operator=(const swap565_t&);
    inline grayscale_t& operator=(const bgr666_t&);
    inline grayscale_t& operator=(const bgr888_t&);
    explicit inline constexpr operator uint8_t(void) const { return raw; }
    explicit inline constexpr operator bool(void) const { return raw; }
    static constexpr uint8_t A8(void) { return 255; }
    inline constexpr uint8_t R8(void) const { return r; }
    inline constexpr uint8_t G8(void) const { return g; }
    inline constexpr uint8_t B8(void) const { return b; }
    inline constexpr uint8_t R6(void) const { return r >> 2; }
    inline constexpr uint8_t G6(void) const { return g >> 2; }
    inline constexpr uint8_t B6(void) const { return b >> 2; }
    inline void R8(uint8_t r8) { r = r8; }
    inline void G8(uint8_t g8) { g = g8; }
    inline void B8(uint8_t b8) { b = b8; }
    inline void set(uint8_t r8, uint8_t g8, uint8_t b8) { raw = (r8 + (g8 << 1) + b8) >> 2; }
  };

  struct raw_color_t
  {
    union
    {
      struct
      {
        uint8_t raw0;
        uint8_t raw1;
        uint8_t raw2;
        uint8_t raw3;
      };
      struct
      {
        uint16_t rawL;
        uint16_t rawH;
      };
      uint32_t raw;
    };
    constexpr raw_color_t() : raw(0) {}
    constexpr raw_color_t(const raw_color_t&) = default;
    constexpr raw_color_t(const uint32_t& rhs) : raw(rhs) {}
  };

#pragma pack(pop)

  inline rgb332_t& rgb332_t::operator=(const rgb565_t&   rhs) { raw = (rhs.r5 >> 2) << 5 | (rhs.g6 >> 3) << 2 | rhs.b5 >> 3; return *this; }
  inline rgb332_t& rgb332_t::operator=(const swap565_t&  rhs) { raw = ((rhs.r5 << 3) & 0xE0) | ((rhs.gh << 2) + (rhs.b5 >> 3)); return *this; }
  inline rgb332_t& rgb332_t::operator=(const bgr666_t&   rhs) { raw = ((rhs.r6 << 2) & 0xE0) | ((rhs.g6 >> 1) & 0x1C) | (rhs.b6 >> 4); return *this; }
  inline rgb332_t& rgb332_t::operator=(const rgb888_t&   rhs) { raw = color332(rhs.R8(), rhs.G8(), rhs.B8()); return *this; }
  inline rgb332_t& rgb332_t::operator=(const bgr888_t&   rhs) { raw = color332(rhs.R8(), rhs.G8(), rhs.B8()); return *this; }
  inline rgb332_t& rgb332_t::operator=(const argb8888_t& rhs) { raw = color332(rhs.R8(), rhs.G8(), rhs.B8()); return *this; }
  inline rgb332_t& rgb332_t::operator=(const bgra8888_t& rhs) { raw = color332(rhs.R8(), rhs.G8(), rhs.B8()); return *this; }
  inline rgb332_t& rgb332_t::operator=(const grayscale_t& rhs){ raw = color332(rhs.R8(), rhs.G8(), rhs.B8()); return *this; }

  inline rgb565_t& rgb565_t::operator=(const rgb332_t&   rhs) { raw = color565(rhs.R8(), rhs.G8(), rhs.B8()); return *this; }
  inline rgb565_t& rgb565_t::operator=(const swap565_t&  rhs) { raw = __builtin_bswap16(rhs.raw);   return *this; }
  inline rgb565_t& rgb565_t::operator=(const bgr666_t&   rhs) { raw = color565(rhs.R8(), rhs.G8(), rhs.B8()); return *this; }
  inline rgb565_t& rgb565_t::operator=(const rgb888_t&   rhs) { raw = color565(rhs.R8(), rhs.G8(), rhs.B8()); return *this; }
  inline rgb565_t& rgb565_t::operator=(const bgr888_t&   rhs) { raw = color565(rhs.R8(), rhs.G8(), rhs.B8()); return *this; }
  inline rgb565_t& rgb565_t::operator=(const argb8888_t& rhs) { raw = color565(rhs.R8(), rhs.G8(), rhs.B8()); return *this; }
  inline rgb565_t& rgb565_t::operator=(const bgra8888_t& rhs) { raw = color565(rhs.R8(), rhs.G8(), rhs.B8()); return *this; }
  inline rgb565_t& rgb565_t::operator=(const grayscale_t& rhs){ raw = color565(rhs.R8(), rhs.G8(), rhs.B8()); return *this; }

  inline rgb888_t& rgb888_t::operator=(const rgb332_t&   rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline rgb888_t& rgb888_t::operator=(const rgb565_t&   rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline rgb888_t& rgb888_t::operator=(const swap565_t&  rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline rgb888_t& rgb888_t::operator=(const bgr666_t&   rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline rgb888_t& rgb888_t::operator=(const bgr888_t&   rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline rgb888_t& rgb888_t::operator=(const argb8888_t& rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline rgb888_t& rgb888_t::operator=(const bgra8888_t& rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline rgb888_t& rgb888_t::operator=(const grayscale_t& rhs){ r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }

  inline swap565_t& swap565_t::operator=(const rgb332_t&   rhs) { *reinterpret_cast<uint16_t*>(this) = ((rhs.b2 * 0x15)>>1)<<8 | rhs.g3 << 13 | rhs.g3 | ((rhs.r3 * 0x09) >> 1) << 3; return *this; }
  inline swap565_t& swap565_t::operator=(const rgb565_t&   rhs) { *reinterpret_cast<uint16_t*>(this) = __builtin_bswap16(rhs.raw);              return *this; }
  inline swap565_t& swap565_t::operator=(const rgb888_t&   rhs) { *reinterpret_cast<uint16_t*>(this) = swap565(rhs.R8(), rhs.G8(), rhs.B8());   return *this; }
  inline swap565_t& swap565_t::operator=(const bgr666_t&   rhs) { raw = (rhs.b6 >> 1)<<8 | rhs.g6 << 13 | rhs.g6 >> 3 | (rhs.r6 >> 1) << 3; return *this; }
  inline swap565_t& swap565_t::operator=(const bgr888_t&   rhs) { *reinterpret_cast<uint16_t*>(this) = swap565(rhs.R8(), rhs.G8(), rhs.B8());   return *this; }
  inline swap565_t& swap565_t::operator=(const argb8888_t& rhs) { *reinterpret_cast<uint16_t*>(this) = swap565(rhs.R8(), rhs.G8(), rhs.B8());   return *this; }
  inline swap565_t& swap565_t::operator=(const bgra8888_t& rhs) { *reinterpret_cast<uint16_t*>(this) = swap565(rhs.R8(), rhs.G8(), rhs.B8());   return *this; }
  inline swap565_t& swap565_t::operator=(const grayscale_t& rhs){ *reinterpret_cast<uint16_t*>(this) = swap565(rhs.R8(), rhs.G8(), rhs.B8());   return *this; }

  inline bgr666_t& bgr666_t::operator=(const rgb332_t&   rhs) { r6 = rhs.R6(); g6 = rhs.G6(); b6 = rhs.B6(); return *this; }
  inline bgr666_t& bgr666_t::operator=(const rgb565_t&   rhs) { r6 = rhs.R6(); g6 = rhs.G6(); b6 = rhs.B6(); return *this; }
  inline bgr666_t& bgr666_t::operator=(const swap565_t&  rhs) { r6 = rhs.R6(); g6 = rhs.G6(); b6 = rhs.B6(); return *this; }
  inline bgr666_t& bgr666_t::operator=(const bgr888_t&   rhs) { r6 = rhs.R6(); g6 = rhs.G6(); b6 = rhs.B6(); return *this; }
  inline bgr666_t& bgr666_t::operator=(const rgb888_t&   rhs) { r6 = rhs.R6(); g6 = rhs.G6(); b6 = rhs.B6(); return *this; }
  inline bgr666_t& bgr666_t::operator=(const argb8888_t& rhs) { r6 = rhs.R6(); g6 = rhs.G6(); b6 = rhs.B6(); return *this; }
  inline bgr666_t& bgr666_t::operator=(const bgra8888_t& rhs) { r6 = rhs.R6(); g6 = rhs.G6(); b6 = rhs.B6(); return *this; }
  inline bgr666_t& bgr666_t::operator=(const grayscale_t& rhs){ r6 = rhs.R6(); g6 = rhs.G6(); b6 = rhs.B6(); return *this; }

  inline bgr888_t& bgr888_t::operator=(const rgb332_t&   rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline bgr888_t& bgr888_t::operator=(const rgb565_t&   rhs) { rg = rhs.R8() | rhs.G8() <<8; b = rhs.B8(); return *this; }
  inline bgr888_t& bgr888_t::operator=(const swap565_t&  rhs) { rg = rhs.R8() | rhs.G8() <<8; b = rhs.B8(); return *this; }
  inline bgr888_t& bgr888_t::operator=(const bgr666_t&   rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline bgr888_t& bgr888_t::operator=(const rgb888_t&   rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline bgr888_t& bgr888_t::operator=(const argb8888_t& rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline bgr888_t& bgr888_t::operator=(const bgra8888_t& rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline bgr888_t& bgr888_t::operator=(const grayscale_t& rhs){ r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }

  inline bgra8888_t& bgra8888_t::operator=(const rgb332_t&  rhs) { a = 255; r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline bgra8888_t& bgra8888_t::operator=(const rgb565_t&  rhs) { a = 255; r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline bgra8888_t& bgra8888_t::operator=(const swap565_t& rhs) { a = 255; r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline bgra8888_t& bgra8888_t::operator=(const bgr666_t&  rhs) { a = 255; r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline bgra8888_t& bgra8888_t::operator=(const rgb888_t&  rhs) { a = 255; r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline bgra8888_t& bgra8888_t::operator=(const bgr888_t&  rhs) { a = 255; r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline bgra8888_t& bgra8888_t::operator=(const grayscale_t& rhs){a = 255; r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline bgra8888_t& bgra8888_t::operator=(const argb8888_t& rhs){ a = rhs.A8(); r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }

  inline argb8888_t& argb8888_t::operator=(const rgb332_t&  rhs) { a = 255; r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline argb8888_t& argb8888_t::operator=(const rgb565_t&  rhs) { a = 255; r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline argb8888_t& argb8888_t::operator=(const swap565_t& rhs) { a = 255; r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline argb8888_t& argb8888_t::operator=(const bgr666_t&  rhs) { a = 255; r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline argb8888_t& argb8888_t::operator=(const rgb888_t&  rhs) { a = 255; r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline argb8888_t& argb8888_t::operator=(const bgr888_t&  rhs) { a = 255; r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline argb8888_t& argb8888_t::operator=(const grayscale_t& rhs){a = 255; r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline argb8888_t& argb8888_t::operator=(const bgra8888_t& rhs){ a = rhs.A8(); r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }

  inline grayscale_t& grayscale_t::operator=(const rgb332_t&   rhs) { raw = (rhs.R8() + (rhs.G8()<<1) + rhs.B8()) >> 2; return *this; }
  inline grayscale_t& grayscale_t::operator=(const rgb565_t&   rhs) { raw = (rhs.R8() + (rhs.G8()<<1) + rhs.B8()) >> 2; return *this; }
  inline grayscale_t& grayscale_t::operator=(const rgb888_t&   rhs) { raw = (rhs.R8() + (rhs.G8()<<1) + rhs.B8()) >> 2; return *this; }
  inline grayscale_t& grayscale_t::operator=(const argb8888_t& rhs) { raw = (rhs.R8() + (rhs.G8()<<1) + rhs.B8()) >> 2; return *this; }
  inline grayscale_t& grayscale_t::operator=(const swap565_t&  rhs) { raw = (rhs.R8() + (rhs.G8()<<1) + rhs.B8()) >> 2; return *this; }
  inline grayscale_t& grayscale_t::operator=(const bgr666_t&   rhs) { raw = (rhs.R8() + (rhs.G8()<<1) + rhs.B8()) >> 2; return *this; }
  inline grayscale_t& grayscale_t::operator=(const bgr888_t&   rhs) { raw = (rhs.R8() + (rhs.G8()<<1) + rhs.B8()) >> 2; return *this; }

  inline constexpr bool operator==(const rgb332_t&   lhs, const rgb332_t&   rhs) { return lhs.raw == rhs.raw; }
  inline constexpr bool operator==(const rgb565_t&   lhs, const rgb565_t&   rhs) { return lhs.raw == rhs.raw; }
  inline constexpr bool operator==(const swap565_t&  lhs, const swap565_t&  rhs) { return lhs.raw == rhs.raw; }
  inline constexpr bool operator==(const bgr666_t&   lhs, const bgr666_t&   rhs) { return (*reinterpret_cast<const uint32_t*>(&lhs) << 8) == (*reinterpret_cast<const uint32_t*>(&rhs) << 8); }
  inline constexpr bool operator==(const rgb888_t&   lhs, const rgb888_t&   rhs) { return (*reinterpret_cast<const uint32_t*>(&lhs) << 8) == (*reinterpret_cast<const uint32_t*>(&rhs) << 8); }
  inline constexpr bool operator==(const bgr888_t&   lhs, const bgr888_t&   rhs) { return (*reinterpret_cast<const uint32_t*>(&lhs) << 8) == (*reinterpret_cast<const uint32_t*>(&rhs) << 8); }
  inline constexpr bool operator==(const argb8888_t& lhs, const argb8888_t& rhs) { return lhs.raw == rhs.raw; }
  inline constexpr bool operator==(const grayscale_t& lhs,const grayscale_t& rhs){ return lhs.raw == rhs.raw; }

  // for compare transparent color.
  inline constexpr bool operator==(const rgb332_t&   lhs, uint32_t rhs) { return  *reinterpret_cast<const uint8_t* >(&lhs) == rhs; }
  inline constexpr bool operator==(const rgb565_t&   lhs, uint32_t rhs) { return  *reinterpret_cast<const uint16_t*>(&lhs) == rhs; }
  inline constexpr bool operator==(const swap565_t&  lhs, uint32_t rhs) { return  *reinterpret_cast<const uint16_t*>(&lhs) == rhs; }
  inline           bool operator==(const bgr666_t&   lhs, uint32_t rhs) { return (pgm_read_dword_unaligned(&lhs) << 8) >> 8 == rhs; }
  inline           bool operator==(const rgb888_t&   lhs, uint32_t rhs) { return (pgm_read_dword_unaligned(&lhs) << 8) >> 8 == rhs; }
  inline           bool operator==(const bgr888_t&   lhs, uint32_t rhs) { return (pgm_read_dword_unaligned(&lhs) << 8) >> 8 == rhs; }
  inline constexpr bool operator==(const argb8888_t& lhs, uint32_t rhs) { return  *reinterpret_cast<const uint32_t*>(&lhs) == rhs; }
  inline constexpr bool operator==(const bgra8888_t& lhs, uint32_t rhs) { return  *reinterpret_cast<const uint32_t*>(&lhs) == rhs; }
  inline constexpr bool operator==(const grayscale_t& lhs,uint32_t rhs) { return  *reinterpret_cast<const uint8_t* >(&lhs) == rhs; }

  inline constexpr bool operator==(const raw_color_t& lhs, const raw_color_t& rhs) { return *reinterpret_cast<const uint32_t*>(&lhs) == *reinterpret_cast<const uint32_t*>(&rhs); }
  inline constexpr bool operator!=(const raw_color_t& lhs, const raw_color_t& rhs) { return *reinterpret_cast<const uint32_t*>(&lhs) != *reinterpret_cast<const uint32_t*>(&rhs); }


  struct get_depth_impl {
  template<typename T> static constexpr std::integral_constant<color_depth_t, T::depth> check(decltype(T::depth)*);
  template<typename T> static constexpr std::integral_constant<color_depth_t, (color_depth_t)(sizeof(T) << 3) > check(...);
  };
  template<typename T> class get_depth : public decltype(get_depth_impl::check<T>(nullptr)) {};

  template <typename TSrc>
  static auto get_fp_convert_src(color_depth_t dst_depth, bool has_palette) -> uint32_t(*)(uint32_t)
  {
    if (std::is_same<TSrc, rgb332_t>::value || std::is_same<TSrc, uint8_t>::value) {
      switch (dst_depth) {
      case argb8888_4Byte: return convert_rgb332_to_bgra8888;
      case rgb888_3Byte: return convert_rgb332_to_bgr888;
      case rgb666_3Byte: return convert_rgb332_to_bgr666;
      case rgb565_2Byte: return convert_rgb332_to_swap565;
      case rgb332_1Byte:  return has_palette
                              ? convert_uint32_to_palette8
                              : no_convert;
      default: break;
      }
    } else if (std::is_same<TSrc, rgb888_t>::value || std::is_same<TSrc, uint32_t>::value) {
      switch (dst_depth) {
      case argb8888_4Byte: return convert_rgb888_to_bgra8888;
      case rgb888_3Byte: return convert_rgb888_to_bgr888;
      case rgb666_3Byte: return convert_rgb888_to_bgr666;
      case rgb565_2Byte: return convert_rgb888_to_swap565;
      case rgb332_1Byte: return has_palette
                              ? convert_uint32_to_palette8
                              : convert_rgb888_to_rgb332;
      default: break;
      }
    } else if (std::is_same<TSrc, bgr888_t>::value) {
      switch (dst_depth) {
      case argb8888_4Byte: return convert_bgr888_to_bgra8888;
      case rgb888_3Byte: return no_convert;
      case rgb666_3Byte: return convert_bgr888_to_bgr666;
      case rgb565_2Byte: return convert_bgr888_to_swap565;
      case rgb332_1Byte: return has_palette
                              ? convert_uint32_to_palette8
                              : convert_bgr888_to_rgb332;
      default: break;
      }
    } else if (std::is_same<TSrc, argb8888_t>::value) {
      switch (dst_depth) {
      case argb8888_4Byte: return convert_argb8888_to_bgra8888;
      case rgb888_3Byte: return convert_rgb888_to_bgr888;
      case rgb666_3Byte: return convert_rgb888_to_bgr666;
      case rgb565_2Byte: return convert_rgb888_to_swap565;
      case rgb332_1Byte: return has_palette
                              ? convert_uint32_to_palette8
                              : convert_rgb888_to_rgb332;
      default: break;
      }
    } else { // if (std::is_same<TSrc, rgb565_t>::value || std::is_same<TSrc, uint16_t>::value || std::is_same<TSrc, int>::value)
      switch (dst_depth) {
      case argb8888_4Byte: return convert_rgb565_to_bgra8888;
      case rgb888_3Byte: return convert_rgb565_to_bgr888;
      case rgb666_3Byte: return convert_rgb565_to_bgr666;
      case rgb565_2Byte: return convert_rgb565_to_swap565;
      case rgb332_1Byte: return has_palette
                              ? convert_uint32_to_palette8
                              : convert_rgb565_to_rgb332;
      default: break;
      }
    }

    switch (dst_depth) {
    case palette_4bit: return convert_uint32_to_palette4;
    case palette_2bit: return convert_uint32_to_palette2;
    case palette_1bit: return convert_uint32_to_palette1;
    default:           return no_convert;
    }
  }


  struct color_conv_t
  {
    uint32_t (*convert_argb8888)(uint32_t)=convert_rgb888_to_swap565;
    uint32_t (*convert_bgr888)(uint32_t) = convert_bgr888_to_swap565;
    uint32_t (*convert_rgb888)(uint32_t) = convert_rgb888_to_swap565;
    uint32_t (*convert_rgb565)(uint32_t) = convert_rgb565_to_swap565;
    uint32_t (*convert_rgb332)(uint32_t) = convert_rgb332_to_swap565;
    color_depth_t depth = rgb565_2Byte;
    uint32_t colormask = 0xFFFF;
    uint8_t bytes  = 2;
    uint8_t bits   = 16;
    uint8_t x_mask = 0;

    color_conv_t() = default;
    color_conv_t(const color_conv_t&) = default;

    void setColorDepth(color_depth_t depth, bool has_palette = false)
    {
      x_mask = 0;
      bits = depth & color_depth_t::bit_mask;
      has_palette = has_palette || (depth & color_depth_t::has_palette);

      if (depth != rgb666_3Byte)
      {
        if (     bits > 24) { depth = argb8888_4Byte; bits = 32; }
        else if (bits > 18) { depth =   rgb888_3Byte; bits = 24; }
        else if (bits > 16) { depth =   rgb666_3Byte; bits = 24; }
        else if (bits >  8) { depth =   rgb565_2Byte; bits = 16; }
        else if (bits >  4) { depth =   rgb332_1Byte; bits =  8; }
        else if (bits >  2) { depth = grayscale_4bit; bits =  4; x_mask = 0b0001; }
        else if (bits == 2) { depth = grayscale_2bit; bits =  2; x_mask = 0b0011; }
        else                { depth = grayscale_1bit; bits =  1; x_mask = 0b0111; }
        if (has_palette && bits <= 8)
        {
          depth = (color_depth_t)((int)depth | color_depth_t::has_palette);
        }
      }
      this->depth = depth;
      bytes = bits >> 3;
      colormask = (1 << bits) - 1;

      convert_argb8888 = get_fp_convert_src<argb8888_t>(depth, has_palette);
      convert_rgb888 = get_fp_convert_src<rgb888_t>(depth, has_palette);
      convert_rgb565 = get_fp_convert_src<rgb565_t>(depth, has_palette);
      convert_rgb332 = get_fp_convert_src<rgb332_t>(depth, has_palette);
      convert_bgr888 = get_fp_convert_src<bgr888_t>(depth, has_palette);
    }

#define TYPECHECK(dType) template < typename T, typename std::enable_if < (sizeof(T) == sizeof(dType)) && (std::is_signed<T>::value == std::is_signed<dType>::value), std::nullptr_t >::type=nullptr > __attribute__ ((always_inline)) inline
    TYPECHECK(int8_t  ) uint32_t convert(T c) { return convert_rgb332(c); }
    TYPECHECK(uint8_t ) uint32_t convert(T c) { return convert_rgb332(c); }
    TYPECHECK(uint16_t) uint32_t convert(T c) { return convert_rgb565(c); }
    TYPECHECK(int16_t ) uint32_t convert(T c) { return convert_rgb565(c); }
    TYPECHECK(int32_t ) uint32_t convert(T c) { return convert_rgb565(c); }
    TYPECHECK(uint32_t) uint32_t convert(T c) { return convert_rgb888(c); }

    __attribute__ ((always_inline)) inline uint32_t convert(const argb8888_t& c) { return convert_argb8888(c.raw); }
    __attribute__ ((always_inline)) inline uint32_t convert(const rgb888_t&   c) { return convert_rgb888(*(uint32_t*)&c); }
    __attribute__ ((always_inline)) inline uint32_t convert(const rgb565_t&   c) { return convert_rgb565(c.raw); }
    __attribute__ ((always_inline)) inline uint32_t convert(const rgb332_t&   c) { return convert_rgb332(c.raw); }
    __attribute__ ((always_inline)) inline uint32_t convert(const bgr888_t&   c) { return convert_bgr888(*(uint32_t*)&c); }
  };

  TYPECHECK(int8_t  ) constexpr uint32_t convert_to_rgb888(T c) { return convert_rgb332_to_rgb888(c); }
  TYPECHECK(uint8_t ) constexpr uint32_t convert_to_rgb888(T c) { return convert_rgb332_to_rgb888(c); }
  TYPECHECK(uint16_t) constexpr uint32_t convert_to_rgb888(T c) { return convert_rgb565_to_rgb888(c); }
  TYPECHECK(int16_t ) constexpr uint32_t convert_to_rgb888(T c) { return convert_rgb565_to_rgb888(c); }
  TYPECHECK(int32_t ) constexpr uint32_t convert_to_rgb888(T c) { return convert_rgb565_to_rgb888(c); }
  TYPECHECK(uint32_t) constexpr uint32_t convert_to_rgb888(T c) { return c & 0xFFFFFF; }
  __attribute__ ((always_inline)) inline constexpr uint32_t convert_to_rgb888(const argb8888_t& c) { return c.R8()<<16|c.G8()<<8|c.B8(); }
  __attribute__ ((always_inline)) inline constexpr uint32_t convert_to_rgb888(const rgb888_t&   c) { return c.R8()<<16|c.G8()<<8|c.B8(); }
  __attribute__ ((always_inline)) inline constexpr uint32_t convert_to_rgb888(const rgb565_t&   c) { return c.R8()<<16|c.G8()<<8|c.B8(); }
  __attribute__ ((always_inline)) inline constexpr uint32_t convert_to_rgb888(const rgb332_t&   c) { return c.R8()<<16|c.G8()<<8|c.B8(); }
  __attribute__ ((always_inline)) inline constexpr uint32_t convert_to_rgb888(const bgr888_t&   c) { return c.R8()<<16|c.G8()<<8|c.B8(); }
  __attribute__ ((always_inline)) inline constexpr uint32_t convert_to_rgb888(const bgr666_t&   c) { return c.R8()<<16|c.G8()<<8|c.B8(); }
  __attribute__ ((always_inline)) inline constexpr uint32_t convert_to_rgb888(const swap565_t&  c) { return c.R8()<<16|c.G8()<<8|c.B8(); }

  TYPECHECK(int8_t  ) constexpr uint32_t convert_to_bgr888(T c) { return convert_rgb332_to_bgr888(c); }
  TYPECHECK(uint8_t ) constexpr uint32_t convert_to_bgr888(T c) { return convert_rgb332_to_bgr888(c); }
  TYPECHECK(uint16_t) constexpr uint32_t convert_to_bgr888(T c) { return convert_rgb565_to_bgr888(c); }
  TYPECHECK(int16_t ) constexpr uint32_t convert_to_bgr888(T c) { return convert_rgb565_to_bgr888(c); }
  TYPECHECK(int32_t ) constexpr uint32_t convert_to_bgr888(T c) { return convert_rgb565_to_bgr888(c); }
  TYPECHECK(uint32_t) constexpr uint32_t convert_to_bgr888(T c) { return convert_rgb888_to_bgr888(c); }
  __attribute__ ((always_inline)) inline constexpr uint32_t convert_to_bgr888(const argb8888_t& c) { return c.B8()<<16|c.G8()<<8|c.R8(); }
  __attribute__ ((always_inline)) inline constexpr uint32_t convert_to_bgr888(const rgb888_t&   c) { return c.B8()<<16|c.G8()<<8|c.R8(); }
  __attribute__ ((always_inline)) inline constexpr uint32_t convert_to_bgr888(const rgb565_t&   c) { return c.B8()<<16|c.G8()<<8|c.R8(); }
  __attribute__ ((always_inline)) inline constexpr uint32_t convert_to_bgr888(const rgb332_t&   c) { return c.B8()<<16|c.G8()<<8|c.R8(); }
  __attribute__ ((always_inline)) inline constexpr uint32_t convert_to_bgr888(const bgr888_t&   c) { return c.B8()<<16|c.G8()<<8|c.R8(); }
  __attribute__ ((always_inline)) inline constexpr uint32_t convert_to_bgr888(const bgr666_t&   c) { return c.B8()<<16|c.G8()<<8|c.R8(); }
  __attribute__ ((always_inline)) inline constexpr uint32_t convert_to_bgr888(const swap565_t&  c) { return c.B8()<<16|c.G8()<<8|c.R8(); }

#undef TYPECHECK

//----------------------------------------------------------------------------

  struct effect_fill_alpha
  {
    effect_fill_alpha(argb8888_t src)
      : _inv { (uint_fast16_t)(256 - src.A8()) }
      , _r8a { (uint_fast16_t)(src.R8() * (1 + src.A8())) }
      , _g8a { (uint_fast16_t)(src.G8() * (1 + src.A8())) }
      , _b8a { (uint_fast16_t)(src.B8() * (1 + src.A8())) }
    {}
    void set(argb8888_t src)
    {
      _inv = (uint_fast16_t)(256 - src.A8());
      _r8a = (uint_fast16_t)(src.R8() * (1 + src.A8()));
      _g8a = (uint_fast16_t)(src.G8() * (1 + src.A8()));
      _b8a = (uint_fast16_t)(src.B8() * (1 + src.A8()));
    }
    template <typename TDstColor>
    void operator() (int32_t x, int32_t y, TDstColor& dst)
    {
      (void)x; (void)y;
      dst.set((_r8a + dst.R8() * _inv) >> 8
             ,(_g8a + dst.G8() * _inv) >> 8
             ,(_b8a + dst.B8() * _inv) >> 8
             );
    }
    template <typename TDstColor, typename TSrcColor>
    void operator() (int32_t x, int32_t y, TDstColor& dst, TSrcColor& src)
    {
      (void)x; (void)y;
      uint_fast16_t  a8 = 1 + src.A8();
      uint_fast16_t inv = 257 - a8;
      uint_fast16_t r8a = a8 * src.R8();
      uint_fast16_t g8a = a8 * src.G8();
      uint_fast16_t b8a = a8 * src.B8();

      dst.set((r8a + dst.R8() * inv) >> 8
             ,(g8a + dst.G8() * inv) >> 8
             ,(b8a + dst.B8() * inv) >> 8
             );
    }
  private:
    uint_fast16_t _inv;
    uint_fast16_t _r8a;
    uint_fast16_t _g8a;
    uint_fast16_t _b8a;
  };

//----------------------------------------------------------------------------
 }
}

using RGBColor = lgfx::bgr888_t;