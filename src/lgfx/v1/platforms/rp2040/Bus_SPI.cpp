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

Porting for RP2040:
 [yasuhirok](https://github.com/yasuhirok-git)
/----------------------------------------------------------------------------*/
#if defined (ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)

#include <hardware/structs/spi.h>
#include "Bus_SPI.hpp"
#include "../../misc/pixelcopy.hpp"

//#include <xprintf.h>
//#define DBGPRINT(fmt, ...)  xprintf("%s %d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
// static char dbg_buf[256];
// #define DBGPRINT(fmt, ...) snprintf(dbg_buf, 256, "%s %d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__); Serial.print(dbg_buf);
#define DBGPRINT(fmt, ...)

// 16bit FIFOを使うとき、#defineする。
#define USE_XFER_16

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  void Bus_SPI::config(const config_t& config)
  {
    _cfg = config;

    _gpio_dc_mask = 0;
    if (_cfg.pin_dc >= 0)
    {
      _gpio_dc_mask = 1 << _cfg.pin_dc;
    }
    _clkdiv_write = spi::FreqToClockDiv(_cfg.freq_write) << SPI_SSPCR0_SCR_LSB;
    _clkdiv_read  = spi::FreqToClockDiv(_cfg.freq_read)  << SPI_SSPCR0_SCR_LSB;
  }

  bool Bus_SPI::init(void)
  {
    // それぞれのPINが、割り当て可能かを確認
    if (lgfx::spi::init(_cfg.spi_host, _cfg.pin_sclk, _cfg.pin_miso, _cfg.pin_mosi).has_error())
    {
      return false;
    }

    uint32_t temp = _spi_regs->cr0 & ~(SPI_SSPCR0_SCR_BITS | SPI_SSPCR0_DSS_BITS);
    _clkdiv_write |= temp;
    _clkdiv_read  |= temp;

    // DCピンを出力に設定
    lgfxPinMode(_cfg.pin_dc, pin_mode_t::output);
    _spi_regs = reinterpret_cast<spi_hw_t *>(_spi_dev[_cfg.spi_host]);

    int dma_ch = dma_claim_unused_channel(true);
    _dma_ch = dma_ch;
    if (dma_ch >= 0)
    {
      _dma_tx_cfg = dma_channel_get_default_config(dma_ch);
      channel_config_set_dreq(&_dma_tx_cfg, _cfg.spi_host ? DREQ_SPI1_TX : DREQ_SPI0_TX);
      channel_config_set_bswap(&_dma_tx_cfg, true);
    }
    return true;
  }

  void Bus_SPI::release(void)
  {
    lgfx::spi::release(_cfg.spi_host);
  }

  void Bus_SPI::beginTransaction(void)
  {
    DBGPRINT("enter %s\n", __func__);
    dc_control(true);
    lgfx::spi::beginTransaction(_cfg.spi_host, _cfg.freq_write, _cfg.spi_mode);
    _sspcr0_mask_8bit  = _clkdiv_write | CR0_DSS_8;
    _sspcr0_mask_16bit = _clkdiv_write | CR0_DSS_16;
    DBGPRINT("return %s\n", __func__);
  }

  void Bus_SPI::endTransaction(void)
  {
    dc_control(true);
  }

  void Bus_SPI::beginRead(void)
  {
    DBGPRINT("enter %s\n", __func__);
    _sspcr0_mask_8bit  = _clkdiv_read | CR0_DSS_8;
    _sspcr0_mask_16bit = _clkdiv_read | CR0_DSS_16;
    dc_h();
  }

  void Bus_SPI::endRead(void)
  {
    _sspcr0_mask_8bit  = _clkdiv_write | CR0_DSS_8;
    _sspcr0_mask_16bit = _clkdiv_write | CR0_DSS_16;
    wait_spi();
  }

  void Bus_SPI::wait(void)
  {
    wait_spi();
  }

  bool Bus_SPI::busy(void) const
  {
    return is_busy();
  }

  bool Bus_SPI::writeCommand(uint32_t data, uint_fast8_t bit_length)
  {
    DBGPRINT("enter %s\n", __func__);
    auto bytes = bit_length >> 3;
    auto &reg_sr = _spi_regs->sr;
    auto &reg_cr0 = _spi_regs->cr0;
    auto &reg_dr = _spi_regs->dr;
    auto &reg_gpio_dc = sio_hw->gpio_clr;
    auto cr0_mask = _sspcr0_mask_8bit;
    auto dc_mask = _gpio_dc_mask;
    while (reg_sr & SPI_SSPSR_BSY_BITS) { }
    reg_gpio_dc = dc_mask;
    reg_cr0 = cr0_mask;
    do
    {
      reg_dr = data;
      data >>= 8;
    } while (--bytes);
    return true;
  }

  void Bus_SPI::writeData(uint32_t data, uint_fast8_t bit_length)
  {
    DBGPRINT("enter %s\n", __func__);
    auto bytes = bit_length >> 3;
    auto &reg_sr = _spi_regs->sr;
    auto &reg_cr0 = _spi_regs->cr0;
    auto &reg_dr = _spi_regs->dr;
    auto &reg_gpio_dc = sio_hw->gpio_set;
    auto cr0_mask = _sspcr0_mask_8bit;
    auto dc_mask = _gpio_dc_mask;
    while (reg_sr & SPI_SSPSR_BSY_BITS) { }
    reg_gpio_dc = dc_mask;
    reg_cr0 = cr0_mask;
    do
    {
      reg_dr = data;
      data >>= 8;
    } while (--bytes);
    DBGPRINT("return %s\n", __func__);
  }

  void Bus_SPI::writeDataRepeat(uint32_t data, uint_fast8_t bit_length, uint32_t length)
  {
    DBGPRINT("enter %s\n", __func__);
    size_t bytes = bit_length >> 3;
    auto &reg_sr = _spi_regs->sr;
    auto &reg_cr0 = _spi_regs->cr0;
    auto &reg_dr = _spi_regs->dr;
    auto &reg_gpio_dc = sio_hw->gpio_set;
    auto cr0_mask = _sspcr0_mask_8bit;
    auto dc_mask = _gpio_dc_mask;

    if (bytes == 2)
    {
      cr0_mask = _sspcr0_mask_16bit;
      data = ((data >> 8) & 0xFF) | data << 8;
      while (reg_sr & SPI_SSPSR_BSY_BITS) { }
      reg_cr0 = cr0_mask;
      reg_gpio_dc = dc_mask;
      reg_dr = data;
      if (--length)
      do
      {
        while (!(reg_sr& SPI_SSPSR_TNF_BITS)) { }
        reg_dr = data;
      } while (--length);
      return;
    }

    size_t b = 0;
    auto buf = (uint8_t*)&data;
    while (reg_sr & SPI_SSPSR_BSY_BITS) { }
    reg_cr0 = cr0_mask;
    reg_gpio_dc = dc_mask;
    do
    {
      reg_dr = buf[b];
    } while (++b != bytes);
    if (--length)
    {
      do
      {
        b = 0;
        do
        {
          while (!(reg_sr& SPI_SSPSR_TNF_BITS)) { }
          reg_dr = buf[b];
        } while (++b != bytes);
      } while (--length);
    }
    DBGPRINT("return %s\n", __func__);
  }

  void Bus_SPI::writePixels(pixelcopy_t* param, uint32_t length)
  {
    DBGPRINT("enter %s\n", __func__);
    const uint8_t dst_bytes = param->dst_bits >> 3;

    if (_dma_ch >= 0)
    {
      uint32_t limit = (dst_bytes == 2) ? 16 : 12;
      uint32_t len;
      do
      {
        if (limit <= 256) limit <<= 1;
        len = (limit <= length) ? limit : length;
        auto dmabuf = _flip_buffer.getBuffer(limit * dst_bytes);
        param->fp_copy(dmabuf, 0, len, param);
        writeBytes(dmabuf, len * dst_bytes, true, true);
      } while (length -= len);
      return;
    }

    uint32_t limit = 12 / dst_bytes;
    uint32_t len;
    do
    {
      len = ((length - 1) % limit) + 1;
      auto buf = _flip_buffer.getBuffer(len * dst_bytes);
      param->fp_copy(buf, 0, len, param);
      writeBytes(buf, len * dst_bytes, true, true);
    } while (length -= len);
    DBGPRINT("return %s\n", __func__);
  }

  void Bus_SPI::writeBytes(const uint8_t* data, uint32_t length, bool dc, bool use_dma)
  {
    DBGPRINT("enter %s len: %d\n", __func__, length);
    auto &reg_dr = _spi_regs->dr;

    if (use_dma && _dma_ch >= 0)
    {
      dma_channel_transfer_size dma_size = DMA_SIZE_8;
      uint32_t cr0_mask = _sspcr0_mask_8bit;
      if (!(length & 1))
      {
        dma_size = DMA_SIZE_16;
        cr0_mask = _sspcr0_mask_16bit;
        length >>= 1;
      }
      channel_config_set_transfer_data_size(&_dma_tx_cfg, dma_size);
      auto &reg_cr0 = _spi_regs->cr0;
      dc_control(dc);
      reg_cr0 = cr0_mask;

      dma_channel_configure(_dma_ch, &_dma_tx_cfg, &reg_dr, data, length, true);
      return;
    }

    dc_control(dc);
    if (length & 1)
    { // 送信バイト数が奇数の時は、最初に1バイト送信する。
      set_dss_8();
      reg_dr = *data++;
      if (--length == 0) return;
      wait_spi();      // 送受信完了を待つ
    }
    length >>= 1;
    // 送受信データサイズを16ビット(2バイト)単位とする
    set_dss_16();
    do
    {
      // 2バイト単位で送信する
      uint_fast16_t w = (*data++) << 8;
      w |= *data++;
      while (!is_tx_fifo_not_full()) {}
      reg_dr = w;
    } while (--length);
    DBGPRINT("return %s\n", __func__);
  }

  uint32_t Bus_SPI::readData(uint_fast8_t bit_length)
  {
    DBGPRINT("enter %s\n", __func__);
    uint32_t res = 0;
    bit_length >>= 3;
    if (bit_length == 0) return res;
    int idx = 0;
    auto tx_bit_length = bit_length;
    clear_rx_fifo();
    set_dss_8();
    do
    {
      // 書き込めるだけ送信データを書き込む
      while (tx_bit_length && is_tx_fifo_not_full())
      {
        // データを送信（中身は何でもよい）
        _spi_regs->dr = 0x00;
        tx_bit_length--;
      }
      // 受信FIFOにデータが入るのを待つ
      while (!is_rx_fifo_not_empty()) {}
      res |= (_spi_regs->dr & 0xFF) << idx;
      idx += 8;
    }
    while (--bit_length);
    DBGPRINT("return %s\n", __func__);
    return res;
  }

  bool Bus_SPI::readBytes(uint8_t* dst, uint32_t length, [[maybe_unused]]bool use_dma)
  {
    DBGPRINT("enter %s\n", __func__);
    clear_rx_fifo();
    set_dss_8();
    auto tx_length = length;
    do
    {
      DBGPRINT("tx_length %d : length %d\n", tx_length, length);
      // 書き込めるだけ送信データを書き込む
      while (tx_length && is_tx_fifo_not_full())
      {
        // データを送信（中身は何でもよい）
        _spi_regs->dr = 0x00;
        tx_length--;
      }
      // 受信FIFOにデータが入るのを待つ
      while (!is_rx_fifo_not_empty()) {}
      *dst++ = _spi_regs->dr;
    } while (--length);
    DBGPRINT("return %s\n", __func__);
    return true;
  }

  void Bus_SPI::readPixels(void* dst, pixelcopy_t* param, uint32_t length)
  {
    uint32_t bytes = param->src_bits >> 3;
    uint32_t dstindex = 0;
    uint32_t len = 4;
    uint8_t buf[24];
    param->src_data = buf;
    do
    {
      if (len > length) len = length;
      readBytes((uint8_t*)buf, len * bytes, true);
      param->src_x = 0;
      dstindex = param->fp_copy(dst, dstindex, dstindex + len, param);
      length -= len;
    }
    while (length > 0);
  }

  spi_inst_t * Bus_SPI::_spi_dev[] = {
    reinterpret_cast<spi_inst_t *>(SPI0_BASE),
    reinterpret_cast<spi_inst_t *>(SPI1_BASE),
  };

//----------------------------------------------------------------------------
 }
}

#endif
