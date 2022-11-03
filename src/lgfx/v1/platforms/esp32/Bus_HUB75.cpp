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

  uint32_t* Bus_HUB75::_gamma_tbl = nullptr;

  void Bus_HUB75::setImageBuffer(void* buffer)
  {
    auto fb = (DividedFrameBuffer*)buffer;
    _frame_buffer = fb;
    _panel_width = fb->getLineSize() >> 1;
    _panel_height = fb->getTotalLines();
  }

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

    for (size_t i = 0; i < 14; ++i)
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

/* DMAディスクリプタリストの各役割、 各行先頭がデータ転送期間、２列目以降が拡張点灯期間 
  [0]  (EOF)↲
  [1] →[ 9]→[10]→[11]→[12]→[13]→[14]→[15]→[16] ↲
  [2] →[17]→[18]→[19] ↲
  [3] →[20] ↲
  [4] ↲
  [5] ↲
  [6] ↲
  [7] ↲
  [8] ↲(次ライン)

  ※ ディスクリプタ[0]と[9]は色信号なし、転送期間半分。
      [0]は前回データに対する点灯のみ行う期間 
     ディスクリプタ[1]のデータ転送は同時にSHIFTREG_ABCの転送期間を兼ねる

   色深度8を再現するために、各ビットに対応した点灯を行うため同一ラインに8回データを送る。
   [1]~[8]の8回の点灯期間は、1回進む毎に点灯期間(OE LOW)が前回の半分になる。
   前半の点灯期間はとても長いため、データ転送をせず点灯のみを行う拡張点灯期間を設ける。
   全ての拡張点灯期間は同一のメモリ範囲を利用することでメモリを節約している。
*/

    static constexpr const uint8_t dma_link_idx_tbl[] = {
      1, 9, 17, 20, 5, 6, 7, 8, 0, 10, 11, 12, 13, 14, 15, 16, 2, 18, 19, 3, 4,
    };

    // (データ転送期間8回 + Y座標変更期間1回 + 拡張点灯期間11回 = 19) * 2ライン分
    if (_dmadesc) heap_caps_free(_dmadesc);
    _dmadesc = (lldesc_t*)heap_caps_malloc(sizeof(lldesc_t) * TOTAL_PERIOD_COUNT * 2, MALLOC_CAP_DMA);

    uint32_t panel_width = _panel_width;

    // ラインバッファ確保。 データ転送期間8回分 + 拡張点灯期間1回分 + Y座標変更期間1回分 の合計10回分を連続領域として確保する
    // 拡張点灯期間は合計11回あるが、同じ領域を使い回すためバッファは1回分でよい;
    static constexpr const size_t buf_linkcount = TRANSFER_PERIOD_COUNT + 1 + (LINECHANGE_HALF_PERIOD_COUNT / 2);
    size_t buf_bytes = panel_width * buf_linkcount * sizeof(uint16_t);
    for (int i = 0; i < 2; i++) {
      _dma_buf[i] = (uint16_t*)heap_alloc_dma(buf_bytes);
      if (_dma_buf[i] == nullptr) {
        ESP_EARLY_LOGE("Bus_HUB75", "memory allocate error.");
      }
      memset(_dma_buf[i], 0x01, buf_bytes); // バッファ初期値として OE(消灯)で埋めておく

      for (int j = 0; j < TOTAL_PERIOD_COUNT; j++) {
        uint32_t idx = i * TOTAL_PERIOD_COUNT + j;
        int bufidx = panel_width * (j < (buf_linkcount - 1) ? j : (buf_linkcount - 1));
        bool eof = j == 0;
        _dmadesc[idx].buf = (volatile uint8_t*)&(_dma_buf[i][bufidx]);
        _dmadesc[idx].eof = eof; // 最後の転送期間のみEOFイベントを発生させる
        _dmadesc[idx].empty = (uint32_t)(&_dmadesc[dma_link_idx_tbl[j] + (i ^ eof) * TOTAL_PERIOD_COUNT]);
        _dmadesc[idx].owner = 1;
        _dmadesc[idx].length = panel_width * sizeof(uint16_t);
        _dmadesc[idx].size = panel_width * sizeof(uint16_t);
      }
      size_t half_line_len = panel_width >> 1;
      _dmadesc[9 + (i * TOTAL_PERIOD_COUNT)].buf = (volatile uint8_t*)&(_dma_buf[i][half_line_len]);
      _dmadesc[9 + (i * TOTAL_PERIOD_COUNT)].length = half_line_len * sizeof(uint16_t);
      _dmadesc[9 + (i * TOTAL_PERIOD_COUNT)].size = half_line_len * sizeof(uint16_t);
      _dmadesc[0 + (i * TOTAL_PERIOD_COUNT)].length = half_line_len * sizeof(uint16_t);
      _dmadesc[0 + (i * TOTAL_PERIOD_COUNT)].size = half_line_len * sizeof(uint16_t);
    }
    setBrightness(_brightness);

    if (_gamma_tbl == nullptr)
    {
      static constexpr const uint8_t gamma_tbl[] =
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
      _gamma_tbl = (uint32_t*)heap_alloc_dma(sizeof(gamma_tbl) * sizeof(uint32_t));
      for (size_t i = 0; i < sizeof(gamma_tbl); ++i)
      {
        uint32_t span3bit = 0;
        for (int j = 0; j < 8; ++j)
        {
          if (gamma_tbl[i] & (1 << j))
          {
            span3bit += 1 << ((j + 2) * 3);
          }
        }
        _gamma_tbl[i] = span3bit;
      }
    }

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
    int br = brightness + 1;
    br = (br * br);
    auto panel_width = _panel_width;
    uint32_t light_len_limit = (panel_width - 8) >> 1;
    uint32_t slen = (light_len_limit * br) >> 16;

    _brightness_period[0] = panel_width >> 1;
    _brightness_period[TRANSFER_PERIOD_COUNT+1] = 2;
    for (int period = 1; period <= TRANSFER_PERIOD_COUNT; ++period)
    {
      if (period > 4) { slen >>= 1; }
      _brightness_period[period] = slen + 1;
    }
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
    if (_dmatask_handle != nullptr)
    {
      return;
    }

    auto i2s_dev = (i2s_dev_t*)_dev;
    i2s_dev->out_link.val = 0;
    i2s_dev->fifo_conf.val = _fifo_conf_dma;
    i2s_dev->sample_rate_conf.val = _sample_rate_conf_reg_direct;

    // 総転送データ量とリフレッシュレートに基づいて送信クロックを設定する
    uint32_t pll_d2_clock = 80 * 1000 * 1000;
    uint32_t period_len = TRANSFER_PERIOD_COUNT + EXTEND_PERIOD_COUNT + (LINECHANGE_HALF_PERIOD_COUNT >> 1);
    uint32_t freq_write = (period_len * _panel_width * _panel_height * _cfg.refresh_rate) >> 1;
    i2s_dev->clkm_conf.val = getClockDivValue(pll_d2_clock, freq_write);

    // esp_intr_enable(_isr_handle);
    i2s_dev->int_clr.val = ~0u;

    i2s_dev->conf.val = _conf_reg_reset;
    i2s_dev->out_link.val = I2S_OUTLINK_START | ((uint32_t)&_dmadesc[0] & I2S_OUTLINK_ADDR);
    i2s_dev->conf.val = _conf_reg_start;

    xTaskCreatePinnedToCore(dmaTask, "hub75dma", 4096, this, 18, &_dmatask_handle, PRO_CPU_NUM);
  }

  void Bus_HUB75::endTransaction(void)
  {
    auto i2s_dev = (i2s_dev_t*)_dev;
    i2s_dev->int_ena.val = 0;
    i2s_dev->int_clr.val = ~0u;
    i2s_dev->out_link.stop = 1;
    i2s_dev->conf.val = _conf_reg_reset;
    i2s_dev->out_link.val = 0;

    if (_dmatask_handle)
    {
      xTaskNotify(_dmatask_handle, 0, eNotifyAction::eSetValueWithOverwrite);
      _dmatask_handle = nullptr;
    }
  }

  void IRAM_ATTR Bus_HUB75::i2s_intr_handler_hub75(void *arg)
  {
    auto me = (Bus_HUB75*)arg;
    auto dev = getDev(me->_cfg.i2s_port);
    auto st = dev->int_st.val;
    bool flg_eof = st & I2S_OUT_EOF_INT_ST;
    dev->int_clr.val = st;
    if (flg_eof)
    {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      auto desc = (lldesc_t*)dev->out_eof_des_addr;
      xTaskNotifyFromISR(me->_dmatask_handle, (uint32_t)desc->buf, eNotifyAction::eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
      portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
  }

  void Bus_HUB75::dmaTask(void *arg)
  {
    auto me = (Bus_HUB75*)arg;
    auto dev = getDev(me->_cfg.i2s_port);

    int intr_source = ETS_I2S0_INTR_SOURCE;
    if (me->_cfg.i2s_port != I2S_NUM_0)
#if !defined (CONFIG_IDF_TARGET_ESP32S2)
    {
      intr_source = ETS_I2S1_INTR_SOURCE;
    }
#endif

    intr_handle_t _isr_handle = nullptr;

    if (esp_intr_alloc(intr_source, ESP_INTR_FLAG_LEVEL2 | ESP_INTR_FLAG_IRAM,
        i2s_intr_handler_hub75, me, &_isr_handle) != ESP_OK) {
      ESP_EARLY_LOGE("Bus_HUB75","esp_intr_alloc failure ");
      return;
    }
    ESP_EARLY_LOGV("Bus_HUB75","esp_intr_alloc success ");

    const auto panel_height = me->_panel_height;
    const auto panel_width = me->_panel_width;
    const uint32_t len32 = panel_width >> 1;
    uint_fast8_t yidx = 0;

    uint8_t bitinvert_tbl[] = { 0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30, 1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31 };
    if (panel_height <= 32)
    {
      for (size_t i = 0; i < sizeof(bitinvert_tbl); ++i) 
      bitinvert_tbl[i] >>= 1;
    }
    const uint16_t* brightness_period = me->_brightness_period;

    while (auto d32 = (uint32_t*)ulTaskNotifyTake( pdTRUE, portMAX_DELAY))
    {
// DEBUG
// lgfx::gpio_hi(15);

      yidx = (yidx + 1) & ((panel_height>>1) - 1);

      uint_fast8_t y = bitinvert_tbl[yidx];
  // uint_fast8_t y = yidx;

      uint32_t yy = y << 9 | y << 25;
      uint32_t yys[] = { yy, _mask_oe | _mask_pin_a_clk, yy, yy, yy, yy, yy, yy, yy };

      auto s1 = (uint32_t*)(me->_frame_buffer->getLineBuffer(y));
      auto s2 = (uint32_t*)(me->_frame_buffer->getLineBuffer(y + (panel_height>>1)));

      uint32_t x = 0;
      uint_fast8_t light_idx = TRANSFER_PERIOD_COUNT;
      for (;;)
      {
        // 16bit RGB565を32bit変数に2ピクセル纏めて取り込む。(画面の上半分用)
        uint32_t swap565x2_L = *s1++;
        // 画面の下半分用のピクセルも同様に2ピクセル纏めて取り込む
        uint32_t swap565x2_H = *s2++;

        // R,G,Bそれぞれの成分に分離する。2ピクセルまとめて処理することで演算回数を削減する
        uint32_t r_L2 = swap565x2_L >> 2;
        uint32_t r_H2 = swap565x2_H >> 2;
        uint32_t g_L2 = swap565x2_L >> 13;
        uint32_t g_H2 = swap565x2_H >> 13;
        uint32_t b_L2 = swap565x2_L >> 7;
        uint32_t b_H2 = swap565x2_H >> 7;
        uint32_t g_L1 = swap565x2_L & 0x070007;
        uint32_t g_H1 = swap565x2_H & 0x070007;
        r_L2 &= 0x3E003E;
        r_H2 &= 0x3E003E;
        g_L2 &= 0x070007;
        g_H2 &= 0x070007;
        b_L2 &= 0x3E003E;
        b_H2 &= 0x3E003E;
        r_L2 += r_L2 >> 5;
        r_H2 += r_H2 >> 5;
        g_L2 += g_L1 << 3;
        g_H2 += g_H1 << 3;
        b_L2 += b_L2 >> 5;
        b_H2 += b_H2 >> 5;
        uint32_t r_L1 = r_L2 & 0x3F;
        uint32_t r_H1 = r_H2 & 0x3F;
        r_L2 >>= 16;
        r_H2 >>= 16;
        g_L1 = g_L2 & 0x3F;
        g_H1 = g_H2 & 0x3F;
        g_L2 >>= 16;
        g_H2 >>= 16;
        uint32_t b_L1 = b_L2 & 0x3F;
        uint32_t b_H1 = b_H2 & 0x3F;
        b_L2 >>= 16;
        b_H2 >>= 16;

        // RGBそれぞれ64階調値を元にガンマテーブルを適用する
        // このテーブルの中身は単にガンマ補正をするだけでなく、
        // 各ビットを3bit間隔に変換する処理を兼ねている。
        // 例えば 0bABCDEFGH -> 0bA00B00C00D00E00F00G00H000 のようになる
        r_L1 = _gamma_tbl[r_L1];
        g_L1 = _gamma_tbl[g_L1];
        b_L1 = _gamma_tbl[b_L1];
        r_H1 = _gamma_tbl[r_H1];
        g_H1 = _gamma_tbl[g_H1];
        b_H1 = _gamma_tbl[b_H1];
        r_L2 = _gamma_tbl[r_L2];
        g_L2 = _gamma_tbl[g_L2];
        b_L2 = _gamma_tbl[b_L2];
        r_H2 = _gamma_tbl[r_H2];
        g_H2 = _gamma_tbl[g_H2];
        b_H2 = _gamma_tbl[b_H2];

        // テーブルから取り込んだ値は3bit間隔となっているので、
        // R,G,Bそれぞれが互いを避けるようにビットシフトすることでまとめることができる。
        uint32_t rgb_L1 = (r_L1 >> 3) + (g_L1 >> 2) + (b_L1 >> 1);
        uint32_t rgb_H1 =  r_H1       + (g_H1 << 1) + (b_H1 << 2);
        uint32_t rgb_L2 = (r_L2 >> 3) + (g_L2 >> 2) + (b_L2 >> 1);
        uint32_t rgb_H2 =  r_H2       + (g_H2 << 1) + (b_H2 << 2);

        // 上記の変数の中身は BGRBGRBGRBGR… の順にビットが並んだ状態となる
        // これを、各色の0,2,4,6ビットと1,3,5,7ビットの成分に分離する
        uint32_t rgb_L1_even = rgb_L1 & 0b000111000111000111000111000111;
        uint32_t rgb_H1_even = rgb_H1 & 0b111000111000111000111000111000;
        uint32_t rgb_L1_odd  = rgb_L1 & 0b111000111000111000111000111000;
        uint32_t rgb_H1_odd  = rgb_H1 & 0b000111000111000111000111000111;
        uint32_t rgb_L2_even = rgb_L2 & 0b000111000111000111000111000111;
        uint32_t rgb_H2_even = rgb_H2 & 0b111000111000111000111000111000;
        uint32_t rgb_L2_odd  = rgb_L2 & 0b111000111000111000111000111000;
        uint32_t rgb_H2_odd  = rgb_H2 & 0b000111000111000111000111000111;

        // パラレルで同時に送信する6bit分のRGB成分(画面の上半分と下半分)が隣接するように纏める。
        uint32_t rgb_even_1 = (rgb_L1_even + rgb_H1_even);
        uint32_t rgb_odd_1  = (rgb_L1_odd  + rgb_H1_odd) << 3;
        uint32_t rgb_even_2 = (rgb_L2_even + rgb_H2_even);
        uint32_t rgb_odd_2  = (rgb_L2_odd  + rgb_H2_odd) << 3;

        d32[0] = yys[0];
        int32_t i = 8;
        do
        {
          rgb_odd_1 >>= 6;
          rgb_odd_2 >>= 6;
          rgb_even_1 >>= 6;
          rgb_even_2 >>= 6;
          uint32_t odd_1 = rgb_odd_1 & 0x3F;
          uint32_t odd_2 = rgb_odd_2 & 0x3F;
          uint32_t even_1 = rgb_even_1 & 0x3F;
          uint32_t even_2 = rgb_even_2 & 0x3F;
          odd_1 <<= 16;
          odd_2 += yys[i];
          even_1 <<= 16;
          even_2 += yys[i-1];
          // 奇数番ビット成分を横２列ぶん同時にバッファにセットする
          d32[  i * len32] = odd_2 + odd_1;
          // 偶数番ビット成分を横２列ぶん同時にバッファにセットする
          d32[--i * len32] = even_2 + even_1;
        } while (--i != 0);

        ++d32;

        if (++x < brightness_period[light_idx]) { continue; }
        if (light_idx == 0) break;
        do {
          yys[(light_idx != TRANSFER_PERIOD_COUNT) ? light_idx + 1 : 0] |= _mask_oe;
        } while (x >= brightness_period[--light_idx]);
      }

      // 転送期間の先頭に非表示OEを設定
      d32[ - len32     ] |= _mask_oe;

      {
        uint_fast8_t y3 = y&3;
        int32_t idx = - ((len32>>1) + y3);

        while (y3 --)
        {
          d32[idx] = (d32[idx] & ~(_mask_pin_a_clk << 16)) | _mask_pin_a_clk;
          ++idx;
        }

        // SHIFTREG_ABCの誤作動防止
        d32[idx] = (d32[idx] & ~_mask_pin_c_dat)
                 | _mask_pin_a_clk << 16
                 | _mask_pin_a_clk;
      }

      d32 += len32;

      // SHIFTREG_ABCの転送の末尾のLAT信号
      d32[- 1] |= _mask_lat | _mask_pin_b_lat;

      for (int i = 1; i < TRANSFER_PERIOD_COUNT; ++i)
      {
        // 各転送期間の先頭に OE を設定
        d32[i * len32    ] |= _mask_oe;
        // 各転送期間の末尾に LAT を設定
        d32[i * len32 - 1] |= _mask_lat;
      }

      // SHIFTREG_ABCのY座標情報をセットする
      d32[ - (y + 1                      )] |= _mask_pin_c_dat;
      d32[ - (y + 1 + (panel_height >> 1))] |= _mask_pin_c_dat;
      d32[ - (y                          )] |= _mask_pin_c_dat << 16;
      d32[ - (y     + (panel_height >> 1))] |= _mask_pin_c_dat << 16;
      d32 += len32 * (TRANSFER_PERIOD_COUNT - 1);

      {
      // 拡張点灯期間の内容を設定する
        uint32_t light_len = brightness_period[1] - 1;
        memset(&d32[0], y << 1 | _mask_oe >> 8, sizeof(uint32_t) * (len32 - light_len));
        memset(&d32[len32 - light_len], y << 1  , sizeof(uint32_t) * (light_len));
      }
// DEBUG
  // lgfx::gpio_lo(15);
    }
  }
//*/

//----------------------------------------------------------------------------
 }
}

#endif
#endif
