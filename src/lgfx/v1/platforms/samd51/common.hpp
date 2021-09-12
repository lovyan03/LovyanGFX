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

#else

 // This has been defined once to prevent the dependency graph from malfunctioning when using platform IO with ESP32.
 #define  INCLUDE_FREERTOS_PATH <FreeRTOS.h>
 #include INCLUDE_FREERTOS_PATH
 #undef   INCLUDE_FREERTOS_PATH

 #define  INCLUDE_TASK_PATH <task.h>
 #include INCLUDE_TASK_PATH
 #undef   INCLUDE_TASK_PATH

 #include <config/default/system/fs/sys_fs.h>
 #include "samd51_arduino_compat.hpp"

 #undef PORT_PINCFG_PULLEN
 #undef PORT_PINCFG_PULLEN_Pos
 #undef PORT_PINCFG_INEN
 #undef PORT_PINCFG_INEN_Pos

 #define _Ul(n) (static_cast<uint32_t>((n)))
 #define PORT_PINCFG_INEN_Pos        1            /**< \brief (PORT_PINCFG) Input Enable */
 #define PORT_PINCFG_INEN            (_Ul(0x1) << PORT_PINCFG_INEN_Pos)
 #define PORT_PINCFG_PULLEN_Pos      2            /**< \brief (PORT_PINCFG) Pull Enable */
 #define PORT_PINCFG_PULLEN          (_Ul(0x1) << PORT_PINCFG_PULLEN_Pos)

#endif

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  namespace samd51
  {
    static constexpr int PORT_SHIFT = 5;
    static constexpr int PIN_MASK = (1 << PORT_SHIFT) - 1;
    enum pin_port
    {
      PORT_A =  0 << PORT_SHIFT,
      PORT_B =  1 << PORT_SHIFT,
      PORT_C =  2 << PORT_SHIFT,
      PORT_D =  3 << PORT_SHIFT,
    };

    struct sercom_data_t
    {
      uintptr_t sercomPtr;
      uint8_t   id_core;
      uint8_t   id_slow;
      int       dmac_id_tx;
      int       dmac_id_rx;
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

  static inline void gpio_hi(uint32_t pin) {        PORT->Group[pin >> samd51::PORT_SHIFT].OUTSET.reg = (1ul << (pin & samd51::PIN_MASK)); }
  static inline void gpio_lo(uint32_t pin) {        PORT->Group[pin >> samd51::PORT_SHIFT].OUTCLR.reg = (1ul << (pin & samd51::PIN_MASK)); }
  static inline bool gpio_in(uint32_t pin) { return PORT->Group[pin >> samd51::PORT_SHIFT].IN.reg     & (1ul << (pin & samd51::PIN_MASK)); }

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
    void close(void) override { if (_fp) _fp->close(); }
    int32_t tell(void) override { return _fp->position(); }

#elif __SAMD51_HARMONY__

    SYS_FS_HANDLE handle = SYS_FS_HANDLE_INVALID;

    bool open(const char* path) override
    {
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
