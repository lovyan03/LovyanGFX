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

#include "../../Bus.hpp"
#include "../common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  class Bus_I2C : public IBus
  {
  public:
    struct config_t
    {
      std::uint32_t freq_write = 400000;
      std::uint32_t freq_read = 400000;
      std::int16_t pin_scl = 22;
      std::int16_t pin_sda = 21;
      std::uint8_t i2c_port = 0;      // e.g. ESP32 0=I2C_NUM_0 / 1=I2C_NUM_1
      std::uint8_t i2c_addr = 0x3C;
      std::uint32_t prefix_cmd = 0x00;
      std::uint32_t prefix_data = 0x40;
      std::uint32_t prefix_len = 1;
    };

    const config_t& config(void) const { return _cfg; }

    void config(const config_t& config);

    bus_type_t busType(void) const override { return bus_type_t::bus_i2c; }

    void init(void) override;
    void release(void) override;

    void beginTransaction(void) override;
    void endTransaction(void) override;
    void wait(void) override;
    bool busy(void) const override;

    void flush(void) override {}
    bool writeCommand(std::uint32_t data, std::uint_fast8_t bit_length) override;
    void writeData(std::uint32_t data, std::uint_fast8_t bit_length) override;
    void writeDataRepeat(std::uint32_t data, std::uint_fast8_t bit_length, std::uint32_t count) override;
    void writePixels(pixelcopy_t* param, std::uint32_t length) override;
    void writeBytes(const std::uint8_t* data, std::uint32_t length, bool dc, bool use_dma) override;

    void initDMA(void) {}
    void addDMAQueue(const std::uint8_t* data, std::uint32_t length) override { writeBytes(data, length, true, true); }
    void execDMAQueue(void) {}
    std::uint8_t* getDMABuffer(std::uint32_t length) override { return _flip_buffer.getBuffer(length); }

    void beginRead(void) override;
    void endRead(void) override;
    std::uint32_t readData(std::uint_fast8_t bit_length) override;
    bool readBytes(std::uint8_t* dst, std::uint32_t length, bool use_dma) override;
    void readPixels(void* dst, pixelcopy_t* param, std::uint32_t length) override;

  protected:

    config_t _cfg;
    SimpleBuffer _flip_buffer;
    bool _need_wait;
    enum state_t
    {
      state_none,
      state_write_none,
      state_write_cmd,
      state_write_data,
      state_read,
    };
    state_t _state = state_none;

    void dc_control(bool dc);
  };

//----------------------------------------------------------------------------
 }
}
