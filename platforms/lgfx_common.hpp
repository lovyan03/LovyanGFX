#ifndef LGFX_COMMON_HPP_
#define LGFX_COMMON_HPP_

#if defined (ARDUINO)
  #include <Arduino.h>
  #ifdef __AVR__
    #include <avr/pgmspace.h>
  #elif defined(ESP8266) || defined(ESP32)
    #include <pgmspace.h>
  #endif
#else
  #ifndef pgm_read_byte
    #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
    #define pgm_read_word(addr)  ({ typeof(addr) _addr = (addr); *(const unsigned short *)(_addr); })
    #define pgm_read_dword(addr) ({ typeof(addr) _addr = (addr); *(const unsigned long *)(_addr); })
    #define pgm_read_float(addr) ({ typeof(addr) _addr = (addr); *(const float *)(_addr); })
    #define pgm_read_ptr(addr)   ({ typeof(addr) _addr = (addr); *(void * const *)(_addr); })
    #define pgm_read_byte_near(addr)  pgm_read_byte(addr)
    #define pgm_read_word_near(addr)  pgm_read_word(addr)
    #define pgm_read_dword_near(addr) pgm_read_dword(addr)
    #define pgm_read_float_near(addr) pgm_read_float(addr)
    #define pgm_read_ptr_near(addr)   pgm_read_ptr(addr)
    #define pgm_read_byte_far(addr)   pgm_read_byte(addr)
    #define pgm_read_word_far(addr)   pgm_read_word(addr)
    #define pgm_read_dword_far(addr)  pgm_read_dword(addr)
    #define pgm_read_float_far(addr)  pgm_read_float(addr)
    #define pgm_read_ptr_far(addr)    pgm_read_ptr(addr)
    #define PROGMEM
  #endif
#endif

namespace lgfx {

  enum color_depth_t : uint8_t
  { palette_1bit =  1 //   2 color
  , palette_2bit =  2 //   4 color
  , palette_4bit =  3 //  16 color
  , palette_8bit =  4 // 256 color
  , rgb332_1Byte =  8 // RRRGGGBB
  , rgb565_2Byte = 16 // RRRRRGGG GGGBBBBB
  , rgb666_3Byte = 18 // xxRRRRRR xxGGGGGG xxBBBBBB
  , rgb888_3Byte = 24 // RRRRRRRR GGGGGGGG BBBBBBBB
  };

  __attribute__ ((always_inline)) inline static uint8_t  color332(uint8_t r, uint8_t g, uint8_t b) { return (r & 0xE0) | ((g & 0xE0) >> 3) | (b >> 6); }
  __attribute__ ((always_inline)) inline static uint16_t color565(uint8_t r, uint8_t g, uint8_t b) { return ((r>>3)<<6 | (g>>2))<<5 | (b>>3); }
  __attribute__ ((always_inline)) inline static uint32_t color888(uint8_t r, uint8_t g, uint8_t b) { return (r << 16) | (g << 8) | b; }
  __attribute__ ((always_inline)) inline static uint16_t swap565( uint8_t r, uint8_t g, uint8_t b) { return ((b >> 3) << 8) | ((g >> 2) << 13) | ((g >> 5) | ((r>>3)<<3)); }
  __attribute__ ((always_inline)) inline static uint32_t swap888( uint8_t r, uint8_t g, uint8_t b) { return (b << 16) | (g << 8) | r; }

  __attribute__ ((always_inline)) inline static uint16_t getSwap16(uint16_t c) { return c<<8 | c>>8; }
  __attribute__ ((always_inline)) inline static uint32_t getSwap24(uint32_t c) { return ((uint8_t)c<<8 | (uint8_t)(c>>8))<<8 | (uint8_t)(c>>16); }

  inline uint16_t convert_rgb888_to_rgb565(uint32_t c) { return (c>>19)<<11 | (((uint16_t)c)>>10)<<5 | ((uint8_t)c)>>3; }

  inline uint32_t convert_rgb888_to_swap888(uint32_t c) { return getSwap24(c);  }
  inline uint32_t convert_rgb888_to_swap666(uint32_t c) { return (c&0xFC)<<14 | ((c&0xFC00)>>2) | ((c>>18)&0x3F);  }
  inline uint32_t convert_rgb888_to_swap565(uint32_t c) { return getSwap16(convert_rgb888_to_rgb565(c));  }

  inline uint32_t convert_rgb888_to_rgb332( uint32_t c) { return ((c>>21)<<5) | ((((uint16_t)c)>>13) << 2) | ((c>>6) & 3); }
  inline uint32_t convert_rgb888_to_palette8(uint32_t c) { return (c&0xFF)*0x01010101; }
  inline uint32_t convert_rgb888_to_palette4(uint32_t c) { return (c&0x0F)*0x11111111; }
  inline uint32_t convert_rgb888_to_palette2(uint32_t c) { return (c&0x03)*0x55555555; }
  inline uint32_t convert_rgb888_to_palette1(uint32_t c) { return c&1?~0:0; }

  inline uint32_t convert_rgb565_to_swap888(uint16_t c) { return ((((c&0x1F)*0x21)>>2)<<8 | ((((c>>5)&0x3F)*0x41)>>4))<<8 | (((c>>11)*0x21)>>2); }

  inline uint32_t convert_rgb565_to_swap666(uint16_t c) { return ((c&0x1F)<<17) | ((c&0x10)<<12) | ((c&0x7E0)<<3) | ((c>>10)&0xF8) | (c>>15); }
  inline uint32_t convert_rgb565_to_swap565(uint16_t c) { return c<<8|c>>8; }
  inline uint32_t convert_rgb565_to_rgb332( uint16_t c) { return ((c>>13) <<5) | ((c>>6) & 0x1C) | ((c>>3) & 3); }
  inline uint32_t convert_rgb565_to_palette8(uint16_t c) { return (c&0xFF)*0x01010101; }
  inline uint32_t convert_rgb565_to_palette4(uint16_t c) { return (c&0x0F)*0x11111111; }
  inline uint32_t convert_rgb565_to_palette2(uint16_t c) { return (c&0x03)*0x55555555; }
  inline uint32_t convert_rgb565_to_palette1(uint16_t c) { return c&1?~0:0; }

  inline uint32_t convert_rgb332_to_swap888(uint8_t c) { return (((c&3)*0x55)<<8 | ((c&0x1C)*0x49)>>3)<<8 | (((c>>5)*0x49) >> 1); }
  inline uint32_t convert_rgb332_to_swap666(uint8_t c) { return (((c&0xE0)*9)>>5) | ((c&0x1C)*0x240) | ((c&3)*0x15)<<16; }
  inline uint32_t convert_rgb332_to_swap565(uint8_t c) { return (((c&3)*0x15)>>1)<<8 | ((c&0x1C)<<11) | ((c&0x1C)>>2) | (((c>>5)*0x24)&0xF8); }
  inline uint32_t convert_rgb332_to_rgb332( uint8_t c) { return c; }
  inline uint32_t convert_rgb332_to_palette8(uint8_t c) { return c * 0x01010101; }
  inline uint32_t convert_rgb332_to_palette4(uint8_t c) { return (c&0x0F)*0x11111111; }
  inline uint32_t convert_rgb332_to_palette2(uint8_t c) { return (c&0x03)*0x55555555; }
  inline uint32_t convert_rgb332_to_palette1(uint8_t c) { return (c&1)?~0:0; }


#pragma pack(1)

  struct palette_t;
  struct rgb332_t;    //  8bpp
  struct rgb565_t;    // 16bpp
  struct rgb888_t;    // 24bpp
  struct argb8888_t;  // 32bpp

  struct swap565_t;   // 16bpp
  struct swap666_t;   // 18bpp
  struct swap888_t;   // 24bpp
  struct dev_color_t;

  struct palette_t {
    union {
      uint8_t raw;
    };
    palette_t() : raw(0) {}
    palette_t(const palette_t&) = default;
    palette_t(uint8_t c) : raw(c) {}
  };

  struct rgb332_t {
    union {
      struct {
        uint8_t b: 2;
        uint8_t g: 3;
        uint8_t r: 3;
      };
      uint8_t raw;
    };
    rgb332_t() : raw(0) {}
    rgb332_t(const rgb332_t&) = default;
    rgb332_t(uint8_t r8, uint8_t g8, uint8_t b8) : raw(color332(r8,g8,b8)) {}
    rgb332_t(uint8_t rgb332) : raw(rgb332) {}
    inline rgb332_t& operator=(const rgb565_t&);
    inline rgb332_t& operator=(const rgb888_t&);
    inline rgb332_t& operator=(const argb8888_t&);
    inline rgb332_t& operator=(const swap565_t&);
    inline rgb332_t& operator=(const swap666_t&);
    inline rgb332_t& operator=(const swap888_t&);
    inline rgb332_t& operator=(uint8_t rgb332) { raw = rgb332; return *this; }
    explicit inline operator uint8_t() const { return raw; }
    explicit inline operator bool() const { return raw; }
//  inline operator rgb565_t() const;
//  inline operator rgb888_t() const;
//  inline operator argb8888_t() const;
//  inline operator swap565_t() const;
//  inline operator swap888_t() const;
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
      uint8_t bytes[2];
      uint16_t raw;
    };
    rgb565_t() : raw(0) {}
    rgb565_t(const rgb565_t&) = default;
    rgb565_t(uint16_t rgb565) : raw(rgb565) {}
    rgb565_t(uint8_t r8, uint8_t g8, uint8_t b8) : raw(color565(r8,g8,b8)) {} // b(b8>>3),g(g8>>2),r(r8>>3) {}
    inline rgb565_t& operator=(const rgb332_t&);
    inline rgb565_t& operator=(const rgb888_t&);
    inline rgb565_t& operator=(const argb8888_t&);
    inline rgb565_t& operator=(const swap565_t&);
    inline rgb565_t& operator=(const swap666_t&);
    inline rgb565_t& operator=(const swap888_t&);
    inline rgb565_t& operator=(uint16_t rgb565) { raw = rgb565; return *this; }
    explicit inline operator uint16_t() const { return raw; }
    explicit inline operator bool() const { return raw; }
    //inline operator rgb332_t() const { return rgb332_t(r<<3,g<<2,b<<3); }
    //inline operator rgb888_t() const;
    //inline operator argb8888_t() const;
    //inline operator swap565_t() const;
    //inline operator swap888_t() const;
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
      uint8_t bytes[3];
    };
    rgb888_t() : b(0), g(0), r(0) {}
    rgb888_t(const rgb888_t&) = default;
    rgb888_t(uint8_t r8, uint8_t g8, uint8_t b8) : b(b8),g(g8),r(r8) {}
    rgb888_t(uint32_t rgb888) : b(rgb888), g(rgb888>>8), r(rgb888>>16) {}
    inline rgb888_t& operator=(const rgb332_t&);
    inline rgb888_t& operator=(const rgb565_t&);
    inline rgb888_t& operator=(const argb8888_t&);
    inline rgb888_t& operator=(const swap565_t&);
    inline rgb888_t& operator=(const swap666_t&);
    inline rgb888_t& operator=(const swap888_t&);
    inline rgb888_t& operator=(uint32_t rgb888) { r = rgb888>>16; g = rgb888>>8; b = rgb888; return *this; }
    explicit inline operator uint32_t() const { return (r<<16)|(g<<8)|b; }
    explicit inline operator bool() const { return r||g||b; }
    //inline operator rgb332_t() const { return rgb332_t(r,g,b); }
    //inline operator rgb565_t() const { return rgb565_t(r,g,b); }
    //inline operator argb8888_t() const;
    //inline operator swap565_t() const;
    //inline operator swap888_t() const;
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

  struct argb8888_t {
    union {
      struct {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;
      };
      uint8_t bytes[4];
      uint32_t raw;
    };
    argb8888_t() : raw(0) {}
    argb8888_t(const argb8888_t&) = default;
    argb8888_t(uint8_t r, uint8_t g, uint8_t b) : b(b),g(g),r(r),a(0) {}
    argb8888_t(uint32_t argb8888) : raw(argb8888) {}
    inline argb8888_t& operator=(const rgb332_t&);
    inline argb8888_t& operator=(const rgb565_t&);
    inline argb8888_t& operator=(const rgb888_t&);
    inline argb8888_t& operator=(const swap565_t&);
    inline argb8888_t& operator=(const swap666_t&);
    inline argb8888_t& operator=(const swap888_t&);
    inline argb8888_t& operator=(uint32_t argb8888) { raw = argb8888; return *this; }
    explicit inline operator uint32_t() const { return raw; }
    explicit inline operator bool() const { return raw; }
    //inline operator rgb332_t() const { return rgb332_t(r,g,b); }
    //inline operator rgb565_t() const { return rgb565_t(r,g,b); }
    //inline operator rgb888_t() const { return operator uint32_t(); }
    //inline operator swap565_t() const;
    //inline operator swap888_t() const;
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
      uint8_t bytes[2];
      uint16_t raw;
    };
    swap565_t() : raw(0) {}
    swap565_t(const swap565_t&) = default;
    swap565_t(uint8_t r8, uint8_t g8, uint8_t b8) : gh(g8>>5),r(r8>>3),b(b8>>3),gl(g8>>2) {}
    swap565_t(uint16_t raw) : raw(raw) {}
    inline swap565_t& operator=(uint32_t rhs) { raw = rhs; }
    inline swap565_t& operator=(const rgb332_t& rhs);
    inline swap565_t& operator=(const rgb565_t& rhs);
    inline swap565_t& operator=(const rgb888_t& rhs);
    inline swap565_t& operator=(const argb8888_t& rhs);
    inline swap565_t& operator=(const swap666_t& rhs);
    inline swap565_t& operator=(const swap888_t& rhs);
    //explicit inline operator uint16_t() const { return raw; }
    explicit inline operator bool() const { return r||gh||gl||b; }
    //inline operator rgb565_t() const { return rgb565_t(raw<<8 | raw>>8); }
    //inline operator swap888_t() const;
    inline uint8_t R8() const { return ( r * 0x21) >> 2; }
    inline uint8_t G8() const { return ((gh* 0x41) >> 1)|(gl << 2); }
    inline uint8_t B8() const { return ( b * 0x21) >> 2; }
    inline uint8_t R6() const { return ( r * 0x21) >> 4; }
    inline uint8_t G6() const { return  gh << 3 | gl; }
    inline uint8_t B6() const { return ( b * 0x21) >> 4; }
  };

  struct swap666_t {
    union {
      struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
      };
      uint8_t bytes[3];
    };
    swap666_t() : r(0), g(0), b(0) {};
    swap666_t(const swap666_t&) = default;
    swap666_t(uint8_t r8, uint8_t g8, uint8_t b8) : r(r8>>2),g(g8>>2),b(b8>>2) {}
    inline swap666_t& operator=(uint32_t rhs) { r = rhs; g = rhs>>8 ; b = rhs>>16; }
    inline swap666_t& operator=(const rgb332_t&);
    inline swap666_t& operator=(const rgb565_t&);
    inline swap666_t& operator=(const rgb888_t&);
    inline swap666_t& operator=(const argb8888_t&);
    inline swap666_t& operator=(const swap565_t&);
    inline swap666_t& operator=(const swap888_t&);
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

  struct swap888_t {
    union {
      struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
      };
      uint8_t bytes[3];
    };
    swap888_t() : r(0), g(0), b(0) {};
    swap888_t(const swap888_t&) = default;
    swap888_t(uint8_t r8, uint8_t g8, uint8_t b8) : r(r8),g(g8),b(b8) {}
    swap888_t(uint32_t bgr888) : r(bgr888), g(bgr888>>8), b(bgr888>>16) {}
    inline swap888_t& operator=(uint32_t rhs) { r = rhs; g = rhs>>8 ; b = rhs>>16; }
    inline swap888_t& operator=(const rgb332_t&);
    inline swap888_t& operator=(const rgb565_t&);
    inline swap888_t& operator=(const rgb888_t&);
    inline swap888_t& operator=(const argb8888_t&);
    inline swap888_t& operator=(const swap565_t&);
    inline swap888_t& operator=(const swap666_t&);
    explicit inline operator bool() const { return r||g||b; }
    //inline operator uint32_t() const { return (b<<16)|(g<<8)|r; }
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
  };

  struct dev_color_t {
    union {
      struct {
        uint8_t raw0;
        uint8_t raw1;
        uint8_t raw2;
        uint8_t raw3;
      };
      uint32_t raw;
    };

    uint32_t (*convert_rgb888)(uint32_t) = convert_rgb888_to_swap565;
    uint32_t (*convert_rgb565)(uint16_t) = convert_rgb565_to_swap565;
    uint32_t (*convert_rgb332)(uint8_t)  = convert_rgb332_to_swap565;
    color_depth_t bpp = rgb565_2Byte;
    uint8_t bytes  = 2;
    uint8_t bits   = 16;
    uint8_t x_mask = 0;
    uint8_t colormask = 0;

    dev_color_t() : raw(0) {}
    dev_color_t(const dev_color_t&) = default;
    dev_color_t(const uint32_t& rhs) : raw(rhs) {}
    inline operator bool() const { return raw & 0x00FFFFFF; }

    void setColorDepth(color_depth_t bpp) {
      x_mask = 0; colormask = 0;
      if (     bpp > 18) { bpp = rgb888_3Byte; bytes = 3; bits = 24; }
      else if (bpp > 16) { bpp = rgb666_3Byte; bytes = 3; bits = 24; }
      else if (bpp >  8) { bpp = rgb565_2Byte; bytes = 2; bits = 16; }
      else if (bpp >  4) { bpp = rgb332_1Byte; bytes = 1; bits =  8; }
      else if (bpp == 4) { bpp = palette_8bit; bytes = 1; bits =  8; x_mask = 0b0000; colormask = 0b11111111;}
      else if (bpp == 3) { bpp = palette_4bit; bytes = 0; bits =  4; x_mask = 0b0001; colormask = 0b00001111;}
      else if (bpp == 2) { bpp = palette_2bit; bytes = 0; bits =  2; x_mask = 0b0011; colormask = 0b00000011;}
      else               { bpp = palette_1bit; bytes = 0; bits =  1; x_mask = 0b0111; colormask = 0b00000001;}
      this->bpp = bpp;
      switch (bpp) {
      case rgb888_3Byte:
        convert_rgb888 = convert_rgb888_to_swap888;
        convert_rgb565 = convert_rgb565_to_swap888;
        convert_rgb332 = convert_rgb332_to_swap888;
        break;
      case rgb666_3Byte:
        convert_rgb888 = convert_rgb888_to_swap666;
        convert_rgb565 = convert_rgb565_to_swap666;
        convert_rgb332 = convert_rgb332_to_swap666;
        break;
      case rgb565_2Byte:
        convert_rgb888 = convert_rgb888_to_swap565;
        convert_rgb565 = convert_rgb565_to_swap565;
        convert_rgb332 = convert_rgb332_to_swap565;
        break;
      case rgb332_1Byte:
        convert_rgb888 = convert_rgb888_to_rgb332;
        convert_rgb565 = convert_rgb565_to_rgb332;
        convert_rgb332 = convert_rgb332_to_rgb332;
        break;
      case palette_8bit:
        convert_rgb888 = convert_rgb888_to_palette8;
        convert_rgb565 = convert_rgb565_to_palette8;
        convert_rgb332 = convert_rgb332_to_palette8;
        break;
      case palette_4bit:
        convert_rgb888 = convert_rgb888_to_palette4;
        convert_rgb565 = convert_rgb565_to_palette4;
        convert_rgb332 = convert_rgb332_to_palette4;
        break;
      case palette_2bit:
        convert_rgb888 = convert_rgb888_to_palette2;
        convert_rgb565 = convert_rgb565_to_palette2;
        convert_rgb332 = convert_rgb332_to_palette2;
        break;
      case palette_1bit:
        convert_rgb888 = convert_rgb888_to_palette1;
        convert_rgb565 = convert_rgb565_to_palette1;
        convert_rgb332 = convert_rgb332_to_palette1;
        break;
      }
    }

#define TYPECHECK(dType) template < typename T, typename std::enable_if < (sizeof(T) == sizeof(dType)) && (std::is_signed<T>::value == std::is_signed<dType>::value), std::nullptr_t >::type=nullptr >
    TYPECHECK(uint8_t ) __attribute__ ((always_inline)) inline uint32_t convertColor(T c) { return convert_rgb332(c); }
    TYPECHECK(uint16_t) __attribute__ ((always_inline)) inline uint32_t convertColor(T c) { return convert_rgb565(c); }
    TYPECHECK(uint32_t) __attribute__ ((always_inline)) inline uint32_t convertColor(T c) { return convert_rgb888(c); }
    TYPECHECK(int     ) __attribute__ ((always_inline)) inline uint32_t convertColor(T c) { return convert_rgb565(c); }
#undef TYPECHECK

    template<typename T>
    __attribute__ ((always_inline)) inline void setColor(T c) { raw = convertColor(c); }
  };

#pragma pack()

  //inline rgb332_t::operator rgb565_t() const   { return rgb565_t(((r*0x2400) & 0xF800) | (g*0x0120) | ((b*0x15)>>1)); }
  //inline rgb332_t::operator rgb888_t() const   { return rgb888_t(R8(),G8(),B8()); }
  //inline rgb332_t::operator argb8888_t() const { return argb8888_t(R8(),G8(),B8()); }
  //inline rgb332_t::operator swap565_t() const  { return operator rgb565_t(); }
  //inline rgb332_t::operator swap888_t() const  { return swap888_t(R8(),G8(),B8()); }
  //inline rgb565_t::operator rgb888_t() const   { return rgb888_t(R8(),G8(),B8()); }
  //inline rgb565_t::operator argb8888_t() const { return argb8888_t(R8(),G8(),B8()); }
  //inline rgb565_t::operator swap565_t() const  { return swap565_t(raw<<8 | raw>>8); }
  //inline rgb565_t::operator swap888_t() const  { return swap888_t(R8(),G8(),B8()); }
  //inline rgb888_t::operator argb8888_t() const { return argb8888_t(r,g,b); }
  //inline rgb888_t::operator swap565_t() const  { return operator rgb565_t(); }
  //inline rgb888_t::operator swap888_t() const  { return swap888_t(r,g,b); }
  //inline argb8888_t::operator swap565_t() const { return operator rgb565_t(); }
  //inline argb8888_t::operator swap888_t() const { return swap888_t(r,g,b); }
  //inline swap565_t::operator swap888_t() const { return operator rgb565_t(); }

  inline rgb332_t& rgb332_t::operator=(const rgb565_t&   rhs) { raw = ((rhs.r<<3)&0xE0) | ((rhs.g>>1)&0x1C) | (rhs.b>>3); return *this; }
  inline rgb332_t& rgb332_t::operator=(const swap565_t&  rhs) { raw = ((rhs.r<<3)&0xE0) | (rhs.gh<<2) | (rhs.b>>3); return *this; }
  inline rgb332_t& rgb332_t::operator=(const swap666_t&  rhs) { raw = ((rhs.r<<2)&0xE0) | ((rhs.g>>1)&0x1C) | (rhs.b>>4); return *this; }
  inline rgb332_t& rgb332_t::operator=(const rgb888_t&   rhs) { raw = color332(rhs.r, rhs.g, rhs.b); return *this; }
  inline rgb332_t& rgb332_t::operator=(const swap888_t&  rhs) { raw = color332(rhs.r, rhs.g, rhs.b); return *this; }
  inline rgb332_t& rgb332_t::operator=(const argb8888_t& rhs) { raw = color332(rhs.r, rhs.g, rhs.b); return *this; }

  inline rgb565_t& rgb565_t::operator=(const rgb332_t&   rhs) { raw = color565(rhs.R8(), rhs.G8(), rhs.B8()); return *this; }
  inline rgb565_t& rgb565_t::operator=(const swap565_t&  rhs) { raw = rhs.raw << 8 | rhs.raw >> 8;   return *this; }
  inline rgb565_t& rgb565_t::operator=(const rgb888_t&   rhs) { raw = color565(rhs.r, rhs.g, rhs.b); return *this; }
  inline rgb565_t& rgb565_t::operator=(const swap888_t&  rhs) { raw = color565(rhs.r, rhs.g, rhs.b); return *this; }
  inline rgb565_t& rgb565_t::operator=(const swap666_t&  rhs) { raw = color565(rhs.R8(), rhs.G8(), rhs.B8()); return *this; }
  inline rgb565_t& rgb565_t::operator=(const argb8888_t& rhs) { raw = color565(rhs.r, rhs.g, rhs.b); return *this; }

  inline rgb888_t& rgb888_t::operator=(const rgb332_t&   rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline rgb888_t& rgb888_t::operator=(const rgb565_t&   rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline rgb888_t& rgb888_t::operator=(const swap565_t&  rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline rgb888_t& rgb888_t::operator=(const swap666_t&  rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline rgb888_t& rgb888_t::operator=(const swap888_t&  rhs) { r = rhs.r;    g = rhs.g;    b = rhs.b;    return *this; }
  inline rgb888_t& rgb888_t::operator=(const argb8888_t& rhs) { r = rhs.r;    g = rhs.g;    b = rhs.b;    return *this; }

  inline swap565_t& swap565_t::operator=(const rgb332_t&   rhs) { raw = swap565(rhs.R8(), rhs.G8(), rhs.B8()); return *this; }
  inline swap565_t& swap565_t::operator=(const rgb565_t&   rhs) { raw = rhs.raw << 8 | rhs.raw >> 8;           return *this; }
  inline swap565_t& swap565_t::operator=(const rgb888_t&   rhs) { raw = swap565(rhs.r,    rhs.g,    rhs.b);    return *this; }
  inline swap565_t& swap565_t::operator=(const swap666_t&  rhs) { raw = swap565(rhs.R8(), rhs.G8(), rhs.B8()); return *this; }
  inline swap565_t& swap565_t::operator=(const swap888_t&  rhs) { raw = swap565(rhs.r,    rhs.g,    rhs.b);    return *this; }
  inline swap565_t& swap565_t::operator=(const argb8888_t& rhs) { raw = swap565(rhs.r,    rhs.g,    rhs.b);    return *this; }

  inline swap666_t& swap666_t::operator=(const rgb332_t&   rhs) { r = rhs.R6(); g = rhs.G6(); b = rhs.B6(); return *this; }
  inline swap666_t& swap666_t::operator=(const rgb565_t&   rhs) { r = rhs.R6(); g = rhs.G6(); b = rhs.B6(); return *this; }
  inline swap666_t& swap666_t::operator=(const swap565_t&  rhs) { r = rhs.R6(); g = rhs.G6(); b = rhs.B6(); return *this; }
  inline swap666_t& swap666_t::operator=(const swap888_t&  rhs) { r = rhs.R6(); g = rhs.G6(); b = rhs.B6(); return *this; }
  inline swap666_t& swap666_t::operator=(const rgb888_t&   rhs) { r = rhs.R6(); g = rhs.G6(); b = rhs.B6(); return *this; }
  inline swap666_t& swap666_t::operator=(const argb8888_t& rhs) { r = rhs.R6(); g = rhs.G6(); b = rhs.B6(); return *this; }

  inline swap888_t& swap888_t::operator=(const rgb332_t&   rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline swap888_t& swap888_t::operator=(const rgb565_t&   rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline swap888_t& swap888_t::operator=(const swap565_t&  rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline swap888_t& swap888_t::operator=(const swap666_t&  rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline swap888_t& swap888_t::operator=(const rgb888_t&   rhs) { r = rhs.r   ; g = rhs.g   ; b = rhs.b   ; return *this; }
  inline swap888_t& swap888_t::operator=(const argb8888_t& rhs) { r = rhs.r   ; g = rhs.g   ; b = rhs.b   ; return *this; }

  inline argb8888_t& argb8888_t::operator=(const rgb332_t&   rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline argb8888_t& argb8888_t::operator=(const rgb565_t&   rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline argb8888_t& argb8888_t::operator=(const swap565_t&  rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline argb8888_t& argb8888_t::operator=(const swap666_t&  rhs) { r = rhs.R8(); g = rhs.G8(); b = rhs.B8(); return *this; }
  inline argb8888_t& argb8888_t::operator=(const rgb888_t&   rhs) { r = rhs.r   ; g = rhs.g   ; b = rhs.b   ; return *this; }
  inline argb8888_t& argb8888_t::operator=(const swap888_t&  rhs) { r = rhs.r   ; g = rhs.g   ; b = rhs.b   ; return *this; }

  inline bool operator==(const rgb332_t& lhs, const rgb332_t& rhs) { return lhs.raw == rhs.raw; }
  inline bool operator==(const rgb565_t& lhs, const rgb565_t& rhs) { return lhs.raw == rhs.raw; }
  inline bool operator==(const swap565_t& lhs, const swap565_t& rhs) { return lhs.raw == rhs.raw; }

  inline bool operator==(const swap666_t&  lhs, const swap666_t&  rhs) { return (*(uint32_t*)&lhs & 0x3F3F3F) == (*(uint32_t*)&rhs & 0x3F3F3F); }
  inline bool operator==(const rgb888_t&   lhs, const rgb888_t&   rhs) { return (*(uint32_t*)&lhs & 0xFCFCFC) == (*(uint32_t*)&rhs & 0xFCFCFC); }
  inline bool operator==(const swap888_t&  lhs, const swap888_t&  rhs) { return (*(uint32_t*)&lhs & 0xFCFCFC) == (*(uint32_t*)&rhs & 0xFCFCFC); }
  inline bool operator==(const argb8888_t& lhs, const argb8888_t& rhs) { return (*(uint32_t*)&lhs & 0xFCFCFC) == (*(uint32_t*)&rhs & 0xFCFCFC); }
//*/
  inline bool operator==(const dev_color_t& lhs, const dev_color_t& rhs) { return lhs.raw == rhs.raw; }
  inline bool operator!=(const dev_color_t& lhs, const dev_color_t& rhs) { return lhs.raw != rhs.raw; }

}
/*
// test code

for (int i = 0; i < 24; i++) {
  uint32_t src = 1<<i;
  printf("src:%06x  res:%06x\r\n", src, getSwapColor565FromSwap24(src));
}

*/
#endif
