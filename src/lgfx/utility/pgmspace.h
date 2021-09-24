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
#ifndef __LGFX_PGMSPACE_H__
#define __LGFX_PGMSPACE_H__

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
 #define pgm_read_ptr(addr) (*(void * const *)(addr))
#endif

/// for  not ESP8266
#if !defined ( pgm_read_dword_with_offset )
 #if defined (__SAMD21__)
  #define pgm_read_word_unaligned(addr) (uint16_t) \
    ( *(const uint8_t *)((uintptr_t)addr) \
    | *(const uint8_t *)((uintptr_t)addr+1) << 8 )
  #define pgm_read_3byte_unaligned(addr) (uint32_t) \
    ( *(const uint8_t *)((uintptr_t)addr) \
    | *(const uint8_t *)((uintptr_t)addr+1) << 8  \
    | *(const uint8_t *)((uintptr_t)addr+2) << 16 )
  #define pgm_read_dword_unaligned(addr) (uint32_t) \
    ( *(const uint8_t *)((uintptr_t)addr) \
    | *(const uint8_t *)((uintptr_t)addr+1) << 8  \
    | *(const uint8_t *)((uintptr_t)addr+2) << 16 \
    | *(const uint8_t *)((uintptr_t)addr+3) << 24 )
 #else
  #define pgm_read_word_unaligned(addr)  (*(const uint16_t *)((uintptr_t)addr))
  #define pgm_read_dword_unaligned(addr) (*(const uint32_t *)((uintptr_t)addr))
 #endif
#endif

#if !defined ( pgm_read_3byte_unaligned )
 #define pgm_read_3byte_unaligned(addr) (pgm_read_dword_unaligned(addr) & 0xFFFFFFu)
#endif

#if defined (__SAMD21__) || defined ( ESP8266 )
  static inline void write_3byte_unaligned(void* addr, uint32_t value)
  {
    reinterpret_cast<uint8_t*>(addr)[0] = value;
    reinterpret_cast<uint8_t*>(addr)[1] = value >> 8;
    reinterpret_cast<uint8_t*>(addr)[2] = value >> 16;
  }
#else
  static inline void write_3byte_unaligned(void* addr, uint32_t value)
  {
    reinterpret_cast<uint16_t*>(addr)[0] = value;
    reinterpret_cast<uint8_t*>(addr)[2] = value >> 16;
  }
#endif

#ifndef PROGMEM
#define PROGMEM
#endif

#endif
