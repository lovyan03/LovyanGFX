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
#if defined (ESP32) || defined (CONFIG_IDF_TARGET_ESP32) || defined (CONFIG_IDF_TARGET_ESP32S2) || defined (ESP_PLATFORM)

#include "Bus_Parallel8.hpp"
#include "../../misc/pixelcopy.hpp"

#include <soc/dport_reg.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  // #define SAFE_I2S_FIFO_WR_REG(i) (0x6000F000 + ((i)*0x1E000))
  // #define SAFE_I2S_FIFO_RD_REG(i) (0x6000F004 + ((i)*0x1E000))
  #define SAFE_I2S_FIFO_WR_REG(i) (0x3FF4F000 + ((i)*0x1E000))
  #define SAFE_I2S_FIFO_RD_REG(i) (0x3FF4F004 + ((i)*0x1E000))

  static constexpr std::size_t CACHE_THRESH = 256;

  static constexpr std::uint32_t _conf_reg_default = I2S_TX_MSB_RIGHT | I2S_TX_RIGHT_FIRST | I2S_RX_RIGHT_FIRST;
  static constexpr std::uint32_t _conf_reg_start   = _conf_reg_default | I2S_TX_START;
  static constexpr std::uint32_t _conf_reg_reset   = _conf_reg_default | I2S_TX_RESET;
  static constexpr std::uint32_t _sample_rate_conf_reg_32bit = 32 << I2S_TX_BITS_MOD_S | 32 << I2S_RX_BITS_MOD_S | 1 << I2S_TX_BCK_DIV_NUM_S | 1 << I2S_RX_BCK_DIV_NUM_S;
  static constexpr std::uint32_t _sample_rate_conf_reg_16bit = 16 << I2S_TX_BITS_MOD_S | 16 << I2S_RX_BITS_MOD_S | 1 << I2S_TX_BCK_DIV_NUM_S | 1 << I2S_RX_BCK_DIV_NUM_S;
  static constexpr std::uint32_t _fifo_conf_default = 1 << I2S_TX_FIFO_MOD | 1 << I2S_RX_FIFO_MOD | 32 << I2S_TX_DATA_NUM_S | 32 << I2S_RX_DATA_NUM_S;
  static constexpr std::uint32_t _fifo_conf_dma     = _fifo_conf_default | I2S_DSCR_EN;

  static __attribute__ ((always_inline)) inline volatile std::uint32_t* reg(std::uint32_t addr) { return (volatile std::uint32_t *)ETS_UNCACHED_ADDR(addr); }

  void Bus_Parallel8::config(const config_t& cfg)
  {
    _cfg = cfg;
    auto port = cfg.i2s_port;
    _dev = (port == 0) ? &I2S0 : &I2S1;

    _i2s_fifo_wr_reg = reg(SAFE_I2S_FIFO_WR_REG(port));
    
    _last_freq_apb = 0;
  }

  bool Bus_Parallel8::init(void)
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
    i2s_dev->lc_conf.val = I2S_OUT_EOF_MODE;

    i2s_dev->in_link.val = 0;
    i2s_dev->out_link.val = 0;

    i2s_dev->conf1.val = I2S_TX_PCM_BYPASS | I2S_TX_STOP_EN;
    i2s_dev->conf2.val = I2S_LCD_EN;
    i2s_dev->conf_chan.val = 1 << I2S_TX_CHAN_MOD_S | 1 << I2S_RX_CHAN_MOD_S;

    memset(&_dmadesc, 0, sizeof(lldesc_t));

    return true;
  }

  void Bus_Parallel8::_init_pin(void)
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

    auto idx_base = (_cfg.i2s_port == I2S_NUM_0) ? I2S0O_DATA_OUT8_IDX : I2S1O_DATA_OUT8_IDX;
    gpio_matrix_out(_cfg.pin_rs, idx_base + 8, 0, 0);
    gpio_matrix_out(_cfg.pin_d7, idx_base + 7, 0, 0);
    gpio_matrix_out(_cfg.pin_d6, idx_base + 6, 0, 0);
    gpio_matrix_out(_cfg.pin_d5, idx_base + 5, 0, 0);
    gpio_matrix_out(_cfg.pin_d4, idx_base + 4, 0, 0);
    gpio_matrix_out(_cfg.pin_d3, idx_base + 3, 0, 0);
    gpio_matrix_out(_cfg.pin_d2, idx_base + 2, 0, 0);
    gpio_matrix_out(_cfg.pin_d1, idx_base + 1, 0, 0);
    gpio_matrix_out(_cfg.pin_d0, idx_base    , 0, 0);

    std::uint32_t dport_clk_en;
    std::uint32_t dport_rst;

    if (_cfg.i2s_port == I2S_NUM_0) {
      idx_base = I2S0O_WS_OUT_IDX;
      dport_clk_en = DPORT_I2S0_CLK_EN;
      dport_rst = DPORT_I2S0_RST;
    } else {
      idx_base = I2S1O_WS_OUT_IDX;
      dport_clk_en = DPORT_I2S1_CLK_EN;
      dport_rst = DPORT_I2S1_RST;
    }
    gpio_matrix_out(_cfg.pin_wr, idx_base, 1, 0); // WR (Write-strobe in 8080 mode, Active-low)

    DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, dport_clk_en);
    DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, dport_rst);
  }

  void Bus_Parallel8::release(void)
  {  }

  void Bus_Parallel8::beginTransaction(void)
  {
    std::uint32_t freq_apb = getApbFrequency();
    if (_last_freq_apb != freq_apb)
    {
      _last_freq_apb = freq_apb;
      // clock = 80MHz(apb_freq) / I2S_CLKM_DIV_NUM
      // I2S_CLKM_DIV_NUM 4=20MHz  /  5=16MHz  /  8=10MHz  /  10=8MHz
      _div_num = std::min(255u, std::max(3u, 1 + (freq_apb / (1 + _cfg.freq_write))));

      _clkdiv_write =             I2S_CLKA_ENA
                    |             I2S_CLK_EN
                    |        1 << I2S_CLKM_DIV_A_S
                    |        0 << I2S_CLKM_DIV_B_S
                    | _div_num << I2S_CLKM_DIV_NUM_S
                    ;
    }
    *reg(I2S_CLKM_CONF_REG(_cfg.i2s_port)) = _clkdiv_write;

    auto i2s_dev = (i2s_dev_t*)_dev;
    i2s_dev->sample_rate_conf.val = _sample_rate_conf_reg_16bit;
    i2s_dev->fifo_conf.val = _fifo_conf_default;

    _cache_index = 0;
    _cache_flip = _cache[0];
  }

  void Bus_Parallel8::endTransaction(void)
  {
    if (_cache_index)
    {
      _cache_index = _flush(_cache_index, true);
    }
    _wait();
  }

  void Bus_Parallel8::_wait(void)
  {
    //i2s_dev->int_clr.val = I2S_TX_REMPTY_INT_CLR;
    auto i2s_dev = (i2s_dev_t*)_dev;
    auto conf_reg = _conf_reg_default | I2S_TX_RESET;
    while (!i2s_dev->state.tx_idle)
    {}
    i2s_dev->conf.val = conf_reg;
  }

  void Bus_Parallel8::wait(void)
  {
    _wait();
  }

  bool Bus_Parallel8::busy(void) const
  {
    auto i2s_dev = (i2s_dev_t*)_dev;
    return !i2s_dev->int_raw.tx_rempty || !i2s_dev->state.tx_idle;
  }

  void Bus_Parallel8::flush(void)
  {
    if (_cache_index)
    {
      _cache_index = _flush(_cache_index, true);
    }
  }

  /// WiFi,BT使用状況確認
  static bool checkWireless(void)
  {
    return *reg(DPORT_WIFI_CLK_EN_REG) & 0x7FF;
  }
//*
// use DMA
  std::size_t Bus_Parallel8::_flush(std::size_t count, bool force)
  {
    bool slow = _div_num > 8;

    std::size_t idx_e = count & ~1;
    auto i2s_dev = (i2s_dev_t*)_dev;

    if (idx_e)
    {
      _dmadesc.buf = (std::uint8_t*)_cache_flip;
      *(std::uint32_t*)&_dmadesc = idx_e << 1 | idx_e << 13 | 0xC0000000;
      while (!i2s_dev->state.tx_idle)
      {}
      i2s_dev->fifo_conf.val = _fifo_conf_dma;
      i2s_dev->conf.val = _conf_reg_reset | I2S_TX_FIFO_RESET;
      i2s_dev->out_link.val = I2S_OUTLINK_START | ((uint32_t)&_dmadesc & I2S_OUTLINK_ADDR);
      i2s_dev->sample_rate_conf.val = _sample_rate_conf_reg_16bit;

      auto cache_old = _cache_flip;
      _cache_flip = (cache_old == _cache[0]) ? _cache[1] : _cache[0];
      while (!i2s_dev->state.tx_fifo_reset_back)
      {}

      // DMAの準備待ちウェイト …無線使用中はウェイトを増やす
      std::size_t wait = (16 << checkWireless()) + (_div_num >> 2);
      do { __asm__ __volatile__ ("nop"); } while (--wait);
      i2s_dev->conf.val = _conf_reg_start;
      if (slow) while (i2s_dev->state.tx_idle) {}
      
      count -= idx_e;
      if (!count) return 0;
      
      //memmove(_cache_flip, &cache_old[idx_e], (count + 1) << 1);
      _cache_flip[0] = cache_old[idx_e  ];
      _cache_flip[1] = cache_old[idx_e+1];
    }

    if (!force)
    {
      return count;
    }

    // ここから DMAで送信しきれなかった端数ぶんの送信処理

//ets_delay_us(2);
    while (!i2s_dev->state.tx_idle)
    {}
    i2s_dev->fifo_conf.val = _fifo_conf_default;
    i2s_dev->conf.val = _conf_reg_reset;// | I2S_TX_FIFO_RESET;
    i2s_dev->out_link.val = 0;
    i2s_dev->sample_rate_conf.val = _sample_rate_conf_reg_32bit;
    std::size_t idx = 0;
    do
    {
      *_i2s_fifo_wr_reg = _cache_flip[idx ^ 1] << 16;
    } while (++idx != count);

    std::size_t wait = (16 << checkWireless()) + (_div_num >> 2);
    do { __asm__ __volatile__ ("nop"); } while (--wait);
    if (slow) ets_delay_us(_div_num >> 6);
    i2s_dev->conf.val = _conf_reg_start;
    if (slow) while (i2s_dev->state.tx_idle) {}
//ets_delay_us(2);

    return 0;
  }
/*/
// use FIFO
  std::size_t Bus_Parallel8::_flush(std::size_t count, bool force)
  {
    bool slow = _div_num > 8;
    while (!i2s_dev->int_raw.tx_rempty || (slow && !i2s_dev->state.tx_idle))
    {}
    i2s_dev->conf.val = _conf_reg_reset;

    std::size_t idx_e = std::min(CACHE_THRESH, count & ~3);

    auto cache = _cache_flip;

    if (idx_e)
    {
      std::size_t idx = 0;
      auto c = (std::uint32_t*)cache;
      do
      {
        *_i2s_fifo_wr_reg = c[idx>>1];
        *_i2s_fifo_wr_reg = c[(idx>>1)+1];
      } while ((idx += 4) != idx_e);
      if (slow) ets_delay_us(_div_num >> 6);
      i2s_dev->conf.val = _conf_reg_start;
      i2s_dev->int_clr.val = I2S_TX_REMPTY_INT_CLR;

      count -= idx_e;

      if (!count) return 0;

      memmove(cache, &cache[idx_e], (count + 1) << 1);
      // idx = 0;
      // {
      //   cache[idx] = cache[idx_e + idx];
      // } while (idx++ != count);
    }
    if (!force)
    {
      return count;
    }

    std::size_t idx = 0;
    while (!i2s_dev->int_raw.tx_rempty || (slow && !i2s_dev->state.tx_idle))
    {}
    i2s_dev->conf.val = _conf_reg_reset;

    i2s_dev->sample_rate_conf.val = _sample_rate_conf_reg_32bit;
    do
    {
      *_i2s_fifo_wr_reg = cache[idx^1] << 16;
    } while (++idx != count);

    if (slow) ets_delay_us(_div_num >> 6);
    i2s_dev->conf.val = _conf_reg_start;
    i2s_dev->int_clr.val = I2S_TX_REMPTY_INT_CLR;
    i2s_dev->sample_rate_conf.val = _sample_rate_conf_reg_16bit;

    return 0;
  }
//*/
  bool Bus_Parallel8::writeCommand(std::uint32_t data, std::uint_fast8_t bit_length)
  {
    auto idx = _cache_index;
    auto bytes = bit_length >> 3;
    auto c = _cache_flip;
    do
    {
      c[idx++ ^ 1] = data & 0xFF;
      data >>= 8;
    } while (--bytes);
    if (idx >= CACHE_THRESH)
    {
      idx = _flush(idx);
    }
    _cache_index = idx;
    return true;
  }

  void Bus_Parallel8::writeData(std::uint32_t data, std::uint_fast8_t bit_length)
  {
    auto idx = _cache_index;
    auto bytes = bit_length >> 3;
    auto c = _cache_flip;
    do
    {
      c[idx++ ^ 1] = data | 0x100;
      data >>= 8;
    } while (--bytes);
    if (idx >= CACHE_THRESH)
    {
      idx = _flush(idx);
    }
    _cache_index = idx;
  }

  void Bus_Parallel8::writeDataRepeat(std::uint32_t color_raw, std::uint_fast8_t bit_length, std::uint32_t length)
  {
    std::size_t bytes = bit_length >> 3;
    std::uint16_t raw[bytes];
    std::size_t b = 0;
    do
    {
      raw[b] = color_raw | 0x100;
      color_raw >>= 8;
    } while (++b != bytes);
    auto idx = _cache_index;
    auto c = _cache_flip;
    b = 0;
    for (;;)
    {
      c[idx ^ 1] = raw[b];
      ++idx;
      if (++b == bytes)
      {
        b = 0;
        if (idx >= CACHE_THRESH)
        {
          idx = _flush(idx);
          c = _cache_flip;
        }
        if (!--length) break;
      }
    }
    _cache_index = idx;
  }

  void Bus_Parallel8::writePixels(pixelcopy_t* param, std::uint32_t length)
  {
    std::uint8_t buf[CACHE_THRESH];
    const std::uint32_t bytes = param->dst_bits >> 3;
    auto fp_copy = param->fp_copy;
    const std::uint32_t limit = CACHE_THRESH / bytes;
    std::uint8_t len = length % limit;
    if (len) {
      fp_copy(buf, 0, len, param);
      writeBytes(buf, len * bytes, true, false);
      if (0 == (length -= len)) return;
    }
    do {
      fp_copy(buf, 0, limit, param);
      writeBytes(buf, limit * bytes, true, false);
    } while (length -= limit);
  }

//*
  void Bus_Parallel8::writeBytes(const std::uint8_t* data, std::uint32_t length, bool dc, bool use_dma)
  {
    std::uint32_t dc_data = dc << 8;
    auto idx = _cache_index;
    auto c = _cache_flip;

    std::size_t limit = std::min(CACHE_THRESH, length + idx);
    length -= limit - idx;
    --data;

    for (;;)
    {
      // --length;
      c[idx^1] = *++data | dc_data;
      if (++idx < limit) continue;
      if (idx >= CACHE_THRESH)
      {
        idx = _flush(idx);
        c = _cache_flip;
      }
      if (!length) break;
      limit = std::min(CACHE_THRESH, length + idx);
      length -= limit - idx;
    }
    _cache_index = idx;
  }
/*/
  void Bus_Parallel8::writeBytes(const std::uint8_t* data, std::uint32_t length, bool dc, bool use_dma)
  {
    std::uint32_t dc_data = dc << 8;
    auto idx = _cache_index;
    auto c = _cache_flip;
    do
    {
      c[idx^1] = *data++ | dc_data;
      if (++idx >= CACHE_THRESH)
      {
        idx = _flush(idx);
        c = _cache_flip;
      }
    } while (--length);
    _cache_index = idx;
  }
//*/
  std::uint_fast8_t Bus_Parallel8::_reg_to_value(std::uint32_t raw_value)
  {
    return ((raw_value >> _cfg.pin_d7) & 1) << 7
         | ((raw_value >> _cfg.pin_d6) & 1) << 6
         | ((raw_value >> _cfg.pin_d5) & 1) << 5
         | ((raw_value >> _cfg.pin_d4) & 1) << 4
         | ((raw_value >> _cfg.pin_d3) & 1) << 3
         | ((raw_value >> _cfg.pin_d2) & 1) << 2
         | ((raw_value >> _cfg.pin_d1) & 1) << 1
         | ((raw_value >> _cfg.pin_d0) & 1) ;
  }

  void Bus_Parallel8::beginRead(void)
  {
    if (_cache_index) { _cache_index = _flush(_cache_index, true); }

    _wait();
    gpio_lo(_cfg.pin_rd);
//      gpio_pad_select_gpio(_gpio_rd);
//      gpio_set_direction(_gpio_rd, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(_cfg.pin_wr);
    gpio_hi(_cfg.pin_wr);
    gpio_set_direction((gpio_num_t)_cfg.pin_wr, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(_cfg.pin_rs);
    gpio_hi(_cfg.pin_rs);
    gpio_set_direction((gpio_num_t)_cfg.pin_rs, GPIO_MODE_OUTPUT);
//      if (_i2s_port == I2S_NUM_0) {
////        gpio_matrix_out(_gpio_rd, I2S0O_WS_OUT_IDX    ,1,0);
//        gpio_matrix_out(_gpio_rd, I2S0O_BCK_OUT_IDX    ,1,0);
//      } else {
////        gpio_matrix_out(_gpio_rd, I2S1O_WS_OUT_IDX    ,1,0);
//        gpio_matrix_out(_gpio_rd, I2S1O_BCK_OUT_IDX    ,1,0);
//      }
//*
//      auto idx_base = (_i2s_port == I2S_NUM_0) ? I2S0O_DATA_OUT8_IDX : I2S1O_DATA_OUT8_IDX;
//      gpio_matrix_in(_gpio_d7, idx_base + 7, 0); // MSB
//      gpio_matrix_in(_gpio_d6, idx_base + 6, 0);
//      gpio_matrix_in(_gpio_d5, idx_base + 5, 0);
//      gpio_matrix_in(_gpio_d4, idx_base + 4, 0);
//      gpio_matrix_in(_gpio_d3, idx_base + 3, 0);
//      gpio_matrix_in(_gpio_d2, idx_base + 2, 0);
//      gpio_matrix_in(_gpio_d1, idx_base + 1, 0);
//      gpio_matrix_in(_gpio_d0, idx_base    , 0); // LSB
//*/
/*
    gpio_pad_select_gpio(_gpio_d7); gpio_set_direction(_gpio_d7, GPIO_MODE_INPUT);
    gpio_pad_select_gpio(_gpio_d6); gpio_set_direction(_gpio_d6, GPIO_MODE_INPUT);
    gpio_pad_select_gpio(_gpio_d5); gpio_set_direction(_gpio_d5, GPIO_MODE_INPUT);
    gpio_pad_select_gpio(_gpio_d4); gpio_set_direction(_gpio_d4, GPIO_MODE_INPUT);
    gpio_pad_select_gpio(_gpio_d3); gpio_set_direction(_gpio_d3, GPIO_MODE_INPUT);
    gpio_pad_select_gpio(_gpio_d2); gpio_set_direction(_gpio_d2, GPIO_MODE_INPUT);
    gpio_pad_select_gpio(_gpio_d1); gpio_set_direction(_gpio_d1, GPIO_MODE_INPUT);
    gpio_pad_select_gpio(_gpio_d0); gpio_set_direction(_gpio_d0, GPIO_MODE_INPUT);
    set_clock_read();
/*/
    gpio_matrix_out(_cfg.pin_d7, 0x100, 0, 0); // MSB
    gpio_matrix_out(_cfg.pin_d6, 0x100, 0, 0);
    gpio_matrix_out(_cfg.pin_d5, 0x100, 0, 0);
    gpio_matrix_out(_cfg.pin_d4, 0x100, 0, 0);
    gpio_matrix_out(_cfg.pin_d3, 0x100, 0, 0);
    gpio_matrix_out(_cfg.pin_d2, 0x100, 0, 0);
    gpio_matrix_out(_cfg.pin_d1, 0x100, 0, 0);
    gpio_matrix_out(_cfg.pin_d0, 0x100, 0, 0); // LSB

    lgfx::pinMode(_cfg.pin_d7, pin_mode_t::input);
    lgfx::pinMode(_cfg.pin_d6, pin_mode_t::input);
    lgfx::pinMode(_cfg.pin_d5, pin_mode_t::input);
    lgfx::pinMode(_cfg.pin_d4, pin_mode_t::input);
    lgfx::pinMode(_cfg.pin_d3, pin_mode_t::input);
    lgfx::pinMode(_cfg.pin_d2, pin_mode_t::input);
    lgfx::pinMode(_cfg.pin_d1, pin_mode_t::input);
    lgfx::pinMode(_cfg.pin_d0, pin_mode_t::input);
//*/
  }

  void Bus_Parallel8::endRead(void)
  {
    _wait();
    _init_pin();
  }

  std::uint32_t Bus_Parallel8::readData(std::uint_fast8_t bit_length)
  {
    union {
      std::uint32_t res;
      std::uint8_t raw[4];
    };
    bit_length = (bit_length + 7) & ~7;

    auto buf = raw;
    do {
      std::uint32_t tmp = GPIO.in;   // dummy read speed tweak.
      tmp = GPIO.in;
      gpio_hi(_cfg.pin_rd);
      gpio_lo(_cfg.pin_rd);
      *buf++ = _reg_to_value(tmp);
    } while (bit_length -= 8);
    return res;
  }

  bool Bus_Parallel8::readBytes(std::uint8_t* dst, std::uint32_t length, bool use_dma)
  {
    do {
      std::uint32_t tmp = GPIO.in;   // dummy read speed tweak.
      tmp = GPIO.in;
      gpio_hi(_cfg.pin_rd);
      gpio_lo(_cfg.pin_rd);
      *dst++ = _reg_to_value(tmp);
    } while (--length);
    return true;
  }

  void Bus_Parallel8::readPixels(void* dst, pixelcopy_t* param, std::uint32_t length)
  {
    std::uint32_t _regbuf[8];
    const auto bytes = param->src_bits >> 3;
    std::uint32_t limit = (bytes == 2) ? 16 : 10;
    param->src_data = _regbuf;
    std::int32_t dstindex = 0;
    do {
      std::uint32_t len2 = (limit > length) ? length : limit;
      length -= len2;
      std::uint32_t i = len2 * bytes;
      auto d = (std::uint8_t*)_regbuf;
      do {
        std::uint32_t tmp = GPIO.in;
        gpio_hi(_cfg.pin_rd);
        gpio_lo(_cfg.pin_rd);
        *d++ = _reg_to_value(tmp);
      } while (--i);
      param->src_x = 0;
      dstindex = param->fp_copy(dst, dstindex, dstindex + len2, param);
    } while (length);
  }

//----------------------------------------------------------------------------
 }
}

#endif