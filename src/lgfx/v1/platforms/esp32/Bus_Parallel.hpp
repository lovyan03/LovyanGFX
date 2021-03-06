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

#include <vector>
#include <cstring>

#if __has_include(<esp32/rom/lldesc.h>)
 #include <esp32/rom/lldesc.h>
#else
 #include <rom/lldesc.h>
#endif

#if __has_include(<freertos/FreeRTOS.h>)
 #include <freertos/FreeRTOS.h>
#endif

#include <driver/i2s.h>

#include "../../Bus.hpp"
#include "../common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  class Bus_Parallel : public IBus
  {
  public:
    struct config_t
    {
      i2s_port_t i2s_port = I2S_NUM_0;

      // max 20MHz , 16MHz , 13.3MHz , 11.43MHz , 10MHz , 8.9MHz  and more ...
      std::uint32_t freq_write = 16000000;
      std::int8_t pin_d0 = -1;
      std::int8_t pin_d1 = -1;
      std::int8_t pin_d2 = -1;
      std::int8_t pin_d3 = -1;
      std::int8_t pin_d4 = -1;
      std::int8_t pin_d5 = -1;
      std::int8_t pin_d6 = -1;
      std::int8_t pin_d7 = -1;
      std::int8_t pin_wr = -1;
      std::int8_t pin_rd = -1;
      std::int8_t pin_rs = -1;  // D/C
      std::uint8_t spi_mode = 0;
      std::uint8_t dma_channel = 0;
    };


    const config_t& config(void) const { return _cfg; }
    void config(const config_t& config);

    void init(void) override;
    void release(void) override;

    void beginTransaction(void) override;
    void endTransaction(void) override;
    void wait(void) override;
    bool busy(void) const override;

    void writeCommand(std::uint32_t data, std::uint_fast8_t bit_length) override;
    void writeData(std::uint32_t data, std::uint_fast8_t bit_length) override;
    void writeDataRepeat(std::uint32_t data, std::uint_fast8_t bit_length, std::uint32_t count) override;
    void writePixels(pixelcopy_t* param, std::uint32_t length) override;
    void writeBytes(const std::uint8_t* data, std::uint32_t length, bool use_dma) override;

    void initDMA(void) override {}
    void addDMAQueue(const std::uint8_t* data, std::uint32_t length) override { writeBytes(data, length, true); }
    void execDMAQueue(void) override;
    std::uint8_t* getDMABuffer(std::uint32_t length) override { return _flip_buffer.getBuffer(length); }

    void beginRead(void) override;
    void endRead(void) override;
    std::uint32_t readData(std::uint_fast8_t bit_length) override;
    void readBytes(std::uint8_t* dst, std::uint32_t length, bool use_dma) override;
    void readPixels(void* dst, pixelcopy_t* param, std::uint32_t length) override;

  private:

    config_t _cfg;
    FlipBuffer _flip_buffer;

    void wait_i2s(void);

    void _init_pin(void);
    void _alloc_dmadesc(size_t len);
    std::uint_fast8_t _reg_to_value(std::uint32_t raw_value);

    std::uint32_t _last_freq_apb;
    std::uint32_t _last_freq_write;
    std::uint32_t _clkdiv_write;
    std::uint32_t _dmadesc_len;
    lldesc_t* _dmadesc;

  };

//----------------------------------------------------------------------------
 }
}
