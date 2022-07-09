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
#pragma once

#include "../v1_init.hpp"
#include "common.hpp"

#include <nvs.h>
#include <memory>
#include <esp_log.h>
#include <driver/i2c.h>
#include <soc/efuse_reg.h>


#if defined( ARDUINO_M5Stack_Core_ESP32 ) || defined( ARDUINO_M5STACK_FIRE )
  #define LGFX_M5STACK
#elif defined( ARDUINO_M5STACK_Core2 ) // M5Stack Core2
  #define LGFX_M5STACK_CORE2
#elif defined( ARDUINO_M5Stick_C ) // M5Stick C / CPlus
  #define LGFX_M5STICK_C
#elif defined( ARDUINO_M5Stick_C_Plus )
  #define LGFX_M5STICK_C
#elif defined( ARDUINO_M5Stack_CoreInk ) // M5Stack CoreInk
  #define LGFX_M5STACK_COREINK
#elif defined( ARDUINO_M5STACK_Paper ) // M5Paper
  #define LGFX_M5PAPER
#elif defined ( ARDUINO_M5STACK_TOUGH )
  #define LGFX_M5TOUGH
#elif defined( ARDUINO_ODROID_ESP32 ) // ODROID-GO
  #define LGFX_ODROID_GO
#elif defined( ARDUINO_TTGO_T1 ) // TTGO TS
  #define LGFX_TTGO_TS
#elif defined( ARDUINO_TWatch ) || defined( ARDUINO_T ) // TTGO T-Watch
  #define LGFX_TTGO_TWATCH
#elif defined( ARDUINO_D ) || defined( ARDUINO_DDUINO32_XS ) // DSTIKE D-duino-32 XS
  #define LGFX_DDUINO32_XS
#elif defined( ARDUINO_LOLIN_D32_PRO )
  #define LGFX_LOLIN_D32_PRO
#elif defined( ARDUINO_ESP32_WROVER_KIT )
  #define LGFX_ESP_WROVER_KIT
#endif


namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  static constexpr char LIBRARY_NAME[] = "LovyanGFX";

  struct Light_TWatch : public lgfx::Light_PWM
  {
    /// TTGO T-Watchはモデルチェンジでバックライトの仕様が何度か変更されている。;
    /// 2019    : GPIO12
    /// 2020 v1 : GPIO12 & AXP202 LDO2
    /// 2020 v2 : GPIO25 & AXP202 LDO2
    /// 2020 v3 : GPIO15 & AXP202 LDO2
    /// これらに対応するため、GPIO12をPWM制御しつつ、AXP202のLDO2も併せて制御する方式とする。;

    static constexpr int32_t axp_i2c_freq = 400000;
    static constexpr int_fast16_t axp_i2c_addr = 0x35;  // axp202 addr
    static constexpr int_fast16_t axp_i2c_port = I2C_NUM_0;
    static constexpr int_fast16_t axp_i2c_sda = GPIO_NUM_21;
    static constexpr int_fast16_t axp_i2c_scl = GPIO_NUM_22;

    bool init(uint8_t brightness) override
    {
      lgfx::i2c::init(axp_i2c_port, axp_i2c_sda, axp_i2c_scl).has_value();
      auto cfg = config();
      cfg.pin_bl = GPIO_NUM_12;
      cfg.freq   = 1200;
      cfg.pwm_channel = 7;
      cfg.invert = false;
      config(cfg);
      bool res = lgfx::Light_PWM::init(brightness);
      setBrightness(brightness);
      return res;
    }

    void setBrightness(uint8_t brightness) override
    {
      lgfx::Light_PWM::setBrightness(brightness);
      if (brightness)
      {
        if (brightness > 4)
        {
          brightness = (brightness / 24) + 5;
        }
        lgfx::i2c::bitOn(axp_i2c_port, axp_i2c_addr, 0x12, 0x04, axp_i2c_freq); // LDO2 enable
      }
      else
      {
        lgfx::i2c::bitOff(axp_i2c_port, axp_i2c_addr, 0x12, 0x04, axp_i2c_freq); // LDO2 disable
      }
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x28, brightness<<4, 0x0F, axp_i2c_freq);
    }
  };

  namespace m5stack
  {
    static constexpr int32_t axp_i2c_freq = 400000;
    static constexpr uint_fast8_t axp_i2c_addr = 0x34;
    static constexpr int_fast16_t axp_i2c_port = I2C_NUM_1;
    static constexpr int_fast16_t axp_i2c_sda = GPIO_NUM_21;
    static constexpr int_fast16_t axp_i2c_scl = GPIO_NUM_22;
  }

  struct Panel_M5Stack : public lgfx::Panel_ILI9342
  {
    Panel_M5Stack(void)
    {
      _cfg.pin_cs  = GPIO_NUM_14;
      _cfg.pin_rst = GPIO_NUM_33;
      _cfg.offset_rotation = 3;

      _rotation = 1;
    }

    bool init(bool use_reset) override
    {
      lgfx::gpio_hi(_cfg.pin_rst);
      lgfx::pinMode(_cfg.pin_rst, lgfx::pin_mode_t::input_pulldown);
      _cfg.invert = lgfx::gpio_in(_cfg.pin_rst);       // get panel type (IPS or TN)
      lgfx::pinMode(_cfg.pin_rst, lgfx::pin_mode_t::output);

      return lgfx::Panel_ILI9342::init(use_reset);
    }
  };

  struct Panel_M5StackCore2 : public lgfx::Panel_ILI9342
  {
    Panel_M5StackCore2(void)
    {
      _cfg.pin_cs = GPIO_NUM_5;
      _cfg.invert = true;
      _cfg.offset_rotation = 3;

      _rotation = 1; // default rotation
    }

    void reset(void) override
    {
      using namespace m5stack;
      // AXP192 reg 0x96 = GPIO3&4 control
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x96, 0, ~0x02, axp_i2c_freq); // GPIO4 LOW (LCD RST)
      lgfx::delay(4);
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x96, 2, ~0x00, axp_i2c_freq); // GPIO4 HIGH (LCD RST)
    }
  };

  struct Light_M5StackCore2 : public lgfx::ILight
  {
    bool init(uint8_t brightness) override
    {
      setBrightness(brightness);
      return true;
    }

    void setBrightness(uint8_t brightness) override
    {
      using namespace m5stack;

      if (brightness)
      {
        brightness = (brightness >> 3) + 72;
        lgfx::i2c::bitOn(axp_i2c_port, axp_i2c_addr, 0x12, 0x02, axp_i2c_freq); // DC3 enable
      }
      else
      {
        lgfx::i2c::bitOff(axp_i2c_port, axp_i2c_addr, 0x12, 0x02, axp_i2c_freq); // DC3 disable
      }
    // AXP192 reg 0x27 = DC3
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x27, brightness, 0x80, axp_i2c_freq);
    }
  };

  struct Light_M5Tough : public lgfx::ILight
  {
    bool init(uint8_t brightness) override
    {
      setBrightness(brightness);
      return true;
    }

    void setBrightness(uint8_t brightness) override
    {
      using namespace m5stack;

      if (brightness)
      {
        if (brightness > 4)
        {
          brightness = (brightness / 24) + 5;
        }
        lgfx::i2c::bitOn(axp_i2c_port, axp_i2c_addr, 0x12, 0x08, axp_i2c_freq); // LDO3 enable
      }
      else
      {
        lgfx::i2c::bitOff(axp_i2c_port, axp_i2c_addr, 0x12, 0x08, axp_i2c_freq); // LDO3 disable
      }
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x28, brightness, 0xF0, axp_i2c_freq);
    }
  };

  struct Touch_M5Tough : public lgfx::ITouch
  {
    Touch_M5Tough(void)
    {
      _cfg.x_min = 0;
      _cfg.x_max = 319;
      _cfg.y_min = 0;
      _cfg.y_max = 239;
    }

    void wakeup(void) override {}
    void sleep(void) override {}

    bool init(void) override
    {
      _inited = false;
      if (isSPI()) return false;

      if (_cfg.pin_int >= 0)
      {
        lgfx::pinMode(_cfg.pin_int, pin_mode_t::input_pullup);
      }
      _inited = lgfx::i2c::init(_cfg.i2c_port, _cfg.pin_sda, _cfg.pin_scl).has_value();
      static constexpr uint8_t irq_modechange_cmd[] = { 0x5a, 0x5a };  /// (INT mode change)
      lgfx::i2c::transactionWrite(_cfg.i2c_port, _cfg.i2c_addr, irq_modechange_cmd, 2);
      return _inited;
    }

    uint_fast8_t getTouchRaw(touch_point_t* __restrict tp, uint_fast8_t count) override
    {
      if (tp) tp->size = 0;
      if (!_inited || count == 0) return 0;
      if (count > 2) count = 2; // max 2 point.

      if (_cfg.pin_int >= 0)
      {
        if (gpio_in(_cfg.pin_int)) return 0;
      }

      size_t len = 3 + count * 6;
      uint8_t buf[2][len];
      int32_t retry = 5;
      bool flip = false;
      uint8_t* tmp;
      for (;;)
      {
        tmp = buf[flip];
        memset(tmp, 0, len);
        if (lgfx::i2c::beginTransaction(_cfg.i2c_port, _cfg.i2c_addr, _cfg.freq, false))
        {
          static constexpr uint8_t reg_number = 2;
          if (lgfx::i2c::writeBytes(_cfg.i2c_port, &reg_number, 1)
          && lgfx::i2c::restart(_cfg.i2c_port, _cfg.i2c_addr, _cfg.freq, true)
          && lgfx::i2c::readBytes(_cfg.i2c_port, tmp, 1)
          && (tmp[0] != 0))
          {
            flip = !flip;
            size_t points = std::min<uint_fast8_t>(count, tmp[0]);
            if (points && lgfx::i2c::readBytes(_cfg.i2c_port, &tmp[1], points * 6 - 2))
            {}
          }
          if (lgfx::i2c::endTransaction(_cfg.i2c_port)) {}
          if (tmp[0] == 0 || memcmp(buf[0], buf[1], len) == 0) break;
        }
        if (0 == --retry) return 0;
      }
      if (count > tmp[0]) count = tmp[0];

      for (size_t idx = 0; idx < count; ++idx)
      {
        auto data = &tmp[1 + idx * 6];
        tp[idx].size = 1;
        tp[idx].x = (data[0] & 0x0F) << 8 | data[1];
        tp[idx].y = (data[2] & 0x0F) << 8 | data[3];
        tp[idx].id = idx;
      }
      return count;
    }
  };

  struct Panel_M5StickC : public lgfx::Panel_ST7735S
  {
    Panel_M5StickC(void)
    {
      _cfg.invert = true;
      _cfg.pin_cs  = GPIO_NUM_5;
      _cfg.pin_rst = GPIO_NUM_18;
      _cfg.panel_width  = 80;
      _cfg.panel_height = 160;
      _cfg.offset_x     = 26;
      _cfg.offset_y     = 1;
      _cfg.offset_rotation = 2;
    }

  protected:

    const uint8_t* getInitCommands(uint8_t listno) const override
    {
      static constexpr uint8_t list[] = {
          CMD_GAMMASET, 1, 0x08,  // Gamma set, curve 4
          0xFF,0xFF, // end
      };
      if (listno == 2)  return list;
      return Panel_ST7735S::getInitCommands(listno);
    }
  };

  struct Light_M5StickC : public lgfx::ILight
  {
    bool init(uint8_t brightness) override
    {
      using namespace m5stack;
      lgfx::i2c::init(axp_i2c_port, axp_i2c_sda, axp_i2c_scl);
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x12, 0x4D, ~0, axp_i2c_freq);
      setBrightness(brightness);
      return true;
    }

    void setBrightness(uint8_t brightness) override
    {
      using namespace m5stack;
      if (brightness)
      {
        brightness = (((brightness >> 1) + 8) / 13) + 5;
        lgfx::i2c::bitOn(axp_i2c_port, axp_i2c_addr, 0x12, 1 << 2, axp_i2c_freq);
      }
      else
      {
        lgfx::i2c::bitOff(axp_i2c_port, axp_i2c_addr, 0x12, 1 << 2, axp_i2c_freq);
      }
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x28, brightness << 4, 0x0F, axp_i2c_freq);
    }
  };

  struct Panel_M5StickCPlus : public lgfx::Panel_ST7789
  {
    Panel_M5StickCPlus(void)
    {
      _cfg.invert = true;
      _cfg.pin_cs  = GPIO_NUM_5;
      _cfg.pin_rst = GPIO_NUM_18;
      _cfg.panel_width  = 135;
      _cfg.panel_height = 240;
      _cfg.offset_x     = 52;
      _cfg.offset_y     = 40;
    }
  };

  class LGFX : public LGFX_Device
  {
    lgfx::Panel_Device* _panel_last = nullptr;
    lgfx::ILight* _light_last = nullptr;
    lgfx::ITouch* _touch_last = nullptr;
    lgfx::Bus_SPI _bus_spi;

    static void _pin_level(int_fast16_t pin, bool level)
    {
      lgfx::pinMode(pin, lgfx::pin_mode_t::output);
      if (level) lgfx::gpio_hi(pin);
      else       lgfx::gpio_lo(pin);
    }

    static void _pin_reset(int_fast16_t pin, bool use_reset)
    {
      lgfx::gpio_hi(pin);
      lgfx::pinMode(pin, lgfx::pin_mode_t::output);
      if (!use_reset) return;
      lgfx::gpio_lo(pin);
      auto time = lgfx::millis();
      do
      {
        lgfx::delay(1);
      } while (lgfx::millis() - time < 2);
        lgfx::gpio_hi(pin);
      time = lgfx::millis();
      do
      {
        lgfx::delay(1);
      } while (lgfx::millis() - time < 10);
    }

    /// TF card dummy clock送信 ;
    static void _send_sd_dummy_clock(int spi_host, int_fast16_t pin_cs)
    {
      static constexpr uint32_t dummy_clock[] = { ~0u, ~0u, ~0u, ~0u };
      _pin_level(pin_cs, true);
      lgfx::spi::writeBytes(spi_host, (const uint8_t*)dummy_clock, sizeof(dummy_clock));
      _pin_level(pin_cs, false);
    }

    /// TF card をSPIモードに移行する ;
    static void _set_sd_spimode(int spi_host, int_fast16_t pin_cs)
    {
      lgfx::spi::beginTransaction(spi_host, 400000, 0);
      _send_sd_dummy_clock(spi_host, pin_cs);

      uint8_t sd_cmd58[] = { 0x7A, 0, 0, 0, 0, 0xFD, 0xFF, 0xFF }; // READ_OCR command.
      lgfx::spi::readBytes(spi_host, sd_cmd58, sizeof(sd_cmd58));

// ESP_LOGW(LIBRARY_NAME, "SD READ_OCR: %02x %02x", sd_cmd58[6], sd_cmd58[7]);
      if (sd_cmd58[6] == sd_cmd58[7])  // not SPI mode
      {
        _send_sd_dummy_clock(spi_host, pin_cs);

        static constexpr uint8_t sd_cmd0[] = { 0x40, 0, 0, 0, 0, 0x95, 0xFF, 0xFF }; // GO_IDLE_STATE command.
        lgfx::spi::writeBytes(spi_host, sd_cmd0, sizeof(sd_cmd0));
      }
      _pin_level(pin_cs, true);
      lgfx::spi::endTransaction(spi_host);
    }

    static uint32_t _read_panel_id(lgfx::Bus_SPI* bus, int32_t pin_cs, uint32_t cmd = 0x04, uint8_t dummy_read_bit = 1) // 0x04 = RDDID command
    {
      bus->beginTransaction();
      _pin_level(pin_cs, true);
      bus->writeCommand(0, 8);
      bus->wait();
      _pin_level(pin_cs, false);
      bus->writeCommand(cmd, 8);
      bus->beginRead(dummy_read_bit);
      uint32_t res = bus->readData(32);
      bus->endTransaction();
      _pin_level(pin_cs, true);

      ESP_LOGW(LIBRARY_NAME, "[Autodetect] read cmd:%02x = %08x", cmd, res);
      return res;
    }

    void _set_backlight(ILight* bl)
    {
      if (_light_last) { delete _light_last; }
      _light_last = bl;
      _panel_last->setLight(bl);
    }

    void _set_pwm_backlight(int16_t pin, uint8_t ch, uint32_t freq = 12000, bool invert = false)
    {
      auto bl = new lgfx::Light_PWM();
      auto cfg = bl->config();
      cfg.pin_bl = pin;
      cfg.freq   = freq;
      cfg.pwm_channel = ch;
      cfg.invert = invert;
      bl->config(cfg);
      _set_backlight(bl);
    }

    bool init_impl(bool use_reset, bool use_clear)
    {
      static constexpr char NVS_KEY[] = "AUTODETECT";
      uint32_t nvs_board = 0;
      uint32_t nvs_handle = 0;
      if (0 == nvs_open(LIBRARY_NAME, NVS_READONLY, &nvs_handle))
      {
        nvs_get_u32(nvs_handle, NVS_KEY, static_cast<uint32_t*>(&nvs_board));
        nvs_close(nvs_handle);
        ESP_LOGW(LIBRARY_NAME, "[Autodetect] load from NVS : board:%d", nvs_board);
      }

      if (0 == nvs_board)
      {
#if defined ( ARDUINO_M5Stack_Core_ESP32 ) || defined ( ARDUINO_M5STACK_FIRE )

        nvs_board = board_t::board_M5Stack;

#elif defined ( ARDUINO_M5STACK_Core2 )

        nvs_board = board_t::board_M5StackCore2;

#elif defined ( ARDUINO_M5Stick_C )

        nvs_board = board_t::board_M5StickC;

#elif defined ( ARDUINO_M5Stick_C_Plus )

        nvs_board = board_t::board_M5StickCPlus;

#elif defined ( ARDUINO_M5Stack_CoreInk )

        nvs_board = board_t::board_M5StackCoreInk;

#elif defined ( ARDUINO_M5STACK_Paper )

        nvs_board = board_t::board_M5Paper;

#elif defined ( ARDUINO_M5STACK_TOUGH )

        nvs_board = board_t::board_M5Tough;

#elif defined ( ARDUINO_M5Stack_ATOM )
//#elif defined ( ARDUINO_M5Stack_Timer_CAM )

#elif defined( ARDUINO_ODROID_ESP32 ) // ODROID-GO

        nvs_board = board_t::board_ODROID_GO;

#elif defined( ARDUINO_TTGO_T1 ) // TTGO TS

        nvs_board = board_t::board_TTGO_TS;

#elif defined( ARDUINO_TWatch ) || defined( ARDUINO_T ) // TTGO T-Watch

        nvs_board = board_t::board_TTGO_TWatch;

#elif defined( ARDUINO_D ) || defined( ARDUINO_DDUINO32_XS ) // DSTIKE D-duino-32 XS

        nvs_board = board_t::board_DDUINO32_XS;

#elif defined( ARDUINO_LOLIN_D32_PRO )

        nvs_board = board_t::board_LoLinD32;

#elif defined( ARDUINO_ESP32_WROVER_KIT )

        nvs_board = board_t::board_ESP_WROVER_KIT;

#endif

      }

      auto board = (board_t)nvs_board;

      int retry = 4;
      do
      {
        if (retry == 1) use_reset = true;
        board = autodetect(use_reset, board);
        //ESP_LOGI(LIBRARY_NAME,"autodetect board:%d", board);
      } while (board_t::board_unknown == board && --retry >= 0);
      _board = board;
      /// autodetectの際にreset済みなのでここではuse_resetをfalseで呼び出す。
      /// M5Paperはreset後の復帰に800msec程度掛かるのでreset省略は起動時間短縮に有効
      bool res = LGFX_Device::init_impl(false, use_clear);

      if (nvs_board != board) {
        if (0 == nvs_open(LIBRARY_NAME, NVS_READWRITE, &nvs_handle)) {
          ESP_LOGW(LIBRARY_NAME, "[Autodetect] save to NVS : board:%d", board);
          nvs_set_u32(nvs_handle, NVS_KEY, board);
          nvs_close(nvs_handle);
        }
      }
      return res;
    }

  public:

    board_t autodetect(bool use_reset = true, board_t board = board_t::board_unknown)
    {
      auto bus_cfg = _bus_spi.config();
//    if (bus_cfg.pin_mosi != -1 && bus_cfg.pin_sclk != -1) return true;

      panel(nullptr);

      if (_panel_last)
      {
        delete _panel_last;
        _panel_last = nullptr;
      }
      if (_light_last)
      {
        delete _light_last;
        _light_last = nullptr;
      }
      if (_touch_last)
      {
        delete _touch_last;
        _touch_last = nullptr;
      }

      bus_cfg.freq_write = 8000000;
      bus_cfg.freq_read  = 8000000;
      bus_cfg.spi_host = VSPI_HOST;
      bus_cfg.spi_mode = 0;
      bus_cfg.use_lock = true;
      bus_cfg.dma_channel = 1;
#if __has_include(<esp_idf_version.h>)
 #include <esp_idf_version.h>
 #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 3, 0)
      bus_cfg.dma_channel = SPI_DMA_CH_AUTO;
 #endif
#endif

      uint32_t id;
      (void)id;  // suppress warning

    std::uint32_t pkg_ver = lgfx::get_pkg_ver();
// ESP_LOGE("LGFX","pkg:%d", pkg_ver);

    if (pkg_ver == EFUSE_RD_CHIP_VER_PKG_ESP32PICOD4)
    {
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STICK_C ) || defined ( LGFX_M5STICKC )

      if (board == 0 || board == board_t::board_M5StickC || board == board_t::board_M5StickCPlus)
      {
        bus_cfg.pin_mosi = GPIO_NUM_15;
        bus_cfg.pin_miso = GPIO_NUM_14;
        bus_cfg.pin_sclk = GPIO_NUM_13;
        bus_cfg.pin_dc   = GPIO_NUM_23;
        bus_cfg.spi_3wire = true;
        _bus_spi.config(bus_cfg);
        _bus_spi.init();
        _pin_reset(GPIO_NUM_18, use_reset); // LCD RST
        id = _read_panel_id(&_bus_spi, GPIO_NUM_5);
        if ((id & 0xFF) == 0x85)
        {  //  check panel (ST7789)
          board = board_t::board_M5StickCPlus;
          ESP_LOGW(LIBRARY_NAME, "[Autodetect] M5StickCPlus");
          bus_cfg.freq_write = 40000000;
          bus_cfg.freq_read  = 15000000;
          _bus_spi.config(bus_cfg);
          auto p = new Panel_M5StickCPlus();
          p->bus(&_bus_spi);
          _panel_last = p;
          _set_backlight(new Light_M5StickC());
          goto init_clear;
        }
        if ((id & 0xFF) == 0x7C)
        {  //  check panel (ST7735)
          board = board_t::board_M5StickC;
          ESP_LOGW(LIBRARY_NAME, "[Autodetect] M5StickC");
          bus_cfg.freq_write = 27000000;
          bus_cfg.freq_read  = 14000000;
          _bus_spi.config(bus_cfg);
          auto p = new Panel_M5StickC();
          p->bus(&_bus_spi);
          _panel_last = p;
          _set_backlight(new Light_M5StickC());
          goto init_clear;
        }
        lgfx::pinMode(GPIO_NUM_18, lgfx::pin_mode_t::input); // LCD RST
        lgfx::pinMode(GPIO_NUM_5 , lgfx::pin_mode_t::input); // LCD CS
        _bus_spi.release();
      }

#endif

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_TTGO_TWRISTBAND )

      if (board == 0 || board == board_t::board_TTGO_TWristband)
      {
        bus_cfg.pin_mosi = GPIO_NUM_19;
        bus_cfg.pin_miso = -1;
        bus_cfg.pin_sclk = GPIO_NUM_18;
        bus_cfg.pin_dc   = GPIO_NUM_23;
        bus_cfg.spi_3wire = true;
        _bus_spi.config(bus_cfg);
        _bus_spi.init();
        _pin_reset(GPIO_NUM_26, use_reset); // LCD RST
        id = _read_panel_id(&_bus_spi, GPIO_NUM_5);
        if ((id & 0xFF) == 0x7C)
        {  //  check panel (ST7735)
          board = board_t::board_TTGO_TWristband;
          ESP_LOGW(LIBRARY_NAME, "[Autodetect] TWristband");
          bus_cfg.freq_write = 27000000;
          bus_cfg.freq_read  = 14000000;
          _bus_spi.config(bus_cfg);
          auto p = new Panel_ST7735S();
          p->bus(&_bus_spi);
          {
            auto cfg = p->config();
            cfg.dummy_read_pixel = 17;
            cfg.invert = true;
            cfg.pin_cs  = GPIO_NUM_5;
            cfg.pin_rst = GPIO_NUM_26;
            cfg.panel_width  = 80;
            cfg.panel_height = 160;
            cfg.offset_x     = 26;
            cfg.offset_y     = 1;
            cfg.offset_rotation = 2;
            p->config(cfg);
          }
          _panel_last = p;
          _set_pwm_backlight(GPIO_NUM_27, 7, 12000);
          goto init_clear;
        }
        lgfx::pinMode(GPIO_NUM_26, lgfx::pin_mode_t::input); // LCD RST
        lgfx::pinMode(GPIO_NUM_5 , lgfx::pin_mode_t::input); // LCD CS
        _bus_spi.release();
      }
#endif

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK_COREINK )

      if (board == 0 || board == board_t::board_M5StackCoreInk)
      {
        _pin_reset(GPIO_NUM_0, true); // EPDがDeepSleepしていると自動認識に失敗するためRST制御は必須とする
        bus_cfg.pin_mosi = GPIO_NUM_23;
        bus_cfg.pin_miso = GPIO_NUM_34;
        bus_cfg.pin_sclk = GPIO_NUM_18;
        bus_cfg.pin_dc   = GPIO_NUM_15;
        bus_cfg.spi_3wire = true;
        _bus_spi.config(bus_cfg);
        _bus_spi.init();
        id = _read_panel_id(&_bus_spi, GPIO_NUM_9, 0x70, 0);
        if (id == 0x00F00000)
        {  //  check panel (e-paper GDEW0154M09)
          _pin_level(GPIO_NUM_12, true);  // POWER_HOLD_PIN 12
          board = board_t::board_M5StackCoreInk;
          ESP_LOGW(LIBRARY_NAME, "[Autodetect] M5StackCoreInk");
          bus_cfg.freq_write = 40000000;
          bus_cfg.freq_read  = 16000000;
          _bus_spi.config(bus_cfg);
          auto p = new lgfx::Panel_GDEW0154M09();
          p->bus(&_bus_spi);
          _panel_last = p;
          auto cfg = p->config();
          cfg.panel_height = 200;
          cfg.panel_width  = 200;
          cfg.pin_cs   = GPIO_NUM_9;
          cfg.pin_rst  = GPIO_NUM_0;
          cfg.pin_busy = GPIO_NUM_4;
          p->config(cfg);
          goto init_clear;
        }
        lgfx::pinMode(GPIO_NUM_0, lgfx::pin_mode_t::input); // RST
        lgfx::pinMode(GPIO_NUM_9, lgfx::pin_mode_t::input); // CS
        _bus_spi.release();
      }
#endif

/// CS と D/C に GPIO 16 17を使っており、PSRAMと競合するため判定順序をなるべく後にする
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_TTGO_TS )
      if (board == 0 || board == board_t::board_TTGO_TS)
      {
        bus_cfg.pin_mosi = GPIO_NUM_23;
        bus_cfg.pin_miso = -1;
        bus_cfg.pin_sclk = GPIO_NUM_5;
        bus_cfg.pin_dc   = GPIO_NUM_17;
        bus_cfg.spi_3wire = true;
        _bus_spi.config(bus_cfg);
        _bus_spi.init();
        _pin_reset(GPIO_NUM_9, use_reset); // LCD RST
        id = _read_panel_id(&_bus_spi, GPIO_NUM_16);
        if ((id & 0xFF) == 0x7C)
        {  //  check panel (ST7735)
          board = board_t::board_TTGO_TS;
          ESP_LOGW(LIBRARY_NAME, "[Autodetect] TTGO TS");
          bus_cfg.freq_write = 20000000;
          bus_cfg.freq_read  = 14000000;
          _bus_spi.config(bus_cfg);
          auto p = new Panel_ST7735S();
          p->bus(&_bus_spi);
          {
            auto cfg = p->config();
            cfg.pin_cs  = GPIO_NUM_16;
            cfg.pin_rst = GPIO_NUM_9;
            cfg.panel_width  = 128;
            cfg.panel_height = 160;
            cfg.offset_x     = 2;
            cfg.offset_y     = 1;
            cfg.offset_rotation = 2;
            p->config(cfg);
          }
          _panel_last = p;
          _set_pwm_backlight(GPIO_NUM_27, 7, 12000);
          goto init_clear;
        }
        lgfx::pinMode(GPIO_NUM_9 , lgfx::pin_mode_t::input); // LCD RST
        lgfx::pinMode(GPIO_NUM_16, lgfx::pin_mode_t::input); // LCD CS
        _bus_spi.release();
      }
#endif

    }
    else
    if (pkg_ver == EFUSE_RD_CHIP_VER_PKG_ESP32D0WDQ5
     || pkg_ver == EFUSE_RD_CHIP_VER_PKG_ESP32D0WDQ6)
    {
    // ODROID_GO 判定 (ボードマネージャでM5StickCを選択していると判定失敗する事に注意);
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_ODROID_GO )

      if (board == 0 || board == board_t::board_ODROID_GO)
      {
        _pin_level(GPIO_NUM_5, true);

        bus_cfg.pin_mosi = GPIO_NUM_23;
        bus_cfg.pin_miso = GPIO_NUM_19;
        bus_cfg.pin_sclk = GPIO_NUM_18;
        bus_cfg.pin_dc   = GPIO_NUM_21;
        bus_cfg.spi_3wire = true;
        _bus_spi.config(bus_cfg);
        _bus_spi.init();

        _set_sd_spimode(bus_cfg.spi_host, GPIO_NUM_22);

        id = _read_panel_id(&_bus_spi, GPIO_NUM_5, 0x09);
        if (id != 0 && id != ~0u && id != 0xFFFFFF00 && (_read_panel_id(&_bus_spi, GPIO_NUM_5) & 0xFF) == 0)
        {   // check panel (ILI9341) panelIDが0なのでステータスリード0x09を併用する;
          board = board_t::board_ODROID_GO;
          ESP_LOGW(LIBRARY_NAME, "[Autodetect] ODROID_GO");
          bus_cfg.freq_write = 40000000;
          bus_cfg.freq_read  = 16000000;
          _bus_spi.config(bus_cfg);
          auto p = new Panel_ILI9341();
          p->bus(&_bus_spi);
          {
            auto cfg = p->config();
            cfg.pin_cs  = GPIO_NUM_5;
            p->config(cfg);
            p->setRotation(1);
          }
          _panel_last = p;
          _set_pwm_backlight(GPIO_NUM_14, 7);
          goto init_clear;
        }
        lgfx::pinMode(GPIO_NUM_22, lgfx::pin_mode_t::input); // ODROID-GO TF card CS
        lgfx::pinMode(GPIO_NUM_5 , lgfx::pin_mode_t::input); // LCD CS
        _bus_spi.release();
      }
#endif

      if (pkg_ver == EFUSE_RD_CHIP_VER_PKG_ESP32D0WDQ6)
      {

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STATION ) || defined ( LGFX_M5STACK_CORE2 ) || defined ( LGFX_M5STACKCORE2 ) || defined ( LGFX_M5TOUGH )

        /// AXP192の有無を最初に判定し、分岐する。
        if (board == 0
        || board == board_t::board_M5Station
        || board == board_t::board_M5StackCore2
        || board == board_t::board_M5Tough)
        {
          using namespace m5stack;
          // I2C addr 0x34 = AXP192
          lgfx::i2c::init(axp_i2c_port, axp_i2c_sda, axp_i2c_scl);
          if (lgfx::i2c::readRegister8(axp_i2c_port, axp_i2c_addr, 0x03, 400000) == 0x03) // AXP192 found
          {
            ESP_LOGW(LIBRARY_NAME, "AXP192 found.");

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STATION )

            if (board == 0 || board == board_t::board_M5Station)
            {
              bus_cfg.pin_mosi = GPIO_NUM_23;
              bus_cfg.pin_miso = -1;
              bus_cfg.pin_sclk = GPIO_NUM_18;
              bus_cfg.pin_dc   = GPIO_NUM_19;
              bus_cfg.spi_3wire = true;
              _bus_spi.config(bus_cfg);
              _bus_spi.init();
              _pin_reset(GPIO_NUM_15, use_reset); // LCD RST;

              id = _read_panel_id(&_bus_spi, GPIO_NUM_5);
              if ((id & 0xFF) == 0x85)
              {   // ST7789
                ESP_LOGW(LIBRARY_NAME, "[Autodetect] M5Station");
                board = board_t::board_M5Station;

                bus_cfg.freq_write = 40000000;
                bus_cfg.freq_read  = 15000000;
                _bus_spi.config(bus_cfg);

                auto p = new Panel_M5StickCPlus();
                {
                  auto cfg = p->config();
                  cfg.pin_rst = GPIO_NUM_15;
                  p->config(cfg);
                  p->setRotation(1);
                }
                p->bus(&_bus_spi);
                _panel_last = p;
                /// M5StationのバックライトはM5Toughと同じ;
                _set_backlight(new Light_M5Tough());
                goto init_clear;
              }
              _bus_spi.release();
              lgfx::pinMode(GPIO_NUM_5 , lgfx::pin_mode_t::input); // LCD CS
              lgfx::pinMode(GPIO_NUM_15, lgfx::pin_mode_t::input); // LCD RST
            }

#endif

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK_CORE2 ) || defined ( LGFX_M5STACKCORE2 ) || defined ( LGFX_M5TOUGH )

            if (board == 0 || board == board_t::board_M5StackCore2 || board == board_t::board_M5Tough)
            {
              using namespace m5stack;

              // AXP192_LDO2 = LCD PWR
              // AXP192_IO4  = LCD RST
              // AXP192_DC3  = LCD BL (Core2)
              // AXP192_LDO3 = LCD BL (Tough)
              // AXP192_IO1  = TP RST (Tough)
              lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x28, 0xF0, ~0, axp_i2c_freq);   // set LDO2 3300mv // LCD PWR
              lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x12, 0x04, ~0, axp_i2c_freq);   // LDO2 enable
              lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x92, 0x00, 0xF8, axp_i2c_freq); // GPIO1 OpenDrain (M5Tough TOUCH)
              lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x95, 0x84, 0x72, axp_i2c_freq); // GPIO4 enable
              if (use_reset)
              {
                lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x96, 0, ~0x02, axp_i2c_freq); // GPIO4 LOW (LCD RST)
                lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x94, 0, ~0x02, axp_i2c_freq); // GPIO1 LOW (M5Tough TOUCH RST)
                lgfx::delay(1);
              }
              lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x96, 0x02, ~0, axp_i2c_freq);   // GPIO4 HIGH (LCD RST)
              lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x94, 0x02, ~0, axp_i2c_freq);   // GPIO1 HIGH (M5Tough TOUCH RST)

              _pin_level(GPIO_NUM_5, true);

              bus_cfg.pin_mosi = GPIO_NUM_23;
              bus_cfg.pin_miso = GPIO_NUM_38;
              bus_cfg.pin_sclk = GPIO_NUM_18;
              bus_cfg.pin_dc   = GPIO_NUM_15;
              bus_cfg.spi_3wire = true;
              _bus_spi.config(bus_cfg);
              _bus_spi.init();

              _set_sd_spimode(bus_cfg.spi_host, GPIO_NUM_4);

              id = _read_panel_id(&_bus_spi, GPIO_NUM_5);
              if ((id & 0xFF) == 0xE3)
              {   // ILI9342c
                bus_cfg.freq_write = 40000000;
                bus_cfg.freq_read  = 16000000;
                _bus_spi.config(bus_cfg);

                auto p = new Panel_M5StackCore2();
                p->bus(&_bus_spi);
                _panel_last = p;

                // Tough のタッチコントローラ有無をチェックする;
                // Core2/Tough 判別条件としてCore2のTP(0x38)の有無を用いた場合、以下の問題が生じる;
                // ・Core2のTPがスリープしている場合は反応が得られない;
                // ・ToughにGoPlus2を組み合わせると0x38に反応がある;
                // 上記のことから、ここではToughのTP(0x2E)の有無によって判定する;
                if ( ! lgfx::i2c::readRegister8(axp_i2c_port, 0x2E, 0, 400000).has_value()) // 0x2E:M5Tough TOUCH
                {
                  ESP_LOGW(LIBRARY_NAME, "[Autodetect] M5StackCore2");
                  board = board_t::board_M5StackCore2;

                  _set_backlight(new Light_M5StackCore2());

                  auto t = new lgfx::Touch_FT5x06();
                  _touch_last = t;
                  auto cfg = t->config();
                  cfg.pin_int  = GPIO_NUM_39;   // INT pin number
                  cfg.pin_sda  = GPIO_NUM_21;   // I2C SDA pin number
                  cfg.pin_scl  = GPIO_NUM_22;   // I2C SCL pin number
                  cfg.i2c_addr = 0x38; // I2C device addr
                  cfg.i2c_port = I2C_NUM_1;// I2C port number
                  cfg.freq = 400000;   // I2C freq
                  cfg.x_min = 0;
                  cfg.x_max = 319;
                  cfg.y_min = 0;
                  cfg.y_max = 279;
                  cfg.bus_shared = false;
                  t->config(cfg);
                  p->touch(t);
                  float affine[6] = { 1, 0, 0, 0, 1, 0 };
                  p->setCalibrateAffine(affine);
                }
                else
                {
                  // AXP192のGPIO1 = タッチコントローラRST
                  ESP_LOGW(LIBRARY_NAME, "[Autodetect] M5Tough");
                  board = board_t::board_M5Tough;

                  _set_backlight(new Light_M5Tough());

                  auto t = new lgfx::Touch_M5Tough();
                  _touch_last = t;
                  auto cfg = t->config();
                  cfg.pin_int  = GPIO_NUM_39;   // INT pin number
                  cfg.pin_sda  = GPIO_NUM_21;   // I2C SDA pin number
                  cfg.pin_scl  = GPIO_NUM_22;   // I2C SCL pin number
                  cfg.i2c_addr = 0x2E; // I2C device addr
                  cfg.i2c_port = I2C_NUM_1;// I2C port number
                  cfg.freq = 400000;   // I2C freq
                  cfg.bus_shared = false;
                  t->config(cfg);
                  p->touch(t);
                }

                goto init_clear;
              }
              lgfx::pinMode(GPIO_NUM_4, lgfx::pin_mode_t::input); // TF card CS
              lgfx::pinMode(GPIO_NUM_5, lgfx::pin_mode_t::input); // LCD CS
              _bus_spi.release();
            }
#endif
          }
          else
          {
            ESP_LOGW(LIBRARY_NAME, "AXP192 not found.");
          }
          lgfx::i2c::release(axp_i2c_port);
        }

#endif

// TTGO T-Watch 判定 (GPIO33を使う判定を先に行うと振動モーターが作動する事に注意);
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_TTGO_TWATCH )

        if (board == 0 || board == board_t::board_TTGO_TWatch)
        {
          bus_cfg.pin_mosi = GPIO_NUM_19;
          bus_cfg.pin_miso = -1;
          bus_cfg.pin_sclk = GPIO_NUM_18;
          bus_cfg.pin_dc   = GPIO_NUM_27;
          bus_cfg.spi_3wire = true;
          _bus_spi.config(bus_cfg);
          _bus_spi.init();

          id = _read_panel_id(&_bus_spi, GPIO_NUM_5);
          if ((id & 0xFF) == 0x85)
          {  //  check panel (ST7789)
            board = board_t::board_TTGO_TWatch;
            ESP_LOGW(LIBRARY_NAME, "[Autodetect] TWatch");
            _bus_spi.release();
            bus_cfg.spi_host = HSPI_HOST;
            bus_cfg.freq_write = 80000000;
            bus_cfg.freq_read  = 20000000;
            _bus_spi.config(bus_cfg);
            _bus_spi.init();
            auto p = new Panel_ST7789();
            p->bus(&_bus_spi);
            {
              auto cfg = p->config();
              cfg.invert = true;
              cfg.pin_cs  = GPIO_NUM_5;
              cfg.pin_rst = GPIO_NUM_26;
              cfg.panel_width  = 240;
              cfg.panel_height = 240;
              p->config(cfg);
            }
            _panel_last = p;

            _pin_level(GPIO_NUM_15, true);
            _pin_level(GPIO_NUM_25, true);
            _set_backlight(new Light_TWatch());

            {
              auto t = new lgfx::Touch_FT5x06();
              _touch_last = t;
              auto cfg = t->config();
              cfg.pin_int  = GPIO_NUM_38;   // INT pin number
              cfg.pin_sda  = GPIO_NUM_23;   // I2C SDA pin number
              cfg.pin_scl  = GPIO_NUM_32;   // I2C SCL pin number
              cfg.i2c_addr = 0x38; // I2C device addr
              cfg.i2c_port = I2C_NUM_1;// I2C port number
              cfg.freq = 400000;   // I2C freq
              cfg.x_min = 0;
              cfg.x_max = 319;
              cfg.y_min = 0;
              cfg.y_max = 319;
              cfg.bus_shared = false;
              t->config(cfg);
              p->touch(t);
            }

            goto init_clear;
          }
          lgfx::pinMode(GPIO_NUM_5, lgfx::pin_mode_t::input); // LCD CS
          _bus_spi.release();
        }
#endif

// TTGO T-Display
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_TTGO_TDISPLAY )

        if (board == 0 || board == board_t::board_TTGO_TDisplay)
        {
          bus_cfg.pin_mosi = GPIO_NUM_19;
          bus_cfg.pin_miso = -1;
          bus_cfg.pin_sclk = GPIO_NUM_18;
          bus_cfg.pin_dc   = GPIO_NUM_16;
          bus_cfg.spi_3wire = true;
          _bus_spi.config(bus_cfg);
          _bus_spi.init();

          id = _read_panel_id(&_bus_spi, GPIO_NUM_5);
          if ((id & 0xFF) == 0x85)
          {  //  check panel (ST7789)
            board = board_t::board_TTGO_TDisplay;
            ESP_LOGW(LIBRARY_NAME, "[Autodetect] TDisplay");
            _bus_spi.release();
            bus_cfg.spi_host = HSPI_HOST;
            bus_cfg.freq_write = 40000000;
            bus_cfg.freq_read  = 14000000;
            _bus_spi.config(bus_cfg);
            _bus_spi.init();
            auto p = new Panel_ST7789();
            p->bus(&_bus_spi);
            {
              auto cfg = p->config();
              cfg.invert = true;
              cfg.pin_cs  = GPIO_NUM_5;
              cfg.pin_rst = GPIO_NUM_23;
              cfg.panel_width  = 135;
              cfg.panel_height = 240;
              cfg.offset_x     = 52;
              cfg.offset_y     = 40;
              cfg.dummy_read_pixel = 16;
              cfg.dummy_read_bits  =  1;
              p->config(cfg);
            }
            _panel_last = p;

            _set_pwm_backlight(GPIO_NUM_4, 7, 44100);

            goto init_clear;
          }
          lgfx::pinMode(GPIO_NUM_5, lgfx::pin_mode_t::input); // LCD CS
          _bus_spi.release();
        }
#endif

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_WIFIBOY_MINI )

        if (board == 0 || board == board_t::board_WiFiBoy_Mini)
        {
          bus_cfg.pin_mosi = GPIO_NUM_13;
          bus_cfg.pin_miso = GPIO_NUM_12;
          bus_cfg.pin_sclk = GPIO_NUM_14;
          bus_cfg.pin_dc   = GPIO_NUM_4;
          bus_cfg.spi_3wire = true;
          _bus_spi.config(bus_cfg);
          _bus_spi.init();
          id = _read_panel_id(&_bus_spi, GPIO_NUM_15);
          if ((id & 0xFF) == 0x7C)
          {  //  check panel (ST7735)
            board = board_t::board_WiFiBoy_Mini;
            ESP_LOGW(LIBRARY_NAME, "[Autodetect] WiFiBoy mini");
            _bus_spi.release();
            bus_cfg.spi_host = HSPI_HOST;
            bus_cfg.freq_write = 20000000;
            bus_cfg.freq_read  =  8000000;
            _bus_spi.config(bus_cfg);
            _bus_spi.init();
            auto p = new Panel_ST7735S();
            p->bus(&_bus_spi);
            {
              auto cfg = p->config();
              cfg.pin_cs  = GPIO_NUM_15;
              cfg.pin_rst = -1;
              cfg.panel_width  = 128;
              cfg.panel_height = 128;
              cfg.memory_width  = 132;
              cfg.memory_height = 132;
              cfg.offset_x     = 2;
              cfg.offset_y     = 1;
              cfg.offset_rotation = 2;
              p->config(cfg);
            }
            _panel_last = p;
            _set_pwm_backlight(GPIO_NUM_27, 7, 12000);
            goto init_clear;
          }
          lgfx::pinMode(GPIO_NUM_15, lgfx::pin_mode_t::input); // LCD CS
          _bus_spi.release();
        }
#endif

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_WIFIBOY_PRO )

        if (board == 0 || board == board_t::board_WiFiBoy_Pro)
        {
          bus_cfg.pin_mosi = GPIO_NUM_13;
          bus_cfg.pin_miso = GPIO_NUM_12;
          bus_cfg.pin_sclk = GPIO_NUM_14;
          bus_cfg.pin_dc   = GPIO_NUM_4;
          bus_cfg.spi_3wire = false;
          _bus_spi.config(bus_cfg);
          _bus_spi.init();
          id = _read_panel_id(&_bus_spi, GPIO_NUM_15);
          if ((id & 0xFF) == 0 && _read_panel_id(&_bus_spi, GPIO_NUM_15, 0x09) != 0)
          {   // check panel (ILI9341) panelIDが0なのでステータスリード0x09を併用する
            board = board_t::board_WiFiBoy_Pro;
            ESP_LOGW(LIBRARY_NAME, "[Autodetect] WiFiBoy Pro");
            _bus_spi.release();
            bus_cfg.spi_host = HSPI_HOST;
            bus_cfg.freq_write = 40000000;
            bus_cfg.freq_read  = 16000000;
            _bus_spi.config(bus_cfg);
            _bus_spi.init();
            auto p = new Panel_ILI9341();
            p->bus(&_bus_spi);
            {
              auto cfg = p->config();
              cfg.pin_cs  = GPIO_NUM_15;
              cfg.pin_rst = -1;
              cfg.offset_rotation = 2;
              p->config(cfg);
            }
            _panel_last = p;
            _set_pwm_backlight(GPIO_NUM_27, 7, 12000);
            goto init_clear;
          }
          lgfx::pinMode(GPIO_NUM_15, lgfx::pin_mode_t::input); // LCD CS
          _bus_spi.release();
        }
#endif

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_MAKERFABS_MAKEPYTHON )

        if (board == 0 || board == board_t::board_Makerfabs_MakePython)
        {
          bus_cfg.pin_mosi = GPIO_NUM_13;
          bus_cfg.pin_miso = GPIO_NUM_12;
          bus_cfg.pin_sclk = GPIO_NUM_14;
          bus_cfg.pin_dc   = GPIO_NUM_22;
          bus_cfg.spi_3wire = true;
          _bus_spi.config(bus_cfg);
          _bus_spi.init();
          _pin_reset(GPIO_NUM_21, use_reset); // LCD RST
          id = _read_panel_id(&_bus_spi, GPIO_NUM_15);
          if ((id & 0xFF) == 0x85)
          {  //  check panel (ST7789)
            board = board_t::board_Makerfabs_MakePython;
            ESP_LOGW(LIBRARY_NAME, "[Autodetect] Makerfabs_Makepython");
            _bus_spi.release();
            bus_cfg.spi_host = HSPI_HOST;
            bus_cfg.freq_write = 80000000;
            bus_cfg.freq_read  = 14000000;
            _bus_spi.config(bus_cfg);
            _bus_spi.init();
            auto p = new Panel_ST7789();
            p->bus(&_bus_spi);
            {
              auto cfg = p->config();
              cfg.invert = true;
              cfg.pin_cs  = GPIO_NUM_15;
              cfg.pin_rst = GPIO_NUM_21;
              cfg.panel_height = 240;
              p->config(cfg);
            }
            _panel_last = p;
            _set_pwm_backlight(GPIO_NUM_5, 7, 12000);
            goto init_clear;
          }
          lgfx::pinMode(GPIO_NUM_15, lgfx::pin_mode_t::input); // LCD CS
          lgfx::pinMode(GPIO_NUM_21, lgfx::pin_mode_t::input); // LCD RST
          _bus_spi.release();
        }
#endif

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK )

        if (board == 0 || board == board_t::board_M5Stack)
        {
          _pin_level(GPIO_NUM_14, true);     // LCD CS;
          bus_cfg.pin_mosi = GPIO_NUM_23;
          bus_cfg.pin_miso = GPIO_NUM_19;
          bus_cfg.pin_sclk = GPIO_NUM_18;
          bus_cfg.pin_dc   = GPIO_NUM_27;

          bus_cfg.spi_3wire = true;
          _bus_spi.config(bus_cfg);
          _bus_spi.init();
          _pin_reset(GPIO_NUM_33, use_reset); // LCD RST;

          _set_sd_spimode(bus_cfg.spi_host, GPIO_NUM_4);

          id = _read_panel_id(&_bus_spi, GPIO_NUM_14);

          if ((id & 0xFF) == 0xE3)
          {   // ILI9342c
            ESP_LOGW(LIBRARY_NAME, "[Autodetect] M5Stack");
            board = board_t::board_M5Stack;

            bus_cfg.freq_write = 40000000;
            bus_cfg.freq_read  = 16000000;
            _bus_spi.config(bus_cfg);

            auto p = new Panel_M5Stack();
            p->bus(&_bus_spi);
            _panel_last = p;
            _set_pwm_backlight(GPIO_NUM_32, 7, 44100);
            goto init_clear;
          }
          _bus_spi.release();

          lgfx::pinMode(GPIO_NUM_4 , lgfx::pin_mode_t::input); // M5Stack and LoLinD32 TF card CS
          lgfx::pinMode(GPIO_NUM_14, lgfx::pin_mode_t::input); // LCD CS
          lgfx::pinMode(GPIO_NUM_33, lgfx::pin_mode_t::input); // LCD RST
        }

#endif

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5PAPER )

        if (board == 0 || board == board_t::board_M5Paper)
        {
          _pin_reset(GPIO_NUM_23, true);
          lgfx::pinMode(GPIO_NUM_27, lgfx::pin_mode_t::input_pullup); // M5Paper EPD busy pin
          if (!lgfx::gpio_in(GPIO_NUM_27))
          {
            _pin_level(GPIO_NUM_2, true);  // M5EPD_MAIN_PWR_PIN 2
            lgfx::pinMode(GPIO_NUM_27, lgfx::pin_mode_t::input);
  //ESP_LOGW(LIBRARY_NAME, "check M5Paper");
            bus_cfg.pin_mosi = GPIO_NUM_12;
            bus_cfg.pin_miso = GPIO_NUM_13;
            bus_cfg.pin_sclk = GPIO_NUM_14;
            bus_cfg.pin_dc   = -1;
            bus_cfg.spi_3wire = false;
            _bus_spi.config(bus_cfg);
            id = lgfx::millis();

            _pin_level(GPIO_NUM_15, true); // M5Paper CS;
            _bus_spi.init();
            _set_sd_spimode(bus_cfg.spi_host, GPIO_NUM_4);
            do
            {
              vTaskDelay(1);
              if (lgfx::millis() - id > 1024) { id = 0; break; }
            } while (!lgfx::gpio_in(GPIO_NUM_27));
            if (id)
            {
  //ESP_LOGW(LIBRARY_NAME, "ms:%d", millis() - id);
              _bus_spi.beginTransaction();
              lgfx::gpio_lo(GPIO_NUM_15);
              _bus_spi.writeData(__builtin_bswap16(0x6000), 16);
              _bus_spi.writeData(__builtin_bswap16(0x0302), 16);  // read DevInfo
              id = lgfx::millis();
              _bus_spi.wait();
              lgfx::gpio_hi(GPIO_NUM_15);
              do
              {
                vTaskDelay(1);
                if (lgfx::millis() - id > 192) { break; }
              } while (!lgfx::gpio_in(GPIO_NUM_27));
              lgfx::gpio_lo(GPIO_NUM_15);
              _bus_spi.writeData(__builtin_bswap16(0x1000), 16);
              _bus_spi.writeData(__builtin_bswap16(0x0000), 16);
              uint8_t buf[40];
              _bus_spi.beginRead();
              _bus_spi.readBytes(buf, 40, false);
              _bus_spi.endRead();
              _bus_spi.endTransaction();
              lgfx::gpio_hi(GPIO_NUM_15);
              id = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
              ESP_LOGW(LIBRARY_NAME, "[Autodetect] panel size :%08x", id);
              if (id == 0x03C0021C)
              {  //  check panel ( panel size 960(0x03C0) x 540(0x021C) )
                board = board_t::board_M5Paper;
                ESP_LOGW(LIBRARY_NAME, "[Autodetect] M5Paper");
                bus_cfg.freq_write = 40000000;
                bus_cfg.freq_read  = 20000000;
                _bus_spi.config(bus_cfg);
                {
                  auto p = new lgfx::Panel_IT8951();
                  p->bus(&_bus_spi);
                  _panel_last = p;
                  auto cfg = p->config();
                  cfg.panel_height = 540;
                  cfg.panel_width  = 960;
                  cfg.pin_cs   = GPIO_NUM_15;
                  cfg.pin_rst  = GPIO_NUM_23;
                  cfg.pin_busy = GPIO_NUM_27;
                  cfg.offset_rotation = 3;
                  p->config(cfg);
                }
                {
                  auto t = new lgfx::Touch_GT911();
                  _touch_last = t;
                  auto cfg = t->config();
                  cfg.pin_int  = GPIO_NUM_36;   // INT pin number
                  cfg.pin_sda  = GPIO_NUM_21;   // I2C SDA pin number
                  cfg.pin_scl  = GPIO_NUM_22;   // I2C SCL pin number
                  cfg.i2c_addr = 0x14; // I2C device addr
  #ifdef _M5EPD_H_
                  cfg.i2c_port = I2C_NUM_0;// I2C port number
  #else
                  cfg.i2c_port = I2C_NUM_1;// I2C port number
  #endif
                  cfg.freq = 400000;   // I2C freq
                  cfg.x_min = 0;
                  cfg.x_max = 539;
                  cfg.y_min = 0;
                  cfg.y_max = 959;
                  cfg.offset_rotation = 1;
                  cfg.bus_shared = false;
                  t->config(cfg);
                  if (!t->init())
                  {
                    cfg.i2c_addr = 0x5D; // addr change (0x14 or 0x5D)
                    t->config(cfg);
                  }
                  _panel_last->touch(t);
                }
                goto init_clear;
              }
            }
            _bus_spi.release();
            lgfx::pinMode(GPIO_NUM_15, lgfx::pin_mode_t::input); // EPD CS
            lgfx::pinMode(GPIO_NUM_4 , lgfx::pin_mode_t::input); // M5Paper TF card CS
            lgfx::pinMode(GPIO_NUM_2, lgfx::pin_mode_t::input); // M5EPD_MAIN_PWR_PIN 2
          }
          lgfx::pinMode(GPIO_NUM_27, lgfx::pin_mode_t::input); // BUSY
          lgfx::pinMode(GPIO_NUM_23, lgfx::pin_mode_t::input); // RST
        }
  #endif

      }
      else
      if (pkg_ver == EFUSE_RD_CHIP_VER_PKG_ESP32D0WDQ5)
      {

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_MAKERFABS_TOUCHCAMERA )

        if (board == 0 || board == board_t::board_Makerfabs_TouchCamera)
        {
          bus_cfg.pin_mosi = GPIO_NUM_13;
          bus_cfg.pin_miso = GPIO_NUM_12;
          bus_cfg.pin_sclk = GPIO_NUM_14;
          bus_cfg.pin_dc   = GPIO_NUM_33;
          bus_cfg.spi_3wire = false;
          _bus_spi.config(bus_cfg);
          _bus_spi.init();
          _pin_level(GPIO_NUM_4, true); // TF card CS
          _pin_level(GPIO_NUM_2, true); // TOUCH CS

          id = _read_panel_id(&_bus_spi, GPIO_NUM_15, 0x09);
          if (id)
          {
            id = _read_panel_id(&_bus_spi, GPIO_NUM_15) & 0xFF;
            if (id == 0x00)
            { // check panel (ILI9341)
              board = board_t::board_Makerfabs_TouchCamera;
              ESP_LOGW(LIBRARY_NAME, "[Autodetect] Makerfabs_TouchCamera(ILI9341)");  // 3.2inch
              bus_cfg.freq_write = 40000000;
              bus_cfg.freq_read  = 16000000;
              _bus_spi.config(bus_cfg);
              auto p = new Panel_ILI9341();
              p->bus(&_bus_spi);
              {
                auto cfg = p->config();
                cfg.pin_cs  = GPIO_NUM_15;
                cfg.pin_rst = -1;
                p->config(cfg);
              }
              _panel_last = p;
              {
                auto t = new lgfx::Touch_STMPE610();
                _touch_last = t;
                auto cfg = t->config();
                cfg.bus_shared = true;
                cfg.spi_host = VSPI_HOST;
                cfg.pin_cs   = GPIO_NUM_2;
                cfg.pin_mosi = GPIO_NUM_13;
                cfg.pin_miso = GPIO_NUM_12;
                cfg.pin_sclk = GPIO_NUM_14;
                cfg.offset_rotation = 2;
                t->config(cfg);
                p->touch(t);
              }

              goto init_clear;

            }
            else
            if (id == 0x54)
            { // check panel (ILI9488)
              board = board_t::board_Makerfabs_TouchCamera;
              ESP_LOGW(LIBRARY_NAME, "[Autodetect] Makerfabs_TouchCamera");  // 3.5inch
              bus_cfg.freq_write = 40000000;
              bus_cfg.freq_read  = 16000000;
              _bus_spi.config(bus_cfg);
              auto p = new Panel_ILI9488();
              p->bus(&_bus_spi);
              {
                auto cfg = p->config();
                cfg.pin_cs  = GPIO_NUM_15;
                cfg.pin_rst = -1;
                p->config(cfg);
              }
              _panel_last = p;

              if (lgfx::i2c::init(I2C_NUM_1, GPIO_NUM_26, GPIO_NUM_27).has_value())
              {
                ITouch* t = nullptr;
                ITouch::config_t cfg;
                if (!lgfx::i2c::readRegister8(I2C_NUM_1, 0x38, 0, 400000).has_value()) // 0x48:NS2009
                {
                  t = new lgfx::Touch_NS2009();
                  cfg = t->config();
                  cfg.i2c_addr = 0x48; // I2C device addr
                  cfg.x_min = 460;
                  cfg.x_max = 3680;
                  cfg.y_min = 150;
                  cfg.y_max = 3780;
                  cfg.pin_int  = GPIO_NUM_0;   // INT pin number
                }
                else
                {
                  t = new lgfx::Touch_FT5x06();
                  cfg = t->config();
                  cfg.i2c_addr = 0x38; // I2C device addr
                  cfg.x_min = 0;
                  cfg.x_max = 319;
                  cfg.y_min = 0;
                  cfg.y_max = 479;
                //cfg.pin_int  = GPIO_NUM_0;   // INT pin number (board ver1.1 doesn't work)
                }

                if (t)
                {
                  cfg.bus_shared = false;
                  cfg.pin_sda  = GPIO_NUM_26;   // I2C SDA pin number
                  cfg.pin_scl  = GPIO_NUM_27;   // I2C SCL pin number
                  cfg.i2c_port = I2C_NUM_1;// I2C port number
                  cfg.freq = 400000;   // I2C freq
                  _touch_last = t;
                  t->config(cfg);
                  p->touch(t);
                }
              }

              goto init_clear;
            }
          }

          lgfx::pinMode(GPIO_NUM_15, lgfx::pin_mode_t::input); // LCD CS
          lgfx::pinMode(GPIO_NUM_4 , lgfx::pin_mode_t::input); // TF card CS
          lgfx::pinMode(GPIO_NUM_2 , lgfx::pin_mode_t::input); // TOUCH CS
          _bus_spi.release();
        }
#endif

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_LOLIN_D32_PRO )

        if (board == 0 || board == board_t::board_LoLinD32)
        {
          _pin_level(GPIO_NUM_14, true);     // LCD CS;
          bus_cfg.pin_mosi = GPIO_NUM_23;
          bus_cfg.pin_miso = GPIO_NUM_19;
          bus_cfg.pin_sclk = GPIO_NUM_18;
          bus_cfg.pin_dc   = GPIO_NUM_27;

          _pin_level(GPIO_NUM_4, true);  // M5Stack and LoLinD32 TF card CS
          #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_LOLIN_D32_PRO )
          _pin_level(GPIO_NUM_12, true);  // LoLinD32 TouchScreen CS
          #endif

          bus_cfg.spi_3wire = true;
          _bus_spi.config(bus_cfg);
          _bus_spi.init();
          _pin_reset(GPIO_NUM_33, use_reset); // LCD RST;

          id = _read_panel_id(&_bus_spi, GPIO_NUM_14);

          if ((id & 0xFF) == 0x7C)
          { //  check panel (ST7735)
            ESP_LOGW(LIBRARY_NAME, "[Autodetect] LoLinD32Pro ST7735");
            board = board_t::board_LoLinD32;

            #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK )
            lgfx::pinMode(GPIO_NUM_4, lgfx::pin_mode_t::input); // M5Stack TF card CS
            #endif

            bus_cfg.freq_write = 27000000;
            bus_cfg.freq_read  = 16000000;
            _bus_spi.config(bus_cfg);

            auto p = new lgfx::Panel_ST7735S();
            {
              auto cfg = p->config();
              cfg.pin_cs  = GPIO_NUM_14;
              cfg.pin_rst = GPIO_NUM_33;
              cfg.memory_width  = 132;
              cfg.memory_height = 132;
              cfg.panel_width  = 128;
              cfg.panel_height = 128;
              cfg.offset_x = 2;
              cfg.offset_y = 1;
              p->config(cfg);
            }
            p->bus(&_bus_spi);
            _panel_last = p;

            _set_pwm_backlight(GPIO_NUM_32, 7, 44100);

            goto init_clear;
          }

          bus_cfg.spi_3wire = false;
          _bus_spi.config(bus_cfg);
          _bus_spi.init();
          _pin_reset(GPIO_NUM_33, use_reset); // LCD RST;
          id = _read_panel_id(&_bus_spi, GPIO_NUM_14);
          if ((id & 0xFF) == 0 && _read_panel_id(&_bus_spi, GPIO_NUM_14, 0x09) != 0)
          {   // check panel (ILI9341) panelIDが0なのでステータスリード0x09を併用する
            ESP_LOGW(LIBRARY_NAME, "[Autodetect] LoLinD32Pro ILI9341");
            board = board_t::board_LoLinD32;

            #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK )
            lgfx::pinMode(GPIO_NUM_4, lgfx::pin_mode_t::input); // M5Stack TF card CS
            #endif

            bus_cfg.freq_write = 40000000;
            bus_cfg.freq_read  = 16000000;
            _bus_spi.config(bus_cfg);

            auto p = new lgfx::Panel_ILI9341();
            {
              auto cfg = p->config();
              cfg.pin_cs  = GPIO_NUM_14;
              cfg.pin_rst = GPIO_NUM_33;
              p->config(cfg);
            }
            p->bus(&_bus_spi);
            _panel_last = p;
            _set_pwm_backlight(GPIO_NUM_32, 7, 44100);

            {
              auto t = new lgfx::Touch_XPT2046();
              _touch_last = t;
              auto cfg = t->config();
              cfg.bus_shared = true;
              cfg.freq = 2700000;
              cfg.spi_host = VSPI_HOST;
              cfg.pin_cs   = GPIO_NUM_12;
              cfg.pin_mosi = GPIO_NUM_23;
              cfg.pin_miso = GPIO_NUM_19;
              cfg.pin_sclk = GPIO_NUM_18;
              t->config(cfg);
              p->touch(t);
            }

            goto init_clear;
          }
          _bus_spi.release();
          lgfx::pinMode(GPIO_NUM_12, lgfx::pin_mode_t::input); // LoLinD32 TouchScreen CS

          lgfx::pinMode(GPIO_NUM_4 , lgfx::pin_mode_t::input); // M5Stack and LoLinD32 TF card CS
          lgfx::pinMode(GPIO_NUM_14, lgfx::pin_mode_t::input); // LCD CS
          lgfx::pinMode(GPIO_NUM_33, lgfx::pin_mode_t::input); // LCD RST
        }
#endif

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_ESP_WROVER_KIT )

        if (board == 0 || board == board_t::board_ESP_WROVER_KIT)
        {
          bus_cfg.pin_mosi = GPIO_NUM_23;
          bus_cfg.pin_miso = GPIO_NUM_25;
          bus_cfg.pin_sclk = GPIO_NUM_19;
          bus_cfg.pin_dc   = GPIO_NUM_21;
          bus_cfg.spi_3wire = false;
          _bus_spi.config(bus_cfg);
          _bus_spi.init();
          _pin_reset(GPIO_NUM_18, use_reset); // LCD RST
          id = _read_panel_id(&_bus_spi, GPIO_NUM_22);
          if ((id & 0xFF) == 0x85)
          {   // check panel (ST7789)
            board = board_t::board_ESP_WROVER_KIT;
            ESP_LOGW(LIBRARY_NAME, "[Autodetect] ESP-WROVER-KIT ST7789");
            bus_cfg.freq_write = 80000000;
            bus_cfg.freq_read  = 16000000;
            _bus_spi.config(bus_cfg);
            auto p = new Panel_ST7789();
            p->bus(&_bus_spi);
            {
              auto cfg = p->config();
              cfg.pin_cs  = GPIO_NUM_22;
              cfg.offset_rotation = 2;
              p->config(cfg);
            }
            _panel_last = p;
            _set_pwm_backlight(GPIO_NUM_5, 7, 12000, true);
            goto init_clear;
          }
          if ((id & 0xFF) == 0 && _read_panel_id(&_bus_spi, GPIO_NUM_22, 0x09) != 0)
          {   // check panel (ILI9341) panelIDが0なのでステータスリード0x09を併用する
            board = board_t::board_ESP_WROVER_KIT;
            ESP_LOGW(LIBRARY_NAME, "[Autodetect] ESP-WROVER-KIT ILI9341");
            bus_cfg.freq_write = 40000000;
            bus_cfg.freq_read  = 20000000;
            _bus_spi.config(bus_cfg);
            auto p = new Panel_ILI9341();
            p->bus(&_bus_spi);
            {
              auto cfg = p->config();
              cfg.pin_cs  = GPIO_NUM_22;
              p->config(cfg);
            }
            _panel_last = p;
            _set_pwm_backlight(GPIO_NUM_5, 7, 12000, true);
            goto init_clear;
          }
          lgfx::pinMode(GPIO_NUM_18, lgfx::pin_mode_t::input); // LCD RST
          lgfx::pinMode(GPIO_NUM_22, lgfx::pin_mode_t::input); // LCD CS
          _bus_spi.release();
        }
#endif

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_WT32_SC01 )

        if (board == 0 || board == board_t::board_WT32_SC01)
        {
          bus_cfg.pin_mosi = GPIO_NUM_13;
          bus_cfg.pin_miso = -1;
          bus_cfg.pin_sclk = GPIO_NUM_14;
          bus_cfg.pin_dc   = GPIO_NUM_21;
          bus_cfg.spi_3wire = true;
          _bus_spi.config(bus_cfg);
          _bus_spi.init();
          _pin_reset(GPIO_NUM_22, use_reset); // LCD RST

          id = _read_panel_id(&_bus_spi, GPIO_NUM_15);
          if (id == 0)
          { // 読出しが出来ない製品のため 0 判定
            lgfx::i2c::init(I2C_NUM_1, GPIO_NUM_18, GPIO_NUM_19);
            // I2C通信でタッチパネルコントローラが存在するかチェックする
            if (0x11 == lgfx::i2c::readRegister8(I2C_NUM_1, 0x38, 0xA8, 400000))
            { /// FocalTech's Panel ID reg=0xA8  value=0x11
              board = board_t::board_WT32_SC01;
              ESP_LOGW(LIBRARY_NAME, "[Autodetect] WT32-SC01");
              bus_cfg.freq_write = 40000000;
              _bus_spi.config(bus_cfg);
              auto p = new Panel_ST7796();
              p->bus(&_bus_spi);
              {
                auto cfg = p->config();
                cfg.pin_cs  = GPIO_NUM_15;
                cfg.pin_rst = GPIO_NUM_22;
                cfg.readable = false;
                p->config(cfg);
              }
              _panel_last = p;
              _set_pwm_backlight(GPIO_NUM_23, 7);

              {
                auto t = new lgfx::Touch_FT5x06();
                _touch_last = t;
                auto cfg = t->config();
                cfg.pin_int  = GPIO_NUM_39;   // INT pin number
                cfg.pin_sda  = GPIO_NUM_18;   // I2C SDA pin number
                cfg.pin_scl  = GPIO_NUM_19;   // I2C SCL pin number
                cfg.i2c_addr = 0x38; // I2C device addr
                cfg.i2c_port = I2C_NUM_1;// I2C port number
                cfg.freq = 400000;   // I2C freq
                cfg.x_min = 0;
                cfg.x_max = 319;
                cfg.y_min = 0;
                cfg.y_max = 479;
                cfg.bus_shared = false;
                t->config(cfg);
                p->touch(t);
              }
              goto init_clear;
            }
            lgfx::i2c::release(I2C_NUM_1);
          }
          lgfx::pinMode(GPIO_NUM_15, lgfx::pin_mode_t::input); // LCD CS
          lgfx::pinMode(GPIO_NUM_22, lgfx::pin_mode_t::input); // LCD RST
          _bus_spi.release();
        }
#endif

  // DSTIKE D-Duino32XS については読出しが出来ないため無条件設定となる。
  // そのためLGFX_AUTO_DETECTでは機能しないようにしておく。
#if defined ( LGFX_DDUINO32_XS )

        if (board == 0 || board == board_t::board_DDUINO32_XS)
        {
          bus_cfg.pin_mosi = GPIO_NUM_26;
          bus_cfg.pin_miso = -1;
          bus_cfg.pin_sclk = GPIO_NUM_27;
          bus_cfg.pin_dc   = GPIO_NUM_23;
          bus_cfg.spi_3wire = true;
          _bus_spi.config(bus_cfg);
          _bus_spi.init();
          _pin_reset(GPIO_NUM_32, use_reset); // LCD RST
          // id = _read_panel_id(&_bus_spi, -1);  // 読出しモードから抜ける事ができないのでコメントアウト;
          // if ((id & 0xFF) == 0x85)
          {
            board = board_t::board_DDUINO32_XS;
            ESP_LOGW(LIBRARY_NAME, "[Autodetect] D-Duino32 XS");
            bus_cfg.freq_write = 80000000;
            _bus_spi.config(bus_cfg);
            auto p = new Panel_ST7789();
            p->bus(&_bus_spi);
            {
              auto cfg = p->config();
              cfg.invert = true;
              cfg.readable  = false;
              cfg.pin_cs  = -1;
              cfg.pin_rst = GPIO_NUM_32;
              cfg.panel_height = 240;
              p->config(cfg);
            }
            _panel_last = p;
            _set_pwm_backlight(GPIO_NUM_22, 7, 12000);
            goto init_clear;
          }
          lgfx::pinMode(GPIO_NUM_32, lgfx::pin_mode_t::input); // LCD RST
          _bus_spi.release();
        }
#endif

      }
    }

      board = board_t::board_unknown;

      goto init_clear;
  init_clear:

      panel(_panel_last);

      return board;
    }
  };

//----------------------------------------------------------------------------
 }
}

using LGFX = lgfx::LGFX;