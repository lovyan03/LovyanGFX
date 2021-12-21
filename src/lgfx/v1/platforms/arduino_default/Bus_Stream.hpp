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
#include <Stream.h>

#include "../../Bus.hpp"
#include "../common.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  class Bus_Stream : public IBus
  {
  public:
    struct config_t
    {
      Stream* stream = nullptr;
    };

    const config_t& config(void) const { return _cfg; }

    void config(const config_t& config);

    bus_type_t busType(void) const override { return bus_type_t::bus_stream; }

    bool init(void) override {return true;}
    void release(void) override {}

    void beginTransaction(void) override {}
    void endTransaction(void) override {}
    void wait(void) override {}
    bool busy(void) const override {return false;}

    void flush(void) override {}
    bool writeCommand(uint32_t data, uint_fast8_t bit_length) override {return false;}
    void writeData(uint32_t data, uint_fast8_t bit_length) override;
    void writeDataRepeat(uint32_t data, uint_fast8_t bit_length, uint32_t count) override;
    void writePixels(pixelcopy_t* param, uint32_t length) override;
    void writeBytes(const uint8_t* data, uint32_t length, bool dc, bool use_dma) override { writeBytes(data, length); }

    void initDMA(void) override {}
    void addDMAQueue(const uint8_t* data, uint32_t length) override { writeBytes(data, length); }
    void execDMAQueue(void) {}
    uint8_t* getDMABuffer(uint32_t length) override { return _flip_buffer.getBuffer(length); }

    void beginRead(void) override {}
    void endRead(void) override {}
    uint32_t readData(uint_fast8_t bit_length) override;
    bool readBytes(uint8_t* dst, uint32_t length, bool use_dma) override;
    void readPixels(void* dst, pixelcopy_t* param, uint32_t length) override;

  private:
    void writeBytes(const uint8_t* data, uint32_t length);
    config_t _cfg;
    SimpleBuffer _flip_buffer;

  };

//----------------------------------------------------------------------------
 }
}
