#ifndef LGFX_SAMD51_COMMON_HPP_
#define LGFX_SAMD51_COMMON_HPP_

#ifdef ARDUINO
  #include <avr/pgmspace.h>
#endif

#include <malloc.h>

namespace lgfx
{
  static void* heap_alloc_psram(size_t length)
  {
    return malloc(length);
  }

  __attribute__((__used__))
  __attribute__((always_inline)) inline 
  static void* heap_alloc_dma(size_t length)
  {
    return memalign(16, length);
  }

  __attribute__((__used__))
  __attribute__((always_inline)) inline 
  static void* heap_alloc(size_t length)
  {
    return malloc(length);
  }

  __attribute__((__used__))
  __attribute__((always_inline)) inline 
  static void heap_free(void* buf)
  {
    free(buf);
  }

  static void gpio_hi(uint32_t pin) { PORT->Group[pin>>8].OUTSET.reg = (1ul << (pin & 0xFF)); }
  static void gpio_lo(uint32_t pin) { PORT->Group[pin>>8].OUTCLR.reg = (1ul << (pin & 0xFF)); }
  static bool gpio_in(uint32_t pin) { return PORT->Group[pin>>8].IN.reg = (1ul << (pin & 0xFF)); }
//*/

  __attribute__((__used__))
  static void initPWM(uint32_t pin, uint32_t pwm_ch, uint8_t duty = 128) {
// unimplemented 
  }

  __attribute__((__used__))
  static void setPWMDuty(uint32_t pwm_ch, uint8_t duty) {
// unimplemented 
  }

  enum pin_mode_t
  { output
  , input
  , input_pullup
  , input_pulldown
  };

  static void lgfxPinMode(uint32_t pin, pin_mode_t mode)
  {
    uint32_t port = pin>>8;
    pin &= 0xFF;
    uint32_t pinMask = (1ul << pin);

    // Set pin mode according to chapter '22.6.3 I/O Pin Configuration'
    switch ( mode )
    {
      case pin_mode_t::input:
        // Set pin to input mode
        PORT->Group[port].PINCFG[pin].reg=(uint8_t)(PORT_PINCFG_INEN) ;
        PORT->Group[port].DIRCLR.reg = pinMask ;
      break ;

      case pin_mode_t::input_pullup:
        // Set pin to input mode with pull-up resistor enabled
        PORT->Group[port].PINCFG[pin].reg=(uint8_t)(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN) ;
        PORT->Group[port].DIRCLR.reg = pinMask ;

        // Enable pull level (cf '22.6.3.2 Input Configuration' and '22.8.7 Data Output Value Set')
        PORT->Group[port].OUTSET.reg = pinMask ;
      break ;

      case pin_mode_t::input_pulldown:
        // Set pin to input mode with pull-down resistor enabled
        PORT->Group[port].PINCFG[pin].reg=(uint8_t)(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN) ;
        PORT->Group[port].DIRCLR.reg = pinMask ;

        // Enable pull level (cf '22.6.3.2 Input Configuration' and '22.8.6 Data Output Value Clear')
        PORT->Group[port].OUTCLR.reg = pinMask ;
      break ;

      case pin_mode_t::output:
        // enable input, to support reading back values, with pullups disabled
        PORT->Group[port].PINCFG[pin].reg=(uint8_t)(PORT_PINCFG_INEN) ;

        // Set pin to output mode
        PORT->Group[port].DIRSET.reg = pinMask ;
      break ;

      default:
        // do nothing
      break ;
    }
  }

//----------------------------------------------------------------------------
  struct FileWrapper : public DataWrapper {
    FileWrapper() : DataWrapper() { need_transaction = true; }
#if defined (ARDUINO) && defined (FS_H)
    fs::File _fp;
  #if defined (_SD_H_)
    fs::FS& _fs = SD;
    void setFS(fs::FS& fs) {
      _fs = fs;
      need_transaction = (&fs == &SD);
    }
    FileWrapper(fs::FS& fs) : DataWrapper(), _fs(fs) { need_transaction = (&fs == &SD); }
  #else
    fs::FS& _fs = SPIFFS;
    void setFS(fs::FS& fs) {
      _fs = fs;
      need_transaction = (&fs != &SPIFFS);
    }
    FileWrapper(fs::FS& fs) : DataWrapper(), _fs(fs) { need_transaction = (&fs != &SPIFFS); }
  #endif

    bool open(fs::FS& fs, const char* path, const char* mode) {
      setFS(fs);
      return (_fp = fs.open(path, mode));
    }
    bool open(const char* path, const char* mode) { return ( _fp = _fs.open(path, mode)); }
    int read(uint8_t *buf, uint32_t len) override { return _fp.read(buf, len); }
    void skip(int32_t offset) override { seek(offset, SeekCur); }
    bool seek(uint32_t offset) override { return seek(offset, SeekSet); }
    bool seek(uint32_t offset, SeekMode mode) { return _fp.seek(offset, mode); }
    void close() override { _fp.close(); }

#else  // dummy.

    bool open(const char* path, const char* mode) { return false; }
    int read(uint8_t *buf, uint32_t len) override { return 0; }
    void skip(int32_t offset) override { }
    bool seek(uint32_t offset) override { return false; }
    bool seek(uint32_t offset, int origin) { return false; }
    void close() override { }

#endif

  };
//----------------------------------------------------------------------------
  struct StreamWrapper : public DataWrapper {
#if defined (ARDUINO) && defined (Stream_h)
    void set(Stream* src, uint32_t length = ~0) { _stream = src; _length = length; _index = 0; }

    int read(uint8_t *buf, uint32_t len) override {
      if (len > _length - _index) { len = _length - _index; }
      _index += len;
      return _stream->readBytes((char*)buf, len);
    }
    void skip(int32_t offset) override { if (0 < offset) { char dummy[offset]; _stream->readBytes(dummy, offset); _index += offset; } }
    bool seek(uint32_t offset) override { if (offset < _index) { return false; } skip(offset - _index); return true; }
    void close() override { }

  private:
    Stream* _stream;
    uint32_t _index;
    uint32_t _length = 0;

#else  // dummy.

    int read(uint8_t *buf, uint32_t len) override { return 0; }
    void skip(int32_t offset) override { }
    bool seek(uint32_t offset) override { return false; }

#endif

  };
};

#endif
