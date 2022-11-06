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
#include <esp_log.h>

#if defined ( ESP_IDF_VERSION_VAL )
 #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
  #define LGFX_IDF_V5
 #endif
#endif

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
#if SOC_I2C_NUM > 1
    return (port == 0) ? &I2S0 : &I2S1;
#else
    return &I2S0;
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

#if defined ( LGFX_IDF_V5 )
      esp_rom_gpio_pad_select_gpio(_cfg.pin_data[i]);
      esp_rom_gpio_connect_out_signal(_cfg.pin_data[i  ], idx_base + i, 0, 0);
#else
      gpio_pad_select_gpio(_cfg.pin_data[i]);
      gpio_matrix_out(_cfg.pin_data[i  ], idx_base + i, 0, 0);
#endif
    }

    idx_base = (_cfg.i2s_port == I2S_NUM_0) ? I2S0O_WS_OUT_IDX : I2S1O_WS_OUT_IDX;
#if defined ( LGFX_IDF_V5 )
    esp_rom_gpio_connect_out_signal(_cfg.pin_clk, idx_base, 1, 0); // clock Active-low
#else
    gpio_matrix_out(_cfg.pin_clk, idx_base, 1, 0); // clock Active-low
#endif

    uint32_t dport_clk_en;
    uint32_t dport_rst;

    if (_cfg.i2s_port == I2S_NUM_0) {
      idx_base = I2S0O_WS_OUT_IDX;
      dport_clk_en = DPORT_I2S0_CLK_EN;
      dport_rst = DPORT_I2S0_RST;
    }
#if SOC_I2C_NUM > 1
    else
    {
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

/*  // DMAディスクリプタが利用するDMAメモリ位置テーブル
 この配列は、14x2セットのDMAディスクリプタそれぞれが使用するバッファメモリの範囲を表す。
  0 : 無データ,x1点灯
  1 : SHIFTREG_ABC座標,無灯
  2 : 輝度1/32データ,無灯
  3 : 輝度1/16データ,1/32点灯
  4 : 輝度1/ 8データ,1/16点灯
  5 : 輝度1/ 4データ,1/8点灯
  6 : 輝度1/ 2データ,1/4点灯
  7 : 輝度 x 1データ,1/2点灯
  8 : 輝度 x 2データ, x1点灯
  9 : 輝度 x 4データ, x1点灯
*/
    static constexpr const uint8_t dma_buf_idx_tbl[] = {
      1, 2, 3, 4, 5, 6, 7, 8, 0, 9, 0, 0, 0, 0, 
    };

/* DMAディスクリプタの各役割は以下の通り
  [ 0](SHIFTREG_ABC座標転送,無灯期間)
  [ 1](輝度1/32成分データ転送,無灯期間)
  [ 2](輝度1/16成分データ転送,1/32点灯期間)
  [ 3](輝度1/ 8成分データ転送,1/16点灯期間)
  [ 4](輝度1/ 4成分データ転送,1/ 8点灯期間)
  [ 5](輝度1/ 2成分データ転送,1/ 4点灯期間)
  [ 6](輝度1/ 1成分データ転送,1/ 2点灯期間)
  [ 7](輝度  x2成分データ転送,1/ 1点灯期間)
  [ 8](            データ無し,  x1点灯期間)
  [ 9](輝度  x4成分データ転送,  x1点灯期間)
  [10](            データ無し,  x1点灯期間)
  [11](            データ無し,  x1点灯期間)
  [12](            データ無し,  x1点灯期間)
  [13](            データ無し,  x1点灯期間)
  ※ 13番の転送が終わったあとは2セットあるディスクリプタ群の先頭にリンクする。
     また、13番の転送が終わった時点でEOF割込みが起こり、次のラインのデータ生成タスクが実行される。

   色深度8を再現するために、同一ラインに輝度成分の異なるデータを8回を送る。
   後半の輝度x2,x4のデータに対しては点灯期間が長いため、x1点灯期間を複数設けることで輝度差を実現する。
   "データ無しx1点灯期間"で送信する内容はすべて同一で良いため、同じメモリ範囲を共有利用してメモリを節約している。
*/

    // (データ転送期間8回 + SHIFTREG_ABC座標期間1回 + 拡張点灯期間5回 = 14) * 2セット分
    if (_dmadesc) heap_caps_free(_dmadesc);
    _dmadesc = (lldesc_t*)heap_caps_malloc(sizeof(lldesc_t) * TOTAL_PERIOD_COUNT * _dma_desc_set, MALLOC_CAP_DMA);

    uint32_t panel_width = _panel_width;

    // DMA用バッファメモリ確保。 無データ点灯期間1回分 + データ転送期間8回分 + SHIFTREG_ABC座標期間1回 の合計10回分を連続領域として確保する
    // 拡張点灯期間は合計11回あるが、同じ領域を使い回すためバッファは1回分でよい;
    static constexpr const size_t buf_linkcount = TRANSFER_PERIOD_COUNT + LINECHANGE_PERIOD_COUNT + 1;
    size_t buf_bytes = panel_width * buf_linkcount * sizeof(uint16_t);
    for (size_t i = 0; i < _dma_desc_set; i++) {
      _dma_buf[i] = (uint16_t*)heap_alloc_dma(buf_bytes);
      if (_dma_buf[i] == nullptr) {
        ESP_EARLY_LOGE("Bus_HUB75", "memory allocate error.");
      }
      for (size_t j = 0; j < (buf_bytes >> 1); ++j)
      { // バッファ初期値として OE(消灯)で埋めておく
        _dma_buf[i][j] = (uint16_t) _mask_oe;
      }

      for (int j = 0; j < TOTAL_PERIOD_COUNT; j++) {
        uint32_t idx = i * TOTAL_PERIOD_COUNT + j;
        size_t bufidx = dma_buf_idx_tbl[j] * panel_width;
        _dmadesc[idx].buf = (volatile uint8_t*)&(_dma_buf[i][bufidx]);
        _dmadesc[idx].eof = j == (TOTAL_PERIOD_COUNT - 1); // 最後の転送期間のみEOFイベントを発生させる
        _dmadesc[idx].empty = (uint32_t)(&_dmadesc[(idx + 1) % (TOTAL_PERIOD_COUNT * _dma_desc_set)]);
        _dmadesc[idx].owner = 1;
        _dmadesc[idx].length = panel_width * sizeof(uint16_t);
        _dmadesc[idx].size = panel_width * sizeof(uint16_t);
      }
    }
    setBrightness(_brightness);

    if (_gamma_tbl == nullptr)
    {
      static constexpr const uint8_t gamma_tbl[] =
      {
          0,   1,   1,   2,   2,   3,   4,   5,
          6,   7,   8,   9,  11,  12,  14,  16,
         18,  20,  23,  25,  28,  30,  33,  36,
         39,  42,  46,  49,  53,  56,  60,  64,
         68,  72,  77,  81,  86,  90,  95, 100,
        105, 110, 116, 121, 127, 132, 138, 144,
        150, 156, 163, 169, 176, 182, 189, 196,
        203, 210, 218, 225, 233, 240, 248, 255
      };



      _gamma_tbl = (uint32_t*)heap_alloc_dma(sizeof(gamma_tbl) * sizeof(uint32_t));
      for (size_t i = 0; i < sizeof(gamma_tbl); ++i)
      {
        uint32_t span3bit = 0;
        for (int j = 0; j < 8; ++j)
        {
          if (gamma_tbl[i] & (1 << j))
          {
            span3bit += 1 << (j * 3 + 5);
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

  static uint32_t getClockDivValue(uint32_t targetFreq)
  {
    // ToDo:get from APB clock.
    uint32_t baseClock = 80 * 1000 * 1000;
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
// ESP_EARLY_LOGE("DEBUG","brightness:%d", brightness);
    _brightness = brightness;
    int br = brightness + 1;
    br = (br * br);
    auto panel_width = _panel_width;
    uint32_t light_len_limit = (panel_width - 8);
    uint32_t slen = (light_len_limit * br) >> 16;

    _brightness_period[TRANSFER_PERIOD_COUNT] = panel_width;
    // _brightness_period[0] = 0;
    for (int period = TRANSFER_PERIOD_COUNT - 1; period >= 0; --period)
    {
      if (period <= 5) { slen >>= 1; }
      _brightness_period[period] = slen + 3;
// ESP_EARLY_LOGE("DEBUG","period%d  = %d", period, slen);
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
    if (_task_running == true)
    {
      return;
    }
    _task_running = true;

#if portNUM_PROCESSORS > 1
    if (((size_t)_cfg.task_pinned_core) < portNUM_PROCESSORS)
    {
      xTaskCreatePinnedToCore(dmaTask, "hub75dma", 2048, this, _cfg.task_priority, &_dmatask_handle, _cfg.task_pinned_core);
    }
    else
#endif
    {
      xTaskCreate(dmaTask, "hub75dma", 2048, this, _cfg.task_priority, &_dmatask_handle);
    }

    auto dev = (i2s_dev_t*)_dev;


    if (_cfg.initialize_mode == config_t::initialize_mode_t::initialize_fm6124)
    {
      // LEDドライバの輝度設定レジスタ操作
      static constexpr const uint8_t fm6124_param_reg[2][16] =
      { { 0, 0, 0, 0, 0,    0, 0x3F, 0x3F,    0, 0x3F, 0x3F,  0, 0, 0, 0, 0 } // REG 11
      , { 0, 0, 0, 0, 0,    0,    0,    0,    0, 0x3F,    0,  0, 0, 0, 0, 0 } // REG 12
      };
      for (size_t i = 0; i < _panel_width; ++i)
      {
        for (size_t j = 0; j < 2; ++j)
        {
          _dma_buf[0][_panel_width * (6 + j) + (i ^ 1)] = (uint16_t)(fm6124_param_reg[j][i & 15] | _mask_oe | (i >= (_panel_width - (11+j)) ? _mask_lat : 0));
        }
      }
    }

    dev->out_link.val = 0;
    dev->fifo_conf.val = _fifo_conf_dma;
    dev->sample_rate_conf.val = _sample_rate_conf_reg_direct;

    dev->clkm_conf.val = getClockDivValue(400000);
    dev->int_clr.val = ~0u;

    dev->conf.val = _conf_reg_reset;
    dev->out_link.val = I2S_OUTLINK_START | ((uint32_t)_dmadesc & I2S_OUTLINK_ADDR);
    dev->conf.val = _conf_reg_start;
  }

  void Bus_HUB75::endTransaction(void)
  {
    _task_running = false;
    if (_dmatask_handle)
    {
      xTaskNotify(_dmatask_handle, 0, eNotifyAction::eSetValueWithOverwrite);
      _dmatask_handle = nullptr;
    }

    auto i2s_dev = (i2s_dev_t*)_dev;
    i2s_dev->int_ena.val = 0;
    i2s_dev->int_clr.val = ~0u;
    i2s_dev->out_link.stop = 1;
    i2s_dev->conf.val = _conf_reg_reset;
    i2s_dev->out_link.val = 0;
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
      auto desc = (lldesc_t*)dev->out_eof_des_addr;
      xTaskNotifyFromISR(me->_dmatask_handle, (uint32_t)desc->buf, eNotifyAction::eSetValueWithOverwrite, nullptr);
      portYIELD_FROM_ISR();
    }
  }

  void Bus_HUB75::dmaTask(void *arg)
  {
    auto me = (Bus_HUB75*)arg;
    auto dev = getDev(me->_cfg.i2s_port);

    int intr_source = ETS_I2S0_INTR_SOURCE;
#if SOC_I2C_NUM > 1
    if (me->_cfg.i2s_port != I2S_NUM_0)
    {
      intr_source = ETS_I2S1_INTR_SOURCE;
    }
#endif
    if (esp_intr_alloc(intr_source, ESP_INTR_FLAG_LEVEL2 | ESP_INTR_FLAG_IRAM,
        i2s_intr_handler_hub75, me, &(me->_isr_handle)) != ESP_OK) {
      ESP_EARLY_LOGE("Bus_HUB75","esp_intr_alloc failure ");
      return;
    }

    ESP_EARLY_LOGV("Bus_HUB75","esp_intr_alloc success ");

    ulTaskNotifyTake( pdFALSE, portMAX_DELAY);

    const auto panel_width = me->_panel_width;

    for (int i = 0; i < panel_width; ++i)
    {
      me->_dma_buf[0][panel_width * 7 + i] = (uint16_t)_mask_oe;
      me->_dma_buf[0][panel_width * 6 + i] = (uint16_t)_mask_oe;
    }

    const auto panel_height = me->_panel_height;

    // 総転送データ量とリフレッシュレートに基づいて送信クロックを設定する
    uint32_t freq_write = (TOTAL_PERIOD_COUNT * panel_width * panel_height * me->_cfg.refresh_rate) >> 1;
    dev->clkm_conf.val = getClockDivValue(freq_write);

    const uint32_t len32 = panel_width >> 1;
    uint_fast8_t y = 0;

    const uint16_t* brightness_period = me->_brightness_period;

    while (me->_task_running)
    {
      auto dst = (uint32_t*)ulTaskNotifyTake( pdTRUE, portMAX_DELAY);
      if (dst == nullptr) { break; }
      auto d32 = dst;
// DEBUG
// lgfx::gpio_hi(15);

      y = (y + 1) & ((panel_height>>1) - 1);

      uint32_t yy = 0;
      if (me->_cfg.address_mode == config_t::address_mode_t::address_binary)
      {
        yy = y << 9 | y << 25;
      }
      uint32_t yys[] = { _mask_pin_a_clk | _mask_oe, yy | _mask_oe, yy, yy, yy, yy, yy, yy, yy, yy, };
      // yy |= _mask_oe;

      auto s1 = (uint32_t*)(me->_frame_buffer->getLineBuffer(y));
      auto s2 = (uint32_t*)(me->_frame_buffer->getLineBuffer(y + (panel_height>>1)));

      uint_fast8_t light_idx = 0;
      uint32_t x = 0;
      uint32_t xe = brightness_period[1];
      for (;;)
      {
        // 16bit RGB565を32bit変数に2ピクセル纏めて取り込む。(画面の上半分用)
        uint32_t swap565x2_L = *s1++;
        // 画面の下半分用のピクセルも同様に2ピクセル纏めて取り込む
        uint32_t swap565x2_H = *s2++;

        // R,G,Bそれぞれの成分に分離する。2x2=4ピクセルまとめて処理することで演算回数を削減する
        uint32_t r_L1 = swap565x2_L & 0xF800F8;
        uint32_t r_H1 = swap565x2_H & 0xF800F8;
        uint32_t g_L2 = swap565x2_L >> 13;
        uint32_t g_H2 = swap565x2_H >> 13;
        uint32_t b_L1 = swap565x2_L & 0x1F001F00;
        uint32_t b_H1 = swap565x2_H & 0x1F001F00;
        uint32_t r_L2 = r_L1 >> 7;
        uint32_t r_H2 = r_H1 >> 7;
        uint32_t g_L1 = swap565x2_L & 0x070007;
        uint32_t g_H1 = swap565x2_H & 0x070007;
        g_L2 &= 0x070007;
        g_H2 &= 0x070007;
        uint32_t b_L2 = b_L1 >> 12;
        uint32_t b_H2 = b_H1 >> 12;
        r_L2 += r_L1 >> 2;
        r_H2 += r_H1 >> 2;
        g_L2 += g_L1 << 3;
        g_H2 += g_H1 << 3;
        b_L2 += b_L1 >> 7;
        b_H2 += b_H1 >> 7;

        r_L1 = r_L2 & 0x3F;
        r_H1 = r_H2 & 0x3F;
        r_L2 >>= 16;
        r_H2 >>= 16;
        g_L1 = g_L2 & 0x3F;
        g_H1 = g_H2 & 0x3F;
        g_L2 >>= 16;
        g_H2 >>= 16;
        b_L1 = b_L2 & 0x3F;
        b_H1 = b_H2 & 0x3F;
        b_L2 >>= 16;
        b_H2 >>= 16;

        // RGBそれぞれ64階調値を元にガンマテーブルを適用する
        // このテーブルの中身は単にガンマ補正をするだけでなく、
        // 各ビットを3bit間隔に変換する処理を兼ねている。
        // 具体的には  0bABCDEFGH  ->  0bA__B__C__D__E__F__G__H_____ のようになる
        r_L1 = _gamma_tbl[r_L1];
        r_H1 = _gamma_tbl[r_H1];
        r_L2 = _gamma_tbl[r_L2];
        r_H2 = _gamma_tbl[r_H2];
        g_L1 = _gamma_tbl[g_L1];
        g_H1 = _gamma_tbl[g_H1];
        g_L2 = _gamma_tbl[g_L2];
        g_H2 = _gamma_tbl[g_H2];
        b_L1 = _gamma_tbl[b_L1];
        b_H1 = _gamma_tbl[b_H1];
        b_L2 = _gamma_tbl[b_L2];
        b_H2 = _gamma_tbl[b_H2];

        // テーブルから取り込んだ値は3bit間隔となっているので、
        // R,G,Bそれぞれが互いを避けるようにビットシフトすることでまとめることができる。
        g_L1 += r_L1 >> 1;
        g_H1 += r_H1 >> 1;
        g_L2 += r_L2 >> 1;
        g_H2 += r_H2 >> 1;
        b_L1 += g_L1 >> 1;
        b_H1 += g_H1 >> 1;
        b_L2 += g_L2 >> 1;
        b_H2 += g_H2 >> 1;

        uint32_t rgb_L1 = b_L1;
        uint32_t rgb_H1 = b_H1 << 3;
        uint32_t rgb_L2 = b_L2;
        uint32_t rgb_H2 = b_H2 << 3;

        // 上記の変数の中身は BGRBGRBGRBGR… の順にビットが並んだ状態となる
        // これを、各色の0,2,4,6ビットと1,3,5,7ビットの成分に分離する
        uint32_t rgb_L1_even = rgb_L1 & 0b11000111000111000111000111000111;
        uint32_t rgb_L1_odd  = rgb_L1 & 0b00111000111000111000111000111000;
        uint32_t rgb_H1_even = rgb_H1 & 0b00111000111000111000111000111000;
        uint32_t rgb_H1_odd  = rgb_H1 & 0b11000111000111000111000111000111;
        uint32_t rgb_L2_even = rgb_L2 & 0b11000111000111000111000111000111;
        uint32_t rgb_L2_odd  = rgb_L2 & 0b00111000111000111000111000111000;
        uint32_t rgb_H2_even = rgb_H2 & 0b00111000111000111000111000111000;
        uint32_t rgb_H2_odd  = rgb_H2 & 0b11000111000111000111000111000111;

        // パラレルで同時に送信する6bit分のRGB成分(画面の上半分と下半分)が隣接するように纏める。
        uint32_t rgb_even_1 = (rgb_L1_even + rgb_H1_even);
        uint32_t rgb_odd_1  = (rgb_L1_odd  + rgb_H1_odd) << 3;
        uint32_t rgb_even_2 = (rgb_L2_even + rgb_H2_even);
        uint32_t rgb_odd_2  = (rgb_L2_odd  + rgb_H2_odd) << 3;

        d32[len32 * 0] = yys[TRANSFER_PERIOD_COUNT-1];
        d32[len32 * 1] = yys[0];
        int32_t i = 1;
        do
        {
          rgb_odd_2 >>= 6;
          rgb_odd_1 >>= 6;
          rgb_even_2 >>= 6;
          rgb_even_1 >>= 6;
          uint32_t odd_2 = rgb_odd_2 & 0x3F;
          uint32_t odd_1 = rgb_odd_1 & 0x3F;
          uint32_t even_2 = rgb_even_2 & 0x3F;
          uint32_t even_1 = rgb_even_1 & 0x3F;
          odd_2 += yys[i];
          odd_1 <<= 16;
          even_2 += yys[i+1];
          odd_1 += odd_2;
          even_1 <<= 16;
          // 奇数番ビット成分を横２列ぶん同時にバッファにセットする
          d32[++i * len32] = odd_1;
          even_1 += even_2;
          // 偶数番ビット成分を横２列ぶん同時にバッファにセットする
          d32[++i * len32] = even_1;
        } while (i <= TRANSFER_PERIOD_COUNT);

        ++d32;

        if ((x += 2) < xe) { continue; }
        if (light_idx == TRANSFER_PERIOD_COUNT) break;
        do {
          // PIN B Discharge+
          yys[++light_idx] |= (me->_cfg.address_mode == config_t::address_binary) ? _mask_oe : ( _mask_pin_b_lat | _mask_oe);
          xe = brightness_period[light_idx];
        } while (x >= xe);
      }

      // 無データ,点灯のみの期間の先頭の点灯防止処理
      d32[0 - len32] |= _mask_oe;
      d32[1 - len32] |= (brightness_period[TRANSFER_PERIOD_COUNT-1] & 1) ? _mask_oe : (_mask_oe & ~0xFFFF);

      d32 += len32;

      // SHIFTREG_ABCのY座標情報をセット;
      d32[- (y + 1                      )] |= _mask_pin_c_dat;
      d32[- (y + 1 + (panel_height >> 1))] |= _mask_pin_c_dat;
      d32[ - 1] |= _mask_lat | _mask_pin_b_lat;

      // データのラッチ及びラッチ直後の点灯防止処理
      for (int i = 1; i <= TRANSFER_PERIOD_COUNT; ++i)
      {
        d32[len32 * i - 1] |= _mask_lat;
        d32[len32 * i + 0] |= _mask_oe;
        d32[len32 * i + 1] |= (brightness_period[i] & 1) ? _mask_oe : (_mask_oe & ~0xFFFF);
      }

// DEBUG
// lgfx::gpio_lo(15);
      // 作画中に次の割込みが発生した場合はビジー状態が続くことを回避するため処理をスキップする
      ulTaskNotifyTake( pdTRUE, 0);
    }

    esp_intr_disable(me->_isr_handle);
    vTaskDelete( nullptr );
  }
//*/

//----------------------------------------------------------------------------
 }
}

#endif
#endif
