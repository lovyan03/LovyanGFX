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
      std::uint32_t freq_write = 16000000;
      std::uint32_t freq_read  =  8000000;
      //bool spi_3wire = true;
      //bool use_lock = true;
      std::int16_t pin_sclk = -1;
      std::int16_t pin_miso = -1;
      std::int16_t pin_mosi = -1;
      std::int16_t pin_dc   = -1;
      std::uint8_t spi_mode = 0;
      SPI_TypeDef *spi_port = SPI1;
      DMA_TypeDef *dma_port = DMA1;
    };

    const config_t& config(void) const { return _cfg; }

    void config(const config_t& config);

    bus_type_t busType(void) const override { return bus_type_t::bus_spi; }

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
    void writeBytes(const std::uint8_t* data, std::uint32_t length, bool dc, bool use_dma) override;

    void initDMA(void) {}
    void addDMAQueue(const std::uint8_t* data, std::uint32_t length) override { writeBytes(data, length, true, true); }
    void execDMAQueue(void) {}
    std::uint8_t* getDMABuffer(std::uint32_t length) override { return _flip_buffer.getBuffer(length); }

    void beginRead(void) override;
    void endRead(void) override;
    std::uint32_t readData(std::uint_fast8_t bit_length) override;
    void readBytes(std::uint8_t* dst, std::uint32_t length, bool use_dma) override;
    void readPixels(void* dst, pixelcopy_t* param, std::uint32_t length) override;

  private:

#if defined(STM32H7xx) || defined(STM32MP1xx)
      static constexpr std::uint32_t sr_mask = SPI_SR_TXP;
#else
      static constexpr std::uint32_t sr_mask = SPI_SR_TXE;
#endif

    __attribute__ ((always_inline)) inline void wait_spi(void)
    {
      volatile std::uint32_t *spisr = &_cfg.spi_port->SR;
      do {} while (*spisr & SPI_SR_BSY);
    }

    __attribute__ ((always_inline)) inline void dc_control(bool flg)
    {
      auto gpio_reg_dc = _gpio_reg_dc;
      auto mask_reg_dc = flg ? _mask_reg_dc_h : _mask_reg_dc_l;
      volatile std::uint32_t *spisr = &_cfg.spi_port->SR;
      do {} while (*spisr & SPI_SR_BSY);
      *gpio_reg_dc = mask_reg_dc;
    }
/*
    __attribute__ ((always_inline)) inline void dc_h(void)
    {
      auto gpio_reg_dc = _gpio_reg_dc;
      auto mask_reg_dc = _mask_reg_dc_h;
      volatile std::uint32_t *spisr = &_cfg.spi_port->SR;
      do {} while (*spisr & SPI_SR_BSY);
      *gpio_reg_dc = mask_reg_dc;
    }

    __attribute__ ((always_inline)) inline void dc_l(void)
    {
      auto gpio_reg_dc = _gpio_reg_dc;
      auto mask_reg_dc = _mask_reg_dc_l;
      volatile std::uint32_t *spisr = &_cfg.spi_port->SR;
      do {} while (*spisr & SPI_SR_BSY);
      *gpio_reg_dc = mask_reg_dc;
    }
//*/
    config_t _cfg;
    FlipBuffer _flip_buffer;
    bool _need_wait;
    std::uint32_t _mask_reg_dc_h;
    std::uint32_t _mask_reg_dc_l;
    std::uint32_t _last_apb_freq = -1;
    std::uint32_t _clkdiv_write;
    std::uint32_t _clkdiv_read;
    volatile std::uint32_t* _gpio_reg_dc;
    DMA_HandleTypeDef _dmaHal;
    SPI_HandleTypeDef _spiHal;
  };

//----------------------------------------------------------------------------
 }
}
