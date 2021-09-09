#pragma once

#define LGFX_USE_V1

#include <LovyanGFX.hpp>

// LGFX for ESP boy

class LGFX : public lgfx::LGFX_Device
{
  struct Panel_ESPboy : public lgfx::Panel_ST7735S
  {
    static constexpr int i2c_freq = 400000;
    static constexpr int pin_sda = 4;
    static constexpr int pin_scl = 5;
    static constexpr int port = 0;

    // for CS control
    static constexpr int MCP23017_ADDR = 0x20;
    static constexpr int MCP23017_GPIO_B = 0x13;
    static constexpr int MCP23017_IODIR_B = 0x01;

    // for Backlight control
    static constexpr int MCP4725_ADDR = 0x60;

    void init_cs(void) override
    {
      lgfx::i2c::init(port, pin_sda, pin_scl);

      // MCP23017 PortB bit0 = CS. set to output.
      lgfx::i2c::writeRegister8(port, MCP23017_ADDR, MCP23017_IODIR_B, 0x00, 0xFE, i2c_freq);

      cs_control(true);
    }

    void cs_control(bool level) override
    {
      // MCP23017 PortB bit0 = CS. set level
      lgfx::i2c::writeRegister8(port, MCP23017_ADDR, MCP23017_GPIO_B, level, 0xFE, i2c_freq);
    }

    void setBrightness(uint8_t brightness) override
    {
      if (brightness)
      {
        brightness = (128+brightness) >> 2;
      }
      uint8_t buf[2];
      buf[0] = brightness>>4;
      buf[1] = brightness>>4 | brightness<<4;
      lgfx::i2c::transactionWrite(port, MCP4725_ADDR, buf, 2, i2c_freq);
    }
  };

  Panel_ESPboy _panel_instance;
  lgfx::Bus_SPI _bus_instance;

public:

  LGFX(void)
  {
    { // バス制御の設定を行います。
      auto cfg = _bus_instance.config();    // バス設定用の構造体を取得します。

// SPIバスの設定
      cfg.spi_mode = 0;             // SPI通信モードを設定 (0 ~ 3)
      cfg.spi_3wire  = true;       // 受信をMOSIピンで行う場合はtrueを設定
      cfg.freq_write = 40000000;    // 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
      cfg.freq_read  =  8000000;    // 受信時のSPIクロック
      cfg.pin_sclk = 14;            // SPIのSCLKピン番号を設定
      cfg.pin_mosi = 13;            // SPIのMOSIピン番号を設定
      cfg.pin_miso = 12;            // SPIのMISOピン番号を設定 (-1 = disable)
      cfg.pin_dc   = 16;            // SPIのD/Cピン番号を設定  (-1 = disable)
     // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。

      _bus_instance.config(cfg);    // 設定値をバスに反映します。
      _panel_instance.setBus(&_bus_instance);      // バスをパネルにセットします。
    }

    { // 表示パネル制御の設定を行います。
      auto cfg = _panel_instance.config();    // 表示パネル設定用の構造体を取得します。

      cfg.memory_width     =   132;  // ドライバICがサポートしている最大の幅
      cfg.memory_height    =   132;  // ドライバICがサポートしている最大の高さ
      cfg.panel_width      =   128;  // 実際に表示可能な幅
      cfg.panel_height     =   128;  // 実際に表示可能な高さ
      cfg.offset_x         =     2;  // パネルのX方向オフセット量
      cfg.offset_y         =     1;  // パネルのY方向オフセット量
      cfg.offset_rotation  =     2;  // 回転方向の値のオフセット 0~7 (4~7は上下反転)
      cfg.dummy_read_pixel =     9;  // ピクセル読出し前のダミーリードのビット数
      cfg.dummy_read_bits  =     1;  // ピクセル以外のデータ読出し前のダミーリードのビット数
      cfg.readable         =  true;  // データ読出しが可能な場合 trueに設定
      cfg.bus_shared       = false;  // SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

      _panel_instance.config(cfg);
    }
    setPanel(&_panel_instance); // 使用するパネルをセットします。
  }
};

