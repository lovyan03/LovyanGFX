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

#include <cstdint>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct pixelcopy_t;

  struct IBus
  {
    virtual ~IBus(void) = default;

    virtual void init(void) = 0;
    virtual void release(void) = 0;
    virtual void beginTransaction(void) = 0;
    virtual void endTransaction(void) = 0;
    virtual void wait(void) = 0;
    virtual bool busy(void) const = 0;

    virtual void initDMA(void) = 0;
    virtual void addDMAQueue(const std::uint8_t* data, std::uint32_t length) = 0; // { writeBytes(data, length, true); }
    virtual void execDMAQueue(void) = 0;
    virtual std::uint8_t* getDMABuffer(std::uint32_t length) = 0;

    virtual void writeCommand(std::uint32_t data, std::uint_fast8_t bit_length) = 0;
    virtual void writeData(std::uint32_t data, std::uint_fast8_t bit_length) = 0;
    virtual void writeDataRepeat(std::uint32_t data, std::uint_fast8_t bit_length, std::uint32_t count) = 0;
    virtual void writePixels(pixelcopy_t* pc, std::uint32_t length) = 0;
    virtual void writeBytes(const std::uint8_t* data, std::uint32_t length, bool use_dma = false) = 0;

    virtual void beginRead(void) = 0;
    virtual void endRead(void) = 0;
    virtual std::uint32_t readData(std::uint_fast8_t bit_length) = 0;
    virtual void readBytes(std::uint8_t* dst, std::uint32_t length, bool use_dma = false) = 0;
    virtual void readPixels(void* dst, pixelcopy_t* pc, std::uint32_t length) = 0;
  };

  struct Bus_NULL : public IBus
  {
    void init(void) override {}
    void release(void) override {}
    void beginTransaction(void) override {}
    void endTransaction(void) override {}
    void wait(void) override {}
    bool busy(void) const override { return false; }

    void initDMA(void) override {}
    void addDMAQueue(const std::uint8_t* data, std::uint32_t length) override {}
    void execDMAQueue(void) override {}
    std::uint8_t* getDMABuffer(std::uint32_t length) override { return nullptr; }

    void writeCommand(std::uint32_t data, std::uint_fast8_t bit_length) override {}
    void writeData(std::uint32_t data, std::uint_fast8_t bit_length) override {}
    void writeDataRepeat(std::uint32_t data, std::uint_fast8_t bit_length, std::uint32_t count) override {}
    void writePixels(pixelcopy_t* pc, std::uint32_t length) override {}
    void writeBytes(const std::uint8_t* data, std::uint32_t length, bool use_dma = false) override {}

    void beginRead(void) override {}
    void endRead(void) override {}
    std::uint32_t readData(std::uint_fast8_t bit_length) override { return 0; }
    void readBytes(std::uint8_t* dst, std::uint32_t length, bool use_dma = false) override {}
    void readPixels(void* dst, pixelcopy_t* pc, std::uint32_t length) override {}
  };

//----------------------------------------------------------------------------
 }
}
