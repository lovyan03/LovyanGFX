#ifndef LGFX_ESP32_COMMON_HPP_
#define LGFX_ESP32_COMMON_HPP_

#include "../lgfx_common.hpp"

#include <cstdint>
#include <driver/i2c.h>

#if defined ARDUINO
  #include <Arduino.h>
  #include <soc/periph_defs.h>

  #include <FS.h>
#else
  #include <freertos/FreeRTOS.h>
  #include <freertos/task.h>
  #include <driver/gpio.h>

  static inline void delay(std::uint32_t ms) { vTaskDelay(ms / portTICK_PERIOD_MS); }
#endif

namespace lgfx
{
  static inline void* heap_alloc(      size_t length) { return heap_caps_malloc(length, MALLOC_CAP_8BIT);  }
  static inline void* heap_alloc_dma(  size_t length) { return heap_caps_malloc((length + 3) & ~3, MALLOC_CAP_DMA);  }
  static inline void* heap_alloc_psram(size_t length) { return heap_caps_malloc(length, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);  }
  static inline void heap_free(void* buf) { heap_caps_free(buf); }

  enum pin_mode_t
  { output
  , input
  , input_pullup
  , input_pulldown
  };

  void lgfxPinMode(std::int_fast8_t pin, pin_mode_t mode);

  void initPWM(std::int_fast8_t pin, std::uint32_t pwm_ch, std::uint32_t freq = 12000, std::uint8_t duty = 128);

  void setPWMDuty(std::uint32_t pwm_ch, std::uint8_t duty);

  static inline volatile std::uint32_t* get_gpio_hi_reg(std::int_fast8_t pin) { return (pin & 32) ? &GPIO.out1_w1ts.val : &GPIO.out_w1ts; }
  static inline volatile std::uint32_t* get_gpio_lo_reg(std::int_fast8_t pin) { return (pin & 32) ? &GPIO.out1_w1tc.val : &GPIO.out_w1tc; }
  static inline void gpio_hi(std::int_fast8_t pin) { if (pin >= 0) *get_gpio_hi_reg(pin) = 1 << (pin & 31); }
  static inline void gpio_lo(std::int_fast8_t pin) { if (pin >= 0) *get_gpio_lo_reg(pin) = 1 << (pin & 31); }
  static inline bool gpio_in(std::int_fast8_t pin) { return ((pin & 32) ? GPIO.in1.data : GPIO.in) & (1 << (pin & 31)); }

  std::uint32_t getApbFrequency(void);
  std::uint32_t FreqToClockDiv(std::uint32_t fapb, std::uint32_t hz);

//----------------------------------------------------------------------------

#if defined (ARDUINO)

  struct FileWrapper : public DataWrapper
  {
    FileWrapper();
    fs::FS* _fs;
    fs::File *_fp;
    fs::File _file;

    FileWrapper(fs::FS& fs);
    FileWrapper(fs::FS& fs, fs::File* fp);

    void setFS(fs::FS& fs);

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
    int read(std::uint8_t *buf, std::uint32_t len) override { return _fp->read(buf, len); }
    void skip(std::int32_t offset) override { seek(offset, SeekCur); }
    bool seek(std::uint32_t offset) override { return seek(offset, SeekSet); }
    bool seek(std::uint32_t offset, SeekMode mode) { return _fp->seek(offset, mode); }
    void close() override { _fp->close(); }
  };

#else // ESP-IDF

  struct FileWrapper : public DataWrapper
  {
    FileWrapper();
    FILE* _fp;
    bool open(const char* path, const char* mode) { return (_fp = fopen(path, mode)); }
    int read(std::uint8_t *buf, std::uint32_t len) override { return fread((char*)buf, 1, len, _fp); }
    void skip(std::int32_t offset) override { seek(offset, SEEK_CUR); }
    bool seek(std::uint32_t offset) override { return seek(offset, SEEK_SET); }
    bool seek(std::uint32_t offset, int origin) { return fseek(_fp, offset, origin); }
    void close() override { fclose(_fp); }
  };

#endif

//----------------------------------------------------------------------------

#if defined (ARDUINO) && defined (Stream_h)

  struct StreamWrapper : public DataWrapper
  {
    void set(Stream* src, std::uint32_t length = ~0) { _stream = src; _length = length; _index = 0; }

    int read(std::uint8_t *buf, std::uint32_t len) override {
      len = std::min<std::uint32_t>(len, _stream->available());
      if (len > _length - _index) { len = _length - _index; }
      _index += len;
      return _stream->readBytes((char*)buf, len);
    }
    void skip(std::int32_t offset) override { if (0 < offset) { char dummy[offset]; _stream->readBytes(dummy, offset); _index += offset; } }
    bool seek(std::uint32_t offset) override { if (offset < _index) { return false; } skip(offset - _index); return true; }
    void close() override { }

  protected:
    Stream* _stream;
    std::uint32_t _index;
    std::uint32_t _length = 0;
  };

#endif

//----------------------------------------------------------------------------
  namespace spi
  {
    void init(int spi_host, int spi_sclk, int spi_miso, int spi_mosi, int dma_channel);
  }
};

#endif
