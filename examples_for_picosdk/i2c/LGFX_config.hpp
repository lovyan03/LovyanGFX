#include <LovyanGFX.hpp>

// SSD1306 (128x64) の接続設定例
#define TFT_SDA  12
#define TFT_SCL  13
#define I2C_PORT 0
#define I2C_ADDR 0x3c

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_SSD1306 _panel_instance;
  lgfx::Bus_I2C       _bus_instance;   // I2Cバスのインスタンス
public:
  LGFX(void)
  {
    {                                    // バス制御の設定を行います。
      auto cfg = _bus_instance.config(); // バス設定用の構造体を取得します。

      // I2Cバスの設定
      cfg.i2c_port    = I2C_PORT;   // 使用するI2Cポートを選択 (0 or 1)
      cfg.freq_write  = 400000;     // 送信時のクロック
      cfg.freq_read   = 400000;     // 受信時のクロック
      cfg.pin_sda     = TFT_SDA;    // SDAを接続しているピン番号
      cfg.pin_scl     = TFT_SCL;    // SCLを接続しているピン番号
      cfg.i2c_addr    = I2C_ADDR;   // I2Cデバイスのアドレス

      _bus_instance.config(cfg);              // 設定値をバスに反映します。
      _panel_instance.setBus(&_bus_instance); // バスをパネルにセットします。
    }

    {                                      // 表示パネル制御の設定を行います。
      auto cfg = _panel_instance.config(); // 表示パネル設定用の構造体を取得します。

      cfg.pin_cs   = -1; // CSが接続されているピン番号   (-1 = disable)
      cfg.pin_rst  = -1; // RSTが接続されているピン番号  (-1 = disable)
      cfg.pin_busy = -1; // BUSYが接続されているピン番号 (-1 = disable)

      cfg.panel_width  = 128; // 実際に表示可能な幅
      cfg.panel_height = 64;  // 実際に表示可能な高さ
      //cfg.offset_x     = 0;   // パネルのX方向オフセット量
      //cfg.offset_y     = 0;   // パネルのY方向オフセット量

      // cfg.offset_rotation = 4;    // 回転方向の値のオフセット 0~7 (4~7は上下反転)
      // cfg.invert          = true; // パネルの明暗が反転してしまう場合 trueに設定
      // cfg.rgb_order       = true; // パネルの赤と青が入れ替わってしまう場合 trueに設定

      _panel_instance.config(cfg);
    }

    setPanel(&_panel_instance); // 使用するパネルをセットします。
  }
};
