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
    uint32_t dport_clk_en;
    uint32_t dport_rst;

    if (_cfg.i2s_port == I2S_NUM_0) {
      dport_clk_en = DPORT_I2S0_CLK_EN;
      dport_rst = DPORT_I2S0_RST;
    }
#if SOC_I2C_NUM > 1
    else
    {
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

    return true;
  }

  void Bus_HUB75::setImageBuffer(void* buffer, color_depth_t depth)
  {
    _depth = depth;
    auto fb = (DividedFrameBuffer*)buffer;
    _frame_buffer = fb;
    _panel_width = fb->getLineSize() / ((depth & color_depth_t::bit_mask) >> 3);
    _panel_height = fb->getTotalLines();
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
    uint32_t a = 1;
    uint32_t b = 0;

    if (n == 0)
    {
      n = 1;
    }
    else
    {
      if (n > 255)
      {
        n = 255;
      }
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
    auto panel_width = _panel_width;
    uint32_t light_len_limit = (panel_width - 7);
    uint32_t slen = (light_len_limit * br) >> 8;

    uint32_t transfer_period_count = TRANSFER_PERIOD_COUNT_332;
    uint32_t half_start = 4;
    if (_depth == color_depth_t::rgb565_2Byte)
    {
      transfer_period_count = TRANSFER_PERIOD_COUNT_565;
      half_start = 6;
    }

    _brightness_period[transfer_period_count] = panel_width;
    for (int period = transfer_period_count - 1; period >= 0; --period)
    {
      _brightness_period[period] = slen + 4;
      if (period < half_start) { slen >>= 1; }
// ESP_EARLY_LOGE("DEBUG","period%d  = %d", period, slen);
    }
  }

  void Bus_HUB75::release(void)
  {
    endTransaction();
  }

  void Bus_HUB75::switch_gpio_control(bool switch_to_dma)
  {
    auto idx_base = SIG_GPIO_OUT_IDX;
    if (switch_to_dma)
    {
#if SOC_I2C_NUM > 1
      idx_base = (_cfg.i2s_port == I2S_NUM_0) ? I2S0O_DATA_OUT8_IDX : I2S1O_DATA_OUT8_IDX;
#else
      idx_base = I2S0O_DATA_OUT8_IDX;
#endif
    }

    for (size_t i = 0; i < 14; ++i)
    {
      if (_cfg.pin_data[i] < 0) { continue; }

#if defined ( LGFX_IDF_V5 )
      esp_rom_gpio_pad_select_gpio(_cfg.pin_data[i]);
      esp_rom_gpio_connect_out_signal(_cfg.pin_data[i  ], idx_base, 0, 0);
#else
      gpio_pad_select_gpio(_cfg.pin_data[i]);
      gpio_matrix_out(_cfg.pin_data[i  ], idx_base, 0, 0);
#endif
      if (switch_to_dma)
      {
        ++idx_base;
      }
      else
      {
        gpio_lo(_cfg.pin_data[i]);
      }
    }

    if (switch_to_dma)
    {
#if SOC_I2C_NUM > 1
      idx_base = (_cfg.i2s_port == I2S_NUM_0) ? I2S0O_WS_OUT_IDX : I2S1O_WS_OUT_IDX;
#else
      idx_base = I2S0O_WS_OUT_IDX;
#endif
#if defined ( LGFX_IDF_V5 )
      esp_rom_gpio_connect_out_signal(_cfg.pin_clk, idx_base, 1, 0); // clock Active-low
#else
      gpio_matrix_out(_cfg.pin_clk, idx_base, 1, 0); // clock Active-low
#endif
    }
    else
    {
      gpio_hi(_cfg.pin_oe);
    }
  }

  void Bus_HUB75::send_led_driver_command(uint8_t latcycle, uint16_t r, uint16_t g, uint16_t b)
  {
    for (size_t i = 0; i < _panel_width; ++i)
    {
      if (i == (_panel_width - latcycle))
      {
        gpio_hi(_cfg.pin_lat);
      }
      uint32_t mask = 0x8000 >> (i & 15);
      if (r & mask)
      {
        gpio_hi(_cfg.pin_r1);
        gpio_hi(_cfg.pin_r2);
      }
      else
      {
        gpio_lo(_cfg.pin_r1);
        gpio_lo(_cfg.pin_r2);
      }
      if (g & mask)
      {
        gpio_hi(_cfg.pin_g1);
        gpio_hi(_cfg.pin_g2);
      }
      else
      {
        gpio_lo(_cfg.pin_g1);
        gpio_lo(_cfg.pin_g2);
      }
      if (b & mask)
      {
        gpio_hi(_cfg.pin_b1);
        gpio_hi(_cfg.pin_b2);
      }
      else
      {
        gpio_lo(_cfg.pin_b1);
        gpio_lo(_cfg.pin_b2);
      }
      gpio_hi(_cfg.pin_clk);
      gpio_lo(_cfg.pin_clk);
    }
    gpio_lo(_cfg.pin_lat);
  }

  void Bus_HUB75::send_led_driver_latch(uint8_t latcycle)
  {
    gpio_hi(_cfg.pin_lat);

    for (size_t i = 0; i < latcycle; ++i)
    {
      gpio_hi(_cfg.pin_clk);
      gpio_lo(_cfg.pin_clk);
    }
    gpio_lo(_cfg.pin_lat);
  }

  void Bus_HUB75::beginTransaction(void)
  {
    if (_dmatask_handle)
    {
      return;
    }


/*  // DMAディスクリプタが利用するDMAメモリ位置テーブル
 この配列は、14x3セットのDMAディスクリプタそれぞれが使用するバッファメモリの範囲を表す。
  0 : 無データ,x1点灯
  1 : 輝度1/32データ,無灯
  2 : 輝度1/16データ,1/32点灯
  3 : 輝度1/ 8データ,1/16点灯
  4 : 輝度1/ 4データ,1/8点灯
  5 : 輝度1/ 2データ,1/4点灯
  6 : 輝度 x 1データ,1/2点灯
  7 : 輝度 x 2データ, x1点灯
  8 : 輝度 x 4データ, x1点灯
  9 : SHIFTREG_ABC座標,無灯 (他の期間と比べてデータサイズが小さい。パネルの高さ相当)
*/
    static constexpr const uint8_t dma_buf_idx_tbl_565[] = {
      9, 1, 2, 3, 4, 5, 6, 7, 0, 8, 0, 0, 0, 0,
    };

// RGB332の場合は階調表現を減らして総データ量を削減している。
// データ転送が5回、無データ点灯は2回になる。SHIFTREG_ABC座標は[6]
    static constexpr const uint8_t dma_buf_idx_tbl_332[] = {
      6, 1, 2, 3, 4, 5, 0, 0,
    };

/* RGB565の場合のDMAディスクリプタの各役割は以下の通り
  [ 0](SHIFTREG_ABC座標転送,無灯期間)
  [ 1](輝度1/32成分データ転送,無灯期間)
  [ 2](輝度1/16成分データ転送,1/32点灯期間)
  [ 3](輝度1/ 8成分データ転送,1/16点灯期間)
  [ 4](輝度1/ 4成分データ転送,1/ 8点灯期間)
  [ 5](輝度1/ 2成分データ転送,1/ 4点灯期間)
  [ 6](輝度1/ 1成分データ転送,1/ 2点灯期間)
  [ 7](輝度  x2成分データ転送,  x1点灯期間)
  [ 8](            データ無し,  x1点灯期間)
  [ 9](輝度  x4成分データ転送,  x1点灯期間)
  [10](            データ無し,  x1点灯期間)
  [11](            データ無し,  x1点灯期間)
  [12](            データ無し,  x1点灯期間)
  [13](            データ無し,  x1点灯期間)
  ※ 13番の転送が終わったあとは次のラインの先頭ディスクリプタにリンクする。
     また、13番の転送が終わった時点でEOF割込みが起こり、次のラインのデータ生成タスクが実行される。

  ※ 0番 SHIFTREG_ABC座標,無灯の転送期間はパネル１枚の高さに比例、それ以外の期間はパネル全体の幅に比例する

   色深度8を再現するために、同一ラインに輝度成分の異なるデータを8回を送る。
   後半の輝度x2,x4のデータに対しては点灯期間が長いため、x1点灯期間を複数設けることで輝度差を実現する。
   "データ無しx1点灯期間"で送信する内容はすべて同一で良いため、同じメモリ範囲を共有利用してメモリを節約している。
*/

    uint32_t transfer_period_count = TRANSFER_PERIOD_COUNT_332;
    uint32_t extend_period_count = EXTEND_PERIOD_COUNT_332;
    uint32_t total_period_count = TOTAL_PERIOD_COUNT_332;
    auto dma_buf_idx_tbl = dma_buf_idx_tbl_332;
    if (_depth == color_depth_t::rgb565_2Byte)
    {
      transfer_period_count = TRANSFER_PERIOD_COUNT_565;
      extend_period_count = EXTEND_PERIOD_COUNT_565;
      total_period_count = TOTAL_PERIOD_COUNT_565;
      dma_buf_idx_tbl = dma_buf_idx_tbl_565;
    }

    _dmadesc = (lldesc_t*)heap_alloc_dma(sizeof(lldesc_t) * total_period_count * _dma_desc_set);
    if (_dmadesc == nullptr)
    {
      ESP_EARLY_LOGE("Bus_HUB75", "memory allocate error.");
      endTransaction();
      return;
    }

    uint32_t panel_width = _panel_width;

    // DMA用バッファメモリ確保。 (無データ点灯期間1回分 + データ転送期間8回分) * パネル幅 + (SHIFTREG_ABC座標期間1回) * (パネル高さ * 2) の合計を連続領域として確保する
    // 無データ点灯期間は合計5回あるが、同じ領域を使い回すためバッファは1回分でよい;
    size_t buf_bytes = (((transfer_period_count + 1) * panel_width) + (2 * _panel_height)) * sizeof(uint16_t);
    _dma_transfer_len = (((transfer_period_count + extend_period_count) * panel_width) + (2 * _panel_height));

    for (size_t i = 0; i < _dma_desc_set; i++) {
      _dma_buf[i] = (uint16_t*)heap_alloc_dma(buf_bytes);
      if (_dma_buf[i] == nullptr) {
        ESP_EARLY_LOGE("Bus_HUB75", "memory allocate error.");
        endTransaction();
        return;
      }
      // バッファ初期値として OE(消灯)で埋めておく
      memset(_dma_buf[i], _mask_oe, buf_bytes);

      for (int j = 0; j < total_period_count; j++) {
        uint32_t idx = i * total_period_count + j;
        size_t bufidx = dma_buf_idx_tbl[j] * panel_width;
        // SHIFTREG_ABCの期間のみデータ長をpanel_height * 2とする
        size_t buflen = ((j == 0) ? _panel_height << 1 : panel_width) * sizeof(uint16_t);
        _dmadesc[idx].buf = (volatile uint8_t*)&(_dma_buf[i][bufidx]);
        _dmadesc[idx].eof = j == (total_period_count - 1); // 最後の転送期間のみEOFイベントを発生させる
        _dmadesc[idx].empty = (uint32_t)(&_dmadesc[(idx + 1) % (total_period_count * _dma_desc_set)]);
        _dmadesc[idx].owner = 1;
        _dmadesc[idx].length = buflen;
        _dmadesc[idx].size = buflen;
      }
    }
    setBrightness(_brightness);

    { // ガンマ補正テーブル生成
    // ガンマ補正と同時に、各ビットの間隔を広げる処理も行うようにデータを生成する。
      if (_depth == color_depth_t::rgb565_2Byte)
      {
        // RGB565の場合は、単一色に対して使用する64要素のテーブルを作成する。
        // 利用時にRGB成分をまとめやすくするため、3bit間隔に変換して作成する。
        _pixel_tbl = (uint32_t*)heap_alloc_dma(64 * sizeof(uint32_t));
        if (_pixel_tbl == nullptr)
        {
          ESP_EARLY_LOGE("Bus_HUB75", "memory allocate error.");
          endTransaction();
          return;
        }
        for (size_t i = 0; i < 64; ++i)
        {
          uint_fast8_t v = ((i * 65) >> 4) + 1;
          v = (v * v) >> 8;
          if (v < i) { v = i; }
          else if (v > 255) { v = 255; }

          // データの各ビット間の間隔を広げる
          uint32_t value = 0;
          for (size_t shift = 0; shift < 8; ++shift)
          {
            uint32_t mask = (1 << shift);
            value |= (v & mask) << (shift * 2);
          }
          _pixel_tbl[i] = value;
        }
      }
      else
      {
        // RGB332の場合は、3色まとめて変換できる256要素のテーブルを作成する。
        // BGRの3ビット+無データ3bitの 6bitが5セット並んだ状態のデータを作成する。
        _pixel_tbl = (uint32_t*)heap_alloc_dma(256 * sizeof(uint32_t));
        if (_pixel_tbl == nullptr)
        {
          ESP_EARLY_LOGE("Bus_HUB75", "memory allocate error.");
          endTransaction();
          return;
        }
        for (size_t rgb332 = 0; rgb332 < 256; ++rgb332)
        {
          uint_fast16_t r = 1 + (((rgb332 & 0xE0u) * 0b01001u) >> 5);
          uint_fast16_t g = 1 + (((rgb332 & 0x1Cu) * 0b01001u) >> 2);
          uint_fast16_t b = 1 + (((rgb332 & 0x03u) * 0b10101u)     );
          r = (r * r + 31) >> 7;
          g = (g * g + 31) >> 7;
          b = (b * b + 31) >> 7;
          if (r > 31) { r = 31; }
          if (g > 31) { g = 31; }
          if (b > 31) { b = 31; }

          uint32_t value = 0;
          for (size_t shift = 0; shift < 5; ++shift)
          {
            uint32_t mask = (1 << shift);
            value |= (r & mask) << (shift * 5 + 0)
                  |  (g & mask) << (shift * 5 + 1)
                  |  (b & mask) << (shift * 5 + 2);
          }
          _pixel_tbl[rgb332] = value;
        }
      }
    }

    if (_cfg.led_driver)
    { // LEDドライバ別のレジスタ設定
      switch_gpio_control(false);

      switch (_cfg.led_driver)
      {
      default:
        break;

      case config_t::led_driver_t::led_driver_FM6124:
        {
          uint8_t br = _cfg.driver_brightness >> 4;
          uint16_t cmd11 = 0b0000000001100000 | br << 7;
          uint16_t cmd12 = 0b0000000001000000;
          send_led_driver_command(11, cmd11, cmd11, cmd11);
          send_led_driver_command(12, cmd12, cmd12, cmd12);
        }
        break;

/* ToDo:implement
      case config_t::led_driver_t::led_driver_FM6047:
        break;

      case config_t::led_driver_t::led_driver_ICN2038:
      case config_t::led_driver_t::led_driver_MBI5038:
          send_led_driver_latch(13);  // Pre-active cmd
        break;
//*/

      case config_t::led_driver_t::led_driver_ICN2053:
      case config_t::led_driver_t::led_driver_MBI5153:
        {
          uint16_t cmd4  = 0b0001111101110000;
          uint16_t cmd6  = 0xffff;
          uint16_t cmd8  = 0b0100000011110011;
          uint16_t cmd10 = 0;

          send_led_driver_latch(14);  // Pre-active cmd
          send_led_driver_latch(12);  // Enable all output ch
          send_led_driver_latch( 3);  // Vsync
          send_led_driver_latch(14);  // Pre-active cmd
          send_led_driver_command(4, cmd4, cmd4, cmd4); // write cfg reg 1
          send_led_driver_latch(14);  // Pre-active cmd
          send_led_driver_command(6, cmd6, cmd6, cmd6); // write cfg reg 2
          send_led_driver_latch(14);  // Pre-active cmd
          send_led_driver_command(8, cmd8, cmd8, cmd8); // write cfg reg 3
          send_led_driver_latch(14);  // Pre-active cmd
          send_led_driver_command(10, cmd10, cmd10, cmd10); // write cfg reg 4
          send_led_driver_latch(14);  // Pre-active cmd
          send_led_driver_command(2, 0, 0, 0); // write debug reg
        }
        break;
      }
    }

    switch_gpio_control(true);

    auto i2s_dev = (i2s_dev_t*)_dev;
    i2s_dev->out_link.val = 0;
    i2s_dev->fifo_conf.val = _fifo_conf_dma;
    i2s_dev->sample_rate_conf.val = _sample_rate_conf_reg_direct;

    i2s_dev->clkm_conf.val = getClockDivValue(_cfg.freq_write);
    i2s_dev->conf.val = _conf_reg_reset;
    i2s_dev->out_link.val = I2S_OUTLINK_START | ((uint32_t)_dmadesc & I2S_OUTLINK_ADDR);

    i2s_dev->int_ena.val = 0;
    i2s_dev->int_clr.val = ~0u;
    i2s_dev->int_ena.out_eof = 1;

#if portNUM_PROCESSORS > 1
    if (((size_t)_cfg.task_pinned_core) < portNUM_PROCESSORS)
    {
      xTaskCreatePinnedToCore(dmaTask, "hub75dma", 4096, this, _cfg.task_priority, &_dmatask_handle, _cfg.task_pinned_core);
    }
    else
#endif
    {
      xTaskCreate(dmaTask, "hub75dma", 4096, this, _cfg.task_priority, &_dmatask_handle);
    }
  }

  void Bus_HUB75::setRefreshRate(uint16_t refresh_rate)
  {
    // 総転送データ量とリフレッシュレートに基づいて送信クロックを設定する
    _cfg.freq_write = (_dma_transfer_len >> 1) * _panel_height * refresh_rate;
    auto i2s_dev = (i2s_dev_t*)_dev;
    if (i2s_dev)
    {
      i2s_dev->clkm_conf.val = getClockDivValue(_cfg.freq_write);
    }
  }

  void Bus_HUB75::endTransaction(void)
  {
    gpio_hi(_cfg.pin_oe);
#if defined ( LGFX_IDF_V5 )
    esp_rom_gpio_connect_out_signal(_cfg.pin_oe, SIG_GPIO_OUT_IDX, 0, 0);
#else
    gpio_matrix_out(_cfg.pin_oe, SIG_GPIO_OUT_IDX, 0, 0);
#endif

    auto i2s_dev = (i2s_dev_t*)_dev;
    i2s_dev->int_ena.val = 0;
    i2s_dev->int_clr.val = ~0u;
    i2s_dev->out_link.stop = 1;
    i2s_dev->conf.val = _conf_reg_reset;
    i2s_dev->out_link.val = 0;

    if (_dmatask_handle)
    {
      auto handle = _dmatask_handle;
      _dmatask_handle = nullptr;
      xTaskNotify(handle, 0, eNotifyAction::eSetValueWithOverwrite);
      vTaskDelay(1);
    }

    if (_pixel_tbl)
    {
      heap_free(_pixel_tbl);
      _pixel_tbl = nullptr;
    }
    if (_dmadesc)
    {
      heap_free(_dmadesc);
      _dmadesc = nullptr;
    }
    for (size_t i = 0; i < _dma_desc_set; i++)
    {
      if (_dma_buf[i])
      {
        heap_free(_dma_buf[i]);
        _dma_buf[i] = nullptr;
      }
    }
  }

  void IRAM_ATTR Bus_HUB75::i2s_intr_handler_hub75(void *arg)
  {
    auto me = (Bus_HUB75*)arg;
    auto dev = getDev(me->_cfg.i2s_port);
    dev->int_clr.val = dev->int_st.val;

    auto desc = (lldesc_t*)dev->out_eof_des_addr;
    xTaskNotifyFromISR(me->_dmatask_handle, (uint32_t)desc->buf, eNotifyAction::eSetValueWithOverwrite, nullptr);
    portYIELD_FROM_ISR();
  }

  struct asm_work_t
  {
    uint32_t* d32;            //  0
    uint32_t* s32h;           //  4
    uint32_t* s32l;           //  8
    uint32_t* pixel_tbl;      // 12
    uint32_t* mixdata;        // 16
    uint16_t* xe_tbl;         // 20
    uint32_t len32;           // 24
    uint32_t xe_idx;          // 28
    uint32_t xe;              // 32
    uint32_t mask3bit;        // 36
    uint32_t mix_value;       // 40
    uint32_t* _retaddr;       // 44 A0保管用
  };

  static void hub75Draw332_asm(asm_work_t* work)
  {
/* 関数が呼び出された直後のレジスタの値
    a0 : リターンアドレス (workに退避し、a0を別の用途に使用)
    a1 : スタックポインタ (変更不可)
    a2 : asm_work_t*      (変更せずそのまま利用する)
*/
    __asm__ __volatile__ (
      "s32i.n  a0,  a2,  44               \n"  // A0 を退避
      "l32i.n  a3,  a2,  32               \n"  // a3  = xe
      "l32i.n  a0,  a2,  0                \n"  // ★a0  = 出力先アドレス
      "l32i.n  a15, a2,  16               \n"  // ★a15 = mixdata アドレス
      "l32i.n  a14, a2,  24               \n"  // ★a14 = len32
      "l32i.n  a13, a2,  12               \n"  // ★a13 = pixel_tbl
      "l32i.n  a12, a2,  8                \n"  // ★a12 = パネル下側の元データ配列
      "l32i.n  a11, a2,  4                \n"  // ★a11 = パネル上側の元データ配列
      "slli    a14, a14, 2                \n"  // len32 を 4倍(d32の加算に使うため)

"HUB75_DRAW332_LOOP_START:          \n"

      "loop    a3, HUB75_DRAW332_LOOP_END \n"  // ループ開始 (a3 にループ回数 xe がセットされた状態でここに来ること)

      "l32i.n  a10, a15, 16               \n"  // ★a10 に mixdata末尾の値を代入 (4*sizeof(uint32_t) = 16)

      "l8ui    a3,  a11, 0                \n"  // a3 = 元データ上側配列から rgb332形式 1ピクセル目取得
      "l8ui    a4,  a12, 0                \n"  // a4 = 元データ下側配列から rgb332形式 1ピクセル目取得
      "l8ui    a5,  a11, 1                \n"  // a5 = 元データ上側配列から rgb332形式 2ピクセル目取得
      "l8ui    a6,  a12, 1                \n"  // a6 = 元データ下側配列から rgb332形式 2ピクセル目取得
      "addi.n  a11, a11, 2                \n"  // 元データのアドレスを2ピクセル進める
      "addi.n  a12, a12, 2                \n"  // 元データのアドレスを2ピクセル進める

      "addx4   a3,  a3,  a13              \n"  // a3  = テーブルアドレスに変換
      "addx4   a4,  a4,  a13              \n"  // a4  = テーブルアドレスに変換
      "addx4   a5,  a5,  a13              \n"  // a5  = テーブルアドレスに変換
      "addx4   a6,  a6,  a13              \n"  // a6  = テーブルアドレスに変換
      "l32i.n  a3,  a3,  0                \n"  // a3  = pixel_tbl[RGB332] 1ピクセル目 上側の RGB成分 完成
      "l32i.n  a4,  a4,  0                \n"  // a4  = pixel_tbl[RGB332] 1ピクセル目 下側の RGB成分 完成
      "l32i.n  a5,  a5,  0                \n"  // a5  = pixel_tbl[RGB332] 2ピクセル目 上側の RGB成分 完成
      "l32i.n  a6,  a6,  0                \n"  // a6  = pixel_tbl[RGB332] 2ピクセル目 下側の RGB成分 完成

//////////////////

      "s32i.n  a10, a0,  0                \n"  // mixdata 末尾のデータを出力先にセット
      "mov.n   a9,  a0                    \n"  // a9 に 出力先 アドレスをコピー
      "addi.n  a0,  a0,  4                \n"  // ★a0 出力先アドレス を1進める

// この時点で a3,a4,a5,a6 に 合計4ピクセル分のデータが入った状態になっている
// テーブルから取得したデータは 0bBGR___BGR___BGR___BGR___BGR となっている。
// (6bit単位で 3bit無データ + BGR)
// ここから、パネル上側と下側のRGB成分が隣接し6bit単位となった状態に変換する
      "addx8   a3,  a4,  a3               \n"  // a3 = (下側1ピクセル目 << 3) + 上側1ピクセル目
      "addx8   a4,  a6,  a5               \n"  // a4 = (下側2ピクセル目 << 3) + 上側2ピクセル目

// ここから出力
// RGB成分 と mixdata(Y座標情報+OE信号) を合わせた16bitデータを2ピクセル分32bit纏めて出力 を 5回(transfer_period_count) 行う

      "l32i.n   a8,  a15, 0               \n"  // a8  = mixdata[0]
      "add.n    a9,  a14, a9              \n"  // a9 出力先アドレス += len32
      "extui    a7,  a3,  0,   6          \n"  // 1ピクセル目 6ビット取得
      "slli     a7,  a7,  16              \n"  // 1ピクセル目のデータを左16bitシフト
      "add.n    a8,  a7,  a8              \n"  // a8 = 1ピクセル目+mixdata
      "extui    a7,  a4,  0,   6          \n"  // 2ピクセル目 6ビット取得
      "add.n    a7,  a7,  a8              \n"  // a7 = 2ピクセル目+a8
      "s32i.n   a7,  a9,  0               \n"  // a7 の値を出力先にセット

      "l32i.n   a8,  a15, 4               \n"  // a8 = mixdata[1]
      "add.n    a9,  a14, a9              \n"  // a9 出力先アドレス += len32
      "extui    a7,  a3,  6,   6          \n"  // 1ピクセル目 6ビット取得
      "slli     a7,  a7,  16              \n"  // 1ピクセル目のデータを左16bitシフト
      "add.n    a8,  a7,  a8              \n"  // a8 = 1ピクセル目+mixdata
      "extui    a7,  a4,  6,   6          \n"  // 2ピクセル目 6ビット取得
      "add.n    a7,  a7,  a8              \n"  // a7 = 2ピクセル目+a8
      "s32i.n   a7,  a9,  0               \n"  // a7 の値を出力先にセット

      "l32i.n   a8,  a15, 8               \n"  // a8  = mixdata[2]
      "add.n    a9,  a14, a9              \n"  // a9 出力先アドレス += len32
      "extui    a7,  a3,  12,  6          \n"  // 1ピクセル目 6ビット取得
      "slli     a7,  a7,  16              \n"  // 1ピクセル目のデータを左16bitシフト
      "add.n    a8,  a7,  a8              \n"  // a8 = 1ピクセル目+mixdata
      "extui    a7,  a4,  12,  6          \n"  // 2ピクセル目 6ビット取得
      "add.n    a7,  a7,  a8              \n"  // a7 = 2ピクセル目+a8
      "s32i.n   a7,  a9,  0               \n"  // a7 の値を出力先にセット

      "l32i.n   a8,  a15, 12              \n"  // a8 = mixdata[3]
      "add.n    a9,  a14, a9              \n"  // a9 出力先アドレス += len32
      "extui    a7,  a3,  18,  6          \n"  // 1ピクセル目 6ビット取得
      "slli     a7,  a7,  16              \n"  // 1ピクセル目のデータを左16bitシフト
      "add.n    a8,  a7,  a8              \n"  // a8 = 1ピクセル目+mixdata
      "extui    a7,  a4,  18,  6          \n"  // 2ピクセル目 6ビット取得
      "add.n    a7,  a7,  a8              \n"  // a7 = 2ピクセル目+a8
      "s32i.n   a7,  a9,  0               \n"  // a7 の値を出力先にセット

      // 最後の1回は mixdata の取得を省略(a10に取得しておいた値を再利用する)
      "add.n    a9,  a14, a9              \n"  // a9 出力先アドレス += len32
      "extui    a7,  a3,  24,  6          \n"  // 1ピクセル目 6ビット取得
      "slli     a7,  a7,  16              \n"  // 1ピクセル目のデータを左16bitシフト
      "add.n    a8,  a7,  a10             \n"  // a8 = 1ピクセル目+mixdata
      "extui    a7,  a4,  24,  6          \n"  // 2ピクセル目 6ビット取得
      "add.n    a7,  a7,  a8              \n"  // a7 = 2ピクセル目+a8
      "s32i.n   a7,  a9,  0               \n"  // a7 の値を出力先にセット

"HUB75_DRAW332_LOOP_END:            \n"

      "l32i.n  a4,  a2,  28               \n" // a4に xe_idx を代入
      "l32i.n  a5,  a2,  20               \n" // a5に xe_tbl を代入
      "l32i.n  a9,  a2,  40               \n" // a9に mixdata テーブル更新用の値を取得
      "beqi    a4,  5,   HUB75_EXIT332    \n" // xe_idx が終端に達していたら処理を終える
      "addx2   a6,  a4,  a5               \n" // a6 に xeテーブル現在インデクスのアドレス
      "l16ui   a3,  a6,  0                \n" // a3 に現在の xe値
      "srli    a3 , a3,  1                \n" // a3 >>= 1
// ここから mixdata の値を更新、 xe の位置を再設定
"HUB75_BR332LOOP_START:             \n"
        "mov     a5,  a3                    \n" // a5 に前の xe値 を移す
        "l16ui   a3,  a6,  2                \n" // a3 に新しい xe値 を代入
        "addi.n  a6,  a6,  2                \n" // a6 xeテーブル位置をひとつ進める
        "addi.n  a4,  a4,  1                \n" // a4 ++xe_idx
        "addx4   a8,  a4,  a15              \n" // A8 に更新対象の mixdata のアドレスをセット
        "s32i.n  a9,  a8,  0                \n" // mixdata 更新
        "srli    a3,  a3,  1                \n" // A3 >>= 1
      "bge     a5,  a3,  HUB75_BR332LOOP_START     \n" // xe 値が同値なら BR_LOOP 再トライ
      "s32i.n  a4,  a2,  28               \n" // xe_idx の値を保存
      "sub     a3,  a3,  a5               \n" // 新しい xe値から前回のxe値を引き、差分を得る
      "j HUB75_DRAW332_LOOP_START         \n" // a3 (xe) が刷新されたので再度先頭からループ

"HUB75_EXIT332:                   \n"

      "l32i   a0,  a2,  44                \n"
      );
  }

  static void hub75Draw565_asm(asm_work_t* work)
  {
/* 関数が呼び出された直後のレジスタの値
    a0 : リターンアドレス (workに退避し、a0を別の用途に使用)
    a1 : スタックポインタ (変更不可)
    a2 : asm_work_t*      (変更せずそのまま利用する)
*/
    __asm__ __volatile__ (
      "s32i.n  a0,  a2,  44               \n"  // a0 を退避
      "l32i.n  a3,  a2,  32               \n"  // a3  = xe
      "l32i.n  a0,  a2,  0                \n"  // ★a0  = 出力先アドレス
      "l32i.n  a11, a2,  4                \n"  // ★a11 = パネル上側の元データ配列
      "l32i.n  a12, a2,  8                \n"  // ★a12 = パネル下側の元データ配列
      "l32i.n  a13, a2,  12               \n"  // ★a13 = pixel_tbl
      "l32i.n  a14, a2,  24               \n"  // ★a14 = len32
      "movi    a15, 0b111000111000111000111000111   \n"  // A15 にマスクパターンをセット
      "slli    a14, a14, 2                \n"  // len32 を 4倍(d32の加算に使うため)
      "s32i.n  a15, a2,  36               \n"  // マスクパターンをworkに退避

"HUB75_DRAW565_LOOP_START:          \n"

      "loop    a3, HUB75_DRAW565_LOOP_END \n"  // ループ開始 (a3 にループ回数 xe がセットされた状態でここに来ること)

      "l32i.n  a9,  a11, 0                \n"  // a9  = 元データ配列から rgb565形式 2ピクセル まとめて取得
      "l32i.n  a10, a12, 0                \n"  // a10 = 元データ配列から rgb565形式 2ピクセル まとめて取得
      "addi.n  a11, a11, 4                \n"  // 元データのアドレスを2ピクセル進める
      "addi.n  a12, a12, 4                \n"  // 元データのアドレスを2ピクセル進める

//////////////////

      "extui   a5,  a9,  0,   5           \n"  // a5  = 青成分取得
      "extui   a6,  a9,  5,   6           \n"  // a6  = 緑成分取得
      "extui   a7,  a9,  11,  5           \n"  // a7  = 赤成分取得
      "addx8   a4,  a5,  a13              \n"  // a4  = テーブルアドレスに変換 (5bitデータを元にuint32_t[64]のテーブルを引くため x8する)
      "addx4   a5,  a6,  a13              \n"  // a5  = テーブルアドレスに変換 (6bitデータを元にuint32_t[64]のテーブルを引くため x4する)
      "addx8   a6,  a7,  a13              \n"  // a6  = テーブルアドレスに変換 (5bitデータを元にuint32_t[64]のテーブルを引くため x8する)
      "l32i.n  a3,  a4,  4                \n"  // a3  = pixel_tbl[青成分]
      "l32i.n  a4,  a5,  0                \n"  // a4  = pixel_tbl[緑成分]
      "l32i.n  a5,  a6,  4                \n"  // a5  = pixel_tbl[赤成分]

      // ロード待ちのため、先の処理を間に挟む
      "extui   a6,  a10, 0,   5           \n"  // a6  = 青成分取得
      "extui   a7,  a10, 5,   6           \n"  // a7  = 緑成分取得
      "extui   a8,  a10, 11,  5           \n"  // a8  = 赤成分取得

      "addx2   a3,  a3,  a4               \n"  // a3  = (青 << 1) + 緑
      "addx2   a3,  a3,  a5               \n"  // a3  = (青緑 << 1) + 赤 1ピクセル目 上側の RGB成分 完成
//////////////////

      "addx8   a5,  a6,  a13              \n"  // a5  = テーブルアドレスに変換
      "addx4   a6,  a7,  a13              \n"  // a6  = テーブルアドレスに変換
      "addx8   a7,  a8,  a13              \n"  // a7  = テーブルアドレスに変換
      "l32i.n  a4,  a5,  4                \n"  // a4  = pixel_tbl[青成分]
      "l32i.n  a5,  a6,  0                \n"  // a5  = pixel_tbl[緑成分]
      "l32i.n  a6,  a7,  4                \n"  // a6  = pixel_tbl[赤成分]

      // ロード待ちのため、先の処理を間に挟む
      "extui   a7,  a9,  16,  5           \n"  // a7  = 青成分取得
      "extui   a8,  a9,  21,  6           \n"  // a8  = 緑成分取得
      "extui   a9,  a9,  27,  5           \n"  // a9  = 赤成分取得

      "addx2   a4,  a4,  a5               \n"  // a4  = (青 << 1) + 緑
      "addx2   a4,  a4,  a6               \n"  // a4  = (青緑 << 1) + 赤 1ピクセル目 下側の RGB成分 完成
//////////////////

      "addx8   a6,  a7,  a13              \n"  // a6  = テーブルアドレスに変換
      "addx4   a7,  a8,  a13              \n"  // a7  = テーブルアドレスに変換
      "addx8   a8,  a9,  a13              \n"  // a8  = テーブルアドレスに変換
      "l32i.n  a5,  a6,  4                \n"  // a5  = pixel_tbl[青成分]
      "l32i.n  a6,  a7,  0                \n"  // a6  = pixel_tbl[緑成分]
      "l32i.n  a7,  a8,  4                \n"  // a7  = pixel_tbl[赤成分]

      // ロード待ちのため、先の処理を間に挟む
      "extui   a8,  a10, 16,  5           \n"  // a8  = 青成分取得
      "extui   a9,  a10, 21,  6           \n"  // a9  = 緑成分取得
      "extui   a10, a10, 27,  5           \n"  // a10 = 赤成分取得

      "addx2   a5,  a5,  a6               \n"  // a5  = (青 << 1) + 緑
      "addx2   a5,  a5,  a7               \n"  // a5  = (青緑 << 1) + 赤 2ピクセル目 上側の RGB成分 完成
//////////////////

      "addx8   a7,  a8,  a13              \n"  // a7  = テーブルアドレスに変換
      "addx4   a8,  a9,  a13              \n"  // a8  = テーブルアドレスに変換
      "addx8   a9,  a10, a13              \n"  // a9  = テーブルアドレスに変換
      "l32i.n  a6,  a7,  4                \n"  // a6  = pixel_tbl[青成分]
      "l32i.n  a7,  a8,  0                \n"  // a7  = pixel_tbl[緑成分]
      "l32i.n  a8,  a9,  4                \n"  // a8  = pixel_tbl[赤成分]

      // ロード待ちのため、先の処理を間に挟む
      "l32i.n  a15, a2,  36               \n"  // a15にマスクパターンを読み込み

      "addx2   a6,  a6,  a7               \n"  // a6  = (青 << 1) + 緑
      "addx2   a6,  a6,  a8               \n"  // a6  = (青緑 << 1) + 赤 2ピクセル目 下側の RGB成分 完成

// この時点で a3,a4,a5,a6 に 合計4ピクセル分のデータが入った状態になっている
// ここから、パネル上側と下側のRGB成分が隣接し6bit単位となった状態に変換する

      "and     a7,  a15, a3               \n"  // a7  = upper_1_odd
      "and     a8,  a15, a4               \n"  // a8  = lower_1_odd
      "and     a9,  a15, a5               \n"  // a9  = upper_2_odd
      "and     a10, a15, a6               \n"  // a10 = lower_2_odd
      "slli    a15, a15, 3                \n"  // a15 マスクパターンを反転
      "and     a3,  a15, a3               \n"  // a3  = upper_1_even
      "and     a4,  a15, a4               \n"  // a4  = lower_1_even
      "and     a5,  a15, a5               \n"  // a5  = upper_2_even
      "and     a6,  a15, a6               \n"  // a6  = lower_2_even

// 出力処理の準備
      "l32i.n  a15, a2,  16               \n"  // a15 に mixdata アドレスを代入

      "addx8   a6,  a6,  a5               \n"  // a6 = (lower_2_even << 3) + upper_2_even
      "addx8   a5,  a4,  a3               \n"  // a5 = (lower_1_even << 3) + upper_1_even
      "addx8   a4,  a10, a9               \n"  // a4 = (lower_2_odd << 3) + upper_2_odd
      "addx8   a3,  a8,  a7               \n"  // a3 = (lower_1_odd << 3) + upper_1_odd

      "l32i.n  a10, a15, 28               \n"  // a7 に mixdata末尾の値を代入 (7*sizeof(uint32_t) = 28)
      "mov.n   a9,  a0                    \n"  // a9 に 出力先 アドレスをコピー
      "addi.n  a0,  a0,  4                \n"  // ★a0 出力先アドレス を1進める

      "s32i.n  a10, a9,  0                \n"  // mixdata 末尾のデータを出力先にセット

// ここから出力
// RGB成分 と mixdata(Y座標情報+OE信号) を合わせた16bitデータを2ピクセル分32bit纏めて出力 を 8回(transfer_period_count) 行う

      "l32i.n   a8,  a15, 0               \n"  // a8  = mixdata[0]
      "add.n    a9,  a14, a9              \n"  // a9 出力先アドレス += len32
      "extui    a7,  a3,  0,   6          \n"  // a7 = a3 todd1 6ビット取得
      "slli     a7,  a7,  16              \n"  // odd1のデータを左16bitシフト
      "add.n    a8,  a7,  a8              \n"  // a8 = odd1 + mixdata
      "extui    a7,  a4,  0,   6          \n"  // a7 = a4 odd2 6ビット取得
      "add.n    a7,  a7,  a8              \n"  // a7 = odd2 + mixdata
      "s32i.n   a7,  a9,  0               \n"  // a7 の値を出力先にセット

      "l32i.n   a8,  a15, 4               \n"  // a8 = mixdata[1]
      "add.n    a9,  a14, a9              \n"  // 出力先アドレス += len32
      "extui    a7,  a5,  3,   6          \n"  // a7 = a5 even1 6ビット取得
      "slli     a7,  a7,  16              \n"  // even1のデータを左16bitシフト
      "add.n    a8,  a7,  a8              \n"  // a8 = even1 + mixdata
      "extui    a7,  a6,  3,   6          \n"  // a7 = a6 even2 6ビット取得
      "add.n    a7,  a7,  a8              \n"  // a7 = even2 + mixdata
      "s32i.n   a7,  a9,  0               \n"  // a7 の値を出力先にセット


      "l32i.n   a8,  a15, 8               \n"  // a8  = mixdata[2]
      "add.n    a9,  a14, a9              \n"  // a9 出力先アドレス += len32
      "extui    a7,  a3,  6,   6          \n"  // a7 = a3 odd1 6ビット取得
      "slli     a7,  a7,  16              \n"  // odd1のデータを左16bitシフト
      "add.n    a8,  a7,  a8              \n"  // a8 = odd1 + mixdata
      "extui    a7,  a4,  6,   6          \n"  // a7 = a4 odd2 6ビット取得
      "add.n    a7,  a7,  a8              \n"  // a7 = odd2 + mixdata
      "s32i.n   a7,  a9,  0               \n"  // a7 の値を出力先にセット

      "l32i.n   a8,  a15, 12              \n"  // a8 = mixdata[3]
      "add.n    a9,  a14, a9              \n"  // 出力先アドレス += len32
      "extui    a7,  a5,  9,   6          \n"  // a7 = a5 even1 6ビット取得
      "slli     a7,  a7,  16              \n"  // even1のデータを左16bitシフト
      "add.n    a8,  a7,  a8              \n"  // a8 = even1 + mixdata
      "extui    a7,  a6,  9,   6          \n"  // a7 = a6 even2 6ビット取得
      "add.n    a7,  a7,  a8              \n"  // a7 = even2 + mixdata
      "s32i.n   a7,  a9,  0               \n"  // a7 の値を出力先にセット


      "l32i.n   a8,  a15, 16              \n"  // a8  = mixdata[4]
      "add.n    a9,  a14, a9              \n"  // a9 出力先アドレス += len32
      "extui    a7,  a3,  12,  6          \n"  // a7 = a3 odd1 6ビット取得
      "slli     a7,  a7,  16              \n"  // odd1のデータを左16bitシフト
      "add.n    a8,  a7,  a8              \n"  // a8 = odd1 + mixdata
      "extui    a7,  a4,  12,  6          \n"  // a7 = a4 odd2 6ビット取得
      "add.n    a7,  a7,  a8              \n"  // a7 = odd2 + mixdata
      "s32i.n   a7,  a9,  0               \n"  // a7 の値を出力先にセット

      "l32i.n   a8,  a15, 20              \n"  // a8 = mixdata[5]
      "add.n    a9,  a14, a9              \n"  // 出力先アドレス += len32
      "extui    a7,  a5,  15,  6          \n"  // a7 = a5 even1 6ビット取得
      "slli     a7,  a7,  16              \n"  // even1のデータを左16bitシフト
      "add.n    a8,  a7,  a8              \n"  // a8 = even1 + mixdata
      "extui    a7,  a6,  15,  6          \n"  // a7 = a6 even2 6ビット取得
      "add.n    a7,  a7,  a8              \n"  // a7 = even2 + mixdata
      "s32i.n   a7,  a9,  0               \n"  // a7 の値を出力先にセット

      // 最後の2回は mixdata の取得を省略(a10に取得しておいた値を再利用する)
      "add.n    a9,  a14, a9              \n"  // a9 出力先アドレス += len32
      "srli     a7,  a3,  18              \n"  // a7 = a3 odd1 6ビット取得
      "slli     a7,  a7,  16              \n"  // odd1のデータを左16bitシフト
      "add.n    a8,  a7,  a10             \n"  // a8 = odd1 + mixdata
      "srli     a7,  a4,  18              \n"  // a7 = a4 odd2 6ビット取得
      "add.n    a7,  a7,  a8              \n"  // a7 = odd2 + mixdata
      "s32i.n   a7,  a9,  0               \n"  // a7 の値を出力先にセット

      "add.n    a9,  a14, a9              \n"  // 出力先アドレス += len32
      "srli     a7,  a5,  21              \n"  // a7 = a5 even1 6ビット取得
      "slli     a7,  a7,  16              \n"  // even1のデータを左16bitシフト
      "add.n    a8,  a7,  a10             \n"  // a8 = even1 + mixdata
      "srli     a7,  a6,  21              \n"  // a7 = a6 even2 6ビット取得
      "add.n    a7,  a7,  a8              \n"  // a7 = even2 + mixdata
      "s32i.n   a7,  a9,  0               \n"  // a7 の値を出力先にセット

"HUB75_DRAW565_LOOP_END:                     \n"

      "l32i.n  a4,  a2,  28               \n" // a4に xe_idx を代入
      "l32i.n  a5,  a2,  20               \n" // a5に xe_tbl を代入
      "l32i.n  a9,  a2,  40               \n" // a9に mixdata テーブル更新用の値を取得
      "beqi    a4,  8,   HUB75_EXIT565    \n" // xe_idx が終端に達していたら処理を終える
      "addx2   a6,  a4,  a5               \n" // a6 に xeテーブル現在インデクスのアドレス
      "l16ui   a3,  a6,  0                \n" // a3 に現在の xe値
      "srli    a3 , a3,  1                \n" // a3 >>= 1
// ここから mixdata の値を更新、 xe の位置を再設定
"HUB75_BR565LOOP_START:             \n"
        "mov     a5,  a3                    \n" // a5 に前回分の xe値 a3 を移す
        "l16ui   a3,  a6,  2                \n" // a3 に新しい xe値 を代入
        "addi.n  a6,  a6,  2                \n" // a6 xeテーブル位置をひとつ進める
        "addi.n  a4,  a4,  1                \n" // a4 ++xe_idx
        "addx4   a8,  a4,  a15              \n" // A8 に更新対象の mixdata のアドレスをセット
        "srli    a3,  a3,  1                \n" // A3 >>= 1
        "s32i.n  a9,  a8,  0                \n" // mixdata 更新
      "bge     a5,  a3,  HUB75_BR565LOOP_START     \n" // xe 値が同値なら BR_LOOP 再トライ
      "s32i.n  a4,  a2,  28               \n" // xe_idx の値を保存
      "sub     a3,  a3,  a5               \n" // 新しい xe値から前回のxe値を引き、差分を得る
      "j HUB75_DRAW565_LOOP_START         \n" // a3 (xe) が刷新されたので再度先頭からループ


"HUB75_EXIT565:                   \n"

      "l32i   a0,  a2,  44                \n"
      );
  }

/*
// C++版 draw332
  static void hub75Draw332_cpp(asm_work_t* work)
  {
    uint8_t* src8_h = (uint8_t*)(work->s32h);
    uint8_t* src8_l = (uint8_t*)(work->s32l);
    uint32_t* d32 = work->d32;
    uint32_t len32 = work->len32;
    uint32_t xe = work->xe;
    uint32_t xe_idx = work->xe_idx;
    auto mixdata = work->mixdata;
    auto xe_tbl = work->xe_tbl;
    for (;;)
    {
      do
      {
        // RGB332の値を基にガンマテーブルを適用する
        // このテーブルの中身は単にガンマ補正をするだけでなく、
        // BGR順に1ビットずつ並んだ状態に変換する処理を兼ねている。
        uint32_t rgb_upper_1 = work->pixel_tbl[*src8_h++];
        uint32_t rgb_upper_2 = work->pixel_tbl[*src8_h++];
        uint32_t rgb_lower_1 = work->pixel_tbl[*src8_l++];
        uint32_t rgb_lower_2 = work->pixel_tbl[*src8_l++];

        // パラレルで同時に送信する6bit分のRGB成分(画面の上半分と下半分)が隣接するように纏める。
        uint32_t rgb_1 = rgb_upper_1 + (rgb_lower_1 << 3);
        uint32_t rgb_2 = rgb_upper_2 + (rgb_lower_2 << 3);

        d32[len32 * 0] = mixdata[5]; // [TRANSFER_PERIOD_COUNT_332];

        int32_t i = 0;
        uint32_t pixel_2 = rgb_2 & 0x3F;
        uint32_t pixel_1 = rgb_1 & 0x3F;
        pixel_2 += mixdata[i];
        pixel_1 <<= 16;
        d32[++i * len32] = pixel_1 + pixel_2;
        do
        {
          rgb_2 >>= 6;
          rgb_1 >>= 6;
          pixel_2 = rgb_2 & 0x3F;
          pixel_1 = rgb_1 & 0x3F;
          pixel_2 += mixdata[i];
          pixel_1 <<= 16;
          // 横２列ぶん同時にバッファにセットする
          d32[++i * len32] = pixel_1 + pixel_2;
        } while (i < 5);  // TRANSFER_PERIOD_COUNT_332;

        ++d32;
      } while (--xe);
      if (xe_idx == 5) break; // TRANSFER_PERIOD_COUNT_332
      uint32_t new_xe = xe_tbl[xe_idx] >> 1;
      uint32_t old_xe;
      do {
        old_xe = new_xe;
        mixdata[++xe_idx] = work->mix_value;
        new_xe = xe_tbl[xe_idx] >> 1;
      } while (new_xe <= old_xe);
      xe = new_xe - old_xe;
    }
  }

 // C++版 draw565
  static void hub75Draw565_cpp(asm_work_t* work)
  {
    uint32_t* d32 = work->d32;
    uint32_t len32 = work->len32;
    uint32_t xe = work->xe;
    uint32_t xe_idx = work->xe_idx;
    auto pixel_tbl = work->pixel_tbl;
    auto mixdata = work->mixdata;
    auto xe_tbl = work->xe_tbl;

    for (;;)
    {
      do
      {
        // 16bit RGB565を32bit変数に2ピクセル纏めて取り込む。(画面の上半分用)
        uint32_t rgb565x2_upper = *work->s32h++;
        // 画面の下半分用のピクセルも同様に2ピクセル纏めて取り込む
        uint32_t rgb565x2_lower = *work->s32l++;

        // R,G,Bそれぞれの成分に分離する。2x2=4ピクセルまとめて処理することで演算回数を削減する
        uint32_t r_upper_1 = rgb565x2_upper >> 11;
        uint32_t r_lower_1 = rgb565x2_lower >> 11;
        uint32_t g_upper_1 = rgb565x2_upper >> 5;
        uint32_t g_lower_1 = rgb565x2_lower >> 5;
        uint32_t b_upper_2 = rgb565x2_upper & 0x1F001F;
        uint32_t b_lower_2 = rgb565x2_lower & 0x1F001F;
        uint32_t r_upper_2 = r_upper_1 & 0x1F001F;
        uint32_t r_lower_2 = r_lower_1 & 0x1F001F;
        uint32_t g_upper_2 = g_upper_1 & 0x3F003F;
        uint32_t g_lower_2 = g_lower_1 & 0x3F003F;

        r_upper_1 = r_upper_2 & 0x1F;
        r_lower_1 = r_lower_2 & 0x1F;
        r_upper_2 >>= 16;
        r_lower_2 >>= 16;
        g_upper_1 = g_upper_2 & 0x3F;
        g_lower_1 = g_lower_2 & 0x3F;
        g_upper_2 >>= 16;
        g_lower_2 >>= 16;
        uint32_t b_upper_1 = b_upper_2 & 0x1F;
        uint32_t b_lower_1 = b_lower_2 & 0x1F;
        b_upper_2 >>= 16;
        b_lower_2 >>= 16;

        // RGBそれぞれ64階調値を元にガンマテーブルを適用する
        // このテーブルの中身は単にガンマ補正をするだけでなく、
        // 各ビットを3bit間隔に変換する処理を兼ねている。
        // 具体的には  0bABCDEFGH  ->  0bA__B__C__D__E__F__G__H__ のようになる
        r_upper_1 = pixel_tbl[r_upper_1 << 1];
        r_lower_1 = pixel_tbl[r_lower_1 << 1];
        r_upper_2 = pixel_tbl[r_upper_2 << 1];
        r_lower_2 = pixel_tbl[r_lower_2 << 1];
        g_upper_1 = pixel_tbl[g_upper_1];
        g_lower_1 = pixel_tbl[g_lower_1];
        g_upper_2 = pixel_tbl[g_upper_2];
        g_lower_2 = pixel_tbl[g_lower_2];
        b_upper_1 = pixel_tbl[b_upper_1 << 1];
        b_lower_1 = pixel_tbl[b_lower_1 << 1];
        b_upper_2 = pixel_tbl[b_upper_2 << 1];
        b_lower_2 = pixel_tbl[b_lower_2 << 1];

        // テーブルから取り込んだ値は3bit間隔となっているので、
        // R,G,Bそれぞれが互いを避けるようにビットシフトすることでまとめることができる。
        g_upper_1 += b_upper_1 << 1;
        g_lower_1 += b_lower_1 << 1;
        g_upper_2 += b_upper_2 << 1;
        g_lower_2 += b_lower_2 << 1;
        uint32_t rgb_upper_1 = r_upper_1 + (g_upper_1 << 1);
        uint32_t rgb_lower_1 = r_lower_1 + (g_lower_1 << 1);
        uint32_t rgb_upper_2 = r_upper_2 + (g_upper_2 << 1);
        uint32_t rgb_lower_2 = r_lower_2 + (g_lower_2 << 1);

        // 上記の変数の中身は BGRBGRBGRBGR… の順にビットが並んだ状態となる
        // これを、各色の0,2,4,6ビットと1,3,5,7ビットの成分に分離する
        uint32_t rgb_upper_1_even = rgb_upper_1 & 0b00111000111000111000111000111000;
        uint32_t rgb_upper_1_odd  = rgb_upper_1 & 0b11000111000111000111000111000111;
        uint32_t rgb_lower_1_even = rgb_lower_1 & 0b00111000111000111000111000111000;
        uint32_t rgb_lower_1_odd  = rgb_lower_1 & 0b11000111000111000111000111000111;
        uint32_t rgb_upper_2_even = rgb_upper_2 & 0b00111000111000111000111000111000;
        uint32_t rgb_upper_2_odd  = rgb_upper_2 & 0b11000111000111000111000111000111;
        uint32_t rgb_lower_2_even = rgb_lower_2 & 0b00111000111000111000111000111000;
        uint32_t rgb_lower_2_odd  = rgb_lower_2 & 0b11000111000111000111000111000111;

        // パラレルで同時に送信する6bit分のRGB成分(画面の上半分と下半分)が隣接するように纏める。
        uint32_t rgb_even_1 = (rgb_lower_1_even    ) + (rgb_upper_1_even >> 3);
        uint32_t rgb_odd_1  = (rgb_lower_1_odd << 3) + (rgb_upper_1_odd      );
        uint32_t rgb_even_2 = (rgb_lower_2_even    ) + (rgb_upper_2_even >> 3);
        uint32_t rgb_odd_2  = (rgb_lower_2_odd << 3) + (rgb_upper_2_odd      );

        d32[0] = mixdata[8]; // TRANSFER_PERIOD_COUNT565

        uint32_t i = 0;
        do
        {
          uint32_t odd_2 = rgb_odd_2 & 0x3F;
          uint32_t odd_1 = rgb_odd_1 & 0x3F;
          uint32_t even_2 = rgb_even_2 & 0x3F;
          uint32_t even_1 = rgb_even_1 & 0x3F;
          rgb_odd_2 >>= 6;
          rgb_odd_1 >>= 6;
          odd_2 += mixdata[i] + (odd_1 << 16);
          // 奇数番ビット成分を横２列ぶん同時にバッファにセットする
          d32[++i * len32] = odd_2;
          even_2 += mixdata[i] + (even_1 << 16);
          rgb_even_2 >>= 6;
          rgb_even_1 >>= 6;
          // 偶数番ビット成分を横２列ぶん同時にバッファにセットする
          d32[++i * len32] = even_2;
        } while (i < 8); // TRANSFER_PERIOD_COUNT565
        ++d32;
      } while (--xe);
      if (xe_idx == 8) break; // TRANSFER_PERIOD_COUNT_565
      uint32_t new_xe = xe_tbl[xe_idx] >> 1;
      uint32_t old_xe;
      do {
        old_xe = new_xe;
        mixdata[++xe_idx] = work->mix_value;
        new_xe = xe_tbl[xe_idx] >> 1;
      } while (new_xe <= old_xe);
      xe = new_xe - old_xe;
    }
  }
//*/

  void Bus_HUB75::dmaTask(void *arg)
  {
    auto me = (Bus_HUB75*)arg;

    auto dev = getDev(me->_cfg.i2s_port);
    dev->conf.val = _conf_reg_start;
    dev->int_clr.val = ~0u;

    int intr_source = ETS_I2S0_INTR_SOURCE;
#if SOC_I2C_NUM > 1
    if (me->_cfg.i2s_port != I2S_NUM_0)
    {
      intr_source = ETS_I2S1_INTR_SOURCE;
    }
#endif

    intr_handle_t isr_handle = nullptr;

    if (esp_intr_alloc(intr_source, ESP_INTR_FLAG_LEVEL1 | ESP_INTR_FLAG_IRAM,
        i2s_intr_handler_hub75, me, &(isr_handle)) != ESP_OK) {
      ESP_EARLY_LOGE("Bus_HUB75","esp_intr_alloc failure ");
      return;
    }

    ESP_EARLY_LOGV("Bus_HUB75","esp_intr_alloc success ");

    // タスク通知が遅れて届いている可能性があるのでここで待機、破棄する
    ulTaskNotifyTake( pdTRUE, 1);

    me->dmaTask_inner();

    esp_intr_free(isr_handle);

    vTaskDelete( nullptr );
  }


  void Bus_HUB75::dmaTask_inner(void)
  {
    const auto panel_width = _panel_width;
    const auto panel_height = _panel_height;
    const uint32_t len32 = panel_width >> 1;
    uint_fast8_t y = 0;

    const uint16_t* xe_tbl = _brightness_period;

    asm_work_t work;

    uint32_t mixdata[TRANSFER_PERIOD_COUNT_565 + 1];
    work.xe_tbl = _brightness_period;
    work.len32 = panel_width >> 1;
    work.pixel_tbl = _pixel_tbl;
    work.mixdata = mixdata;

    auto fp_draw = hub75Draw332_asm;
    // auto fp_draw = hub75Draw332_cpp;
    auto transfer_period_count = TRANSFER_PERIOD_COUNT_332;

    if (_depth == color_depth_t::rgb565_2Byte)
    {
      fp_draw = hub75Draw565_asm;
      // fp_draw = hub75Draw565_cpp;
      transfer_period_count = TRANSFER_PERIOD_COUNT_565;
    }

    while (_dmatask_handle)
    {
// DEBUG
// lgfx::gpio_lo(15);
      auto dst = (uint32_t*)ulTaskNotifyTake( pdTRUE, portMAX_DELAY);
// DEBUG
// lgfx::gpio_hi(15);
      if (dst == nullptr) { break; }
      auto d32 = &dst[len32 * (transfer_period_count + 1)];

      y = (y + 1) & ((panel_height>>1) - 1);

      {
      // SHIFTREG_ABCのY座標情報をセット;
        uint32_t poi = (~y) & ((panel_height >> 1) - 1);
        d32[poi                      ] = _mask_pin_a_clk | _mask_oe | _mask_pin_c_dat;
        d32[poi + (panel_height >> 1)] = _mask_pin_a_clk | _mask_oe | _mask_pin_c_dat;
        for (int i = 0; i < _dma_desc_set; ++i)
        { // 以前のY座標ビットを消去;
          poi = (poi + 1) & ((panel_height >> 1) - 1);
          d32[poi                      ] = _mask_pin_a_clk | _mask_oe;
          d32[poi + (panel_height >> 1)] = _mask_pin_a_clk | _mask_oe;
        }
        // 末尾にラッチを追加;
        // パネルの仕様の差により、LATピンとBピンどちらがラッチに使用されているか不明なため、BとLATの両方とも立てる;
        d32[panel_height - 1] |= _mask_pin_b_lat | _mask_lat | _mask_lat << 16;
      }

      uint32_t yy = 0;
      uint32_t yy_oe = _mask_oe | _mask_pin_b_lat; // PIN B Discharge
      if (_cfg.address_mode == config_t::address_mode_t::address_binary)
      {
        yy = y << 8 | y << 24;
        yy_oe = yy | _mask_oe;
      }
      mixdata[0] = yy_oe;
      for (size_t i = 1; i <= transfer_period_count; ++i)
      {
        mixdata[i] = yy;
      }

      work.d32 = dst;
      work.s32h = (uint32_t*)(_frame_buffer->getLineBuffer(y));
      work.s32l = (uint32_t*)(_frame_buffer->getLineBuffer(y + (panel_height>>1)));
      work.xe = xe_tbl[0] >> 1;
      work.xe_idx = 0;
      work.mix_value = yy_oe;

      // データ生成用の関数をコール
      fp_draw(&work);

      d32 = &dst[len32];

      // 無データ,点灯のみの期間の先頭の点灯防止処理
      d32[0 - len32] |= _mask_oe;
      d32[1 - len32] |= (xe_tbl[transfer_period_count - 1] & 1) ? (_mask_oe & ~0xFFFF) : _mask_oe;

      d32 += len32;
      // データのラッチ及びラッチ直後の点灯防止処理
      for (int i = 0; i < transfer_period_count; ++i)
      {
        d32[len32 * i - 1] |= _mask_lat;
        d32[len32 * i + 0] |= _mask_oe;
        d32[len32 * i + 1] |= (xe_tbl[i] & 1) ? (_mask_oe & ~0xFFFF) :  _mask_oe;
      }

      // 作画中に次の割込みが発生した場合は、ビジー状態が続くことを回避するため処理をスキップする
      ulTaskNotifyTake( pdTRUE, 0); // 通知が届いていたら捨てる
    }
  }

//----------------------------------------------------------------------------
 }
}

#endif
#endif
