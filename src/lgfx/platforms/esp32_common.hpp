#ifndef LGFX_ESP32_COMMON_HPP_
#define LGFX_ESP32_COMMON_HPP_

#include "../lgfx_common.hpp"

#include <cstdint>

#if defined ARDUINO
  #include <Arduino.h>
  #include <soc/periph_defs.h>
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

  void initPWM(std::int_fast8_t pin, std::uint32_t pwm_ch, std::uint8_t duty = 128);

  void setPWMDuty(std::uint32_t pwm_ch, std::uint8_t duty);

  static inline volatile std::uint32_t* get_gpio_hi_reg(std::int_fast8_t pin) { return (pin & 32) ? &GPIO.out1_w1ts.val : &GPIO.out_w1ts; }
  static inline volatile std::uint32_t* get_gpio_lo_reg(std::int_fast8_t pin) { return (pin & 32) ? &GPIO.out1_w1tc.val : &GPIO.out_w1tc; }
  static inline void gpio_hi(std::int_fast8_t pin) { *get_gpio_hi_reg(pin) = 1 << (pin & 31); }
  static inline void gpio_lo(std::int_fast8_t pin) { *get_gpio_lo_reg(pin) = 1 << (pin & 31); }
  static inline bool gpio_in(std::int_fast8_t pin) { return ((pin & 32) ? GPIO.in1.data : GPIO.in) & (1 << (pin & 31)); }


/*
  static void initGPIO(std::int_fast8_t pin, gpio_mode_t mode = GPIO_MODE_OUTPUT, bool pullup = false, bool pulldown = false) {
    if (pin == -1) return;
#ifdef ARDUINO
    std::uint8_t pm = 0;
    if (mode & GPIO_MODE_DEF_INPUT)  pm |= INPUT;
    if (mode & GPIO_MODE_DEF_OUTPUT) pm |= OUTPUT;
    if (mode & GPIO_MODE_DEF_OD)     pm |= OPEN_DRAIN;
    if (pullup)                      pm |= PULLUP;
    if (pulldown)                    pm |= PULLDOWN;
    pinMode(pin, pm);
#else
    if (rtc_gpio_is_valid_gpio((gpio_num_t)pin)) rtc_gpio_deinit((gpio_num_t)pin);
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = mode;
    io_conf.pin_bit_mask = (std::uint64_t)1 << pin;
    io_conf.pull_down_en = pulldown ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = pullup ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
#endif
  }
  template<std::uint8_t PIN, std::uint32_t MASK>
  struct ESP32PIN {
    __attribute__ ((always_inline)) inline static void init(gpio_mode_t mode = GPIO_MODE_OUTPUT) { initGPIO((gpio_num_t)PIN, mode); }
    __attribute__ ((always_inline)) inline static void enableOutput()  { if (PIN < 32) { GPIO.enable_w1ts = MASK; } else { GPIO.enable1_w1ts.val = MASK; } }
    __attribute__ ((always_inline)) inline static void disableOutput() { if (PIN < 32) { GPIO.enable_w1tc = MASK; } else { GPIO.enable1_w1tc.val = MASK; } }
    __attribute__ ((always_inline)) inline static void hi()    { if (PIN < 32) GPIO.out_w1ts = MASK; else GPIO.out1_w1ts.val = MASK; }
    __attribute__ ((always_inline)) inline static void lo()    { if (PIN < 32) GPIO.out_w1tc = MASK; else GPIO.out1_w1tc.val = MASK; }
    __attribute__ ((always_inline)) inline static bool get()   { if (PIN < 32) return GPIO.in  & MASK; else return GPIO.in1.data & MASK; }
    __attribute__ ((always_inline)) inline static bool isset() { if (PIN < 32) return GPIO.out & MASK; else return GPIO.out1.val & MASK; }
  };
  struct ESP32NOPIN {
  public:
    __attribute__ ((always_inline)) inline static void init(gpio_mode_t mode = GPIO_MODE_OUTPUT) {}
    __attribute__ ((always_inline)) inline static void enableInput() {}
    __attribute__ ((always_inline)) inline static void enableOutput() {}
    __attribute__ ((always_inline)) inline static void disableOutput() {}
    __attribute__ ((always_inline)) inline static void hi() {}
    __attribute__ ((always_inline)) inline static void lo() {}
    __attribute__ ((always_inline)) inline static bool get() { return false; }
    __attribute__ ((always_inline)) inline static bool isset() { return false; }
  };

  template<int PIN>
  struct TPin : public ESP32PIN<PIN, ((PIN<32)?((std::uint32_t)1 << PIN):((std::uint32_t)1 << (PIN-32)))> {};

  template<>
  struct TPin<-1> : public ESP32NOPIN {};
//*/
//----------------------------------------------------------------------------
  struct FileWrapper : public DataWrapper {
    FileWrapper() : DataWrapper() { need_transaction = true; }
#if defined (ARDUINO) && defined (FS_H)
    fs::File _file;
    fs::File *_fp;
  #if defined (_SD_H_)
    fs::FS& _fs = SD;
    void setFS(fs::FS& fs) {
      _fs = fs;
      need_transaction = (&fs == &SD);
    }
    FileWrapper(fs::FS& fs) : DataWrapper(), _fp(nullptr), _fs(fs) { need_transaction = (&fs == &SD); }
    FileWrapper(fs::FS& fs, fs::File* fp) : DataWrapper(), _fp(fp), _fs(fs) { need_transaction = (&fs == &SD); }
  #else
    fs::FS& _fs = SPIFFS;
    void setFS(fs::FS& fs) {
      _fs = fs;
      need_transaction = (&fs != &SPIFFS);
    }
    FileWrapper(fs::FS& fs) : DataWrapper(), _fp(nullptr), _fs(fs) { need_transaction = (&fs != &SPIFFS); }
    FileWrapper(fs::FS& fs, fs::File* fp) : DataWrapper(), _fp(fp), _fs(fs) { need_transaction = (&fs != &SPIFFS); }
  #endif

    bool open(fs::FS& fs, const char* path, const char* mode) {
      setFS(fs);
      _file = fs.open(path, mode);
      _fp = &_file;
      return _file;
    }
    bool open(const char* path, const char* mode) {
      _file = _fs.open(path, mode);
      _fp = &_file;
      return _file;
    }
    int read(std::uint8_t *buf, std::uint32_t len) override { return _fp->read(buf, len); }
    void skip(std::int32_t offset) override { seek(offset, SeekCur); }
    bool seek(std::uint32_t offset) override { return seek(offset, SeekSet); }
    bool seek(std::uint32_t offset, SeekMode mode) { return _fp->seek(offset, mode); }
    void close() override { _fp->close(); }

#elif defined (CONFIG_IDF_TARGET_ESP32)  // ESP-IDF

    FILE* _fp;
    bool open(const char* path, const char* mode) { return (_fp = fopen(path, mode)); }
    int read(std::uint8_t *buf, std::uint32_t len) override { return fread((char*)buf, 1, len, _fp); }
    void skip(std::int32_t offset) override { seek(offset, SEEK_CUR); }
    bool seek(std::uint32_t offset) override { return seek(offset, SEEK_SET); }
    bool seek(std::uint32_t offset, int origin) { return fseek(_fp, offset, origin); }
    void close() override { fclose(_fp); }

#else  // dummy.

    bool open(const char* path, const char* mode) { return false; }
    int read(std::uint8_t *buf, std::uint32_t len) override { return 0; }
    void skip(std::int32_t offset) override { }
    bool seek(std::uint32_t offset) override { return false; }
    bool seek(std::uint32_t offset, int origin) { return false; }
    void close() override { }

#endif

  };
//----------------------------------------------------------------------------
  struct StreamWrapper : public DataWrapper {
#if defined (ARDUINO) && defined (Stream_h)
    void set(Stream* src, std::uint32_t length = ~0) { _stream = src; _length = length; _index = 0; }

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

    int read(std::uint8_t *buf, std::uint32_t len) override { return 0; }
    void skip(std::int32_t offset) override { }
    bool seek(std::uint32_t offset) override { return false; }

#endif

  };
};

#endif
