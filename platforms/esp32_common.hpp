#ifndef LGFX_ESP32_COMMON_HPP_
#define LGFX_ESP32_COMMON_HPP_


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
//  #include <esp32-hal-spi.h>
#else
  void delay(uint32_t ms) { vTaskDelay(ms / portTICK_PERIOD_MS); }

  static constexpr uint32_t MATRIX_DETACH_OUT_SIG = 0x100;
  static constexpr uint32_t MATRIX_DETACH_IN_LOW_PIN = 0x30;
  static constexpr uint32_t MATRIX_DETACH_IN_LOW_HIGH = 0x38;
  void IRAM_ATTR pinMatrixOutAttach(uint8_t pin, uint8_t function, bool invertOut, bool invertEnable) { gpio_matrix_out(pin,              function, invertOut, invertEnable); }
  void IRAM_ATTR pinMatrixOutDetach(uint8_t pin                  , bool invertOut, bool invertEnable) { gpio_matrix_out(pin, MATRIX_DETACH_OUT_SIG, invertOut, invertEnable); }
  void IRAM_ATTR pinMatrixInAttach( uint8_t pin, uint8_t signal           , bool inverted) { gpio_matrix_in(pin, signal, inverted); }
  void IRAM_ATTR pinMatrixInDetach(              uint8_t signal, bool high, bool inverted) { gpio_matrix_in(high?MATRIX_DETACH_IN_LOW_HIGH:MATRIX_DETACH_IN_LOW_PIN, signal, inverted); }

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

    rtc_cpu_freq_config_t conf;
    rtc_clk_cpu_freq_get_config(&conf);

    uint32_t apb_freq = ((conf.freq_mhz >= 80) ? 80 : (conf.source_freq_mhz / conf.div)) * 1000000;

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

  typedef struct {
      uint8_t reg;      /*!< GPIO register offset from DR_REG_IO_MUX_BASE */
      int8_t rtc;       /*!< RTC GPIO number (-1 if not RTC GPIO pin) */
      int8_t adc;       /*!< ADC Channel number (-1 if not ADC pin) */
      int8_t touch;     /*!< Touch Channel number (-1 if not Touch pin) */
  } esp32_gpioMux_t;
  const DRAM_ATTR esp32_gpioMux_t esp32_gpioMux[GPIO_PIN_COUNT]={
      {0x44, 11, 11, 1},
      {0x88, -1, -1, -1},
      {0x40, 12, 12, 2},
      {0x84, -1, -1, -1},
      {0x48, 10, 10, 0},
      {0x6c, -1, -1, -1},
      {0x60, -1, -1, -1},
      {0x64, -1, -1, -1},
      {0x68, -1, -1, -1},
      {0x54, -1, -1, -1},
      {0x58, -1, -1, -1},
      {0x5c, -1, -1, -1},
      {0x34, 15, 15, 5},
      {0x38, 14, 14, 4},
      {0x30, 16, 16, 6},
      {0x3c, 13, 13, 3},
      {0x4c, -1, -1, -1},
      {0x50, -1, -1, -1},
      {0x70, -1, -1, -1},
      {0x74, -1, -1, -1},
      {0x78, -1, -1, -1},
      {0x7c, -1, -1, -1},
      {0x80, -1, -1, -1},
      {0x8c, -1, -1, -1},
      {0, -1, -1, -1},
      {0x24, 6, 18, -1}, //DAC1
      {0x28, 7, 19, -1}, //DAC2
      {0x2c, 17, 17, 7},
      {0, -1, -1, -1},
      {0, -1, -1, -1},
      {0, -1, -1, -1},
      {0, -1, -1, -1},
      {0x1c, 9, 4, 9},
      {0x20, 8, 5, 8},
      {0x14, 4, 6, -1},
      {0x18, 5, 7, -1},
      {0x04, 0, 0, -1},
      {0x08, 1, 1, -1},
      {0x0c, 2, 2, -1},
      {0x10, 3, 3, -1}
  };

#endif

namespace lgfx {

  template<uint8_t PIN, uint32_t MASK>
  struct ESP32PIN {
    static void init() {
      volatile uint32_t* rtc_reg = (volatile uint32_t*)( rtc_gpio_desc[PIN].reg );
      if (rtc_reg) {
        *rtc_reg = *rtc_reg & ~(rtc_gpio_desc[PIN].mux);
        *rtc_reg = *rtc_reg & ~(rtc_gpio_desc[PIN].pullup | rtc_gpio_desc[PIN].pulldown);
      }
      *(volatile uint32_t*)(DR_REG_IO_MUX_BASE + esp32_gpioMux[PIN].reg)
        = ((uint32_t)2 << FUN_DRV_S)
        | ((uint32_t)2 << MCU_SEL_S);
      GPIO.pin[PIN].val = 0;
    }

    inline static void enableInput() {
      *(volatile uint32_t*)(DR_REG_IO_MUX_BASE + esp32_gpioMux[PIN].reg) |= FUN_IE;
    }

    inline static void enableOutput() __attribute__ ((always_inline)) { if (PIN < 32) { GPIO.enable_w1ts = MASK; } else { GPIO.enable1_w1ts.val = MASK; } }
    inline static void disableOutput() __attribute__ ((always_inline)) { if (PIN < 32) { GPIO.enable_w1tc = MASK; } else { GPIO.enable1_w1tc.val = MASK; } }
    inline static void hi() __attribute__ ((always_inline)) { if (PIN < 32) GPIO.out_w1ts = MASK; else GPIO.out1_w1ts.val = MASK; }
    inline static void lo() __attribute__ ((always_inline)) { if (PIN < 32) GPIO.out_w1tc = MASK; else GPIO.out1_w1tc.val = MASK; }
    inline static bool isset() __attribute__ ((always_inline)) { if (PIN < 32) return GPIO.out & MASK; else return GPIO.out1.val & MASK; }
  };
  struct ESP32NOPIN {
  public:
    inline static void init() __attribute__ ((always_inline)) {}
    inline static void enableInput() __attribute__ ((always_inline)) {}
    inline static void enableOutput() __attribute__ ((always_inline)) {}
    inline static void disableOutput() __attribute__ ((always_inline)) {}
    inline static void hi() __attribute__ ((always_inline)) {}
    inline static void lo() __attribute__ ((always_inline)) {}
    inline static bool isset() __attribute__ ((always_inline)) { return false; }
  };

  template<int PIN>
  struct TPin : public ESP32PIN<PIN, ((PIN<32)?((uint32_t)1 << PIN):((uint32_t)1 << (PIN-32)))> {};

  template<>
  struct TPin<-1> : public ESP32NOPIN {};


};

#endif
