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
  #ifndef PROGMEM
    #define PROGMEM
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
  #endif
#endif

namespace lgfx {

  inline static uint8_t  color332(uint8_t r, uint8_t g, uint8_t b) { return (r & 0xE0) | ((g & 0xE0) >> 3) | (b >> 6); }
  inline static uint16_t color565(uint8_t r, uint8_t g, uint8_t b) { return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3); }
  inline static uint32_t color888(uint8_t r, uint8_t g, uint8_t b) { return (r << 16) | (g << 8) | b; }

  inline static uint16_t getSwap16(uint16_t c) { return c<<8 | c>>8; }
  inline static uint32_t getSwap24(uint32_t c) { return (uint8_t)c<<16 | ((uint16_t)c&0xFF00) | (uint8_t)(c>>16); }

  inline static uint16_t color565(uint32_t color888) { return ((uint16_t)(color888>>8) & 0xF800) | ((uint16_t)(color888>>5) & 0x07E0) | (uint8_t)(color888 >> 3); }

  inline static uint32_t getColor565FromSwap888(uint32_t d) { return ((uint8_t)d&0xF8)<<8 | ((uint16_t)d>>5&0x07E0) | ((uint8_t)(d>>19)&0x1F); }
  inline static uint32_t getColor888FromSwap888(uint32_t d) { return getSwap24(d); }
//  inline static uint16_t getSwapColor565FromSwap888(uint32_t d) { return ((uint16_t)(d>>11)&0x1F00) | ((uint16_t)d<<3&0xE000) | ((uint8_t)(d>>13)&0x07) | ((uint8_t)d&0xF8); }
//  inline static uint32_t getSwapColor888FromSwap888(uint32_t d) { return d & 0xFFFFFF; }

#pragma pack(1)

  struct rgb332_t;   // 8bpp
  struct rgb565_t;   // 16bpp
  struct rgb888_t;   // 24bpp
  struct argb8888_t;  // 32bpp

  struct swap565_t;   // 16bpp
  struct swap888_t;   // 24bpp

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
    rgb332_t(uint8_t color332) : raw(color332) {}
    inline operator uint8_t() const { return raw; }
    inline operator rgb565_t() const;
    inline operator rgb888_t() const;
    inline operator argb8888_t() const;
    inline operator swap565_t() const;
    inline operator swap888_t() const;
    inline uint8_t R8() const { return (r * 0x49) >> 1; } // (r<<5)|(r<<2)|(r>>1);
    inline uint8_t G8() const { return (g * 0x49) >> 1; } // (g<<5)|(g<<2)|(g>>1); }
    inline uint8_t B8() const { return  b * 0x55; }       // (b<<6)|(b<<4)|(b<<2)|b; }
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
    rgb565_t(uint16_t color565) : raw(color565) {}
    rgb565_t(uint8_t r8, uint8_t g8, uint8_t b8) : raw(color565(r8,g8,b8)) {} // b(b8>>3),g(g8>>2),r(r8>>3) {}
    inline operator uint16_t() const { return raw; }
    inline operator rgb332_t() const { return rgb332_t(r<<3,g<<2,b<<3); }
    inline operator rgb888_t() const;
    inline operator argb8888_t() const;
    inline operator swap565_t() const;
    inline operator swap888_t() const;
    inline uint8_t R8() const { return (r * 0x21) >> 2; } // (r << 3) | (r >> 2); }
    inline uint8_t G8() const { return (g * 0x41) >> 4; } // (g << 2) | (g >> 4); }
    inline uint8_t B8() const { return (b * 0x21) >> 2; } // (b << 3) | (b >> 2); }
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
    rgb888_t(uint32_t color888) : b(color888), g(color888>>8), r(color888>>16) {}
    inline operator uint32_t() const { return (r<<16)|(g<<8)|b; }
    inline operator rgb332_t() const { return rgb332_t(r,g,b); }
    inline operator rgb565_t() const { return rgb565_t(r,g,b); }
    inline operator argb8888_t() const;
    inline operator swap565_t() const;
    inline operator swap888_t() const;
    inline uint8_t R8() const { return r; }
    inline uint8_t G8() const { return g; }
    inline uint8_t B8() const { return b; }
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
    argb8888_t(uint32_t color888) : raw(color888) {}
    inline operator uint32_t() const { return raw; }
    inline operator rgb332_t() const { return rgb332_t(r,g,b); }
    inline operator rgb565_t() const { return rgb565_t(r,g,b); }
    inline operator rgb888_t() const { return operator uint32_t(); }
    inline operator swap565_t() const;
    inline operator swap888_t() const;
    inline uint8_t R8() const { return r; }
    inline uint8_t G8() const { return g; }
    inline uint8_t B8() const { return b; }
    inline void R8(uint8_t r8) { r = r8; }
    inline void G8(uint8_t g8) { g = g8; }
    inline void B8(uint8_t b8) { b = b8; }
  };

  struct swap565_t {
    union {
      struct {
        uint16_t g_h:3;
        uint16_t r:5;
        uint16_t b:5;
        uint16_t g_l:3;
      };
      uint8_t bytes[2];
      uint16_t raw;
    };
    swap565_t() : raw(0) {}
    swap565_t(const swap565_t&) = default;
    swap565_t(uint8_t r8, uint8_t g8, uint8_t b8) : g_h(g8>>5),r(r8>>3),b(b8>>3),g_l(g8>>2) {}
    swap565_t(uint16_t raw) : raw(raw) {}
    inline operator uint16_t() const { return raw; }
    inline operator rgb565_t() const { return rgb565_t(raw<<8 | raw>>8); }
    inline operator swap888_t() const;
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
    inline operator uint32_t() const { return (b<<16)|(g<<8)|r; }
    inline operator rgb332_t() const { return rgb332_t(r,g,b); }
    inline operator rgb565_t() const { return rgb565_t(r,g,b); }
    inline operator rgb888_t() const { return rgb888_t(r,g,b); }
    inline operator swap565_t() const { return swap565_t(r,g,b); }
  };

  inline rgb332_t::operator rgb565_t() const   { return rgb565_t(((r*0x2400) & 0xF800) | (g*0x0120) | ((b*0x15)>>1)); }
  inline rgb332_t::operator rgb888_t() const   { return rgb888_t(R8(),G8(),B8()); }
  inline rgb332_t::operator argb8888_t() const { return argb8888_t(R8(),G8(),B8()); }
  inline rgb332_t::operator swap565_t() const  { return operator rgb565_t(); }
  inline rgb332_t::operator swap888_t() const  { return swap888_t(R8(),G8(),B8()); }

  inline rgb565_t::operator rgb888_t() const   { return rgb888_t(R8(),G8(),B8()); }
  inline rgb565_t::operator argb8888_t() const { return argb8888_t(R8(),G8(),B8()); }
  inline rgb565_t::operator swap565_t() const  { return swap565_t(raw<<8 | raw>>8); }
  inline rgb565_t::operator swap888_t() const  { return swap888_t(R8(),G8(),B8()); }

  inline rgb888_t::operator argb8888_t() const { return argb8888_t(r,g,b); }
  inline rgb888_t::operator swap565_t() const  { return operator rgb565_t(); }
  inline rgb888_t::operator swap888_t() const  { return swap888_t(r,g,b); }

  inline argb8888_t::operator swap565_t() const { return operator rgb565_t(); }
  inline argb8888_t::operator swap888_t() const { return swap888_t(r,g,b); }

  inline swap565_t::operator swap888_t() const { return operator rgb565_t(); }

#pragma pack()

}
/*
// test code

for (int i = 0; i < 24; i++) {
  uint32_t src = 1<<i;
  printf("src:%06x  res:%06x\r\n", src, getSwapColor565FromSwap24(src));
}

*/
#endif
