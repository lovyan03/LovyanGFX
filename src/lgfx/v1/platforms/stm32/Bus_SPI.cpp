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
#if defined (STM32F2xx) || defined (STM32F4xx) || defined (STM32F7xx)

#include "Bus_SPI.hpp"
#include "../../misc/pixelcopy.hpp"
#include <Arduino.h>
#include <SPI.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------
  static DMA_HandleTypeDef *dmaHal_single;
  
  extern "C" void DMA1_Stream4_IRQHandler();
  void DMA1_Stream4_IRQHandler(void)
  {
    // Call the default end of buffer handler
    HAL_DMA_IRQHandler(dmaHal_single);
  }

  void Bus_SPI::config(const config_t& config)
  {
    _cfg = config;

    if (_cfg.pin_dc >= 0)
    {
      _gpio_reg_dc = get_gpio_out_reg(_cfg.pin_dc);
      _mask_reg_dc_h = 1ul <<  (_cfg.pin_dc & 0x0F);
      _mask_reg_dc_l = 1ul << ((_cfg.pin_dc & 0x0F)+16);
    }
    else
    {
      _gpio_reg_dc = get_gpio_out_reg(0);
      _mask_reg_dc_h = 0;
      _mask_reg_dc_l = 0;
    }
  }

  bool Bus_SPI::init(void)
  {
    lgfx::pinMode(_cfg.pin_dc, pin_mode_t::output);

    dmaHal_single = &_dmaHal;

    //_spiHal.Init.

    _spiHal.Instance = _cfg.spi_port;
    if (_cfg.spi_port == SPI1)
    {
      __HAL_RCC_DMA2_CLK_ENABLE();                           // Enable DMA2 clock
      _dmaHal.Init.Channel = DMA_CHANNEL_3;                   // DMA channel 3 is for SPI1 TX
      _dmaHal.Instance = DMA2_Stream3;
    }
    else if (_cfg.spi_port == SPI2)
    {
      __HAL_RCC_DMA1_CLK_ENABLE();                           // Enable DMA2 clock
      _dmaHal.Init.Channel = DMA_CHANNEL_0;                   // DMA channel 0 is for SPI2 TX
      _dmaHal.Instance = DMA1_Stream4;
    }

    _dmaHal.Init.Mode =  DMA_NORMAL; //DMA_CIRCULAR;   //   // Normal = send buffer once
    _dmaHal.Init.Direction = DMA_MEMORY_TO_PERIPH;          // Copy memory to the peripheral
    _dmaHal.Init.PeriphInc = DMA_PINC_DISABLE;              // Don't increment peripheral address
    _dmaHal.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE; // Peripheral is byte aligned
    _dmaHal.Init.MemInc = DMA_MINC_ENABLE;                  // Increment memory address
    _dmaHal.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;    // Memory is byte aligned

    if (HAL_DMA_Init(&_dmaHal) != HAL_OK)
    {                  // Init DMA with settings
      // for (;;) {};
      // return; // error
    };

    if (_cfg.spi_port == SPI1)
    {
      HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);  // Enable DMA end interrupt handler
    }
    else if (_cfg.spi_port == SPI2)
    {
      HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);  // Enable DMA end interrupt handler
    }

    __HAL_LINKDMA(&_spiHal, hdmatx, _dmaHal);   // Attach DMA engine to SPI peripheral
//*/

    return true;
  }

  void Bus_SPI::release(void)
  {
  }

  void Bus_SPI::beginTransaction(void)
  {
    SPISettings setting(_cfg.freq_write, BitOrder::MSBFIRST, _cfg.spi_mode, true);
    SPI.beginTransaction(setting);
  }

  void Bus_SPI::endTransaction(void)
  {
    dc_control(true);
    SPI.endTransaction();
  }

  void Bus_SPI::beginRead(void)
  {
    SPI.endTransaction();
    SPISettings setting(_cfg.freq_read, BitOrder::MSBFIRST, _cfg.spi_mode, false);
    SPI.beginTransaction(setting);
  }

  void Bus_SPI::endRead(void)
  {
    SPI.endTransaction();
    beginTransaction();
  }

  void Bus_SPI::wait(void)
  {
    wait_spi();
  }

  bool Bus_SPI::busy(void) const
  {
    return _cfg.spi_port->SR  & SPI_SR_BSY;
  }

  bool Bus_SPI::writeCommand(uint32_t data, uint_fast8_t bit_length)
  {
    if (0 == (bit_length >>= 3)) { return true; }
    auto spidr = reinterpret_cast<volatile uint8_t*>(&_cfg.spi_port->DR);
    auto spisr = &_cfg.spi_port->SR;
    dc_control(false);
    *spidr = data;
    while (--bit_length)
    {
      data >>= 8;
      do {} while (!(*spisr & sr_mask));
      *spidr = data;
    }
    return true;
  }

  void Bus_SPI::writeData(uint32_t data, uint_fast8_t bit_length)
  {
    if (0 == (bit_length >>= 3)) return;
    auto spidr = reinterpret_cast<volatile uint8_t*>(&_cfg.spi_port->DR);
    auto spisr = &_cfg.spi_port->SR;
    dc_control(true);
    *spidr = data;
    while (--bit_length)
    {
      data >>= 8;
      do {} while (!(*spisr & sr_mask));
      *spidr = data;
    }
  }

  void Bus_SPI::writeDataRepeat(uint32_t data, uint_fast8_t bit_length, uint32_t length)
  {
/*
    auto spisr    = &_cfg.spi_port->SR;
    auto spidr = &_cfg.spi_port->DR;
    size_t bytes = bit_length >> 3;
    if (bytes == 3)
    {
      uint32_t surplus = length & 3;
      if (surplus)
      {
        length -= surplus;
        do
        {
          writeData(data, bit_length);
        } while (--surplus);
      }
      if (0 == length) return;
      uint32_t buf[3];
      buf[0] = data       | data << 24;
      buf[1] = data >>  8 | data << 16;
      buf[2] = data >> 16 | data <<  8;
      dc_control(true);
      do
      {
        do {} while (!(*spisr & sr_mask));
        *spidr = buf[0];
        do {} while (!(*spisr & sr_mask));
        *spidr = buf[1];
        do {} while (!(*spisr & sr_mask));
        *spidr = buf[2];
      } while (--length);
    }
    else
    {
      do
      {
        writeData(data, bit_length);
      } while (--length);
    }
/*/
    const uint8_t dst_bytes = bit_length >> 3;
    uint32_t limit = (dst_bytes == 3) ? 12 : 16;
    auto dmabuf = _flip_buffer.getBuffer(1024);
    size_t fillpos = 0;
    reinterpret_cast<uint32_t*>(dmabuf)[0] = data;
    fillpos += dst_bytes;
    uint32_t len;
    do
    {
      len = ((length - 1) % limit) + 1;
      if (limit <= 512) limit <<= 1;

      while (fillpos < len * dst_bytes)
      {
        memcpy(&dmabuf[fillpos], dmabuf, fillpos);
        fillpos += fillpos;
      }

      writeBytes(dmabuf, len * dst_bytes, true, true);
    } while (length -= len);
//*/
  }

  void Bus_SPI::writePixels(pixelcopy_t* param, uint32_t length)
  {
    const uint8_t dst_bytes = param->dst_bits >> 3;
    uint32_t limit = (dst_bytes == 3) ? 12 : 16;
    uint32_t len;
    do
    {
      len = ((length - 1) % limit) + 1;
      if (limit <= 512) limit <<= 1;
      auto dmabuf = _flip_buffer.getBuffer(len * dst_bytes);
      param->fp_copy(dmabuf, 0, len, param);
      writeBytes(dmabuf, len * dst_bytes, true, true);
    } while (length -= len);
  }

  void Bus_SPI::writeBytes(const uint8_t* data, uint32_t length, bool dc, bool use_dma)
  {
    dc_control(dc);

    if (length < 16)
    {
      SPI.transfer(const_cast<uint8_t*>(data), length);
      return;
    }
    _spiHal.State = HAL_SPI_STATE_READY;
    while (length > 0xFFFF)
    {
      HAL_SPI_Transmit(&_spiHal, (uint8_t*)data, 0x1000, HAL_MAX_DELAY);
      length -= 0x1000; data += 0x1000;
    }
    HAL_SPI_Transmit_DMA(&_spiHal, (uint8_t*)data, length);
  }

  uint32_t Bus_SPI::readData(uint_fast8_t bit_length)
  {
    uint32_t res = 0;
    bit_length >>= 3;
    if (!bit_length) return res;
    int idx = 0;
    do
    {
      res |= SPI.transfer(0) << idx;
      idx += 8;
    } while (--bit_length);
    return res;
  }

  bool Bus_SPI::readBytes(uint8_t* dst, uint32_t length, bool use_dma)
  {
    do
    {
      dst[0] = SPI.transfer(0);
      ++dst;
    } while (--length);
    return true;
  }

  void Bus_SPI::readPixels(void* dst, pixelcopy_t* param, uint32_t length)
  {
    uint32_t bytes = param->src_bits >> 3;
    uint32_t dstindex = 0;
    uint32_t len = 4;
    uint8_t buf[24];
    param->src_data = buf;
    do {
      if (len > length) len = length;
      readBytes((uint8_t*)buf, len * bytes, true);
      param->src_x = 0;
      dstindex = param->fp_copy(dst, dstindex, dstindex + len, param);
      length -= len;
    } while (length);
  }

//----------------------------------------------------------------------------
 }
}

#endif
