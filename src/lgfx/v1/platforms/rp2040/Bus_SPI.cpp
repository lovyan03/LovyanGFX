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
#if defined (ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)

#include <hardware/structs/spi.h>
#include "Bus_SPI.hpp"
#include "../../misc/pixelcopy.hpp"

//#include <xprintf.h>
//#define DBGPRINT(fmt, ...)  xprintf("%s %d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
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
  }

  bool Bus_SPI::init(void)
  {
    // それぞれのPINが、割り当て可能かを確認
    if (lgfx::spi::init(_cfg.spi_host, _cfg.pin_sclk, _cfg.pin_miso, _cfg.pin_mosi).has_error())
    {
      return false;
    }
    // DCピンを出力に設定
    lgfxPinMode(_cfg.pin_dc, pin_mode_t::output);
    _spi_regs = reinterpret_cast<spi_hw_t *>(_spi_dev[_cfg.spi_host]);
    _need_wait = false;
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
    _need_wait = false;
    DBGPRINT("return %s\n", __func__);
  }

  void Bus_SPI::endTransaction(void)
  {
    dc_control(true);
  }

  void Bus_SPI::beginRead(void)
  {
    wait_spi();
    lgfx::spi::lgfx_spi_set_frequency(_cfg.spi_host,  _cfg.freq_read);
  }

  void Bus_SPI::endRead(void)
  {
    wait_spi();
    lgfx::spi::lgfx_spi_set_frequency(_cfg.spi_host , _cfg.freq_write);
  }

  void Bus_SPI::wait(void)
  {
    wait_spi();
  }

  bool Bus_SPI::busy(void) const
  {
    return _need_wait && is_busy();
  }

  bool Bus_SPI::writeCommand(uint32_t data, uint_fast8_t bit_length)
  {
    DBGPRINT("enter %s\n", __func__);
    auto bytes = bit_length >> 3;
    dc_control(false);
#ifdef USE_XFER_16
    // 送受信データサイズを8ビット(1バイト)単位とする
    set_dss_8();
#endif
    DBGPRINT("byte = %d sr = %08x\n", bytes, _spi_regs->sr);
    _need_wait = true;
    while (bytes > 0)
    {
      while (!is_tx_fifo_not_full()) { }
      send8(static_cast<uint8_t>(data));
      data >>= 8;
      bytes--;
    }
    DBGPRINT("return %s\n", __func__);
    return true;
  }

  void Bus_SPI::writeData(uint32_t data, uint_fast8_t bit_length)
  {
    DBGPRINT("enter %s\n", __func__);
    auto bytes = bit_length >> 3;
    dc_control(true);
#ifdef USE_XFER_16
    // 送受信データサイズを8ビット(1バイト)単位とする
    set_dss_8();
#endif
    _need_wait = true;

    while (bytes > 0)
    {
      while (!is_tx_fifo_not_full()) { }
      send8(static_cast<uint8_t>(data));
      data >>= 8;
      bytes--;
    }
    DBGPRINT("return %s\n", __func__);
  }

  void Bus_SPI::writeDataRepeat(uint32_t data, uint_fast8_t bit_length, uint32_t length)
  {
    DBGPRINT("enter %s\n", __func__);
    size_t bytes = bit_length >> 3;
    auto buf = (uint8_t*)&data;
    dc_control(true);
#ifdef USE_XFER_16
    // 送受信データサイズを8ビット(1バイト)単位とする
    set_dss_8();
#endif
    _need_wait = true;
    do
    {
      size_t b = 0;
      do
      {
        while (!is_tx_fifo_not_full()) { }
        send8(static_cast<uint8_t>(buf[b]));
      }
      while (++b != bytes);
    }
    while (--length);
    DBGPRINT("return %s\n", __func__);
  }

  void Bus_SPI::writePixels(pixelcopy_t* param, uint32_t length)
  {
    DBGPRINT("enter %s\n", __func__);
    const uint8_t dst_bytes = param->dst_bits >> 3;
    uint32_t limit = 12 / dst_bytes;
    uint32_t len;
    _need_wait = true;
    do
    {
      len = ((length - 1) % limit) + 1;
      auto buf = _flip_buffer.getBuffer(len * dst_bytes);
      param->fp_copy(buf, 0, len, param);
      writeBytes(buf, len * dst_bytes, true, true);
    }
    while (length -= len);
    DBGPRINT("return %s\n", __func__);
  }

  void Bus_SPI::writeBytes(const uint8_t* data, uint32_t length, bool dc, [[maybe_unused]]bool use_dma)
  {
    DBGPRINT("enter %s len: %d\n", __func__, length);
    dc_control(dc);
    _need_wait = true;
#ifdef USE_XFER_16
    if ((length & 0x00000001U) == 0b1U)
    {
      // 送信バイト数が奇数の時は、最初に1バイト送信する。
      // 送受信データサイズを8ビット(1バイト)単位とする
      set_dss_8();
      while (!is_tx_fifo_not_full()) {}
      send8(*data++);
      // 送受信完了を待つ
      while (is_busy()) {}
    }
    // 送受信データサイズを16ビット(2バイト)単位とする
    set_dss_16();
    length >>= 1;
    while (length > 0)
    {
      // 2バイト単位で送信する
      uint16_t w;
      w = (*data++) << 8;
      w |= *data++;
      while (!is_tx_fifo_not_full()) {}
      send16(w);
      length--;
    }
#else
    while (length > 0)
    {
      while (!is_tx_fifo_not_full()) {}
      send8(*data++);
      length--;
    }
#endif
    DBGPRINT("return %s\n", __func__);
  }

  uint32_t Bus_SPI::readData(uint_fast8_t bit_length)
  {
    uint32_t res = 0;
    bit_length >>= 3;
    if (bit_length == 0) return res;
    int idx = 0;
    auto tx_bit_length = bit_length;
#ifdef USE_XFER_16
    while (is_busy()) {}
    // 送受信データサイズを8ビット(1バイト)単位とする
    set_dss_8();
#endif
    clear_rx_fifo();
    // この時点で送信FIFOは空になっている
    do
    {
      // 書き込めるだけ送信データを書き込む
      while (is_tx_fifo_not_full() && tx_bit_length > 0) 
      {
        // データを送信（中身は何でもよい）
        send8(0x00);
        tx_bit_length--;
      }
      // 受信FIFOにデータが入るのを待つ
      while (!is_rx_fifo_not_empty()) {}
      res |= recv8() << idx;
      idx += 8;
    }
    while (--bit_length);
    return res;
  }

  bool Bus_SPI::readBytes(uint8_t* dst, uint32_t length, [[maybe_unused]]bool use_dma)
  {
#ifdef USE_XFER_16
    while (is_busy()) {}
    // 送受信データサイズを8ビット(1バイト)単位とする
    set_dss_8();
#endif
    clear_rx_fifo();
    // この時点で送信FIFOは空になっているはず
    auto tx_length = length;
    while (length > 0)
    {
            // 書き込めるだけ送信データを
      while (is_tx_fifo_not_full() && tx_length > 0) 
      {
        // データを送信（中身は何でもよい）
        send8(0x00);
        tx_length--;
      }
      // 受信FIFOにデータが入るのを待つ
      while (!is_rx_fifo_not_empty()) {}
      *dst++ = recv8();
      length--;
    }
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
