#ifndef LGFX_COMMON_HPP_
#define LGFX_COMMON_HPP_

#include <type_traits>
#include <algorithm>
#include <string.h>

namespace lgfx
{
  enum jpeg_div_t {
    JPEG_DIV_NONE,
    JPEG_DIV_2,
    JPEG_DIV_4,
    JPEG_DIV_8,
    JPEG_DIV_MAX
  };

  enum textdatum_t : uint8_t
  //  0:left   1:centre   2:right
  //  0:top    4:middle   8:bottom   16:baseline
  { top_left        =  0  // Top left (default)
  , top_center      =  1  // Top center
  , top_right       =  2  // Top right
  , middle_left     =  4  // Middle left
  , middle_center   =  5  // Middle center
  , middle_right    =  6  // Middle right
  , bottom_left     =  8  // Bottom left
  , bottom_center   =  9  // Bottom center
  , bottom_right    = 10  // Bottom right
  , baseline_left   = 16  // Left character baseline (Line the 'A' character would sit on)
  , baseline_center = 17  // Centre character baseline
  , baseline_right  = 18  // Right character baseline
  };

  enum color_depth_t : uint8_t
  { palette_1bit   =  1 //   2 color
  , palette_2bit   =  2 //   4 color
  , palette_4bit   =  4 //  16 color
  , rgb332_1Byte   =  8 // RRRGGGBB
  , rgb565_2Byte   = 16 // RRRRRGGG GGGBBBBB
  , rgb666_3Byte   = 18 // xxRRRRRR xxGGGGGG xxBBBBBB
  , rgb888_3Byte   = 24 // RRRRRRRR GGGGGGGG BBBBBBBB
  , argb8888_4Byte = 32 // AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB
  };




  __attribute__ ((always_inline)) inline static uint8_t  color332(uint8_t r, uint8_t g, uint8_t b) { return (r >> 5) << 5 | (g >> 5) << 2 | b >> 6; }
  __attribute__ ((always_inline)) inline static uint16_t color565(uint8_t r, uint8_t g, uint8_t b) { return (r >> 3) <<11 | (g >> 2) << 5 | b >> 3; }
  __attribute__ ((always_inline)) inline static uint32_t color888(uint8_t r, uint8_t g, uint8_t b) { return  r << 16 | g << 8 | b; }
  __attribute__ ((always_inline)) inline static uint16_t swap565( uint8_t r, uint8_t g, uint8_t b) { return ((b >> 3) << 8) | ((g >> 2) << 13) | ((g >> 5) | ((r>>3)<<3)); }
  __attribute__ ((always_inline)) inline static uint32_t swap888( uint8_t r, uint8_t g, uint8_t b) { return (b << 16) | (g << 8) | r; }

  __attribute__ ((always_inline)) inline static uint16_t getSwap16(uint16_t c) { return __builtin_bswap16(c); }
  __attribute__ ((always_inline)) inline static uint32_t getSwap24(uint32_t c) { return ((uint8_t)c<<8 | (uint8_t)(c>>8))<<8 | (uint8_t)(c>>16); }
/*
  uint32_t convert_bgr888_to_rgb888( uint32_t c) { return getSwap24(c);  }
  uint32_t convert_bgr888_to_bgr666( uint32_t c) { return (c>>2) & 0x3F3F3F;  }
  uint32_t convert_bgr888_to_swap565(uint32_t c) { return  (((uint8_t)c) >> 3) << 3 |  ((uint16_t)c) >> 13 | (c & 0x1C00) << 3 | (c>>19) << 8; }
  uint32_t convert_bgr888_to_rgb565( uint32_t c) { return  (((uint8_t)c) >> 3) << 11 | (((uint16_t)c)>>10)<<5 | c>>19; }
  uint32_t convert_bgr888_to_rgb332( uint32_t c) { return ((uint8_t)c >> 5) << 5 | (((uint16_t)c)>>13) << 2 | c>>22; }
//*/
  uint32_t convert_rgb565_to_rgb888( uint32_t c) { return ((((c>>11)*0x21)>>2)<<8 | ((((c>>5)&0x3F)*0x41)>>4))<<8 | (((c&0x1F)*0x21)>>2); }
  uint32_t convert_rgb332_to_rgb888( uint32_t c) { return ((((c>>5)*0x49) >> 1)<<8 | ((c&0x1C)*0x49)>>3)<<8 | ((c&3)*0x55); }
  uint32_t convert_rgb888_to_bgr888( uint32_t c) { return getSwap24(c);  }
  uint32_t convert_rgb888_to_bgr666( uint32_t c) { return ((c>>2) & 0x3F) << 16 | ((c >> 10) & 0x3F) << 8 | ((c>>18)&0x3F);  }
  uint32_t convert_rgb888_to_swap565(uint32_t c) { return  (c>>19) << 3 |  ((uint16_t)c) >> 13 | (c & 0x1C00) << 3 | (((uint8_t)c) >> 3) << 8; }
  uint32_t convert_rgb888_to_rgb565( uint32_t c) { return  (c>>19) << 11 | (((uint16_t)c)>>10)<<5 | ((uint8_t)c) >> 3;   }
  uint32_t convert_rgb888_to_rgb332( uint32_t c) { return ((c>>21) << 5) | ((((uint16_t)c)>>13) << 2) | ((c>>6) & 3); }
  uint32_t convert_rgb565_to_bgr888( uint32_t c) { return ((((c&0x1F)*0x21)>>2)<<8 | ((((c>>5)&0x3F)*0x41)>>4))<<8 | (((c>>11)*0x21)>>2); }
  uint32_t convert_rgb565_to_bgr666( uint32_t c) { return ((c&0x1F)<<17) | ((c&0x10)<<12) | ((c&0x7E0)<<3) | ((c>>10)&0xF8) | (c>>15); }
  uint32_t convert_rgb565_to_swap565(uint32_t c) { return c<<8|c>>8; }
  uint32_t convert_rgb565_to_rgb332( uint32_t c) { return ((c>>13) <<5) | ((c>>6) & 0x1C) | ((c>>3) & 3); }
  uint32_t convert_rgb332_to_bgr888( uint32_t c) { return (((c&3)*0x55)<<8 | ((c&0x1C)*0x49)>>3)<<8 | (((c>>5)*0x49) >> 1); }
  uint32_t convert_rgb332_to_bgr666( uint32_t c) { return (((c&0xE0)*9)>>5) | ((c&0x1C)*0x240) | ((c&3)*0x15)<<16; }
  uint32_t convert_rgb332_to_swap565(uint32_t c) { return (((c&3)*0x15)>>1)<<8 | ((c&0x1C)<<11) | ((c&0x1C)>>2) | (((c>>5)*0x24)&0xF8); }
  uint32_t convert_uint32_to_palette8(uint32_t c) { return  c&0xFF; }
  uint32_t convert_uint32_to_palette4(uint32_t c) { return (c&0x0F) * 0x11; }
  uint32_t convert_uint32_to_palette2(uint32_t c) { return (c&0x03) * 0x55; }
  uint32_t convert_uint32_to_palette1(uint32_t c) { return (c&1)?0xFF:0; }
  uint32_t no_convert(uint32_t c) { return c; }

#pragma pack(1)
  struct rgb332_t;    //  8bpp
  struct rgb565_t;    // 16bpp
  struct rgb888_t;    // 24bpp
  struct argb8888_t;  // 32bpp
  struct swap565_t;   // 16bpp
  struct bgr666_t;    // 18bpp
  struct bgr888_t;    // 24bpp


  struct rgb332_t {
    union {
      struct {
        uint8_t b: 2;
        uint8_t g: 3;
        uint8_t r: 3;
      };
      uint8_t raw;
    };
    static constexpr uint8_t bits = 8;
    static constexpr bool swapped = false;
    static constexpr color_depth_t depth = rgb332_1Byte;
    rgb332_t() : raw(0) {}
    rgb332_t(const rgb332_t&) = default;
    rgb332_t(uint8_t r8, uint8_t g8, uint8_t b8) : raw(color332(r8,g8,b8)) {}
    rgb332_t(uint8_t rgb332) : raw(rgb332) {}
    inline rgb332_t& operator=(const rgb565_t&);
    inline rgb332_t& operator=(const rgb888_t&);
    inline rgb332_t& operator=(const argb8888_t&);
    inline rgb332_t& operator=(const swap565_t&);
    inline rgb332_t& operator=(const bgr666_t&);
    inline rgb332_t& operator=(const bgr888_t&);
    inline rgb332_t& operator=(uint8_t rgb332) { raw = rgb332; return *this; }
    explicit inline operator uint8_t() const { return raw; }
//  explicit inline operator uint32_t() const { return raw; }
    explicit inline operator bool() const { return raw; }
//  inline operator rgb565_t() const;
//  inline operator rgb888_t() const;
//  inline operator argb8888_t() const;
//  inline operator swap565_t() const;
//  inline operator bgr888_t() const;
    inline uint8_t R8() const { return (r * 0x49) >> 1; } // (r<<5)|(r<<2)|(r>>1);
    inline uint8_t G8() const { return (g * 0x49) >> 1; } // (g<<5)|(g<<2)|(g>>1);
    inline uint8_t B8() const { return  b * 0x55; }       // (b<<6)|(b<<4)|(b<<2)|b;
    inline uint8_t R6() const { return  r | r<<3; }
    inline uint8_t G6() const { return  g | g<<3; }
    inline uint8_t B6() const { return  b * 0x15; }
    inline void R8(uint8_t r8) { r = r8 >> 5; }
    inline void G8(uint8_t g8) { g = g8 >> 5; }
    inline void B8(uint8_t b8) { b = b8 >> 6; }
  };

  struct rgb565_t {
    union {
      struct {
        uint16_t b: 5;
        uint16_t g: 6;
        uint16_t r: 5;
      };
      uint16_t raw;
    };
    static constexpr uint8_t bits = 16;
    static constexpr bool swapped = false;
    static constexpr color_depth_t depth = rgb565_2Byte;
    rgb565_t() : raw(0) {}
    rgb565_t(const rgb565_t&) = default;
    rgb565_t(uint16_t rgb565) : raw(rgb565) {}
    rgb565_t(uint8_t r8, uint8_t g8, uint8_t b8) : raw(color565(r8,g8,b8)) {} // b(b8>>3),g(g8>>2),r(r8>>3) {}
    inline rgb565_t& operator=(const rgb332_t&);
    inline rgb565_t& operator=(const rgb888_t&);
    inline rgb565_t& operator=(const argb8888_t&);
    inline rgb565_t& operator=(const swap565_t&);
    inline rgb565_t& operator=(const bgr666_t&);
    inline rgb565_t& operator=(const bgr888_t&);
    inline rgb565_t& operator=(uint16_t rgb565) { raw = rgb565; return *this; }
    explicit inline operator uint16_t() const { return raw; }
    explicit inline operator bool() const { return raw; }
    //inline operator rgb332_t() const { return rgb332_t(r<<3,g<<2,b<<3); }
    //inline operator rgb888_t() const;
    //inline operator argb8888_t() const;
    //inline operator swap565_t() const;
    //inline operator bgr888_t() const;
    inline uint8_t R8() const { return (r * 0x21) >> 2; } // (r << 3) | (r >> 2);
    inline uint8_t G8() const { return (g * 0x41) >> 4; } // (g << 2) | (g >> 4);
    inline uint8_t B8() const { return (b * 0x21) >> 2; } // (b << 3) | (b >> 2);
    inline uint8_t R6() const { return (r * 0x21) >> 4; }
    inline uint8_t G6() const { return  g; }
    inline uint8_t B6() const { return (b * 0x21) >> 4; }
    inline void R8(uint8_t r8) { r = r8 >> 3; }
    inline void G8(uint8_t g8) { g = g8 >> 2; }
    inline void B8(uint8_t b8) { b = b8 >> 3; }
  };

  struct rgb888_t {
    union {
      struct {
        uint8_t b;
        uint8_t g;
        uint8_t r;
      };
    };
    static constexpr uint8_t bits = 24;
    static constexpr bool swapped = false;
    static constexpr color_depth_t depth = rgb888_3Byte;
    rgb888_t() : b(0), g(0), r(0) {}
    rgb888_t(const rgb888_t&) = default;
    rgb888_t(uint8_t r8, uint8_t g8, uint8_t b8) : b(b8),g(g8),r(r8) {}
    rgb888_t(uint32_t rgb888) : b(rgb888), g(rgb888>>8), r(rgb888>>16) {}
    inline rgb888_t& operator=(const rgb332_t&);
    inline rgb888_t& operator=(const rgb565_t&);
    inline rgb888_t& operator=(const argb8888_t&);
    inline rgb888_t& operator=(const swap565_t&);
    inline rgb888_t& operator=(const bgr666_t&);
    inline rgb888_t& operator=(const bgr888_t&);
    inline rgb888_t& operator=(uint32_t rgb888) { r = rgb888>>16; g = rgb888>>8; b = rgb888; return *this; }
//  explicit inline operator uint32_t() const { return *(uint32_t*)this & ((1<<24)-1); }
    explicit inline operator bool() const { return r||g||b; }
    //inline operator rgb332_t() const { return rgb332_t(r,g,b); }
    //inline operator rgb565_t() const { return rgb565_t(r,g,b); }
    //inline operator argb8888_t() const;
    //inline operator swap565_t() const;
    //inline operator bgr888_t() const;
    inline uint8_t R8() const { return r; }
    inline uint8_t G8() const { return g; }
    inline uint8_t B8() const { return b; }
    inline uint8_t R6() const { return r>>2; }
    inline uint8_t G6() const { return g>>2; }
    inline uint8_t B6() const { return b>>2; }
    inline void R8(uint8_t r8) { r = r8; }
    inline void G8(uint8_t g8) { g = g8; }
    inline void B8(uint8_t b8) { b = b8; }
    inline void set(uint8_t r8, uint8_t g8, uint8_t b8) { r = r8; g = g8; b = b8; }
  };

  struct argb8888_t {
    union {
      struct {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;
      };
      uint32_t raw;
    };
    static constexpr uint8_t bits = 32;
    static constexpr bool swapped = false;
    static constexpr color_depth_t depth = argb8888_4Byte;
    argb8888_t() : raw(0) {}
    argb8888_t(const argb8888_t&) = default;
    argb8888_t(uint8_t r, uint8_t g, uint8_t b) : b(b),g(g),r(r),a(0) {}
    argb8888_t(uint32_t argb8888) : raw(argb8888) {}
    inline argb8888_t& operator=(const rgb332_t&);
    inline argb8888_t& operator=(const rgb565_t&);
    inline argb8888_t& operator=(const rgb888_t&);
    inline argb8888_t& operator=(const swap565_t&);
    inline argb8888_t& operator=(const bgr666_t&);
    inline argb8888_t& operator=(const bgr888_t&);
    inline argb8888_t& operator=(uint32_t argb8888) { raw = argb8888; return *this; }
//  explicit inline operator uint32_t() const { return raw; }
    explicit inline operator bool() const { return raw; }
    //inline operator rgb332_t() const { return rgb332_t(r,g,b); }
    //inline operator rgb565_t() const { return rgb565_t(r,g,b); }
    //inline operator rgb888_t() const { return operator uint32_t(); }
    //inline operator swap565_t() const;
    //inline operator bgr888_t() const;
    inline uint8_t R8() const { return r; }
    inline uint8_t G8() const { return g; }
    inline uint8_t B8() const { return b; }
    inline uint8_t R6() const { return r>>2; }
    inline uint8_t G6() const { return g>>2; }
    inline uint8_t B6() const { return b>>2; }
    inline void R8(uint8_t r8) { r = r8; }
    inline void G8(uint8_t g8) { g = g8; }
    inline void B8(uint8_t b8) { b = b8; }
  };

  struct swap565_t {
    union {
      struct {
        uint16_t gh:3;
        uint16_t r:5;
        uint16_t b:5;
        uint16_t gl:3;
      };
      uint16_t raw;
    };
    static constexpr uint8_t bits = 16;
    static constexpr bool swapped = true;
    static constexpr color_depth_t depth = rgb565_2Byte;
    swap565_t() : raw(0) {}
    swap565_t(const swap565_t&) = default;
    swap565_t(uint8_t r8, uint8_t g8, uint8_t b8) : gh(g8>>5),r(r8>>3),b(b8>>3),gl(g8>>2) {}
//    swap565_t(uint16_t raw) : raw(raw) {}
    inline swap565_t& operator=(uint32_t rhs) { raw = rhs; return *this; }
    inline swap565_t& operator=(const rgb332_t& rhs);
    inline swap565_t& operator=(const rgb565_t& rhs);
    inline swap565_t& operator=(const rgb888_t& rhs);
    inline swap565_t& operator=(const argb8888_t& rhs);
    inline swap565_t& operator=(const bgr666_t& rhs);
    inline swap565_t& operator=(const bgr888_t& rhs);
    //explicit inline operator uint16_t() const { return raw; }
//  explicit inline operator uint32_t() const { return raw; }
    explicit inline operator bool() const { return r||gh||gl||b; }
    //inline operator rgb565_t() const { return rgb565_t(raw<<8 | raw>>8); }
    //inline operator bgr888_t() const;
    inline uint8_t R8() const { return ( r * 0x21) >> 2; }
    inline uint8_t G8() const { return ((gh* 0x41) >> 1)|(gl << 2); }
    inline uint8_t B8() const { return ( b * 0x21) >> 2; }
    inline uint8_t R6() const { return ( r * 0x21) >> 4; }
    inline uint8_t G6() const { return  gh << 3 | gl; }
    inline uint8_t B6() const { return ( b * 0x21) >> 4; }
  };

  struct bgr666_t {
    union {
      struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
      };
    };
    static constexpr uint8_t bits = 24;
    static constexpr bool swapped = true;
    static constexpr color_depth_t depth = rgb666_3Byte;
    bgr666_t() : r(0), g(0), b(0) {};
    bgr666_t(const bgr666_t&) = default;
    bgr666_t(uint8_t r8, uint8_t g8, uint8_t b8) : r(r8>>2),g(g8>>2),b(b8>>2) {}
    inline bgr666_t& operator=(uint32_t rhs) { r = rhs; g = rhs>>8 ; b = rhs>>16; return *this; }
    inline bgr666_t& operator=(const rgb332_t&);
    inline bgr666_t& operator=(const rgb565_t&);
    inline bgr666_t& operator=(const rgb888_t&);
    inline bgr666_t& operator=(const argb8888_t&);
    inline bgr666_t& operator=(const swap565_t&);
    inline bgr666_t& operator=(const bgr888_t&);
//  explicit inline operator uint32_t() const { return *(uint32_t*)this & ((1<<24)-1); }
    explicit inline operator bool() const { return r||g||b; }
    //inline operator uint32_t() const { return (b<<16)|(g<<8)|r; }
    //inline operator rgb332_t() const { return rgb332_t(r,g,b); }
    //explicit inline operator rgb565_t() const { return rgb565_t(r,g,b); }
    //inline operator rgb888_t() const { return rgb888_t(r,g,b); }
    //inline operator swap565_t() const { return swap565_t(r,g,b); }
    inline uint8_t R8() const { return r<<2; }
    inline uint8_t G8() const { return g<<2; }
    inline uint8_t B8() const { return b<<2; }
    inline uint8_t R6() const { return r; }
    inline uint8_t G6() const { return g; }
    inline uint8_t B6() const { return b; }
    inline void R8(uint8_t r8) { r = r8>>2; }
    inline void G8(uint8_t g8) { g = g8>>2; }
    inline void B8(uint8_t b8) { b = b8>>2; }
  };

  struct bgr888_t {
    union {
      struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
      };
    };
    static constexpr uint8_t bits = 24;
    static constexpr bool swapped = true;
    static constexpr color_depth_t depth = rgb888_3Byte;
    bgr888_t() : r{0}, g{0}, b{0} {};
    bgr888_t(const bgr888_t&) = default;
    bgr888_t(uint8_t r8, uint8_t g8, uint8_t b8) : r(r8),g(g8),b(b8) {}
//    bgr888_t(uint32_t bgr888) : r(bgr888), g(bgr888>>8), b(bgr888>>16) {}
    inline bgr888_t& operator=(uint32_t rhs) { r = rhs; g = rhs>>8 ; b = rhs>>16; return *this; }
    inline bgr888_t& operator=(const rgb332_t&);
    inline bgr888_t& operator=(const rgb565_t&);
    inline bgr888_t& operator=(const rgb888_t&);
    inline bgr888_t& operator=(const argb8888_t&);
    inline bgr888_t& operator=(const swap565_t&);
    inline bgr888_t& operator=(const bgr666_t&);
    explicit inline operator bool() const { return r||g||b; }
    explicit inline operator uint32_t() const { return *(uint32_t*)this & ((1<<24)-1); }
    //inline operator rgb332_t() const { return rgb332_t(r,g,b); }
    explicit inline operator rgb565_t() const { return rgb565_t(r,g,b); }
    //inline operator rgb888_t() const { return rgb888_t(r,g,b); }
    //inline operator swap565_t() const { return swap565_t(r,g,b); }
    inline uint8_t R8() const { return r; }
    inline uint8_t G8() const { return g; }
    inline uint8_t B8() const { return b; }
    inline uint8_t R6() const { return r>>2; }
    inline uint8_t G6() const { return g>>2; }
    inline uint8_t B6() const { return b>>2; }
    inline void R8(uint8_t r8) { r = r8; }
    inline void G8(uint8_t g8) { g = g8; }
    inline void B8(uint8_t b8) { b = b8; }
    inline void set(uint8_t r8, uint8_t g8, uint8_t b8) { r = r8; g = g8; b = b8; }
//    inline void set( uint16_t rgb565 ) { operator=(*(rgb565_t*)&color565); }
  };

  struct raw_color_t
  {
    union {
      struct {
        uint8_t raw0;
        uint8_t raw1;
        uint8_t raw2;
        uint8_t raw3;
      };
      struct {
        uint16_t rawL;
        uint16_t rawH;
      };
      uint32_t raw;
    };
    raw_color_t() : raw(0) {}
    raw_color_t(const raw_color_t&) = default;
    raw_color_t(const uint32_t& rhs) : raw(rhs) {}
    inline operator bool() const { return raw & 0x00FFFFFF; }
  };

  struct color_conv_t
  {
//    uint32_t (*convert_bgr888)(uint32_t) = convert_bgr888_to_swap565;
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

    void setColorDepth(color_depth_t bpp, bool hasPalette = false) {
      x_mask = 0;
      if (     bpp > 18) { bpp = rgb888_3Byte; bytes = 3; bits = 24; }
      else if (bpp > 16) { bpp = rgb666_3Byte; bytes = 3; bits = 24; }
      else if (bpp >  8) { bpp = rgb565_2Byte; bytes = 2; bits = 16; }
      else if (bpp >  4) { bpp = rgb332_1Byte; bytes = 1; bits =  8; }
      else if (bpp == 4) { bpp = palette_4bit; bytes = 0; bits =  4; x_mask = 0b0001; }
      else if (bpp == 2) { bpp = palette_2bit; bytes = 0; bits =  2; x_mask = 0b0011; }
      else               { bpp = palette_1bit; bytes = 0; bits =  1; x_mask = 0b0111; }

      colormask = (1 << bits) - 1;
      depth = bpp;
      switch (bpp) {
      case rgb888_3Byte:
//        convert_bgr888 = no_convert;
        convert_rgb888 = convert_rgb888_to_bgr888;
        convert_rgb565 = convert_rgb565_to_bgr888;
        convert_rgb332 = convert_rgb332_to_bgr888;
        break;
      case rgb666_3Byte:
//        convert_bgr888 = convert_bgr888_to_bgr666;
        convert_rgb888 = convert_rgb888_to_bgr666;
        convert_rgb565 = convert_rgb565_to_bgr666;
        convert_rgb332 = convert_rgb332_to_bgr666;
        break;
      default:
      case rgb565_2Byte:
//        convert_bgr888 = convert_bgr888_to_swap565;
        convert_rgb888 = convert_rgb888_to_swap565;
        convert_rgb565 = convert_rgb565_to_swap565;
        convert_rgb332 = convert_rgb332_to_swap565;
        break;
      case rgb332_1Byte:
        if (!hasPalette) {
//          convert_bgr888 = convert_bgr888_to_rgb332;
          convert_rgb888 = convert_rgb888_to_rgb332;
          convert_rgb565 = convert_rgb565_to_rgb332;
          convert_rgb332 = no_convert;
          break;
        }
//        convert_bgr888 = convert_uint32_to_palette8;
        convert_rgb888 = convert_uint32_to_palette8;
        convert_rgb565 = convert_uint32_to_palette8;
        convert_rgb332 = convert_uint32_to_palette8;
        break;
      case palette_4bit:
//        convert_bgr888 = convert_uint32_to_palette4;
        convert_rgb888 = convert_uint32_to_palette4;
        convert_rgb565 = convert_uint32_to_palette4;
        convert_rgb332 = convert_uint32_to_palette4;
        break;
      case palette_2bit:
//        convert_bgr888 = convert_uint32_to_palette2;
        convert_rgb888 = convert_uint32_to_palette2;
        convert_rgb565 = convert_uint32_to_palette2;
        convert_rgb332 = convert_uint32_to_palette2;
        break;
      case palette_1bit:
//        convert_bgr888 = convert_uint32_to_palette1;
        convert_rgb888 = convert_uint32_to_palette1;
        convert_rgb565 = convert_uint32_to_palette1;
        convert_rgb332 = convert_uint32_to_palette1;
        break;
      }
    }

#define TYPECHECK(dType) template < typename T, typename std::enable_if < (sizeof(T) == sizeof(dType)) && (std::is_signed<T>::value == std::is_signed<dType>::value), std::nullptr_t >::type=nullptr >
    TYPECHECK(uint8_t ) __attribute__ ((always_inline)) inline uint32_t convert(T c) { return convert_rgb332(c); }
    TYPECHECK(uint16_t) __attribute__ ((always_inline)) inline uint32_t convert(T c) { return convert_rgb565(c); }
    TYPECHECK(int16_t ) __attribute__ ((always_inline)) inline uint32_t convert(T c) { return convert_rgb565(c); }
    TYPECHECK(int     ) __attribute__ ((always_inline)) inline uint32_t convert(T c) { return convert_rgb565(c); }
    TYPECHECK(uint32_t) __attribute__ ((always_inline)) inline uint32_t convert(T c) { return convert_rgb888(c); }

    __attribute__ ((always_inline)) inline uint32_t convert(const argb8888_t& c) { return convert_rgb888(c.raw); }
    __attribute__ ((always_inline)) inline uint32_t convert(const rgb888_t&   c) { return convert_rgb888(*(uint32_t*)&c); }
    __attribute__ ((always_inline)) inline uint32_t convert(const rgb565_t&   c) { return convert_rgb565(c.raw); }
    __attribute__ ((always_inline)) inline uint32_t convert(const rgb332_t&   c) { return convert_rgb332(c.raw); }
//    __attribute__ ((always_inline)) inline uint32_t convert(const bgr888_t&   c) { return convert_bgr888(*(uint32_t*)&c); }

//  template<typename T> __attribute__ ((always_inline)) inline void setColor(T c) { raw = convert(c); }
  };

  TYPECHECK(uint8_t ) __attribute__ ((always_inline)) inline uint32_t convert_to_rgb888(T c) { return convert_rgb332_to_rgb888(c); }
  TYPECHECK(uint16_t) __attribute__ ((always_inline)) inline uint32_t convert_to_rgb888(T c) { return convert_rgb565_to_rgb888(c); }
  TYPECHECK(int16_t ) __attribute__ ((always_inline)) inline uint32_t convert_to_rgb888(T c) { return convert_rgb565_to_rgb888(c); }
  TYPECHECK(int     ) __attribute__ ((always_inline)) inline uint32_t convert_to_rgb888(T c) { return convert_rgb565_to_rgb888(c); }
  TYPECHECK(uint32_t) __attribute__ ((always_inline)) inline uint32_t convert_to_rgb888(T c) { return c; }

#undef TYPECHECK

#pragma pack()

  //inline rgb332_t::operator rgb565_t() const   { return rgb565_t(((r*0x2400) & 0xF800) | (g*0x0120) | ((b*0x15)>>1)); }
  //inline rgb332_t::operator rgb888_t() const   { return rgb888_t(R8(),G8(),B8()); }
  //inline rgb332_t::operator argb8888_t() const { return argb8888_t(R8(),G8(),B8()); }
  //inline rgb332_t::operator swap565_t() const  { return operator rgb565_t(); }
  //inline rgb332_t::operator bgr888_t() const  { return bgr888_t(R8(),G8(),B8()); }
  //inline rgb565_t::operator rgb888_t() const   { return rgb888_t(R8(),G8(),B8()); }
  //inline rgb565_t::operator argb8888_t() const { return argb8888_t(R8(),G8(),B8()); }
  //inline rgb565_t::operator swap565_t() const  { return swap565_t(raw<<8 | raw>>8); }
  //inline rgb565_t::operator bgr888_t() const  { return bgr888_t(R8(),G8(),B8()); }
  //inline rgb888_t::operator argb8888_t() const { return argb8888_t(r,g,b); }
  //inline rgb888_t::operator swap565_t() const  { return operator rgb565_t(); }
  //inline rgb888_t::operator bgr888_t() const  { return bgr888_t(r,g,b); }
  //inline argb8888_t::operator swap565_t() const { return operator rgb565_t(); }
  //inline argb8888_t::operator bgr888_t() const { return bgr888_t(r,g,b); }
  //inline swap565_t::operator bgr888_t() const { return operator rgb565_t(); }

  inline rgb332_t& rgb332_t::operator=(const rgb565_t&   rhs) { raw = ((rhs.r<<3)&0xE0) | ((rhs.g>>1)&0x1C) | (rhs.b>>3); return *this; }
  inline rgb332_t& rgb332_t::operator=(const swap565_t&  rhs) { raw = ((rhs.r<<3)&0xE0) | (rhs.gh<<2) | (rhs.b>>3); return *this; }
  inline rgb332_t& rgb332_t::operator=(const bgr666_t&   rhs) { raw = ((rhs.r<<2)&0xE0) | ((rhs.g>>1)&0x1C) | (rhs.b>>4); return *this; }
  inline rgb332_t& rgb332_t::operator=(const rgb888_t&   rhs) { raw = color332(rhs.r, rhs.g, rhs.b); return *this; }
  inline rgb332_t& rgb332_t::operator=(const bgr888_t&   rhs) { raw = color332(rhs.r, rhs.g, rhs.b); return *this; }
  inline rgb332_t& rgb332_t::operator=(const argb8888_t& rhs) { raw = color332(rhs.r, rhs.g, rhs.b); return *this; }

  inline rgb565_t& rgb565_t::operator=(const rgb332_t&   rhs) { raw = color565(rhs.R8(), rhs.G8(), rhs.B8()); return *this; }
  inline rgb565_t& rgb565_t::operator=(const swap565_t&  rhs) { raw = __builtin_bswap16(rhs.raw);   return *this; }
  inline rgb565_t& rgb565_t::operator=(const rgb888_t&   rhs) { raw = color565(rhs.r, rhs.g, rhs.b); return *this; }
  inline rgb565_t& rgb565_t::operator=(const bgr888_t&   rhs) { raw = color565(rhs.r, rhs.g, rhs.b); return *this; }
  inline rgb565_t& rgb565_t::operator=(const bgr666_t&   rhs) { raw = color565(rhs.R8(), rhs.G8(), rhs.B8()); return *this; }
  inline rgb565_t& rgb565_t::operator=(const argb8888_t& rhs) { raw = color565(rhs.r, rhs.g, rhs.b); return *this; }

  inline rgb888_t& rgb888_t::operator=(const rgb332_t&   rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline rgb888_t& rgb888_t::operator=(const rgb565_t&   rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline rgb888_t& rgb888_t::operator=(const swap565_t&  rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline rgb888_t& rgb888_t::operator=(const bgr666_t&   rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline rgb888_t& rgb888_t::operator=(const bgr888_t&   rhs) { r = rhs.r;    g = rhs.g;    b = rhs.b;    return *this; }
  inline rgb888_t& rgb888_t::operator=(const argb8888_t& rhs) { r = rhs.r;    g = rhs.g;    b = rhs.b;    return *this; }

  inline swap565_t& swap565_t::operator=(const rgb332_t&   rhs) { raw = ((rhs.b * 0x15)>>1)<<8 | rhs.g << 13 | rhs.g | ((rhs.r * 0x09) >> 1) << 3; return *this; }
  inline swap565_t& swap565_t::operator=(const rgb565_t&   rhs) { raw = __builtin_bswap16(rhs.raw);            return *this; }
  inline swap565_t& swap565_t::operator=(const rgb888_t&   rhs) { raw = swap565(rhs.r,    rhs.g,    rhs.b);    return *this; }
  inline swap565_t& swap565_t::operator=(const bgr666_t&   rhs) { raw = (rhs.b>>1)<<8 | rhs.g << 13 | rhs.g >> 3 | (rhs.r >> 1) << 3; return *this; }
  inline swap565_t& swap565_t::operator=(const bgr888_t&   rhs) { raw = swap565(rhs.r,    rhs.g,    rhs.b);    return *this; }
  inline swap565_t& swap565_t::operator=(const argb8888_t& rhs) { raw = swap565(rhs.r,    rhs.g,    rhs.b);    return *this; }

  inline bgr666_t& bgr666_t::operator=(const rgb332_t&   rhs) { r = rhs.R6(); g = rhs.G6(); b = rhs.B6(); return *this; }
  inline bgr666_t& bgr666_t::operator=(const rgb565_t&   rhs) { r = rhs.R6(); g = rhs.G6(); b = rhs.B6(); return *this; }
  inline bgr666_t& bgr666_t::operator=(const swap565_t&  rhs) { r = rhs.R6(); g = rhs.G6(); b = rhs.B6(); return *this; }
  inline bgr666_t& bgr666_t::operator=(const bgr888_t&   rhs) { r = rhs.R6(); g = rhs.G6(); b = rhs.B6(); return *this; }
  inline bgr666_t& bgr666_t::operator=(const rgb888_t&   rhs) { r = rhs.R6(); g = rhs.G6(); b = rhs.B6(); return *this; }
  inline bgr666_t& bgr666_t::operator=(const argb8888_t& rhs) { r = rhs.R6(); g = rhs.G6(); b = rhs.B6(); return *this; }

  inline bgr888_t& bgr888_t::operator=(const rgb332_t&   rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline bgr888_t& bgr888_t::operator=(const rgb565_t&   rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline bgr888_t& bgr888_t::operator=(const swap565_t&  rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline bgr888_t& bgr888_t::operator=(const bgr666_t&   rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline bgr888_t& bgr888_t::operator=(const rgb888_t&   rhs) { r = rhs.r   ; g = rhs.g   ; b = rhs.b   ; return *this; }
  inline bgr888_t& bgr888_t::operator=(const argb8888_t& rhs) { r = rhs.r   ; g = rhs.g   ; b = rhs.b   ; return *this; }

  inline argb8888_t& argb8888_t::operator=(const rgb332_t&  rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline argb8888_t& argb8888_t::operator=(const rgb565_t&  rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline argb8888_t& argb8888_t::operator=(const swap565_t& rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline argb8888_t& argb8888_t::operator=(const bgr666_t&  rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline argb8888_t& argb8888_t::operator=(const rgb888_t&  rhs) { r = rhs.r   ; g = rhs.g   ; b = rhs.b   ; return *this; }
  inline argb8888_t& argb8888_t::operator=(const bgr888_t&  rhs) { r = rhs.r   ; g = rhs.g   ; b = rhs.b   ; return *this; }

  inline bool operator==(const rgb332_t&   lhs, const rgb332_t&   rhs) { return lhs.raw == rhs.raw; }
  inline bool operator==(const rgb565_t&   lhs, const rgb565_t&   rhs) { return lhs.raw == rhs.raw; }
  inline bool operator==(const swap565_t&  lhs, const swap565_t&  rhs) { return lhs.raw == rhs.raw; }
  inline bool operator==(const bgr666_t&  lhs, const bgr666_t&  rhs) { return (*(uint32_t*)&lhs & 0x3F3F3F) == (*(uint32_t*)&rhs & 0x3F3F3F); }
  inline bool operator==(const rgb888_t&   lhs, const rgb888_t&   rhs) { return (*(uint32_t*)&lhs & 0xFFFFFF) == (*(uint32_t*)&rhs & 0xFFFFFF); }
  inline bool operator==(const bgr888_t&  lhs, const bgr888_t&  rhs) { return (*(uint32_t*)&lhs & 0xFFFFFF) == (*(uint32_t*)&rhs & 0xFFFFFF); }
  inline bool operator==(const argb8888_t& lhs, const argb8888_t& rhs) { return lhs.raw == rhs.raw; }

  inline bool operator==(const rgb332_t&   lhs, const uint32_t& rhs) { return lhs.raw == rhs; }
  inline bool operator==(const rgb565_t&   lhs, const uint32_t& rhs) { return lhs.raw == rhs; }
  inline bool operator==(const swap565_t&  lhs, const uint32_t& rhs) { return lhs.raw == rhs; }
  inline bool operator==(const bgr666_t&  lhs, const uint32_t& rhs) { return ((*(uint32_t*)&lhs) & 0x3F3F3F) == rhs; }
  inline bool operator==(const rgb888_t&   lhs, const uint32_t& rhs) { return ((*(uint32_t*)&lhs) & 0xFFFFFF) == rhs; }
  inline bool operator==(const bgr888_t&  lhs, const uint32_t& rhs) { return ((*(uint32_t*)&lhs) & 0xFFFFFF) == rhs; }
  inline bool operator==(const argb8888_t& lhs, const uint32_t& rhs) { return lhs.raw == rhs; }
//*/
  inline bool operator==(const raw_color_t& lhs, const raw_color_t& rhs) { return lhs.raw == rhs.raw; }
  inline bool operator!=(const raw_color_t& lhs, const raw_color_t& rhs) { return lhs.raw != rhs.raw; }



//----------------------------------------------------------------------------
  static constexpr uint32_t FP_SCALE = 16;

  struct pixelcopy_t {
    union {
      uint32_t src_x32 = 0;
      struct {
        uint16_t src_xl;
        uint16_t src_x;
      };
    };
    union {
      uint32_t src_y32 = 0;
      struct {
        uint16_t src_yl;
        uint16_t src_y;
      };
    };

    uint32_t src_x32_add = 1 << FP_SCALE;
    uint32_t src_y32_add = 0;
    uint32_t src_width = 0;
    uint32_t transp   = ~0;
    uint32_t src_bits = 8;
    uint32_t dst_bits = 8;
    uint8_t src_mask  = ~0;
    uint8_t dst_mask  = ~0;
    bool no_convert = false;
    const void* src_data = nullptr;
    const void* palette = nullptr;
    int32_t (*fp_copy)(void*, int32_t, int32_t, pixelcopy_t*);
    int32_t (*fp_skip)(       int32_t, int32_t, pixelcopy_t*);

    pixelcopy_t(void) {}


    pixelcopy_t( const void* src_data
               , color_depth_t dst_depth
               , color_depth_t src_depth
               , bool dst_palette = false
               , const void* src_palette = nullptr
               , uint32_t src_transp = ~0
               ) 
    : transp    ( src_transp )
    , src_data  ( src_data   )
    , palette   ( src_palette)
    {
      init(dst_depth, src_depth, dst_palette, src_palette);
    }

    template<typename TSrc>
    static auto get_fp_normalcopy(color_depth_t dst_depth) -> int32_t(*)(void*, int32_t, int32_t, pixelcopy_t*)
    {
      return (dst_depth == rgb565_2Byte) ? normalcopy<swap565_t, TSrc>
           : (dst_depth == rgb332_1Byte) ? normalcopy<rgb332_t , TSrc>
           : (dst_depth == rgb888_3Byte) ? normalcopy<bgr888_t, TSrc>
                                         : (std::is_same<bgr666_t, TSrc>::value)
                                           ? normalcopy<bgr888_t, bgr888_t>
                                           : normalcopy<bgr666_t, TSrc>;
/*
           : (dst_depth == rgb888_3Byte) ? normalcopy<bgr888_t, TSrc>
      switch (dst_depth) {
      case rgb332_1Byte: return normalcopy<rgb332_t , TSrc>;
      case rgb565_2Byte: return normalcopy<swap565_t, TSrc>;
      case rgb666_3Byte: return normalcopy<bgr666_t, TSrc>;
      case rgb888_3Byte: return normalcopy<bgr888_t, TSrc>;
      default:
        break;
      }
//*/
    }

    template<typename TDst>
    static auto get_fp_normalcopy_dst(color_depth_t src_depth) -> int32_t(*)(void*, int32_t, int32_t, pixelcopy_t*)
    {
      return (src_depth == rgb565_2Byte) ? normalcopy<TDst, swap565_t>
           : (src_depth == rgb332_1Byte) ? normalcopy<TDst, rgb332_t >
           : (src_depth == rgb888_3Byte) ? normalcopy<TDst, bgr888_t >
                                         : (std::is_same<bgr666_t, TDst>::value)
                                           ? normalcopy<bgr888_t, bgr888_t>
                                           : normalcopy<TDst, bgr666_t>;
    }

    template<typename TPalette>
    static auto get_fp_palettecopy(color_depth_t dst_depth) -> int32_t(*)(void*, int32_t, int32_t, pixelcopy_t*)
    {
      return (dst_depth == rgb565_2Byte) ? palettecopy<swap565_t, TPalette>
           : (dst_depth == rgb332_1Byte) ? palettecopy<rgb332_t , TPalette>
           : (dst_depth == rgb888_3Byte) ? palettecopy<bgr888_t, TPalette>
                                         : palettecopy<bgr666_t, TPalette>;
/*
      if (dst_depth > rgb565_2Byte) {
        if (dst_depth == rgb888_3Byte) {
          return palettecopy<bgr888_t, TPalette>;
        } else { // dst_depth == rgb666_3Byte:
          return palettecopy<bgr666_t, TPalette>;
        }
      } else {
        if (dst_depth == rgb565_2Byte) {
          return palettecopy<swap565_t, TPalette>;
        } else { // dst_depth == rgb332_1Byte:
          return palettecopy<rgb332_t, TPalette>;
        }
      }
//*/
    }

  protected:
    void init( color_depth_t dst_depth
             , color_depth_t src_depth
             , bool dst_palette
             , bool src_palette)
    {
      dst_bits = dst_depth > 8 ? (dst_depth + 7) & ~7 : dst_depth;
      dst_mask = (1 << dst_bits) - 1;
      src_bits = src_depth > 8 ? (src_depth + 7) & ~7 : src_depth;
      src_mask = (1 << src_bits) - 1;

      if (dst_palette || dst_depth < 8) {
        if (src_palette && (dst_depth == 8) && (src_depth == 8)) {
          no_convert = true;
          fp_copy = pixelcopy_t::normalcopy<rgb332_t, rgb332_t>;
          fp_skip = pixelcopy_t::normalskip<rgb332_t>;
        } else {
          fp_copy = pixelcopy_t::bitcopy;
          fp_skip = pixelcopy_t::bitskip;
        }
      } else 
      if (src_palette || src_depth < 8) {
        fp_copy = pixelcopy_t::get_fp_palettecopy<bgr888_t>(dst_depth);
        fp_skip = pixelcopy_t::bitskip;
      } else {
        no_convert = (src_depth == dst_depth);
        if (src_depth > rgb565_2Byte) {
          fp_skip = pixelcopy_t::normalskip<bgr888_t>;
          if (src_depth == rgb888_3Byte) {
            fp_copy = pixelcopy_t::get_fp_normalcopy<bgr888_t>(dst_depth);
          } else if (src_depth == rgb666_3Byte) {
            fp_copy = pixelcopy_t::get_fp_normalcopy<bgr666_t>(dst_depth);
          }
        } else {
          if (src_depth == rgb565_2Byte) {
            fp_copy = pixelcopy_t::get_fp_normalcopy<swap565_t>(dst_depth);
            fp_skip = pixelcopy_t::normalskip<swap565_t>;
          } else { // src_depth == rgb332_1Byte:
            fp_copy = pixelcopy_t::get_fp_normalcopy<rgb332_t >(dst_depth);
            fp_skip = pixelcopy_t::normalskip<rgb332_t>;
          }
        }
      }
    }

    static int32_t bitcopy(void* dst, int32_t index, int32_t last, pixelcopy_t* param)
    {
      auto s = (const uint8_t*)param->src_data;
      auto d = (uint8_t*)dst;
      auto src_x32     = param->src_x32;
      auto src_y32     = param->src_y32;
      auto src_x32_add = param->src_x32_add;
      auto src_y32_add = param->src_y32_add;
      auto src_width   = param->src_width;
      auto transp      = param->transp;
      auto src_bits    = param->src_bits;
      auto dst_bits    = param->dst_bits;
      auto src_mask    = param->src_mask;
      auto dst_mask    = param->dst_mask;
      do {
        uint32_t i = ((src_x32 >> FP_SCALE) + (src_y32 >> FP_SCALE) * src_width) * src_bits;
        src_x32 += src_x32_add;
        src_y32 += src_y32_add;
        uint32_t raw = (s[i >> 3] >> (-(i + src_bits) & 7)) & src_mask;
        if (raw != transp) {
          auto dstidx = index * dst_bits;
          auto shift = (-(dstidx + dst_bits)) & 7;
          auto tmp = &d[dstidx >> 3];
          *tmp = (*tmp & ~(dst_mask << shift)) | ((dst_mask & raw) << shift);
        }
      } while (++index != last);
      param->src_x32 = src_x32;
      param->src_y32 = src_y32;
      return index;
    }

    template <typename TDst, typename TPalette>
    static int32_t palettecopy(void* dst, int32_t index, int32_t last, pixelcopy_t* param)
    {
      auto s = (const uint8_t*)param->src_data;
      auto d = (TDst*)dst;
      auto src_x32     = param->src_x32;
      auto src_y32     = param->src_y32;
      auto src_x32_add = param->src_x32_add;
      auto src_y32_add = param->src_y32_add;
      auto src_width   = param->src_width;
      auto transp      = param->transp;
      auto src_bits    = param->src_bits;
      auto src_mask    = param->src_mask;
      auto pal  = (const TPalette*)param->palette;
      do {
        uint32_t i = ((src_x32 >> FP_SCALE) + (src_y32 >> FP_SCALE) * src_width) * src_bits;
        uint32_t raw = (s[i >> 3] >> (-(i + src_bits) & 7)) & src_mask;
        if (raw == transp) break;
        d[index] = pal[raw];
        src_x32 += src_x32_add;
        src_y32 += src_y32_add;
      } while (++index != last);
      param->src_x32 = src_x32;
      param->src_y32 = src_y32;
      return index;
    }

    template <typename TDst, typename TSrc>
    static int32_t normalcopy(void* dst, int32_t index, int32_t last, pixelcopy_t* param)
    {
      auto s = (const TSrc*)param->src_data;
      auto d = (TDst*)dst;
      auto src_x32     = param->src_x32;
      auto src_y32     = param->src_y32;
      auto src_x32_add = param->src_x32_add;
      auto src_y32_add = param->src_y32_add;
      auto src_width   = param->src_width;
      auto transp      = param->transp;
      do {
        uint32_t i = (src_x32 >> FP_SCALE) + (src_y32 >> FP_SCALE) * src_width;
        if (s[i] == transp) break;
        d[index] = s[i];
        src_x32 += src_x32_add;
        src_y32 += src_y32_add;
      } while (++index != last);
      param->src_x32 = src_x32;
      param->src_y32 = src_y32;
      return index;
    }
/*
    static int32_t directcopy(void* dst, int32_t index, int32_t last, pixelcopy_t* param)
    {
      auto s = (const uint8_t*)param->src_data;
      auto d = (uint8_t*)dst;
      memcpy(&d[index], &s[param->src_x + param->src_y * param->src_width], (last - index) * param->src_bits >> 3);
      param->src_x += last - index;
      return last;
    }
//*/
    static int32_t bitskip(int32_t index, int32_t last, pixelcopy_t* param)
    {
      auto s = (const uint8_t*)param->src_data;
      auto src_x32     = param->src_x32;
      auto src_y32     = param->src_y32;
      auto src_x32_add = param->src_x32_add;
      auto src_y32_add = param->src_y32_add;
      auto src_width   = param->src_width;
      auto transp      = param->transp;
      auto src_bits    = param->src_bits;
      auto src_mask    = param->src_mask;
      do {
        uint32_t i = ((src_x32 >> FP_SCALE) + (src_y32 >> FP_SCALE) * src_width) * src_bits;
        uint32_t raw = (s[i >> 3] >> (-(i + src_bits) & 7)) & src_mask;
        if (raw != transp) break;
        src_x32 += src_x32_add;
        src_y32 += src_y32_add;
      } while (++index != last);
      param->src_x32 = src_x32;
      param->src_y32 = src_y32;
      return index;
    }

    template <typename TSrc>
    static int32_t normalskip(int32_t index, int32_t last, pixelcopy_t* param)
    {
      auto s = (const TSrc*)param->src_data;
      auto src_x32     = param->src_x32;
      auto src_y32     = param->src_y32;
      auto src_x32_add = param->src_x32_add;
      auto src_y32_add = param->src_y32_add;
      auto src_width   = param->src_width;
      auto transp      = param->transp;
      do {
        uint32_t i = (src_x32 >> FP_SCALE) + (src_y32 >> FP_SCALE) * src_width;
        if (!(s[i] == transp)) break;
        src_x32 += src_x32_add;
        src_y32 += src_y32_add;
      } while (++index != last);
      param->src_x32 = src_x32;
      param->src_y32 = src_y32;
      return index;
    }
  };

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
  struct DataWrapper {
    bool need_transaction = false;

    uint16_t read16(void) {
      uint16_t result;
      read(reinterpret_cast<uint8_t*>(&result), 2);
      return result;
    }

    uint32_t read32(void) {
      uint32_t result;
      read(reinterpret_cast<uint8_t*>(&result), 4);
      return result;
    }

    __attribute__ ((always_inline)) inline uint16_t read16swap(void) {
      return __builtin_bswap16(read16());
    }

    __attribute__ ((always_inline)) inline uint32_t read32swap(void) {
      return __builtin_bswap32(read32());
    }

    virtual int read(uint8_t *buf, uint32_t len) = 0;
    virtual void skip(int32_t offset) = 0;
    virtual bool seek(uint32_t offset) = 0;
    virtual void close(void) = 0;
  };
//----------------------------------------------------------------------------
  struct PointerWrapper : public DataWrapper {
    void set(const uint8_t* src, uint32_t length = ~0) { _ptr = src; _length = length; _index = 0; }
    int read(uint8_t *buf, uint32_t len) override {
      if (len > _length - _index) { len = _length - _index; }
      memcpy(buf, &_ptr[_index], len);
      _index += len;
      return len;
    }
    void skip(int32_t offset) override { _index += offset; }
    bool seek(uint32_t offset) override { _index = offset; return true; }
    void close(void) override { }

  private:
    const uint8_t* _ptr;
    uint32_t _index = 0;
    uint32_t _length = 0;
  };
//----------------------------------------------------------------------------

  static void memset_multi(uint8_t* buf, uint32_t c, size_t size, size_t length)
  {
    size_t l = length;
    if (l & ~0xF) {
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
//----------------------------------------------------------------------------
}


#if defined (ESP32) || (CONFIG_IDF_TARGET_ESP32)

  #include "platforms/esp32_common.hpp"

#elif defined (__AVR__)

  #include <avr/io.h>
  #include <avr/pgmspace.h>

#elif defined(ESP8266)

  #include <pgmspace.h>
#elif defined(__IMXRT1052__) || defined(__IMXRT1062__)

// PROGMEM is defefind for T4 to place data in specific memory section
 #undef PROGMEM
 #define PROGMEM

#else

 #define PROGMEM

#endif



#endif
