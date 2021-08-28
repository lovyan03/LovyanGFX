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
#if defined (ESP_PLATFORM)
#include <sdkconfig.h>
#if defined (CONFIG_IDF_TARGET_ESP32S2)

#include "Bus_Parallel16.hpp"
#include "../../misc/pixelcopy.hpp"

#include <soc/dport_reg.h>
#include <soc/i2s_struct.h>
#include <esp_log.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

#ifndef I2S_CLKA_ENA
#define I2S_CLKA_ENA  (BIT(22)) // clk_sel = 2
#endif

  static constexpr size_t CACHE_THRESH = 128;

  // static constexpr uint32_t _conf_reg_default = I2S_TX_MSB_RIGHT | I2S_TX_RIGHT_FIRST | I2S_RX_RIGHT_FIRST | I2S_TX_DMA_EQUAL;
  static constexpr uint32_t _conf_reg_default = I2S_TX_RIGHT_FIRST | I2S_RX_RIGHT_FIRST | I2S_TX_DMA_EQUAL;
  static constexpr uint32_t _conf_reg_start   = _conf_reg_default | I2S_TX_START;
  static constexpr uint32_t _conf_reg_reset   = _conf_reg_default | I2S_TX_RESET;
  static constexpr uint32_t _sample_rate_conf_reg_bufferd = 32 << I2S_TX_BITS_MOD_S | 32 << I2S_RX_BITS_MOD_S | 1 << I2S_TX_BCK_DIV_NUM_S | 1 << I2S_RX_BCK_DIV_NUM_S;
  static constexpr uint32_t _sample_rate_conf_reg_direct = 16 << I2S_TX_BITS_MOD_S | 16 << I2S_RX_BITS_MOD_S | 1 << I2S_TX_BCK_DIV_NUM_S | 1 << I2S_RX_BCK_DIV_NUM_S;
  static constexpr uint32_t _fifo_conf_default = 1 << I2S_TX_FIFO_MOD | 1 << I2S_RX_FIFO_MOD | 32 << I2S_TX_DATA_NUM_S | 32 << I2S_RX_DATA_NUM_S | I2S_TX_FIFO_MOD_FORCE_EN;
  static constexpr uint32_t _fifo_conf_dma     = _fifo_conf_default | I2S_DSCR_EN;

  static __attribute__ ((always_inline)) inline volatile uint32_t* reg(uint32_t addr) { return (volatile uint32_t *)ETS_UNCACHED_ADDR(addr); }

  static i2s_dev_t* getDev(i2s_port_t port)
  {
#if defined (CONFIG_IDF_TARGET_ESP32S2)
    return &I2S0;
#else
    return (port == 0) ? &I2S0 : &I2S1;
#endif
  }

  void Bus_Parallel16::config(const config_t& cfg)
  {
    _cfg = cfg;
    auto port = cfg.i2s_port;
    _dev = getDev(port);
    
    _mask_reg_dc = (cfg.pin_rs < 0) ? 0 : (1ul << (cfg.pin_rs & 31));
    _gpio_reg_dc[0] = get_gpio_lo_reg(cfg.pin_rs);
    _gpio_reg_dc[1] = get_gpio_hi_reg(cfg.pin_rs);
    _last_freq_apb = 0;
  }

  bool Bus_Parallel16::init(void)
  {
    _init_pin();

    auto i2s_dev = (i2s_dev_t*)_dev;
    //Reset I2S subsystem
    i2s_dev->conf.val = I2S_TX_RESET | I2S_RX_RESET | I2S_TX_FIFO_RESET | I2S_RX_FIFO_RESET;
    i2s_dev->conf.val = _conf_reg_default;

    i2s_dev->int_ena.val = 0;
    i2s_dev->timing.val = 0;

    //Reset DMA
    i2s_dev->lc_conf.val = I2S_IN_RST | I2S_OUT_RST | I2S_AHBM_RST | I2S_AHBM_FIFO_RST;
    i2s_dev->lc_conf.val = I2S_OUT_EOF_MODE | I2S_OUTDSCR_BURST_EN | I2S_OUT_DATA_BURST_EN;

    i2s_dev->in_link.val = 0;
    i2s_dev->out_link.val = 0;

    i2s_dev->conf1.val = I2S_TX_PCM_BYPASS | I2S_TX_STOP_EN;
    i2s_dev->conf2.val = I2S_LCD_EN;
    i2s_dev->conf_chan.val = 1 << I2S_TX_CHAN_MOD_S | 1 << I2S_RX_CHAN_MOD_S;

    _alloc_dmadesc(1);

    return true;
  }

  void Bus_Parallel16::_init_pin(void)
  {
    gpio_pad_select_gpio(_cfg.pin_d0);
    gpio_pad_select_gpio(_cfg.pin_d1);
    gpio_pad_select_gpio(_cfg.pin_d2);
    gpio_pad_select_gpio(_cfg.pin_d3);
    gpio_pad_select_gpio(_cfg.pin_d4);
    gpio_pad_select_gpio(_cfg.pin_d5);
    gpio_pad_select_gpio(_cfg.pin_d6);
    gpio_pad_select_gpio(_cfg.pin_d7);

    gpio_pad_select_gpio(_cfg.pin_rd);
    gpio_pad_select_gpio(_cfg.pin_wr);
    gpio_pad_select_gpio(_cfg.pin_rs);

    gpio_hi(_cfg.pin_rd);
    gpio_hi(_cfg.pin_wr);
    gpio_hi(_cfg.pin_rs);

    gpio_set_direction((gpio_num_t)_cfg.pin_rd, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)_cfg.pin_wr, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)_cfg.pin_rs, GPIO_MODE_OUTPUT);

    gpio_set_direction((gpio_num_t)_cfg.pin_d0, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction((gpio_num_t)_cfg.pin_d1, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction((gpio_num_t)_cfg.pin_d2, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction((gpio_num_t)_cfg.pin_d3, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction((gpio_num_t)_cfg.pin_d4, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction((gpio_num_t)_cfg.pin_d5, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction((gpio_num_t)_cfg.pin_d6, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction((gpio_num_t)_cfg.pin_d7, GPIO_MODE_INPUT_OUTPUT);

    auto idx_base = I2S0O_DATA_OUT7_IDX;

    gpio_matrix_out(_cfg.pin_rs , idx_base     , 0, 0);
    gpio_matrix_out(_cfg.pin_d0 , idx_base +  9, 0, 0);
    gpio_matrix_out(_cfg.pin_d1 , idx_base + 10, 0, 0);
    gpio_matrix_out(_cfg.pin_d2 , idx_base + 11, 0, 0);
    gpio_matrix_out(_cfg.pin_d3 , idx_base + 12, 0, 0);
    gpio_matrix_out(_cfg.pin_d4 , idx_base + 13, 0, 0);
    gpio_matrix_out(_cfg.pin_d5 , idx_base + 14, 0, 0);
    gpio_matrix_out(_cfg.pin_d6 , idx_base + 15, 0, 0);
    gpio_matrix_out(_cfg.pin_d7 , idx_base + 16, 0, 0);
    gpio_matrix_out(_cfg.pin_d8 , idx_base +  1, 0, 0);
    gpio_matrix_out(_cfg.pin_d9 , idx_base +  2, 0, 0);
    gpio_matrix_out(_cfg.pin_d10, idx_base +  3, 0, 0);
    gpio_matrix_out(_cfg.pin_d11, idx_base +  4, 0, 0);
    gpio_matrix_out(_cfg.pin_d12, idx_base +  5, 0, 0);
    gpio_matrix_out(_cfg.pin_d13, idx_base +  6, 0, 0);
    gpio_matrix_out(_cfg.pin_d14, idx_base +  7, 0, 0);
    gpio_matrix_out(_cfg.pin_d15, idx_base +  8, 0, 0);

    _direct_dc = false;

    uint32_t dport_clk_en;
    uint32_t dport_rst;

    if (_cfg.i2s_port == I2S_NUM_0) {
      idx_base = I2S0O_WS_OUT_IDX;
      dport_clk_en = DPORT_I2S0_CLK_EN;
      dport_rst = DPORT_I2S0_RST;
    }
#if !defined (CONFIG_IDF_TARGET_ESP32S2)
    else
    {
      idx_base = I2S1O_WS_OUT_IDX;
      dport_clk_en = DPORT_I2S1_CLK_EN;
      dport_rst = DPORT_I2S1_RST;
    }
#endif
    gpio_matrix_out(_cfg.pin_wr, idx_base, 1, 0); // WR (Write-strobe in 8080 mode, Active-low)

    DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, dport_clk_en);
    DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, dport_rst);
  }

  void Bus_Parallel16::release(void)
  {
    if (_dmadesc)
    {
      heap_caps_free(_dmadesc);
      _dmadesc = nullptr;
      _dmadesc_size = 0;
    }
  }

  void Bus_Parallel16::beginTransaction(void)
  {
    uint32_t freq_apb = getApbFrequency();
    if (_last_freq_apb != freq_apb)
    {
      _last_freq_apb = freq_apb;
      // clock = 80MHz(apb_freq) / I2S_CLKM_DIV_NUM
      // I2S_CLKM_DIV_NUM 2=40MHz  /  3=27MHz  /  4=20MHz  /  5=16MHz  /  8=10MHz  /  10=8MHz
      _div_num = std::min(255u, 1 + (freq_apb / (1 + _cfg.freq_write)));

      _clkdiv_write =             I2S_CLKA_ENA
                    |             I2S_CLK_EN
                    |        1 << I2S_CLKM_DIV_A_S
                    |        0 << I2S_CLKM_DIV_B_S
                    | _div_num << I2S_CLKM_DIV_NUM_S
                    ;
    }
    *reg(I2S_CLKM_CONF_REG(_cfg.i2s_port)) = _clkdiv_write;

    auto i2s_dev = (i2s_dev_t*)_dev;
    i2s_dev->out_link.val = 0;
    i2s_dev->sample_rate_conf.val = _sample_rate_conf_reg_bufferd;
    i2s_dev->fifo_conf.val = _fifo_conf_dma;

    _cache_index = 0;
    _cache_flip = _cache[0];
/////////////////
/*
uint8_t data[16];
data[ 0] = 0x11;
data[ 1] = 0x22;
data[ 2] = 0x33;
data[ 3] = 0x44;
data[ 4] = 0x55;
data[ 5] = 0x66;
data[ 6] = 0x77;
data[ 7] = 0x88;
data[ 8] = 0x99;
data[ 9] = 0xAA;
data[10] = 0xBB;
data[11] = 0xCC;
data[12] = 0xDD;
data[13] = 0xEE;
data[14] = 0xFF;
data[15] = 0x00;

int count = 8;

    //auto i2s_dev = (i2s_dev_t*)_dev;
    if (i2s_dev->out_link.val)
    {
#if defined (CONFIG_IDF_TARGET_ESP32S2)
      while (!(i2s_dev->lc_state0.out_empty)) {}
#else
      while (!(i2s_dev->lc_state0 & 0x80000000)) {} // I2S_OUT_EMPTY
#endif
    }
    // _setup_dma_desc_links((const uint8_t*)_cache_flip, count << 1);
    _dmadesc[0].buf = data;
    *(uint32_t*)&_dmadesc[0] = count | count << 12 | 0xC0000000;

    _dmadesc[0].empty = 0;

    while (!i2s_dev->state.tx_idle) {}

    i2s_dev->conf.val = _conf_reg_reset;
    i2s_dev->sample_rate_conf.val = _sample_rate_conf_reg_bufferd;
    // i2s_dev->sample_rate_conf.val = _sample_rate_conf_reg_direct;
    i2s_dev->out_link.val = I2S_OUTLINK_START | ((uint32_t)&_dmadesc[0] & I2S_OUTLINK_ADDR);

    if (_direct_dc)
    {
      _direct_dc = false;
      auto idx_base = I2S0O_DATA_OUT7_IDX;
      gpio_matrix_out(_cfg.pin_rs, idx_base, 0, 0);
    }
    else if (_div_num < 4)
    { /// OUTLINK_START～TX_STARTの時間が短すぎるとデータの先頭を送り損じる事があるのでnopウェイトを入れる
      size_t wait = (8 - _div_num) << 2;
      do { __asm__ __volatile__ ("nop"); } while (--wait);
    }
    i2s_dev->conf.val = _conf_reg_start;

delay(1000);
//*/
  }

  void Bus_Parallel16::endTransaction(void)
  {
    if (_cache_index)
    {
      _flush(_cache_index, true);
      _cache_index = 0;
      ets_delay_us(1);
    }
    _wait();
  }

  void Bus_Parallel16::_wait(void)
  {
    //i2s_dev->int_clr.val = I2S_TX_REMPTY_INT_CLR;
    auto i2s_dev = (i2s_dev_t*)_dev;
    if (i2s_dev->out_link.val)
    {
#if defined (CONFIG_IDF_TARGET_ESP32S2)
      while (!(i2s_dev->lc_state0.out_empty)) {}
#else
      while (!(i2s_dev->lc_state0 & 0x80000000)) {} // I2S_OUT_EMPTY
#endif
      i2s_dev->out_link.val = 0;
    }
    while (!i2s_dev->state.tx_idle) {}
  }

  void Bus_Parallel16::wait(void)
  {
    _wait();
  }

  bool Bus_Parallel16::busy(void) const
  {
    auto i2s_dev = (i2s_dev_t*)_dev;
#if defined (CONFIG_IDF_TARGET_ESP32S2)
    return !i2s_dev->state.tx_idle
         || (i2s_dev->out_link.val && !(i2s_dev->lc_state0.out_empty)); // I2S_OUT_EMPTY
#else
    return !i2s_dev->state.tx_idle
         || (i2s_dev->out_link.val && !(i2s_dev->lc_state0 & 0x80000000)); // I2S_OUT_EMPTY
#endif
  }

  void Bus_Parallel16::flush(void)
  {
    if (_cache_index)
    {
      _flush(_cache_index, true);
      _cache_index = 0;
    }
  }

  size_t Bus_Parallel16::_flush(size_t count, bool dc)
  {
    auto i2s_dev = (i2s_dev_t*)_dev;
    if (i2s_dev->out_link.val)
    {
#if defined (CONFIG_IDF_TARGET_ESP32S2)
      while (!(i2s_dev->lc_state0.out_empty)) {}
#else
      while (!(i2s_dev->lc_state0 & 0x80000000)) {} // I2S_OUT_EMPTY
#endif
    }
    _dmadesc[0].buf = (uint8_t *)_cache_flip;
    *(uint32_t*)&_dmadesc[0] = count << 2 | count << 14 | 0xC0000000;
    _dmadesc[0].empty = 0;

    while (!i2s_dev->state.tx_idle) {}

    i2s_dev->conf.val = _conf_reg_reset;
    i2s_dev->sample_rate_conf.val = _sample_rate_conf_reg_bufferd;
    i2s_dev->out_link.val = I2S_OUTLINK_START | ((uint32_t)&_dmadesc[0] & I2S_OUTLINK_ADDR);

    if (_direct_dc)
    {
      _direct_dc = false;
      auto idx_base = I2S0O_DATA_OUT7_IDX;
      gpio_matrix_out(_cfg.pin_rs, idx_base, 0, 0);
    }
    if (_div_num < 4)
    { /// OUTLINK_START～TX_STARTの時間が短すぎるとデータの先頭を送り損じる事があるのでnopウェイトを入れる
      size_t wait = (8 - _div_num) << 1;
      do { __asm__ __volatile__ ("nop"); } while (--wait);
    }
    i2s_dev->conf.val = _conf_reg_start;

    _cache_flip = (_cache_flip == _cache[0]) ? _cache[1] : _cache[0];
    return 0;
  }

  bool Bus_Parallel16::writeCommand(uint32_t data, uint_fast8_t bit_length)
  {
    auto idx = _cache_index;
    auto bytes = bit_length >> 3;
    auto c = _cache_flip;
    do
    {
      c[idx++] = data << 16;
      data >>= 16;
    } while (1 < (bytes -= 2));
    if (idx >= CACHE_THRESH)
    {
      _flush(idx);
      idx = 0;
    }
    _cache_index = idx;
    return true;
  }

  void Bus_Parallel16::writeData(uint32_t data, uint_fast8_t bit_length)
  {
    auto idx = _cache_index;
    auto bytes = bit_length >> 3;
    auto c = _cache_flip;
    do
    {
      c[idx++] = (data << 16) | 0xFFFF;
      data >>= 16;
    } while (1 < (bytes -= 2));
    if (idx >= CACHE_THRESH)
    {
      _flush(idx);
      idx = 0;
    }
    _cache_index = idx;
  }

  void Bus_Parallel16::writeDataRepeat(uint32_t color_raw, uint_fast8_t bit_length, uint32_t length)
  {
    size_t bytes = bit_length >> 3;
    auto idx = _cache_index;
    auto c = _cache_flip;

    if (bytes == 2)
    {
      color_raw = color_raw << 16 | 0xFFFF;
      while (length)
      {
        --length;
        *(uint32_t*)(&c[idx]) = color_raw;
        ++idx;
        if (idx >= CACHE_THRESH)
        {
          idx = _flush(idx);
          c = _cache_flip;
        }
      }
    }
    else // if (bytes == 3)
    {
      uint32_t raw[3] = 
      { (uint8_t)(color_raw) << 8 |  color_raw    << 16 | 0xFF00FF 
      ,           color_raw  >> 8 |  color_raw    << 24 | 0xFF00FF 
      ,           color_raw       | (color_raw>>16)<<24 | 0xFF00FF 
      };
      if (length & 1)
      {
        --length;
        *(uint32_t*)(&c[idx  ]) = raw[0];
        *(uint32_t*)(&c[idx+2]) = raw[1];
        idx += 3;
      }
      if (idx >= CACHE_THRESH - 4)
      {
        idx = _flush(idx);
        c = _cache_flip;
      }
      while (length)
      {
        length -= 2;
        *(uint32_t*)(&c[idx  ]) = raw[0];
        *(uint32_t*)(&c[idx+2]) = raw[1];
        *(uint32_t*)(&c[idx+4]) = raw[2];
        idx += 6;
        if (idx >= CACHE_THRESH - 4)
        {
          idx = _flush(idx);
          c = _cache_flip;
        }
      }
    }
    _cache_index = idx;
  }

  void Bus_Parallel16::writePixels(pixelcopy_t* param, uint32_t length)
  {
    if (_cache_index)
    {
      _flush(_cache_index);
      _cache_index = 0;
    }
    const uint32_t bytes = param->dst_bits >> 3;
    auto fp_copy = param->fp_copy;
    const uint32_t limit = CACHE_SIZE / bytes;
    uint8_t len = length % limit;
    if (len)
    {
      fp_copy(_cache_flip, 0, len, param);
      writeBytes((uint8_t*)_cache_flip, len * bytes, true, true);
      _cache_flip = (_cache_flip == _cache[0]) ? _cache[1] : _cache[0];
      if (0 == (length -= len)) return;
    }
    do
    {
      fp_copy(_cache_flip, 0, limit, param);
      writeBytes((uint8_t*)_cache_flip, limit * bytes, true, true);
      _cache_flip = (_cache_flip == _cache[0]) ? _cache[1] : _cache[0];
    } while (length -= limit);
  }

  void Bus_Parallel16::writeBytes(const uint8_t* data, uint32_t length, bool dc, bool use_dma)
  {
    if (_cache_index)
    {
      _flush(_cache_index);
      _cache_index = 0;
    }
    auto i2s_dev = (i2s_dev_t*)_dev;
    do
    {
      dc_control(dc);
      i2s_dev->sample_rate_conf.val = _sample_rate_conf_reg_direct;
      i2s_dev->conf.val = _conf_reg_reset;
      if (use_dma)
      {
        _setup_dma_desc_links(data, length);
        length = 0;
      }
      else
      {
        size_t len = ((length - 1) % CACHE_SIZE) + 1;
        length -= len;
        memcpy(_cache_flip, data, len);
        data += len;
        _setup_dma_desc_links((const uint8_t*)_cache_flip, len);
        _cache_flip = (_cache_flip == _cache[0]) ? _cache[1] : _cache[0];
      }
      i2s_dev->out_link.val = I2S_OUTLINK_START | ((uint32_t)_dmadesc & I2S_OUTLINK_ADDR);
      if (!_direct_dc)
      {
        _direct_dc = true;
        gpio_matrix_out(_cfg.pin_rs, 0x100, 0, 0);
      }
      else
      {
        if (_div_num <= 4)
        {
          size_t wait = (8 - _div_num) << 2;
          do { __asm__ __volatile__ ("nop"); } while (--wait);
        }
      }
      i2s_dev->conf.val = _conf_reg_start;
    } while (length);
  }

  void Bus_Parallel16::beginRead(void)
  {
    if (_cache_index) { _cache_index = _flush(_cache_index, true); }

    dc_control(true);
    if (!_direct_dc)
    {
      _direct_dc = true;
      gpio_matrix_out(_cfg.pin_rs, 0x100, 0, 0);
    }
    gpio_lo(_cfg.pin_rd);
  }

  void Bus_Parallel16::endRead(void)
  {
    gpio_hi(_cfg.pin_rd);
  }

  void Bus_Parallel16::_read_bytes(uint8_t* dst, uint32_t length)
  {
    uint8_t in[8];

    uint_fast8_t m7 = 1ul << (_cfg.pin_d7 & 7);
    uint_fast8_t m6 = 1ul << (_cfg.pin_d6 & 7);
    uint_fast8_t m5 = 1ul << (_cfg.pin_d5 & 7);
    uint_fast8_t m4 = 1ul << (_cfg.pin_d4 & 7);
    uint_fast8_t m3 = 1ul << (_cfg.pin_d3 & 7);
    uint_fast8_t m2 = 1ul << (_cfg.pin_d2 & 7);
    uint_fast8_t m1 = 1ul << (_cfg.pin_d1 & 7);
    uint_fast8_t m0 = 1ul << (_cfg.pin_d0 & 7);

    uint_fast8_t i7 = _cfg.pin_d7 >> 3;
    uint_fast8_t i6 = _cfg.pin_d6 >> 3;
    uint_fast8_t i5 = _cfg.pin_d5 >> 3;
    uint_fast8_t i4 = _cfg.pin_d4 >> 3;
    uint_fast8_t i3 = _cfg.pin_d3 >> 3;
    uint_fast8_t i2 = _cfg.pin_d2 >> 3;
    uint_fast8_t i1 = _cfg.pin_d1 >> 3;
    uint_fast8_t i0 = _cfg.pin_d0 >> 3;

    auto reg_rd_h = get_gpio_hi_reg(_cfg.pin_rd);
    auto reg_rd_l = get_gpio_lo_reg(_cfg.pin_rd);
    uint32_t mask_rd = 1ul << (_cfg.pin_rd & 31);
    uint32_t val;
    do
    {
      ((uint32_t*)in)[0] = GPIO.in;
      ((uint32_t*)in)[1] = GPIO.in1.val;
      *reg_rd_h = mask_rd;

      val = ((((bool)(in[i7] & m7) << 1)
            +  (bool)(in[i6] & m6)     ) << 2)
            + ((bool)(in[i5] & m5) << 1)
            + ((bool)(in[i4] & m4)     );

      *reg_rd_l = mask_rd;
      val = (((val << 2)
          + ((bool)(in[i3] & m3) << 1)
          + ((bool)(in[i2] & m2)     )) << 2)
          + ((bool)(in[i1] & m1) << 1)
          + ((bool)(in[i0] & m0)     )
          ;
      *dst++ = val;
    } while (--length);
  }

  uint32_t Bus_Parallel16::readData(uint_fast8_t bit_length)
  {
    union {
      uint32_t res;
      uint8_t raw[4];
    };
    _read_bytes(raw, (bit_length + 7) >> 3);
    return res;
  }

  bool Bus_Parallel16::readBytes(uint8_t* dst, uint32_t length, bool use_dma)
  {
    _read_bytes(dst, length);
    return true;
  }

  void Bus_Parallel16::readPixels(void* dst, pixelcopy_t* param, uint32_t length)
  {
    uint32_t _regbuf[8];
    const auto bytes = param->src_bits >> 3;
    uint32_t limit = (bytes == 2) ? 16 : 10;
    param->src_data = _regbuf;
    int32_t dstindex = 0;
    do {
      uint32_t len2 = (limit > length) ? length : limit;
      length -= len2;

      _read_bytes((uint8_t*)_regbuf, len2 * bytes);
      param->src_x = 0;
      dstindex = param->fp_copy(dst, dstindex, dstindex + len2, param);
    } while (length);
  }

  void Bus_Parallel16::_alloc_dmadesc(size_t len)
  {
    if (_dmadesc) heap_caps_free(_dmadesc);
    _dmadesc_size = len;
    _dmadesc = (lldesc_t*)heap_caps_malloc(sizeof(lldesc_t) * len, MALLOC_CAP_DMA);
  }

  void Bus_Parallel16::_setup_dma_desc_links(const uint8_t *data, int32_t len)
  {
    static constexpr size_t MAX_DMA_LEN = (4096-4);

    if (_dmadesc_size * MAX_DMA_LEN < len)
    {
      _alloc_dmadesc(len / MAX_DMA_LEN + 1);
    }
    lldesc_t *dmadesc = _dmadesc;

    while (len > MAX_DMA_LEN)
    {
      len -= MAX_DMA_LEN;
      dmadesc->buf = (uint8_t *)data;
      data += MAX_DMA_LEN;
      *(uint32_t*)dmadesc = MAX_DMA_LEN | MAX_DMA_LEN << 12 | 0x80000000;
      dmadesc->qe.stqe_next = dmadesc + 1;
      dmadesc++;
    }
    *(uint32_t*)dmadesc = ((len + 3) & ( ~3 )) | len << 12 | 0xC0000000;
    dmadesc->buf = (uint8_t *)data;
    dmadesc->qe.stqe_next = nullptr;
  }

//----------------------------------------------------------------------------
 }
}

#endif
#endif