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
#if defined (CONFIG_IDF_TARGET_ESP32)

#include "Bus_HUB75.hpp"
#include "../../misc/pixelcopy.hpp"

#include <soc/dport_reg.h>
#include <rom/gpio.h>
#include <esp_log.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  static constexpr uint32_t _conf_reg_default = I2S_TX_MSB_RIGHT | I2S_TX_RIGHT_FIRST | I2S_RX_RIGHT_FIRST | I2S_TX_MONO;
  static constexpr uint32_t _conf_reg_start   = _conf_reg_default | I2S_TX_START;
  static constexpr uint32_t _conf_reg_reset   = _conf_reg_default | I2S_TX_RESET;
  static constexpr uint32_t _sample_rate_conf_reg_direct = 16 << I2S_TX_BITS_MOD_S | 16 << I2S_RX_BITS_MOD_S | 1 << I2S_TX_BCK_DIV_NUM_S | 1 << I2S_RX_BCK_DIV_NUM_S;
  static constexpr uint32_t _fifo_conf_default = 1 << I2S_TX_FIFO_MOD | 1 << I2S_RX_FIFO_MOD | 16 << I2S_TX_DATA_NUM_S | 16 << I2S_RX_DATA_NUM_S;
  static constexpr uint32_t _fifo_conf_dma     = _fifo_conf_default | I2S_DSCR_EN;

  static __attribute__ ((always_inline)) inline volatile uint32_t* reg(uint32_t addr) { return (volatile uint32_t *)ETS_UNCACHED_ADDR(addr); }

  __attribute__((always_inline))
  static inline i2s_dev_t* getDev(i2s_port_t port)
  {
#if defined (CONFIG_IDF_TARGET_ESP32S2)
    return &I2S0;
#else
    return (port == 0) ? &I2S0 : &I2S1;
#endif
  }

  void Bus_HUB75::config(const config_t& cfg)
  {
    _cfg = cfg;
    _dev = getDev(cfg.i2s_port);
  }

  bool Bus_HUB75::init(void)
  {
    auto idx_base = (_cfg.i2s_port == I2S_NUM_0) ? I2S0O_DATA_OUT8_IDX : I2S1O_DATA_OUT8_IDX;

    for (size_t i = 0; i < 15; ++i)
    {
      if (_cfg.pin_data[i] < 0) continue;
      gpio_pad_select_gpio(_cfg.pin_data[i]);
      gpio_matrix_out(_cfg.pin_data[i  ], idx_base + i, 0, 0);
    }

    idx_base = (_cfg.i2s_port == I2S_NUM_0) ? I2S0O_WS_OUT_IDX : I2S1O_WS_OUT_IDX;
    gpio_matrix_out(_cfg.pin_clk, idx_base, 1, 0); // clock Active-low

    uint32_t dport_clk_en;
    uint32_t dport_rst;

    int intr_source = ETS_I2S0_INTR_SOURCE;
    if (_cfg.i2s_port == I2S_NUM_0) {
      idx_base = I2S0O_WS_OUT_IDX;
      dport_clk_en = DPORT_I2S0_CLK_EN;
      dport_rst = DPORT_I2S0_RST;
    }
#if !defined (CONFIG_IDF_TARGET_ESP32S2)
    else
    {
      intr_source = ETS_I2S1_INTR_SOURCE;
      idx_base = I2S1O_WS_OUT_IDX;
      dport_clk_en = DPORT_I2S1_CLK_EN;
      dport_rst = DPORT_I2S1_RST;
    }
#endif

    DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, dport_clk_en);
    DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, dport_rst);

    auto i2s_dev = (i2s_dev_t*)_dev;
    //Reset I2S subsystem
    i2s_dev->conf.val = I2S_TX_RESET | I2S_RX_RESET | I2S_TX_FIFO_RESET | I2S_RX_FIFO_RESET;
    i2s_dev->conf.val = _conf_reg_default;

    i2s_dev->timing.val = 0;

    //Reset DMA
    i2s_dev->lc_conf.val = I2S_IN_RST | I2S_OUT_RST | I2S_AHBM_RST | I2S_AHBM_FIFO_RST;
    i2s_dev->lc_conf.val = I2S_OUT_EOF_MODE | I2S_OUTDSCR_BURST_EN | I2S_OUT_DATA_BURST_EN;

    i2s_dev->in_link.val = 0;
    i2s_dev->out_link.val = 0;

    i2s_dev->conf1.val = I2S_TX_PCM_BYPASS;
    i2s_dev->conf2.val = I2S_LCD_EN;
    i2s_dev->conf_chan.val = 1 << I2S_TX_CHAN_MOD_S | 1 << I2S_RX_CHAN_MOD_S;

    i2s_dev->int_ena.val = 0;
    i2s_dev->int_clr.val = ~0u;
    i2s_dev->int_ena.out_eof = 1;

/* DMAディスクリプタリストの各役割、 各行先頭が消灯データ転送期間 (OE=HIGH) / ２列目以降が点灯転送停止期間 
  ↓消灯 ↓点灯 
  [19]→ 0 ↲
  [20]→ 1 ↲
  [21]→ 2 ↲
  [22]→ 3 ↲
  [23]→ 4 ↲
  [24]→ 5→ 6 ↲
  [25]→ 7→ 8→ 9→10 ↲
  [26]→11→12→13→14→15→16→17→18↲(EOF,次ライン)
   色深度8を再現するために、各ビットに対応した点灯を行うため同一ラインに8回データを送る。
   8回の点灯期間は、1回進む毎に点灯期間が前回の2倍になる。
   後半の点灯期間がとても長くなるため、DMAディスクリプタを複数設定する。
   全ての点灯期間はメモリ上の同一地点を利用しメモリを節約している。
*/
    static constexpr const uint8_t dma_link_idx_tbl[] = {
      20,21,22,23,24,6,25,8,9,10,26,12,13,14,15,16,17,18,19,0,1,2,3,4,5,7,11
    };


    // (データ転送期間8回 + 点灯期間19回 = 27) * 2ライン分
    if (_dmadesc) heap_caps_free(_dmadesc);
    _dmadesc = (lldesc_t*)heap_caps_malloc(sizeof(lldesc_t) * TOTAL_PERIOD_COUNT * 2, MALLOC_CAP_DMA);

    uint32_t chain_length = _panel_width;// + 2);

    size_t buf_bytes = chain_length * (TRANSFER_PERIOD_COUNT + 1) * sizeof(uint16_t);
    for (int i = 0; i < 2; i++) {
      // ラインバッファ確保。 点灯期間1回分 + データ転送期間 8回分の合計9回分を連続領域として確保する
      // 点灯期間は合計34回あるが、同じ領域を使い回しできるので1回分でよい
      _dma_buf[i] = (uint16_t*)heap_alloc_dma(buf_bytes);
      if (_dma_buf[i] == nullptr) {
        ESP_EARLY_LOGE("Bus_HUB75", "memory allocate error.");
      }

      memset(_dma_buf[i], 0x40, buf_bytes); // OE(消灯)で埋める

      // ディスクリプタリストの先頭に点灯期間19回分のDMA情報を纏めて配置する
      for (int j = 0; j < TOTAL_PERIOD_COUNT; j++) {
        uint32_t idx = i * TOTAL_PERIOD_COUNT + j;
        int bufidx = chain_length * (j - (SHINE_PERIOD_COUNT - 1));
        if (bufidx < 0) bufidx = 0;
        bool eof = j == (SHINE_PERIOD_COUNT - 1);
        _dmadesc[idx].buf = (volatile uint8_t*)&(_dma_buf[i][bufidx]);
        _dmadesc[idx].eof = eof; // 最後の点灯期間のみEOFイベントを発生させる
        _dmadesc[idx].empty = (uint32_t)(&_dmadesc[dma_link_idx_tbl[j] + (i ^ eof) * TOTAL_PERIOD_COUNT]);
        _dmadesc[idx].owner = 1;
        _dmadesc[idx].length = chain_length * sizeof(uint16_t);
        _dmadesc[idx].size = chain_length * sizeof(uint16_t);
      }
    }
    setBrightness(_brightness);
    if (esp_intr_alloc(intr_source, ESP_INTR_FLAG_LEVEL1 | ESP_INTR_FLAG_IRAM,
        i2s_intr_handler_hub75, this, &_isr_handle) != ESP_OK) {
      ESP_EARLY_LOGE("Bus_HUB75","esp_intr_alloc failure ");
      return false;
    }
    ESP_EARLY_LOGV("Bus_HUB75","esp_intr_alloc success ");

    return true;
  }

  __attribute__((always_inline))
  static inline uint32_t _gcd(uint32_t a, uint32_t b)
  {
    uint32_t c = a % b;
    while (c != 0) {
      a = b;
      b = c;
      c = a % b;
    }
    return b;
  }

  static uint32_t getClockDivValue(uint32_t baseClock, uint32_t targetFreq)
  {
    uint32_t n = baseClock / targetFreq;
    if (n > 255) { n = 255; }
    uint32_t a = 1;
    uint32_t b = 0;

    // div_nが小さい場合に小数成分を含めると誤動作するのでここ除外する
    if (n > 4)
    {
      uint32_t delta_hz = baseClock - targetFreq * n;
      if (delta_hz) {
        uint32_t gcd = _gcd(targetFreq, delta_hz);
        a = targetFreq / gcd;
        b = delta_hz / gcd;
        uint32_t d = a / 63 + 1;
        a /= d;
        b /= d;
      }
    }

    return       I2S_CLK_EN
          | a << I2S_CLKM_DIV_A_S
          | b << I2S_CLKM_DIV_B_S
          | n << I2S_CLKM_DIV_NUM_S
          ;
  }

  void Bus_HUB75::setBrightness(uint8_t brightness)
  {
    _brightness = brightness;
    if (_dmadesc == nullptr) { return; }
    int br = brightness + 1;
    br = (br * br);
    auto panel_width = _panel_width;
    uint32_t slen = (panel_width * br) >> 12;
    uint32_t total_len = (panel_width + 1) * TRANSFER_PERIOD_COUNT;
    // int32_t dsclen_shifter = 5;
    int32_t dsclen_shifter = 4;

    auto i2s_dev = (i2s_dev_t*)_dev;
    uint32_t int_ena = i2s_dev->int_ena.val;
    i2s_dev->int_ena.val = 0;
    i2s_dev->int_clr.val = int_ena;

    // 点灯期間のDMAディスクリプタのインデクスを順に処理する
    // for (uint8_t i: { 18, 10, 6, 4, 3, 2, 1, 0} )
    for (uint8_t i: { 11, 7, 5, 4, 3, 2, 1, 0} )
    {
      if (--dsclen_shifter < 0) { dsclen_shifter = 0; }
      uint_fast8_t dsclen = 1 << dsclen_shifter;
      slen >>= 1;
      uint32_t shine_len = slen;
      uint32_t div_len = shine_len >> dsclen_shifter;
      uint32_t fraction = shine_len - (div_len << dsclen_shifter);
      for (int k = 0; k < dsclen; ++k)
      {
        uint32_t len = div_len;
        if (k && fraction) { --fraction; ++len; }
        if (len > panel_width-2) { len = panel_width-2; }
        shine_len -= len;
        // 点灯期間の先頭2Byteのみ不点灯扱いとなっている (輝度0に対応するため)
        len += 2;
        total_len += len;
        len *= sizeof(uint16_t);
        for (int j = 0; j < 2; ++j) {
          uint32_t idx = j * TOTAL_PERIOD_COUNT + i + k;
          auto dsc = _dmadesc[idx];
          dsc.size = len;
          dsc.length = (len + 3) & ~3;
          _dmadesc[idx] = dsc;
        }
      }
    }

    // 総転送データ量とリフレッシュレートに基づいて送信クロックを設定する
    uint32_t freq_write = (total_len * _panel_height * _cfg.refresh_rate) >> 1;
    uint32_t pll_d2_clock = 80 * 1000 * 1000;
    i2s_dev->clkm_conf.val = getClockDivValue(pll_d2_clock, freq_write);

    i2s_dev->int_ena.val = int_ena;
  }

  void Bus_HUB75::release(void)
  {
    endTransaction();
    if (_dmadesc)
    {
      heap_caps_free(_dmadesc);
      _dmadesc = nullptr;
    }
    for (int i = 0; i < 2; i++) {
      if (_dma_buf[i])
      {
        heap_free(_dma_buf[i]);
        _dma_buf[i] = nullptr;
      }
    }
  }

  void Bus_HUB75::beginTransaction(void)
  {
    auto i2s_dev = (i2s_dev_t*)_dev;
    i2s_dev->out_link.val = 0;
    i2s_dev->fifo_conf.val = _fifo_conf_dma;
    i2s_dev->sample_rate_conf.val = _sample_rate_conf_reg_direct;

    esp_intr_enable(_isr_handle);
    i2s_dev->int_clr.val = ~0u;

    i2s_dev->conf.val = _conf_reg_reset;
    i2s_dev->out_link.val = I2S_OUTLINK_START | ((uint32_t)&_dmadesc[0] & I2S_OUTLINK_ADDR);
    i2s_dev->conf.val = _conf_reg_start;
  }

  void Bus_HUB75::endTransaction(void)
  {
    esp_intr_disable(_isr_handle);

    auto i2s_dev = (i2s_dev_t*)_dev;
    i2s_dev->out_link.stop = 1;
    i2s_dev->conf.val = _conf_reg_reset;
    i2s_dev->out_link.val = 0;

    i2s_dev->int_clr.val = ~0u;
  }

  void IRAM_ATTR Bus_HUB75::i2s_intr_handler_hub75(void *arg)
  {
    auto me = (Bus_HUB75*)arg;
    auto dev = getDev(me->_cfg.i2s_port);
    auto st = dev->int_st.val;
    bool flg_eof = st & I2S_OUT_EOF_INT_ST;
    dev->int_clr.val = st;
    if (!flg_eof) { return; }

// DEBUG
// lgfx::gpio_hi(15);

    int y = me->_dma_y;
    auto panel_height = me->_panel_height;

    uint32_t prev_addr = y << 8 | y << 24 | _mask_oe;
    y = (y + 1) & ((panel_height>>1) - 1);
    me->_dma_y = y;

    auto desc = (lldesc_t*)dev->out_eof_des_addr;
    auto dst = (uint16_t*)desc->buf;

    uint32_t addr = y << 8 | y << 24;

    auto panel_width = me->_panel_width;
    const uint32_t chain_length = panel_width; // + 2);
    // 点灯期間のYアドレス設定
    memset(dst, y, chain_length * sizeof(uint16_t));
    auto d32 = (uint32_t*)dst;
    // for (int i = 1; i < chain_length >> 1; ++i) {
    //   d32[i] = addr;
    // }
    // 点灯期間の先頭2ByteのみOE不点灯とする
    addr |= _mask_oe;
    d32[0] = addr;

    // dst[0] = addr; // 点灯期間の先頭2ByteのみOE不点灯とする
    // dst[1] = addr;

    dst += chain_length;

    // 意図的にRAMにテーブルを展開させる, constexprは付与しない;
    uint8_t gamma_tbl[] =
    {
        0,   1,   2,   3,   4,   5,   6,   7,
        8,  10,  12,  13,  15,  17,  19,  21,
       24,  26,  29,  31,  34,  37,  40,  43,
       46,  49,  53,  56,  60,  64,  67,  71,
       75,  80,  84,  88,  93,  97, 102, 107,
      112, 117, 122, 127, 133, 138, 144, 149,
      155, 161, 167, 173, 179, 186, 192, 199,
      205, 212, 219, 226, 233, 241, 248, 255
    };

    auto src1 = &((uint16_t*)(me->_frame_buffer))[y * panel_width];
    auto src2 = &src1[panel_width * (panel_height>>1)];

    // uint32_t x = panel_width;
    auto d = (uint32_t*)dst;
    auto s1 = (uint32_t*)src1;
    auto s2 = (uint32_t*)src2;
    auto len = chain_length >> 1;
    uint32_t x = len;
    do {
      uint32_t rgb1 = *s1++;
      uint32_t rgb2 = *s2++;
      uint32_t rgb3 = rgb1 >> 16;
      uint32_t rgb4 = rgb2 >> 16;

      uint32_t b1 =  rgb1 >> 7;
      uint32_t b2 =  rgb2 >> 7;
      uint32_t b3 =  rgb3 >> 7;
      uint32_t b4 =  rgb4 >> 7;
      uint32_t g1 =  rgb1 & 0x07;
      uint32_t g2 =  rgb2 & 0x07;
      uint32_t g3 =  rgb3 & 0x07;
      uint32_t g4 =  rgb4 & 0x07;
      uint32_t r1 =  rgb1 >> 2;
      uint32_t r2 =  rgb2 >> 2;
      uint32_t r3 =  rgb3 >> 2;
      uint32_t r4 =  rgb4 >> 2;
      g1 = (0x07 & (rgb1 >> 13)) + (g1 << 3);
      g2 = (0x07 & (rgb2 >> 13)) + (g2 << 3);
      g3 =         (rgb3 >> 13)  + (g3 << 3);
      g4 =         (rgb4 >> 13)  + (g4 << 3);
      b1 &= 0x3E;
      b2 &= 0x3E;
      b3 &= 0x3E;
      b4 &= 0x3E;
      g1 &= 0x3F;
      g2 &= 0x3F;
      g3 &= 0x3F;
      g4 &= 0x3F;
      r1 &= 0x3E;
      r2 &= 0x3E;
      r3 &= 0x3E;
      r4 &= 0x3E;

      b3 = gamma_tbl[b3];
      b4 = gamma_tbl[b4];
      b1 = gamma_tbl[b1];
      b2 = gamma_tbl[b2];
      g3 = gamma_tbl[g3];
      g4 = gamma_tbl[g4];
      g1 = gamma_tbl[g1];
      g2 = gamma_tbl[g2];
      r3 = gamma_tbl[r3];
      r4 = gamma_tbl[r4];
      r1 = gamma_tbl[r1];
      r2 = gamma_tbl[r2];

      uint32_t bbbb = (b1 << 16) + (b2 << 24) + b3 + (b4 <<  8);
      uint32_t gggg = (g1 << 16) + (g2 << 24) + g3 + (g4 <<  8);
      uint32_t rrrr = (r1 << 16) + (r2 << 24) + r3 + (r4 <<  8);

      int32_t i = TRANSFER_PERIOD_COUNT;
      uint32_t mask = 0x80808080u;
      do
      {
        --i;
        uint32_t b = bbbb & mask;
        uint32_t g = gggg & mask;
        uint32_t r = rrrr & mask;
        b >>= i;
        g >>= i;
        r >>= i;
        uint32_t rgb = r + (g << 1) + (b << 2);
        mask >>= 1;
        rgb += (rgb >> 5);
        d[i * len] = addr | (rgb & 0x3F003F);
      } while (i);
      ++d;
    } while (--x);

    // // 前のラインの色が映り込まないよう、先頭のみ前回のアドレス値を指定する
    // for (int i = 0; i < 8; ++i) {
    //   dst[i] = (dst[i] & 0x3F) | prev_addr;
    // }

    dst = &dst[(chain_length - 1) ^ 1];
    for (int i = 0; i < TRANSFER_PERIOD_COUNT; ++i)
    {
      // 各転送期間の末尾にLATを指定する
      dst[i * chain_length] |= _mask_lat;
    }

// DEBUG
// lgfx::gpio_lo(15);
  }
//*/

//----------------------------------------------------------------------------
 }
}

#endif
#endif
