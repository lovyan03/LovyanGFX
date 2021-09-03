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

#if defined (ARDUINO)
  #include <Adafruit_ZeroDMA.h>
#endif

#include <vector>
#include <string.h>

#include "../../Bus.hpp"
#include "../common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  class Bus_SPI : public IBus
  {
  public:
    struct config_t
    {
      uint8_t sercom_index = 7;
      int8_t  sercom_clksrc = 0;   // -1=notchange / 0=select GCLK0
      uint32_t sercom_clkfreq = F_CPU;

      uint32_t freq_write = 16000000;
      uint32_t freq_read  =  8000000;
      //bool spi_3wire = true;
      //bool use_lock = true;
      int16_t pin_sclk = samd51::PORT_B | 20;
      int16_t pin_miso = samd51::PORT_B | 18;
      int16_t pin_mosi = samd51::PORT_B | 19;
      int16_t pin_dc   = -1;
      uint8_t spi_mode = 0;
    };


    const config_t& config(void) const { return _cfg; }

    void config(const config_t& config);

    bus_type_t busType(void) const override { return bus_type_t::bus_spi; }

    bool init(void) override;
    void release(void) override;

    void beginTransaction(void) override;
    void endTransaction(void) override;
    void wait(void) override;
    bool busy(void) const override;

    void flush(void) override {}
    bool writeCommand(uint32_t data, uint_fast8_t bit_length) override;
    void writeData(uint32_t data, uint_fast8_t bit_length) override;
    void writeDataRepeat(uint32_t data, uint_fast8_t bit_length, uint32_t count) override;
    void writePixels(pixelcopy_t* param, uint32_t length) override;
    void writeBytes(const uint8_t* data, uint32_t length, bool dc, bool use_dma) override;

    void initDMA(void) {}
    void addDMAQueue(const uint8_t* data, uint32_t length) override { writeBytes(data, length, true, true); }
    void execDMAQueue(void) {}
    uint8_t* getDMABuffer(uint32_t length) override { return _flip_buffer.getBuffer(length); }

    void beginRead(void) override;
    void endRead(void) override;
    uint32_t readData(uint_fast8_t bit_length) override;
    bool readBytes(uint8_t* dst, uint32_t length, bool use_dma) override;
    void readPixels(void* dst, pixelcopy_t* param, uint32_t length) override;

  private:

    uint32_t FreqToClockDiv(uint32_t freq);
    void setFreqDiv(uint32_t div);

    __attribute__ ((always_inline)) inline void set_clock_write(void) { setFreqDiv(_clkdiv_write); }
    __attribute__ ((always_inline)) inline void set_clock_read(void)  { setFreqDiv(_clkdiv_read ); }
    __attribute__ ((always_inline)) inline void wait_spi(void) { if (_need_wait != true) return; auto *intflag = &_sercom->SPI.INTFLAG.bit; while (intflag->TXC == 0); }
    __attribute__ ((always_inline)) inline void dc_control(bool flg)
    {
      auto mask_reg_dc = _mask_reg_dc;
      auto gpio_reg_dc = flg ? _gpio_reg_dc_h : _gpio_reg_dc_l;
      wait_spi();
      *gpio_reg_dc = mask_reg_dc;
    }      

    config_t _cfg;
    FlipBuffer _flip_buffer;
    bool _need_wait = false;
    Sercom* _sercom = nullptr;
    uint32_t _mask_reg_dc;
    uint32_t _last_apb_freq = -1;
    uint32_t _clkdiv_write;
    uint32_t _clkdiv_read;
    volatile uint32_t* _gpio_reg_dc_h;
    volatile uint32_t* _gpio_reg_dc_l;

#if defined (ARDUINO)
    Adafruit_ZeroDMA _dma_adafruit;
    DmacDescriptor* _dma_write_desc = nullptr;
#endif

  };

//----------------------------------------------------------------------------
 }
}
