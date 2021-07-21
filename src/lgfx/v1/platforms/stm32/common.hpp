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

#include <malloc.h>

#include <Arduino.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  namespace stm32
  {
    static constexpr int PORT_SHIFT = 4;
    enum pin_port
    {
      PORT_A =  0 << PORT_SHIFT,
      PORT_B =  1 << PORT_SHIFT,
      PORT_C =  2 << PORT_SHIFT,
      PORT_D =  3 << PORT_SHIFT,
      PORT_E =  4 << PORT_SHIFT,
      PORT_F =  5 << PORT_SHIFT,
      PORT_G =  6 << PORT_SHIFT,
      PORT_H =  7 << PORT_SHIFT,
      PORT_I =  8 << PORT_SHIFT,
      PORT_J =  9 << PORT_SHIFT,
      PORT_K = 10 << PORT_SHIFT,
    };
  }

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
  static inline void* heap_alloc_dma(  size_t length) { return memalign(16, length); }
  static inline void heap_free(void* buf) { free(buf); }

  static inline volatile uint32_t* get_gpio_out_reg(int_fast8_t pin)
  {
    static constexpr size_t _offset_bsrr = 0x18;
    return (volatile uint32_t*)( AHB1PERIPH_BASE + _offset_bsrr + (0x0400UL * ((pin >> 4)&0x0F)));
  }

  static inline volatile uint32_t* get_gpio_in_reg(int_fast8_t pin)
  {
    static constexpr size_t _offset_idr = 0x10;
    return (volatile uint32_t*)( AHB1PERIPH_BASE + _offset_idr + (0x0400UL * ((pin >> 4)&0x0F)));
  }

  static inline void gpio_hi(int_fast8_t pin) { if (pin >= 0) *get_gpio_out_reg(pin) = 1ul <<  (pin & 15); }
  static inline void gpio_lo(int_fast8_t pin) { if (pin >= 0) *get_gpio_out_reg(pin) = 1ul << ((pin & 15) + 16); }
  static inline bool gpio_in(int_fast8_t pin) {         return *get_gpio_in_reg(pin) & (1ul << (pin & 0x0F));  }

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
  struct FileWrapper : public DataWrapper
  {
    FileWrapper() : DataWrapper() { need_transaction = true; }

#if defined (ARDUINO) && defined (__SEEED_FS__)

    fs::File _file;
    fs::File *_fp;

    fs::FS *_fs = nullptr;
    void setFS(fs::FS& fs) {
      _fs = &fs;
      need_transaction = false;
    }
    FileWrapper(fs::FS& fs) : DataWrapper(), _fp(nullptr) { setFS(fs); }
    FileWrapper(fs::FS& fs, fs::File* fp) : DataWrapper(), _fp(fp) { setFS(fs); }

    bool open(fs::FS& fs, const char* path) {
      setFS(fs);
      return open(path);
    }

    bool open(const char* path) override {
      fs::File file = _fs->open(path, "r");
      // この邪悪なmemcpyは、Seeed_FSのFile実装が所有権moveを提供してくれないのにデストラクタでcloseを呼ぶ実装になっているため、
      // 正攻法ではFileをクラスメンバに保持できない状況を打開すべく応急処置的に実装したものです。
      memcpy(&_file, &file, sizeof(fs::File));
      // memsetにより一時変数の中身を吹っ飛ばし、デストラクタによるcloseを予防します。
      memset(&file, 0, sizeof(fs::File));
      _fp = &_file;
      return _file;
    }

    int read(uint8_t *buf, uint32_t len) override { return _fp->read(buf, len); }
    void skip(int32_t offset) override { seek(offset, SeekCur); }
    bool seek(uint32_t offset) override { return seek(offset, SeekSet); }
    bool seek(uint32_t offset, SeekMode mode) { return _fp->seek(offset, mode); }
    void close() override { _fp->close(); }
    int32_t tell(void) override { return _fp->position(); }

#else  // dummy.

    bool open(const char*) override { return false; }
    int read(uint8_t*, uint32_t) override { return 0; }
    void skip(int32_t) override { }
    bool seek(uint32_t) override { return false; }
    bool seek(uint32_t, int) { return false; }
    void close() override { }
    int32_t tell(void) override { return 0; }

#endif

  };

//----------------------------------------------------------------------------

#if defined (ARDUINO) && defined (Stream_h)

  struct StreamWrapper : public DataWrapper
  {
    void set(Stream* src, uint32_t length = ~0u) { _stream = src; _length = length; _index = 0; }

    int read(uint8_t *buf, uint32_t len) override {
      if (len > _length - _index) { len = _length - _index; }
      _index += len;
      return _stream->readBytes((char*)buf, len);
    }
    void skip(int32_t offset) override { if (0 < offset) { char dummy[offset]; _stream->readBytes(dummy, offset); _index += offset; } }
    bool seek(uint32_t offset) override { if (offset < _index) { return false; } skip(offset - _index); return true; }
    void close() override { }
    int32_t tell(void) override { return _index; }

  private:
    Stream* _stream;
    uint32_t _index;
    uint32_t _length = 0;

  };

#endif

//----------------------------------------------------------------------------
 }
}
