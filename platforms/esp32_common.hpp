#ifndef LGFX_ESP32_COMMON_HPP_
#define LGFX_ESP32_COMMON_HPP_

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/periph_ctrl.h>
#include <driver/rtc_io.h>
#include <driver/spi_common.h>
#include <driver/spi_master.h>
#include <soc/dport_reg.h>
#include <soc/periph_defs.h>
#include <soc/rtc.h>
#include <soc/spi_reg.h>
#include <soc/spi_struct.h>

#ifdef ARDUINO
  #include <esp32-hal-cpu.h>
#else
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

  uint32_t spiFrequencyToClockDiv(uint32_t freq) {
    typedef union {
        uint32_t value;
        struct {
                uint32_t clkcnt_l:       6;                     /*it must be equal to spi_clkcnt_N.*/
                uint32_t clkcnt_h:       6;                     /*it must be floor((spi_clkcnt_N+1)/2-1).*/
                uint32_t clkcnt_n:       6;                     /*it is the divider of spi_clk. So spi_clk frequency is system/(spi_clkdiv_pre+1)/(spi_clkcnt_N+1)*/
                uint32_t clkdiv_pre:    13;                     /*it is pre-divider of spi_clk.*/
                uint32_t clk_equ_sysclk: 1;                     /*1: spi_clk is eqaul to system 0: spi_clk is divided from system clock.*/
        };
    } spiClk_t;

    uint32_t apb_freq = getApbFrequency();

    if(freq >= apb_freq) {
      return SPI_CLK_EQU_SYSCLK;
    }

    const spiClk_t minFreqReg = { 0x7FFFF000 };
    uint32_t minFreq = apb_freq / ((minFreqReg.clkdiv_pre + 1) * (minFreqReg.clkcnt_n + 1));

    if(freq < minFreq) {
        return minFreqReg.value;
    }

    uint8_t calN = 1;
    spiClk_t bestReg = { 0 };
    int32_t bestFreq = 0;

    while(calN <= 0x3F) {
        spiClk_t reg = { 0 };
        int32_t calFreq;
        int32_t calPre;
        int8_t calPreVari = -2;

        reg.clkcnt_n = calN;

        while(calPreVari++ <= 1) {
            calPre = (((apb_freq / (reg.clkcnt_n + 1)) / freq) - 1) + calPreVari;
            if(calPre > 0x1FFF) {
                reg.clkdiv_pre = 0x1FFF;
            } else if(calPre <= 0) {
                reg.clkdiv_pre = 0;
            } else {
                reg.clkdiv_pre = calPre;
            }
            reg.clkcnt_l = ((reg.clkcnt_n + 1) / 2);
            calFreq = apb_freq / ((reg.clkdiv_pre + 1) * (reg.clkcnt_n + 1));
            if(calFreq == (int32_t) freq) {
                memcpy(&bestReg, &reg, sizeof(bestReg));
                break;
            } else if(calFreq < (int32_t) freq) {
                if(abs(freq - calFreq) < abs(freq - bestFreq)) {
                    bestFreq = calFreq;
                    memcpy(&bestReg, &reg, sizeof(bestReg));
                }
            }
        }
        if(calFreq == (int32_t) freq) {
            break;
        }
        calN++;
    }
    return bestReg.value;
  }
#endif


namespace lgfx
{
  static void gpioInit(gpio_num_t pin, gpio_mode_t mode = GPIO_MODE_OUTPUT) {
    if (pin == -1) return;
    if (rtc_gpio_is_valid_gpio(pin)) rtc_gpio_deinit(pin);
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = mode;
    io_conf.pin_bit_mask = (uint64_t)1 << pin;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
  }

  template<uint8_t PIN, uint32_t MASK>
  struct ESP32PIN {
    __attribute__ ((always_inline)) inline static void init(gpio_mode_t mode = GPIO_MODE_OUTPUT) { gpioInit((gpio_num_t)PIN, mode); }
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

  struct FileWrapper {
    bool need_transaction = true;

    uint16_t read16(void) {
      uint16_t result;
      read(reinterpret_cast<uint8_t*>(&result), 2);
      return result;
    }

    uint32_t read32(void) {
      uint32_t result;
      read(reinterpret_cast<uint8_t*>(&result), 4);
      return result;
    }

    __attribute__ ((always_inline)) inline uint16_t read16swap(void) {
      return __builtin_bswap16(read16());
    }

    __attribute__ ((always_inline)) inline uint32_t read32swap(void) {
      return __builtin_bswap32(read32());
    }

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
    bool seek(uint32_t offset) { return seek(offset, SeekSet); }
    bool seek(uint32_t offset, SeekMode mode) { return _fp.seek(offset, mode); }
    void skip(uint32_t offset) { seek(offset, SeekCur); }
    void read(uint8_t *buf, uint32_t len) { _fp.read(buf, len); }
    void close() { _fp.close(); }

#elif defined (CONFIG_IDF_TARGET_ESP32)  // ESP-IDF

    FILE* _fp;
    bool open(const char* path, const char* mode) { return (_fp = fopen(path, mode)); }
    bool seek(uint32_t offset) { return seek(offset, SEEK_SET); }
    bool seek(uint32_t offset, int origin) { return fseek(_fp, offset, origin); }
    void skip(uint32_t offset) { seek(offset, SEEK_CUR); }
    void read(uint8_t *buf, uint32_t len) { fread((char*)buf, 1, len, _fp); }
    void close() { fclose(_fp); }

#else  // dummy.

    bool open(const char* path, const char* mode) { return false; }
    bool seek(uint32_t offset) { return false; }
    bool seek(uint32_t offset, int origin) { return false; }
    void skip(uint32_t offset) { }
    void read(uint8_t *buf, uint32_t len) { }
    void close() { }

#endif

  };

};

#endif
