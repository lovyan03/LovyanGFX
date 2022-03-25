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
#pragma once

#include "../../misc/DataWrapper.hpp"
#include "../../misc/enum.hpp"
#include "../../../utility/result.hpp"

#if defined ( CONFIG_ARCH_BOARD_SPRESENSE )
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#include <Arduino.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  __attribute__ ((unused))
  static inline unsigned long millis(void)
  {
    return ::millis();
  }
  __attribute__ ((unused))
  static inline unsigned long micros(void)
  {
    return ::micros();
  }
  __attribute__ ((unused))
  static inline void delay(unsigned long milliseconds)
  {
    ::delay(milliseconds);
  }
  __attribute__ ((unused))
  static void delayMicroseconds(unsigned int us)
  {
    ::delayMicroseconds(us);
  }

  static inline void* heap_alloc(      size_t length) { return malloc(length); }
  static inline void* heap_alloc_psram(size_t length) { return malloc(length); }
  static inline void* heap_alloc_dma(  size_t length) { return malloc(length); } // aligned_alloc(16, length);
  static inline void heap_free(void* buf) { free(buf); }

  static inline void gpio_hi(uint32_t pin) { digitalWrite(pin, HIGH); }
  static inline void gpio_lo(uint32_t pin) { digitalWrite(pin, LOW); }
  static inline bool gpio_in(uint32_t pin) { return digitalRead(pin); }

  enum pin_mode_t
  { output
  , input
  , input_pullup
  , input_pulldown
  };

  void pinMode(int_fast16_t pin, pin_mode_t mode);
  inline void lgfxPinMode(int_fast16_t pin, pin_mode_t mode)
  {
    pinMode(pin, mode);
  }

//----------------------------------------------------------------------------

#if defined (ARDUINO)
 #if defined (__STORAGE_H__)

  struct FileWrapper : public DataWrapper
  {

public:
    FileWrapper() : DataWrapper()
    {
      _fs = nullptr;
      _fp = nullptr;
    }

    StorageClass* _fs;
    File *_fp;
    File _file;

    FileWrapper(StorageClass& fs, File* fp = nullptr) : DataWrapper(), _fs(&fs), _fp(fp) {}
    void setFS(StorageClass& fs) {
      _fs = &fs;
    }

    bool open(StorageClass& fs, const char* path)
    {
      setFS(fs);
      return open(path);
    }
    bool open(const char* path) override
    {
      _file = _fs->open(path);
      _fp = &_file;
      return _file;
    }
    int read(uint8_t *buf, uint32_t len) override { return _fp->read(buf, len); }
    void skip(int32_t offset) override { _fp->seek(_fp->position() + offset); }
    bool seek(uint32_t offset) override { return _fp->seek(offset); }
    void close(void) override { if (_fp) _fp->close(); }
    int32_t tell(void) override { return _fp->position(); }
  };
 #else

  struct FileWrapper : public DataWrapper
  {
    FileWrapper() : DataWrapper()
    {
      need_transaction = false;
    }
    void* _fp;

    template <typename T>
    void setFS(T& fs) {}

    bool open(const char* , const char* ) { return false; }
    int read(uint8_t *, uint32_t ) override { return false; }
    void skip(int32_t ) override { }
    bool seek(uint32_t ) override { return false; }
    bool seek(uint32_t , int ) { return false; }
    void close() override { }
    int32_t tell(void) override { return 0; }
  };

 #endif

#endif

//----------------------------------------------------------------------------
 }
}
