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

#include <stdint.h>
#include <SPI.h>
#include <drivers/SPIMaster.h>
#include <hardware/structs/spi.h>

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
      uint32_t freq_write = 16000000;
      uint32_t freq_read  =  8000000;
      //bool spi_3wire = true;
      //bool use_lock = true;
      int16_t pin_sclk = -1;
      int16_t pin_miso = -1;
      int16_t pin_mosi = -1;
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

    bool writeCommand(uint32_t data, uint_fast8_t bit_length) override;
    void writeData(uint32_t data, uint_fast8_t bit_length) override;
    void writeDataRepeat(uint32_t data, uint_fast8_t bit_length, uint32_t count) override;
    void writePixels(pixelcopy_t* param, uint32_t length) override;
    void writeBytes(const uint8_t* data, uint32_t length, bool dc, bool use_dma) override;

    void initDMA(void) {}
    void flush(void) {}
    void addDMAQueue(const uint8_t* data, uint32_t length) override { writeBytes(data, length, true, true); }
    void execDMAQueue(void) {}
    uint8_t* getDMABuffer(uint32_t length) override { return _flip_buffer.getBuffer(length); }

    void beginRead(void) override;
    void endRead(void) override;
    uint32_t readData(uint_fast8_t bit_length) override;
    bool readBytes(uint8_t* dst, uint32_t length, bool use_dma) override;
    void readPixels(void* dst, pixelcopy_t* param, uint32_t length) override;

  private:
    static constexpr uint32_t SPI_SR_TFE = 0x01;
    static constexpr uint32_t SPI_SR_TNF = 0x02;
    static constexpr uint32_t SPI_SR_RNE = 0x04;
    static constexpr uint32_t SPI_SR_RFF = 0x08;
    static constexpr uint32_t SPI_SR_BSY = 0x10;

    mbed::SPI _spi = { (PinName)11, (PinName)12, (PinName)10 };
    arduino::MbedSPI SPI = { 11, 12, 10 };

    __attribute__ ((always_inline)) inline void dc_h(void) {
      while (*_spi_reg_sr & SPI_SR_BSY) {}
      gpio_hi(_cfg.pin_dc);
    }
    __attribute__ ((always_inline)) inline void dc_l(void) {
      while (*_spi_reg_sr & SPI_SR_BSY) {}
      gpio_lo(_cfg.pin_dc);
    }

    config_t _cfg;
    FlipBuffer _flip_buffer;
    bool _need_wait;
    uint32_t _mask_reg_dc;
    uint32_t _last_apb_freq = -1;
    uint32_t _clkdiv_write;
    uint32_t _clkdiv_read;
    uint32_t _cr0;
    volatile uint32_t* _gpio_reg_dc_h;
    volatile uint32_t* _gpio_reg_dc_l;
    volatile uint32_t* _spi_reg_sr;
    volatile uint32_t* _spi_reg_dr;
    uintptr_t _spibase;
  };

//----------------------------------------------------------------------------
 }
}
