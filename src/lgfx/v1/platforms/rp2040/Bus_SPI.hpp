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

#include <Arduino.h> 
#include <hardware/spi.h>

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
      uint8_t spi_host = default_spi_host;
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
    static constexpr uint32_t CR0_DSS_8    = 0b0111U;
    static constexpr uint32_t CR0_DSS_16   = 0b1111U;

    __attribute__ ((always_inline)) inline bool is_busy()              const { return ((_spi_regs->sr & SPI_SSPSR_BSY_BITS) != 0);}
    __attribute__ ((always_inline)) inline bool is_rx_fifo_full()      const { return ((_spi_regs->sr & SPI_SSPSR_RFF_BITS)  != 0); }
    __attribute__ ((always_inline)) inline bool is_rx_fifo_not_empty() const { return ((_spi_regs->sr & SPI_SSPSR_RNE_BITS)  != 0); }
    __attribute__ ((always_inline)) inline bool is_tx_fifo_not_full()  const { return ((_spi_regs->sr & SPI_SSPSR_TNF_BITS)  != 0); }
    __attribute__ ((always_inline)) inline bool is_tx_fifo_empty()     const { return ((_spi_regs->sr & SPI_SSPSR_TFE_BITS)  != 0); }
    __attribute__ ((always_inline)) inline void send8(uint8_t data)    const { _spi_regs->dr = data; }
    __attribute__ ((always_inline)) inline void send16(uint16_t data)  const { _spi_regs->dr = data; }
    __attribute__ ((always_inline)) inline uint8_t  recv8()            const { return static_cast<uint8_t>(_spi_regs->dr); }
    __attribute__ ((always_inline)) inline uint16_t recv16()           const { return static_cast<uint16_t>(_spi_regs->dr); }

    __attribute__ ((always_inline)) inline void wait_spi(void)
    {
      if (!_need_wait) return;
      while (is_busy()) {}
      _need_wait = false;
    }

    __attribute__ ((always_inline)) inline void dc_control(bool flg)
    {
      // 送受信完了待ち
      wait_spi();
      if (flg)
      {
        gpio_hi(_cfg.pin_dc);
      }
      else
      {
        gpio_lo(_cfg.pin_dc);
      }
    }

    // FIFOを8bitモードにする。
    __attribute__ ((always_inline)) void set_dss_8() const
    {
      auto temp = _spi_regs->cr0;
      temp &= ~(SPI_SSPCR0_DSS_BITS);
      temp |= CR0_DSS_8;
      _spi_regs->cr0 = temp;
    }

    // FIFOを16bitモードにする。
    __attribute__ ((always_inline)) void set_dss_16() const
    {
      auto temp = _spi_regs->cr0;
      temp &= ~(SPI_SSPCR0_DSS_BITS);
      temp |= CR0_DSS_16;
      _spi_regs->cr0 = temp;
    }
    
    __attribute__ ((always_inline)) inline void clear_rx_fifo()
    {
      // 送受信完了待ち
      wait_spi();
      // FIFO内のデータをすべて読みだす
      while (is_rx_fifo_not_empty())
      {
        static_cast<void>(recv8());
      }
    }

    config_t _cfg;
    FlipBuffer _flip_buffer;
    bool _need_wait;
    uint32_t _last_apb_freq = -1;
    uint32_t _clkdiv_write;
    uint32_t _clkdiv_read;

    static constexpr uint8_t default_spi_host = 1; 
    volatile spi_hw_t * _spi_regs;
    static spi_inst_t * _spi_dev[];
  };

//----------------------------------------------------------------------------
 }
}
