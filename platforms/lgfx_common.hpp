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

  inline static uint16_t getSwap16(uint16_t c) { return c<<8 | c>>8; }
  inline static uint32_t getSwap24(uint32_t c) { return (uint8_t)c<<16 | ((uint16_t)c&0xFF00) | (uint8_t)(c>>16); }

  inline static uint16_t getColor565(uint32_t color888) { return ((color888>>8) & 0xF800) | ((color888>>5) & 0x07E0) | (uint8_t)(color888 >> 3); }
  inline static uint16_t getColor565(uint8_t r, uint8_t g, uint8_t b) { return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3); }
  inline static uint32_t getColor888(uint8_t r, uint8_t g, uint8_t b) { return (r << 16) | (g << 8) | b; }
  inline static uint32_t getColor565FromSwap888(uint32_t d) { return ((uint8_t)d&0xF8)<<8 | ((uint16_t)d>>5&0x07E0) | ((uint8_t)(d>>19)&0x1F); }
  inline static uint32_t getColor888FromSwap888(uint32_t d) { return getSwap24(d); }
  inline static uint16_t getSwapColor565FromSwap888(uint32_t d) { return ((uint16_t)(d>>11)&0x1F00) | ((uint16_t)d<<3&0xE000) | ((uint8_t)(d>>13)&0x07) | ((uint8_t)d&0xF8); }
  inline static uint32_t getSwapColor888FromSwap888(uint32_t d) { return d & 0xFFFFFF; }
}
/*
// test code

for (int i = 0; i < 24; i++) {
  uint32_t src = 1<<i;
  printf("src:%06x  res:%06x\r\n", src, getSwapColor565FromSwap24(src));
}

*/
#endif
