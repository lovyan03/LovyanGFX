#ifndef LGFX_ESP32_COMMON_HPP_
#define LGFX_ESP32_COMMON_HPP_

#include "../lgfx_common.hpp"

#include <stdint.h>
#include <driver/i2c.h>

#if defined ARDUINO
  #include <Arduino.h>
  #include <soc/periph_defs.h>
  #include <soc/dport_reg.h>
#else
  #include <freertos/FreeRTOS.h>
  #include <freertos/task.h>
  #include <driver/gpio.h>

  __attribute__ ((unused)) static inline unsigned long millis(void) { return (unsigned long) (esp_timer_get_time() / 1000ULL); }

  __attribute__ ((unused)) static inline unsigned long micros(void) { return (unsigned long) (esp_timer_get_time()); }

  __attribute__ ((unused)) static inline void delayMicroseconds(uint32_t us) { ets_delay_us(us); }

  __attribute__ ((unused)) static inline void delay(uint32_t ms)
  {
    uint32_t time = micros();
    vTaskDelay( (ms >= portTICK_PERIOD_MS) ? (ms / portTICK_PERIOD_MS - 1) : 0);
    ms *= 1000;
    time = micros() - time;
    if (time < ms)
    {
      ets_delay_us(ms - time);
    }
  }

#endif

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------

  static inline void* heap_alloc(      size_t length) { return heap_caps_malloc(length, MALLOC_CAP_8BIT);  }
  static inline void* heap_alloc_dma(  size_t length) { return heap_caps_malloc((length + 3) & ~3, MALLOC_CAP_DMA);  }
  static inline void* heap_alloc_psram(size_t length) { return heap_caps_malloc(length, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);  }
  static inline void heap_free(void* buf) { heap_caps_free(buf); }

  inline void delay(uint32_t ms) { ::delay(ms); }
  inline unsigned long millis(void) { return ::millis(); }
  inline unsigned long micros(void) { return ::micros(); }

  enum pin_mode_t
  { output
  , input
  , input_pullup
  , input_pulldown
  };

  void lgfxPinMode(int_fast8_t pin, pin_mode_t mode);

  void initPWM(int_fast8_t pin, uint32_t pwm_ch, uint32_t freq = 12000, uint8_t duty = 128);

  void setPWMDuty(uint32_t pwm_ch, uint8_t duty);

  static inline volatile uint32_t* get_gpio_hi_reg(int_fast8_t pin) { return (pin & 32) ? &GPIO.out1_w1ts.val : &GPIO.out_w1ts; }
  static inline volatile uint32_t* get_gpio_lo_reg(int_fast8_t pin) { return (pin & 32) ? &GPIO.out1_w1tc.val : &GPIO.out_w1tc; }
  static inline void gpio_hi(int_fast8_t pin) { if (pin >= 0) *get_gpio_hi_reg(pin) = 1 << (pin & 31); }
  static inline void gpio_lo(int_fast8_t pin) { if (pin >= 0) *get_gpio_lo_reg(pin) = 1 << (pin & 31); }
  static inline bool gpio_in(int_fast8_t pin) { return ((pin & 32) ? GPIO.in1.data : GPIO.in) & (1 << (pin & 31)); }

  uint32_t getApbFrequency(void);
  uint32_t FreqToClockDiv(uint32_t fapb, uint32_t hz);

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

    bool open(fs::FS& fs, const char* path, const char* mode) {
      setFS(fs);
      _file = fs.open(path, mode);
      _fp = &_file;
      return _file;
    }
    bool open(const char* path, const char* mode) {
      _file = _fs->open(path, mode);
      _fp = &_file;
      return _file;
    }
    int read(uint8_t *buf, uint32_t len) override { return _fp->read(buf, len); }
    void skip(int32_t offset) override { seek(offset, SeekCur); }
    bool seek(uint32_t offset) override { return seek(offset, SeekSet); }
    bool seek(uint32_t offset, SeekMode mode) { return _fp->seek(offset, mode); }
    void close() override { _fp->close(); }
    int32_t tell(void) override { return _fp->position(); }
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
    bool open(const char* path, const char* mode) { return false; }
    int read(uint8_t *buf, uint32_t len) override { return false; }
    void skip(int32_t offset) override { }
    bool seek(uint32_t offset) override { return false; }
    bool seek(uint32_t offset, int origin) { return false; }
    void close() override { }
    int32_t tell(void) override { return 0; }
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
    bool open(const char* path, const char* mode) { return (_fp = fopen(path, mode)); }
    int read(uint8_t *buf, uint32_t len) override { return fread((char*)buf, 1, len, _fp); }
    void skip(int32_t offset) override { seek(offset, SEEK_CUR); }
    bool seek(uint32_t offset) override { return seek(offset, SEEK_SET); }
    bool seek(uint32_t offset, int origin) { return fseek(_fp, offset, origin); }
    void close() override { fclose(_fp); }
    int32_t tell(void) override { return ftell(_fp); }
  };

#endif

//----------------------------------------------------------------------------

#if defined (ARDUINO) && defined (Stream_h)

  struct StreamWrapper : public DataWrapper
  {
    void set(Stream* src, uint32_t length = ~0) { _stream = src; _length = length; _index = 0; }

    int read(uint8_t *buf, uint32_t len) override {
      len = std::min<uint32_t>(len, _stream->available());
      if (len > _length - _index) { len = _length - _index; }
      _index += len;
      return _stream->readBytes((char*)buf, len);
    }
    void skip(int32_t offset) override { if (0 < offset) { char dummy[offset]; _stream->readBytes(dummy, offset); _index += offset; } }
    bool seek(uint32_t offset) override { if (offset < _index) { return false; } skip(offset - _index); return true; }
    void close() override { }
    int32_t tell(void) override { return _index; }

  protected:
    Stream* _stream;
    uint32_t _index;
    uint32_t _length = 0;
  };

#endif

//----------------------------------------------------------------------------

  namespace spi
  {
    void init(int spi_host, int spi_sclk, int spi_miso, int spi_mosi, int dma_channel);
  }
//----------------------------------------------------------------------------
 }
}

#endif
