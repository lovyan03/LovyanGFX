#include <LovyanGFX.hpp>

//#define USE_BACKLIGHT

// ST7735S (80x160) の接続設定例
#define TFT_MISO -1
#define TFT_MOSI 19
#define TFT_SCLK 18
#define TFT_CS   17
#define TFT_DC   26
#define TFT_RST  27
#ifdef USE_BACKLIGHT
#define TFT_BLK  28
#endif
#define SPI_PORT  0

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ST7735S _panel_instance;
  lgfx::Bus_SPI       _bus_instance; // SPIバスのインスタンス
#ifdef USE_BACKLIGHT
  lgfx::Light_PWM     _light_instance;
#endif

public:
  LGFX(void)
  {
    {                                    // バス制御の設定を行います。
      auto cfg = _bus_instance.config(); // バス設定用の構造体を取得します。

      // SPIバスの設定
      cfg.spi_host = SPI_PORT;   // 使用するSPIポートを選択(0 or 1)
      cfg.spi_mode = 0;          // SPI通信モードを設定 (0 ~ 3)
      cfg.freq_write = 40000000; // 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
      cfg.freq_read = 20000000;  // 受信時のSPIクロック

      cfg.pin_sclk = TFT_SCLK; // SPIのSCLKピン番号を設定
      cfg.pin_mosi = TFT_MOSI; // SPIのMOSIピン番号を設定
      cfg.pin_miso = TFT_MISO; // SPIのMISOピン番号を設定 (-1 = disable)
      cfg.pin_dc   = TFT_DC;   // SPIのD/Cピン番号を設定  (-1 = disable)

      _bus_instance.config(cfg);              // 設定値をバスに反映します。
      _panel_instance.setBus(&_bus_instance); // バスをパネルにセットします。
    }

    {                                      // 表示パネル制御の設定を行います。
      auto cfg = _panel_instance.config(); // 表示パネル設定用の構造体を取得します。

      cfg.pin_cs = TFT_CS;   // CSが接続されているピン番号   (-1 = disable)
      cfg.pin_rst = TFT_RST; // RSTが接続されているピン番号  (-1 = disable)
      cfg.pin_busy = -1;     // BUSYが接続されているピン番号 (-1 = disable)

      cfg.panel_width = 80;   // 実際に表示可能な幅
      cfg.panel_height = 160; // 実際に表示可能な高さ
      cfg.offset_x = 26;      // パネルのX方向オフセット量
      cfg.offset_y = 1;       // パネルのY方向オフセット量

      // cfg.offset_rotation = 4; // 回転方向の値のオフセット 0~7 (4~7は上下反転)
      cfg.invert = true; // パネルの明暗が反転してしまう場合 trueに設定
      // cfg.rgb_order = true;   // パネルの赤と青が入れ替わってしまう場合 trueに設定

      _panel_instance.config(cfg);
    }

#ifdef USE_BACKLIGHT
    { // バックライト制御の設定を行います。（必要なければ削除）
      auto cfg = _light_instance.config();    // バックライト設定用の構造体を取得します。

      cfg.pin_bl = TFT_BLK;         // バックライトが接続されているピン番号
      cfg.invert = false;           // バックライトの輝度を反転させる場合 true
      cfg.freq   = 44100;           // バックライトのPWM周波数
      cfg.pwm_channel = 6;          // 使用するPWMのチャンネル番号

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);  // バックライトをパネルにセットします。
    }
#endif

    setPanel(&_panel_instance); // 使用するパネルをセットします。
  }
};
