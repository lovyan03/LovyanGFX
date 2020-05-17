#ifndef LGFX_SAMD51_COMMON_HPP_
#define LGFX_SAMD51_COMMON_HPP_

#include "../lgfx_common.hpp"

#include <malloc.h>
#include <sam.h>

namespace lgfx
{
  static inline void* heap_alloc(      size_t length) { return malloc(length); }
  static inline void* heap_alloc_psram(size_t length) { return malloc(length); }
  static inline void* heap_alloc_dma(  size_t length) { return memalign(16, length); }
  static inline void heap_free(void* buf) { free(buf); }

  static inline void gpio_hi(std::uint32_t pin) { PORT->Group[pin>>8].OUTSET.reg = (1ul << (pin & 0xFF)); }
  static inline void gpio_lo(std::uint32_t pin) { PORT->Group[pin>>8].OUTCLR.reg = (1ul << (pin & 0xFF)); }
  static inline bool gpio_in(std::uint32_t pin) { return PORT->Group[pin>>8].IN.reg & (1ul << (pin & 0xFF)); }


//  static void initPWM(std::uint32_t pin, std::uint32_t pwm_ch, std::uint8_t duty = 128) 
  static inline void initPWM(std::uint32_t , std::uint32_t , std::uint8_t = 0) {
// unimplemented 
  }

//  static void setPWMDuty(std::uint32_t pwm_ch, std::uint8_t duty = 128) 
  static inline void setPWMDuty(std::uint32_t , std::uint8_t = 0 ) {
// unimplemented 
  }

  enum pin_mode_t
  { output
  , input
  , input_pullup
  , input_pulldown
  };

  void lgfxPinMode(std::uint32_t pin, pin_mode_t mode);

//----------------------------------------------------------------------------
  struct FileWrapper : public DataWrapper {
    FileWrapper() : DataWrapper() { need_transaction = true; }
#if defined (ARDUINO) && defined (__SEEED_FS__) && defined (__SD_H__)
    fs::File _fp;

    fs::FS& _fs = SD;
    void setFS(fs::FS& fs) {
      _fs = fs;
      need_transaction = (&fs == &SD);
    }
    FileWrapper(fs::FS& fs) : DataWrapper(), _fs(fs) { need_transaction = (&fs == &SD); }

    bool open(fs::FS& fs, const char* path, const char* mode) {
      setFS(fs);
      return open(path, mode);
    }

    bool open(const char* path, const char* mode) { 
      fs::File fp = _fs.open(path, mode);
      // この邪悪なmemcpyは、Seeed_FSのFile実装が所有権moveを提供してくれないのにデストラクタでcloseを呼ぶ実装になっているため、
      // 正攻法ではFileをクラスメンバに保持できない状況を打開すべく応急処置的に実装したものです。
      memcpy(&_fp, &fp, sizeof(fs::File));
      // memsetにより一時変数の中身を吹っ飛ばし、デストラクタによるcloseを予防します。
      memset(&fp, 0, sizeof(fs::File));
      return _fp;
    }

    int read(std::uint8_t *buf, std::uint32_t len) override { return _fp.read(buf, len); }
    void skip(std::int32_t offset) override { seek(offset, SeekCur); }
    bool seek(std::uint32_t offset) override { return seek(offset, SeekSet); }
    bool seek(std::uint32_t offset, SeekMode mode) { return _fp.seek(offset, mode); }
    void close() override { _fp.close(); }

#else  // dummy.

    bool open(const char*, const char*) { return false; }
    int read(std::uint8_t*, std::uint32_t) override { return 0; }
    void skip(std::int32_t) override { }
    bool seek(std::uint32_t) override { return false; }
    bool seek(std::uint32_t, int) { return false; }
    void close() override { }

#endif

  };
//----------------------------------------------------------------------------
  struct StreamWrapper : public DataWrapper {
#if defined (ARDUINO) && defined (Stream_h)
    void set(Stream* src, std::uint32_t length = ~0u) { _stream = src; _length = length; _index = 0; }

    int read(std::uint8_t *buf, std::uint32_t len) override {
      if (len > _length - _index) { len = _length - _index; }
      _index += len;
      return _stream->readBytes((char*)buf, len);
    }
    void skip(std::int32_t offset) override { if (0 < offset) { char dummy[offset]; _stream->readBytes(dummy, offset); _index += offset; } }
    bool seek(std::uint32_t offset) override { if (offset < _index) { return false; } skip(offset - _index); return true; }
    void close() override { }

  private:
    Stream* _stream;
    std::uint32_t _index;
    std::uint32_t _length = 0;

#else  // dummy.

    int read(std::uint8_t*, std::uint32_t) override { return 0; }
    void skip(std::int32_t) override { }
    bool seek(std::uint32_t) override { return false; }

#endif

  };
};

#endif
