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
#include <SPI.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  /// spi_host に対応する Arduino の SPIClass インスタンスを返す。
  /// 複数の SPI を持たない環境では常に既定の SPI を返す。
  __attribute__ ((unused))
  static inline SPIClass* getSPIInstance(int spi_host)
  {
#if defined ( TEENSYDUINO )
 #if defined (__MK64FX512__) || defined (__MK66FX1M0__) || defined (__IMXRT1062__)
    if (spi_host == 1) { return &SPI1; }
    if (spi_host == 2) { return &SPI2; }
 #elif defined (__MKL26Z64__)
    if (spi_host == 1) { return &SPI1; }
 #endif
#endif
    (void)spi_host;
    return &SPI;
  }

  /// SPI_MODE0~3 の実値は環境依存のため、spi_mode の生値を SPISettings に渡さず変換する。
  __attribute__ ((unused))
  static inline uint8_t getSPIDataMode(int spi_mode)
  {
    switch (spi_mode & 3)
    {
    default:
    case 0: return SPI_MODE0;
    case 1: return SPI_MODE1;
    case 2: return SPI_MODE2;
    case 3: return SPI_MODE3;
    }
  }

  /// SPIClass::transfer(void*, size_t) は受信データでバッファを上書きするため、
  /// 送信専用の転送は複製を経由して呼び出し元の入力を守る。
  __attribute__ ((unused))
  static inline void spiWriteBytes(SPIClass* spi, const uint8_t* data, size_t length)
  {
    uint8_t buf[64];
    while (length)
    {
      size_t len = (length < sizeof(buf)) ? length : sizeof(buf);
      memcpy(buf, data, len);
      spi->transfer(buf, len);
      data += len;
      length -= len;
    }
  }

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
  static inline bool heap_capable_dma(const void* ptr) { return false; }

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
      // この邪悪なmemcpyは、Seeed_FSのFile実装が所有権moveを提供してくれないのにデストラクタでcloseを呼ぶ実装になっているため、;
      // 正攻法ではFileをクラスメンバに保持できない状況を打開すべく応急処置的に実装したものです。;
      memcpy(&_file, &file, sizeof(fs::File));
      // memsetにより一時変数の中身を吹っ飛ばし、デストラクタによるcloseを予防します。;
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
 }
}
