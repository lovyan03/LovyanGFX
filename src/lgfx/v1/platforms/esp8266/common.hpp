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

#include <malloc.h>

#if defined ( ARDUINO )
 #include <Arduino.h>
#endif

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

#if defined ( ARDUINO )
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

#else

  unsigned long millis(void);
  unsigned long micros(void);
  void delay(unsigned long ms);

#endif

  __attribute__ ((unused))
  static void delayMicroseconds(unsigned int us)
  {
    ets_delay_us(us);
  }

  static inline void* heap_alloc(      size_t length) { return malloc(length); }
  static inline void* heap_alloc_psram(size_t length) { return malloc(length); }
  static inline void* heap_alloc_dma(  size_t length) { return malloc(length); } // aligned_alloc(16, length);
  static inline void heap_free(void* buf) { free(buf); }

  static inline void gpio_hi(std::int_fast8_t pin) { if (pin & 16) { if (pin == 16) *(volatile uint32_t*)(0x60000768) |=  1; } else { *(volatile uint32_t*)(0x60000304) = 1 << (pin & 15); } }
  static inline void gpio_lo(std::int_fast8_t pin) { if (pin & 16) { if (pin == 16) *(volatile uint32_t*)(0x60000768) &= ~1; } else { *(volatile uint32_t*)(0x60000308) = 1 << (pin & 15); } }
  static inline bool gpio_in(std::int_fast8_t pin)
  {
    return *(volatile uint32_t*)((pin & 16)
         ? 0x6000078C // GP16I
         : 0x60000318 // GPI
         ) & (1 << (pin & 15));
  }

  enum pin_mode_t
  { output
  , input
  , input_pullup
  , input_pulldown
  };

  void pinMode(std::int_fast16_t pin, pin_mode_t mode);
  inline void lgfxPinMode(std::int_fast16_t pin, pin_mode_t mode)
  {
    pinMode(pin, mode);
  }

//----------------------------------------------------------------------------

#if defined (ARDUINO)
 #if defined (FS_H)

  struct FileWrapper : public DataWrapper
  {
private:
#if defined (_SD_H_)
    bool _check_need_transaction(void) const { return _fs == &SD; }
#elif defined (_SPIFFS_H_)
    bool _check_need_transaction(void) const { return _fs != &SPIFFS; }
#else
    bool _check_need_transaction(void) const { return false; }
#endif

public:
    FileWrapper() : DataWrapper()
    {
#if defined (_SD_H_)
      _fs = &SD;
#elif defined (_SPIFFS_H_)
      _fs = &SPIFFS;
#else
      _fs = nullptr;
#endif
      need_transaction = _check_need_transaction();
      _fp = nullptr;
    }

    fs::FS* _fs;
    fs::File *_fp;
    fs::File _file;

    FileWrapper(fs::FS& fs, fs::File* fp = nullptr) : DataWrapper(), _fs(&fs), _fp(fp) { need_transaction = _check_need_transaction(); }
    void setFS(fs::FS& fs) {
      _fs = &fs;
      need_transaction = _check_need_transaction();
    }

    bool open(fs::FS& fs, const char* path)
    {
      setFS(fs);
      return open(path);
    }
    bool open(const char* path) override
    {
      _file = _fs->open(path, "r");
      _fp = &_file;
      return _file;
    }
    int read(std::uint8_t *buf, std::uint32_t len) override { return _fp->read(buf, len); }
    void skip(std::int32_t offset) override { seek(offset, SeekCur); }
    bool seek(std::uint32_t offset) override { return seek(offset, SeekSet); }
    bool seek(std::uint32_t offset, SeekMode mode) { return _fp->seek(offset, mode); }
    void close(void) override { if (_fp) _fp->close(); }
    std::int32_t tell(void) override { return _fp->position(); }
  };
 #else
  // dummy
  struct FileWrapper : public DataWrapper
  {
    FileWrapper() : DataWrapper()
    {
      need_transaction = true;
    }
    void* _fp;

    template <typename T>
    void setFS(T& fs) {}

    bool open(const char* path, const char* mode) { return false; }
    int read(std::uint8_t *buf, std::uint32_t len) override { return false; }
    void skip(std::int32_t offset) override { }
    bool seek(std::uint32_t offset) override { return false; }
    bool seek(std::uint32_t offset, int origin) { return false; }
    void close() override { }
    std::int32_t tell(void) override { return 0; }
  };

 #endif
#else // ESP-IDF

  struct FileWrapper : public DataWrapper
  {
    FileWrapper() : DataWrapper()
    {
      need_transaction = true;
    }
    FILE* _fp;
    bool open(const char* path) override { return (_fp = fopen(path, "r")); }
    int read(std::uint8_t *buf, std::uint32_t len) override { return fread((char*)buf, 1, len, _fp); }
    void skip(std::int32_t offset) override { seek(offset, SEEK_CUR); }
    bool seek(std::uint32_t offset) override { return seek(offset, SEEK_SET); }
    bool seek(std::uint32_t offset, int origin) { return fseek(_fp, offset, origin); }
    void close() override { if (_fp) fclose(_fp); }
    std::int32_t tell(void) override { return ftell(_fp); }
  };

#endif

//----------------------------------------------------------------------------

#if defined (ARDUINO) && defined (Stream_h)

  struct StreamWrapper : public DataWrapper
  {
    void set(Stream* src, std::uint32_t length = ~0u) { _stream = src; _length = length; _index = 0; }

    int read(std::uint8_t *buf, std::uint32_t len) override {
      len = std::min<std::uint32_t>(len, _stream->available());
      if (len > _length - _index) { len = _length - _index; }
      _index += len;
      return _stream->readBytes((char*)buf, len);
    }
    void skip(std::int32_t offset) override { if (0 < offset) { char dummy[offset]; _stream->readBytes(dummy, offset); _index += offset; } }
    bool seek(std::uint32_t offset) override { if (offset < _index) { return false; } skip(offset - _index); return true; }
    void close() override { }
    std::int32_t tell(void) override { return _index; }

  protected:
    Stream* _stream;
    std::uint32_t _index;
    std::uint32_t _length = 0;
  };

#endif

//----------------------------------------------------------------------------
 }
}
