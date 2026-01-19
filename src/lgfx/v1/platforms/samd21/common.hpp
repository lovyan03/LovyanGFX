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

 #include <sam.h>
 #include <delay.h>
 #include <Arduino.h>

#endif

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  namespace samd21
  {
    static constexpr int PORT_SHIFT = 5;
    static constexpr int PIN_MASK = (1 << PORT_SHIFT) - 1;
    enum pin_port
    {
      PORT_A =  0 << PORT_SHIFT,
      PORT_B =  1 << PORT_SHIFT,
    };

    struct sercom_data_t {
      uintptr_t sercomPtr;
      uint8_t   clock;
      IRQn_Type irqn;
    };
    const sercom_data_t* getSercomData(size_t sercom_number);
  }

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
  __attribute__ ((unused))
  static void delayMicroseconds(unsigned int us)
  {
    ::delayMicroseconds(us);
  }

#else

  static inline void delay(size_t milliseconds)
  {
    vTaskDelay(pdMS_TO_TICKS(milliseconds));
  }

  static void delayMicroseconds(unsigned int us)
  {
    uint32_t start, elapsed;
    uint32_t count;

    if (us == 0)
      return;

    count = us * (VARIANT_MCK / 1000000) - 20;  // convert us to cycles.
    start = DWT->CYCCNT;  //CYCCNT is 32bits, takes 37s or so to wrap.
    while (1) {
      elapsed = DWT->CYCCNT - start;
      if (elapsed >= count)
        return;
    }
  }

#endif

  static inline void* heap_alloc(      size_t length) { return malloc(length); }
  static inline void* heap_alloc_psram(size_t length) { return malloc(length); }
  static inline void* heap_alloc_dma(  size_t length) { return memalign(16, length); }
  static inline void heap_free(void* buf) { free(buf); }
  static inline bool heap_capable_dma(const void* ptr) { return false; }

  static inline void gpio_hi(uint32_t pin) { if (pin > 255) return;              PORT->Group[pin >> samd21::PORT_SHIFT].OUTSET.reg = (1ul << (pin & samd21::PIN_MASK)); }
  static inline void gpio_lo(uint32_t pin) { if (pin > 255) return;              PORT->Group[pin >> samd21::PORT_SHIFT].OUTCLR.reg = (1ul << (pin & samd21::PIN_MASK)); }
  static inline bool gpio_in(uint32_t pin) { if (pin > 255) return false; return PORT->Group[pin >> samd21::PORT_SHIFT].IN.reg     & (1ul << (pin & samd21::PIN_MASK)); }

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
  void pinAssignPeriph(int pin_and_port, int type = PIO_SERCOM);

//----------------------------------------------------------------------------

#if defined (__SEEED_FS__)

  template <>
  struct DataWrapperT<fs::File> : public DataWrapper {
    DataWrapperT(fs::File* fp = nullptr) : DataWrapper{}, _fp { fp } {
      need_transaction = true;
    }
    int read(uint8_t *buf, uint32_t len) override { return _fp->read(buf, len); }
    void skip(int32_t offset) override { _fp->seek(offset, SeekCur); }
    bool seek(uint32_t offset) override { return _fp->seek(offset, SeekSet); }
    bool seek(uint32_t offset, SeekMode mode) { return _fp->seek(offset, mode); }
    void close(void) override { if (_fp) _fp->close(); }
    int32_t tell(void) override { return _fp->position(); }
protected:
    fs::File *_fp;
  };

  template <>
  struct DataWrapperT<fs::FS> : public DataWrapperT<fs::File> {
    DataWrapperT(fs::FS* fs, fs::File* fp = nullptr) : DataWrapperT<fs::File> { fp }, _fs { fs } {}
    bool open(const char* path) override
    {
      _file = _fs->open(path, "r");
      DataWrapperT<fs::File>::_fp = &_file;
      return _file;
    }

protected:
    fs::FS* _fs;
    fs::File _file;
  };

#if defined (__SD_H__)
  template <>
  struct DataWrapperT<fs::SDFS> : public DataWrapperT<fs::FS> {
    DataWrapperT(fs::FS* fs, fs::File* fp = nullptr) : DataWrapperT<fs::FS>(fs, fp) {}
  };
#endif
#endif

#if defined (__SAMD51_HARMONY__) && ( defined (__FILE_defined) || defined (_FILE_DEFINED) || defined (_FSTDIO) )

  template <>
  struct DataWrapperT<FILE> : public DataWrapper
  {
    DataWrapperT(FILE* fp = nullptr) : DataWrapper() , _fp { fp }
    {
      need_transaction = true;
    }
    bool open(const char* path) override {
      this->handle = SYS_FS_FileOpen(path, SYS_FS_FILE_OPEN_ATTRIBUTES::SYS_FS_FILE_OPEN_READ);
      return this->handle != SYS_FS_HANDLE_INVALID;
    }
    int read(uint8_t* buffer, uint32_t length) override
    {
      return SYS_FS_FileRead(this->handle, buffer, length);
    }
    void skip(int32_t offset) override
    {
      SYS_FS_FileSeek(this->handle, offset, SYS_FS_FILE_SEEK_CONTROL::SYS_FS_SEEK_CUR);
    }
    bool seek(uint32_t offset) override
    {
      return SYS_FS_FileSeek(this->handle, offset, SYS_FS_FILE_SEEK_CONTROL::SYS_FS_SEEK_SET) >= 0;
    }
    bool seek(uint32_t offset, SYS_FS_FILE_SEEK_CONTROL mode)
    {
      return SYS_FS_FileSeek(this->handle, offset, mode) >= 0;
    }
    void close(void) override
    {
      if( this->handle != SYS_FS_HANDLE_INVALID ) {
        SYS_FS_FileClose(this->handle);
        this->handle = SYS_FS_HANDLE_INVALID;
      }
    }
    int32_t tell(void) override
    {
      return SYS_FS_FileTell(this->handle);
    }
  protected:
    SYS_FS_HANDLE handle = SYS_FS_HANDLE_INVALID;
  };

  template <>
  struct DataWrapperT<void> : public DataWrapperT<FILE>
  {
    DataWrapperT(void) : DataWrapperT<FILE>() {}
  };
#endif

//----------------------------------------------------------------------------
 }
}
