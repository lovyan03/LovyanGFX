#ifndef LGFX_ESP32_COMMON_HPP_
#define LGFX_ESP32_COMMON_HPP_

#include <freertos/task.h>
#include <driver/rtc_io.h>
#include <soc/rtc.h>

#ifdef ARDUINO
  #include <driver/periph_ctrl.h>
  #include <soc/dport_reg.h>
  #include <soc/periph_defs.h>
  #include <soc/spi_reg.h>
  #include <soc/spi_struct.h>

  #include <esp32-hal-cpu.h>
  #include <esp32-hal-ledc.h>
  #include <pgmspace.h>
#else
  #include <driver/ledc.h>

  #ifndef pgm_read_byte
    #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
    #define pgm_read_word(addr)  ({ typeof(addr) _addr = (addr); *(const unsigned short *)(_addr); })
    #define pgm_read_dword(addr) ({ typeof(addr) _addr = (addr); *(const unsigned long *)(_addr); })
    #define pgm_read_float(addr) ({ typeof(addr) _addr = (addr); *(const float *)(_addr); })
    #define pgm_read_ptr(addr)   ({ typeof(addr) _addr = (addr); *(void * const *)(_addr); })
    #define pgm_read_byte_near(addr)  pgm_read_byte(addr)
    #define pgm_read_word_near(addr)  pgm_read_word(addr)
    #define pgm_read_dword_near(addr) pgm_read_dword(addr)
    #define pgm_read_float_near(addr) pgm_read_float(addr)
    #define pgm_read_ptr_near(addr)   pgm_read_ptr(addr)
    #define pgm_read_byte_far(addr)   pgm_read_byte(addr)
    #define pgm_read_word_far(addr)   pgm_read_word(addr)
    #define pgm_read_dword_far(addr)  pgm_read_dword(addr)
    #define pgm_read_float_far(addr)  pgm_read_float(addr)
    #define pgm_read_ptr_far(addr)    pgm_read_ptr(addr)
    #define PROGMEM
  #endif

  void delay(uint32_t ms) { vTaskDelay(ms / portTICK_PERIOD_MS); }

  static constexpr uint32_t MATRIX_DETACH_OUT_SIG = 0x100;
  static constexpr uint32_t MATRIX_DETACH_IN_LOW_PIN = 0x30;
  static constexpr uint32_t MATRIX_DETACH_IN_LOW_HIGH = 0x38;
  void IRAM_ATTR pinMatrixOutAttach(uint8_t pin, uint8_t function, bool invertOut, bool invertEnable) { gpio_matrix_out(pin,              function, invertOut, invertEnable); }
  void IRAM_ATTR pinMatrixOutDetach(uint8_t pin                  , bool invertOut, bool invertEnable) { gpio_matrix_out(pin, MATRIX_DETACH_OUT_SIG, invertOut, invertEnable); }
  void IRAM_ATTR pinMatrixInAttach( uint8_t pin, uint8_t signal           , bool inverted) { gpio_matrix_in(pin, signal, inverted); }
  void IRAM_ATTR pinMatrixInDetach(              uint8_t signal, bool high, bool inverted) { gpio_matrix_in(high?MATRIX_DETACH_IN_LOW_HIGH:MATRIX_DETACH_IN_LOW_PIN, signal, inverted); }

  uint32_t getApbFrequency() {
    rtc_cpu_freq_config_t conf;
    rtc_clk_cpu_freq_get_config(&conf);
    if (conf.freq_mhz >= 80){
      return 80 * 1000000;
    }
    return (conf.source_freq_mhz * 1000000) / conf.div;
  }
#endif


namespace lgfx
{
  static uint32_t FreqToClockDiv(uint32_t fapb, uint32_t hz)
  {
    if (hz > ((fapb >> 2) * 3)) {
      return SPI_CLK_EQU_SYSCLK;
    }
    uint32_t besterr = fapb;
    uint32_t halfhz = hz >> 1;
    uint32_t bestn = 0;
    uint32_t bestpre = 0;
    for (uint32_t n = 2; n <= 64; n++) {
      uint32_t pre = ((fapb / n) + halfhz) / hz;
      if (pre == 0) pre = 1;
      else if (pre > 8192) pre = 8192;

      int errval = abs((int32_t)(fapb / (pre * n) - hz));
      if (errval < besterr) {
        besterr = errval;
        bestn = n - 1;
        bestpre = pre - 1;
        if (!besterr) break;
      }
    }
    return bestpre << 18 | bestn << 12 | ((bestn-1)>>1) << 6 | bestn;
  }

  static void initGPIO(int pin, gpio_mode_t mode = GPIO_MODE_OUTPUT, bool pullup = false, bool pulldown = false) {
    if (pin == -1) return;
#ifdef ARDUINO
    uint8_t pm = 0;
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
    io_conf.pin_bit_mask = (uint64_t)1 << pin;
    io_conf.pull_down_en = pulldown ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = pullup ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
#endif
  }

  static void initPWM(int pin, uint32_t pwm_ch, uint8_t duty = 128) {

#ifdef ARDUINO

    ledcSetup(pwm_ch, 12000, 8);
    ledcAttachPin(pin, pwm_ch);
    ledcWrite(pwm_ch, duty);

#else

    static ledc_channel_config_t ledc_channel;
    {
     ledc_channel.gpio_num   = (gpio_num_t)pin;
     ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
     ledc_channel.channel    = (ledc_channel_t)pwm_ch;
     ledc_channel.intr_type  = LEDC_INTR_DISABLE;
     ledc_channel.timer_sel  = (ledc_timer_t)((pwm_ch >> 1) & 3);
     ledc_channel.duty       = duty; // duty;
     ledc_channel.hpoint     = 0;
    };
    ledc_channel_config(&ledc_channel);
    static ledc_timer_config_t ledc_timer;
    {
      ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;     // timer mode
      ledc_timer.duty_resolution = (ledc_timer_bit_t)8; // resolution of PWM duty
      ledc_timer.freq_hz = 12000;                        // frequency of PWM signal
      ledc_timer.timer_num = ledc_channel.timer_sel;    // timer index
    };
    ledc_timer_config(&ledc_timer);

#endif
  }

  static void setPWMDuty(uint32_t pwm_ch, uint8_t duty) {
#ifdef ARDUINO
    ledcWrite(pwm_ch, duty);
#else
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)pwm_ch, duty);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)pwm_ch);
#endif
  }


  template<uint8_t PIN, uint32_t MASK>
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
  struct TPin : public ESP32PIN<PIN, ((PIN<32)?((uint32_t)1 << PIN):((uint32_t)1 << (PIN-32)))> {};

  template<>
  struct TPin<-1> : public ESP32NOPIN {};

//----------------------------------------------------------------------------
  struct FileWrapper : public DataWrapper {
#if defined (ARDUINO) && defined (FS_H)
    fs::File _fp;
  #if defined (_SD_H_)
    fs::FS& _fs = SD;
    void setFS(fs::FS& fs) {
      _fs = fs;
      need_transaction = (&fs == &SD);
    }
  #else
    fs::FS& _fs = SPIFFS;
    void setFS(fs::FS& fs) {
      _fs = fs;
      need_transaction = (&fs != &SPIFFS);
    }
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

#elif defined (CONFIG_IDF_TARGET_ESP32)  // ESP-IDF

    FILE* _fp;
    bool open(const char* path, const char* mode) { return (_fp = fopen(path, mode)); }
    int read(uint8_t *buf, uint32_t len) override { return fread((char*)buf, 1, len, _fp); }
    void skip(int32_t offset) override { seek(offset, SEEK_CUR); }
    bool seek(uint32_t offset) override { return seek(offset, SEEK_SET); }
    bool seek(uint32_t offset, int origin) { return fseek(_fp, offset, origin); }
    void close() override { fclose(_fp); }

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
    void set(Stream* src) { _stream = src; _index = 0; }

    int read(uint8_t *buf, uint32_t len) override { _index += len; return _stream->readBytes((char*)buf, len); }
    void skip(int32_t offset) override { if (0 < offset) { char dummy[offset]; _stream->readBytes(dummy, offset); _index += offset; } }
    bool seek(uint32_t offset) override { if (offset < _index) { return false; } skip(offset - _index); return true; }
    void close() override { }

private:
    Stream* _stream;
    uint32_t _index;

#else  // dummy.

    int read(uint8_t *buf, uint32_t len) override { return 0; }
    void skip(int32_t offset) override { }
    bool seek(uint32_t offset) override { return false; }

#endif

  };
};

#endif
