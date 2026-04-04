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
#include <soc/gpio_periph.h>
#include <soc/gpio_reg.h>
#include <soc/io_mux_reg.h>
#if __has_include(<hal/gpio_types.h>)
 #include <hal/gpio_types.h>
#endif
#if __has_include(<esp_idf_version.h>)
 #include <esp_idf_version.h>
#endif

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  static constexpr char LIBRARY_NAME[] = "LovyanGFX";

  static void i2c_write_register8_array(int_fast16_t i2c_port, uint_fast8_t i2c_addr, const uint8_t* reg_data_mask, uint32_t freq)
  {
    while (reg_data_mask[0] != 0xFF || reg_data_mask[1] != 0xFF || reg_data_mask[2] != 0xFF)
    {
      lgfx::i2c::writeRegister8(i2c_port, i2c_addr, reg_data_mask[0], reg_data_mask[1], reg_data_mask[2], freq);
      reg_data_mask += 3;
    }
  }

#if defined (CONFIG_IDF_TARGET_ESP32S3)

#if defined ( ARDUINO_ESP32_S3_BOX )
  #define LGFX_ESP32_S3_BOX
  #define LGFX_ESP32_S3_BOX_LITE
  #define LGFX_ESP32_S3_BOX_V3
  #define LGFX_DEFAULT_BOARD board_t::board_ESP32_S3_BOX_V3

#elif defined ( ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT )
  #define LGFX_FEATHER_ESP32_S3_TFT
  #define LGFX_DEFAULT_BOARD board_t::board_Feather_ESP32_S3_TFT

#elif /*defined ( ARDUINO_LOLIN_S3_PRO ) ||*/ defined ( LGFX_LOLIN_S3_PRO )
  #define LGFX_DEFAULT_BOARD board_t::board_LoLinS3Pro
#endif

  namespace m5stack
  {
    static constexpr int32_t i2c_freq = 400000;
    static constexpr int_fast16_t aw9523_i2c_addr = 0x58;  // AW9523B
    static constexpr int_fast16_t axp_i2c_addr = 0x34;     // AXP2101
    static constexpr int_fast16_t i2c_port = I2C_NUM_1;
    static constexpr int_fast16_t i2c_sda = GPIO_NUM_12;
    static constexpr int_fast16_t i2c_scl = GPIO_NUM_11;
  }

  struct Panel_M5StackCoreS3 : public lgfx::Panel_ILI9342
  {
    static constexpr gpio_num_t pin_miso_dc = GPIO_NUM_35;
    Panel_M5StackCoreS3(void)
    {
      // _cfg.pin_cs = GPIO_NUM_3;
      _cfg.invert = true;
      _cfg.offset_rotation = 3;

      _rotation = 1; // default rotation
    }

    void rst_control(bool level) override
    {
      using namespace m5stack;
      uint8_t bits = level ? (1<<5) : 0;
      uint8_t mask = level ? ~0 : ~(1<<5);
      // LCD_RST
      lgfx::i2c::writeRegister8(i2c_port, aw9523_i2c_addr, 0x03, bits, mask, i2c_freq);
    }

    void cs_control(bool flg) override
    {
      lgfx::Panel_ILI9342::cs_control(flg);

      // 0x43==FSPI MISO / 0x100==GPIO OUT
      *(volatile uint32_t*)GPIO_FUNC35_OUT_SEL_CFG_REG = flg ? 0x43 : 0x100;
    }
  };

  struct Touch_M5StackCoreS3 : public lgfx::Touch_FT5x06
  {
    Touch_M5StackCoreS3(void)
    {
      using namespace m5stack;
      _cfg.pin_int  = GPIO_NUM_21;
      _cfg.pin_sda  = i2c_sda;
      _cfg.pin_scl  = i2c_scl;
      _cfg.i2c_addr = 0x38;
      _cfg.i2c_port = i2c_port;
      _cfg.freq = i2c_freq;
      _cfg.x_min = 0;
      _cfg.x_max = 319;
      _cfg.y_min = 0;
      _cfg.y_max = 239;
      _cfg.bus_shared = false;
    }

    uint_fast8_t getTouchRaw(touch_point_t* tp, uint_fast8_t count) override
    {
      using namespace m5stack;
      uint_fast8_t res = 0;
      if (!gpio_in(_cfg.pin_int))
      {
        res = lgfx::Touch_FT5x06::getTouchRaw(tp, count);
        if (res == 0)
        { /// clear INT.
          // レジスタ 0x00を読み出すとPort0のINTがクリアされ、レジスタ 0x01を読み出すとPort1のINTがクリアされる。
          lgfx::i2c::readRegister8(i2c_port, aw9523_i2c_addr, 0x00, i2c_freq);
          lgfx::i2c::readRegister8(i2c_port, aw9523_i2c_addr, 0x01, i2c_freq);
        }
      }
      return res;
    }
  };

  struct Light_M5StackCoreS3 : public lgfx::ILight
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
        brightness = (brightness / 25) + 18;
    // AXP2101 reg 0x90 = LDOS ON/OFF control
        lgfx::i2c::bitOn(i2c_port, axp_i2c_addr, 0x90, 0x80, i2c_freq); // DLDO1 enable
      }
      else
      {
        lgfx::i2c::bitOff(i2c_port, axp_i2c_addr, 0x90, 0x80, i2c_freq); // DLDO1 disable
      }
    // AXP2101 reg 0x99 = DLDO1 voltage setting
      lgfx::i2c::writeRegister8(i2c_port, axp_i2c_addr, 0x99, brightness, 0, i2c_freq);
    }
  };

#elif defined (CONFIG_IDF_TARGET_ESP32S2)

#if defined ( ARDUINO_ADAFRUIT_FEATHER_ESP32S2_TFT )
  #define LGFX_FEATHER_ESP32_S2_TFT
  #define LGFX_DEFAULT_BOARD board_t::board_Feather_ESP32_S2_TFT
#endif

#if defined ( ARDUINO_FUNHOUSE_ESP32S2 )
  #define LGFX_FUNHOUSE
  #define LGFX_DEFAULT_BOARD board_t::board_FunHouse
#endif

#elif defined (CONFIG_IDF_TARGET_ESP32C3)
#elif defined (CONFIG_IDF_TARGET_ESP32) || !defined (CONFIG_IDF_TARGET)

#if defined ARDUINO_M5STACK_CORE_ESP32 || defined ARDUINO_M5Stack_Core_ESP32 || defined ARDUINO_M5STACK_CORE || defined ARDUINO_M5STACK_FIRE
  #define LGFX_M5STACK
  #define LGFX_DEFAULT_BOARD board_t::board_M5Stack
#elif defined( ARDUINO_M5STACK_CORE2 ) || defined( ARDUINO_M5STACK_Core2 ) // M5Stack Core2
  #define LGFX_M5STACK_CORE2
  #define LGFX_DEFAULT_BOARD board_t::board_M5StackCore2
#elif defined ( ARDUINO_M5STACK_TOUGH )
  #define LGFX_M5TOUGH
  #define LGFX_DEFAULT_BOARD board_t::board_M5Tough
#elif defined( ARDUINO_M5STICK_C ) || defined( ARDUINO_M5Stick_C ) || defined ARDUINO_M5STACK_STICKC // M5Stick C
  #define LGFX_M5STICK_C
  #define LGFX_DEFAULT_BOARD board_t::board_M5StickC
#elif defined( ARDUINO_M5STICK_C_PLUS ) || defined( ARDUINO_M5Stick_C_Plus ) || defined ARDUINO_M5STACK_STICKC_PLUS // M5Stick C+
  #define LGFX_M5STICK_C
  #define LGFX_DEFAULT_BOARD board_t::board_M5StickCPlus
#elif defined( ARDUINO_M5STACK_COREINK ) || defined( ARDUINO_M5Stack_CoreInk ) // M5Stack CoreInk
  #define LGFX_M5STACK_COREINK
  #define LGFX_DEFAULT_BOARD board_t::board_M5StackCoreInk
#elif defined( ARDUINO_M5STACK_PAPER ) || defined( ARDUINO_M5STACK_Paper ) // M5Paper
  #define LGFX_M5PAPER
  #define LGFX_DEFAULT_BOARD board_t::board_M5Paper
#elif defined( ARDUINO_ODROID_ESP32 ) // ODROID-GO
  #define LGFX_ODROID_GO
  #define LGFX_DEFAULT_BOARD board_t::board_ODROID_GO
#elif defined( ARDUINO_TTGO_T1 ) // TTGO TS
  #define LGFX_TTGO_TS
  #define LGFX_DEFAULT_BOARD board_t::board_TTGO_TS
#elif defined( ARDUINO_TWATCH ) || defined( ARDUINO_TWatch ) || defined( ARDUINO_T ) // TTGO T-Watch
  #define LGFX_TTGO_TWATCH
  #define LGFX_DEFAULT_BOARD board_t::board_TTGO_TWatch
#elif defined( ARDUINO_D ) || defined( ARDUINO_DDUINO32_XS ) // DSTIKE D-duino-32 XS
  #define LGFX_DDUINO32_XS
  #define LGFX_DEFAULT_BOARD board_t::board_DDUINO32_XS
#elif defined( ARDUINO_LOLIN_D32_PRO )
  #define LGFX_LOLIN_D32_PRO
  #define LGFX_DEFAULT_BOARD board_t::board_LoLinD32
#elif defined( ARDUINO_ESP32_WROVER_KIT )
  #define LGFX_ESP_WROVER_KIT
  #define LGFX_DEFAULT_BOARD board_t::board_ESP_WROVER_KIT
#endif

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
    static constexpr gpio_num_t axp_i2c_sda = GPIO_NUM_21;
    static constexpr gpio_num_t axp_i2c_scl = GPIO_NUM_22;
  }

  struct Panel_M5Stack : public lgfx::Panel_ILI9342
  {
    Panel_M5Stack(void)
    {
      // _cfg.pin_cs  = GPIO_NUM_14;
      // _cfg.pin_rst = GPIO_NUM_33;
      _cfg.offset_rotation = 3;

      _rotation = 1;
    }

    bool init(bool use_reset) override
    {
      _cfg.invert = lgfx::gpio::command(
        (const uint8_t[]) {
        lgfx::gpio::command_mode_output        , GPIO_NUM_33,
        lgfx::gpio::command_write_low          , GPIO_NUM_33,
        lgfx::gpio::command_mode_input_pulldown, GPIO_NUM_33,
        lgfx::gpio::command_write_high         , GPIO_NUM_33,
        lgfx::gpio::command_read               , GPIO_NUM_33,
        lgfx::gpio::command_mode_output        , GPIO_NUM_33,
        lgfx::gpio::command_end
        });
      return lgfx::Panel_ILI9342::init(use_reset);
    }
  };

  struct Panel_M5StackCore2 : public lgfx::Panel_ILI9342
  {
    Panel_M5StackCore2(void)
    {
      // _cfg.pin_cs = GPIO_NUM_5;
      _cfg.invert = true;
      _cfg.offset_rotation = 3;

      _rotation = 1; // default rotation
    }

    void rst_control(bool level) override
    {
      using namespace m5stack;
      uint8_t bits = level ? 2 : 0;
      uint8_t mask = level ? ~0 : ~2;
      // AXP192 reg 0x96 = GPIO3&4 control
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x96, bits, mask, axp_i2c_freq);
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

  struct Light_M5StackCore2_AXP2101 : public lgfx::ILight
  {
    bool init(uint8_t brightness) override
    {
      setBrightness(brightness);
      return true;
    }

    void setBrightness(uint8_t brightness) override
    {
      using namespace m5stack;

      // BLDO1
      if (brightness)
      {
        brightness = ((brightness + 641) >> 5);
        lgfx::i2c::bitOn(axp_i2c_port, axp_i2c_addr, 0x90, 0x10, axp_i2c_freq); // BLDO1 enable
      }
      else
      {
        lgfx::i2c::bitOff(axp_i2c_port, axp_i2c_addr, 0x90, 0x10, axp_i2c_freq); // BLDO1 disable
      }
    // AXP192 reg 0x96 = BLO1 voltage setting (0.5v ~ 3.5v  100mv/step)
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x96, brightness, 0, axp_i2c_freq);
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
      _cfg.i2c_addr = 0x2E;
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
      // _cfg.pin_cs  = GPIO_NUM_5;
      // _cfg.pin_rst = GPIO_NUM_18;
      _cfg.panel_width  = 80;
      _cfg.panel_height = 160;
      _cfg.offset_x     = 26;
      _cfg.offset_y     = 1;
      _cfg.offset_rotation = 2;
      // _cfg.bus_shared = false;
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
      // _cfg.pin_cs  = GPIO_NUM_5;
      // _cfg.pin_rst = GPIO_NUM_18;
      _cfg.panel_width  = 135;
      _cfg.panel_height = 240;
      _cfg.offset_x     = 52;
      _cfg.offset_y     = 40;
      // _cfg.bus_shared = false;
    }
  };

#endif

  class LGFX : public LGFX_Device
  {
    struct _detector_result_t
    {
      // 検出されたパネル
      Panel_Device* panel;

      // 検出されたバス
      IBus* bus;

      // 検出されたボード
      board_t board;
    };

    struct _detector_base_t
    {
      virtual bool detect(_detector_result_t* result, bool use_reset) const = 0;
      virtual void setup(_detector_result_t* result) const = 0;

    protected:

      class _pin_backup_t
      {
      public:
        _pin_backup_t(int8_t pin_num)
        : _pin_num { static_cast<gpio_num_t>(pin_num) }
        {
          if (pin_num >= 0)
          {
            _io_mux_gpio_reg   = *reinterpret_cast<uint32_t*>(GPIO_PIN_MUX_REG[pin_num]);
            _gpio_pin_reg      = *reinterpret_cast<uint32_t*>(GPIO_PIN0_REG              + (pin_num * 4));
            _gpio_func_out_reg = *reinterpret_cast<uint32_t*>(GPIO_FUNC0_OUT_SEL_CFG_REG + (pin_num * 4));
#if defined ( GPIO_ENABLE1_REG )
            _gpio_enable = *reinterpret_cast<uint32_t*>(((_pin_num & 32) ? GPIO_ENABLE1_REG : GPIO_ENABLE_REG)) & (1 << (_pin_num & 31));
#else
            _gpio_enable = *reinterpret_cast<uint32_t*>(GPIO_ENABLE_REG) & (1 << (_pin_num & 31));
#endif
          }
        }

        void restore(void) const
        {
          if ((int)_pin_num >= 0) {
  // ESP_LOGD("DEBUG","restore pin:%d ", _pin_num);
  // ESP_LOGD("DEBUG","restore IO_MUX_GPIO0_REG          :%08x -> %08x ", *reinterpret_cast<uint32_t*>(GPIO_PIN_MUX_REG[_pin_num]                 ), _io_mux_gpio_reg   );
  // ESP_LOGD("DEBUG","restore GPIO_PIN0_REG             :%08x -> %08x ", *reinterpret_cast<uint32_t*>(GPIO_PIN0_REG              + (_pin_num * 4)), _gpio_pin_reg      );
  // ESP_LOGD("DEBUG","restore GPIO_FUNC0_OUT_SEL_CFG_REG:%08x -> %08x ", *reinterpret_cast<uint32_t*>(GPIO_FUNC0_OUT_SEL_CFG_REG + (_pin_num * 4)), _gpio_func_out_reg );
            *reinterpret_cast<uint32_t*>(GPIO_PIN_MUX_REG[_pin_num]) = _io_mux_gpio_reg;
            *reinterpret_cast<uint32_t*>(GPIO_PIN0_REG              + (_pin_num * 4)) = _gpio_pin_reg;
            *reinterpret_cast<uint32_t*>(GPIO_FUNC0_OUT_SEL_CFG_REG + (_pin_num * 4)) = _gpio_func_out_reg;
#if defined ( GPIO_ENABLE1_REG )
            auto gpio_enable_reg = reinterpret_cast<uint32_t*>(((_pin_num & 32) ? GPIO_ENABLE1_REG : GPIO_ENABLE_REG));
#else
            auto gpio_enable_reg = reinterpret_cast<uint32_t*>(GPIO_ENABLE_REG);
#endif

            uint32_t pin_mask = 1 << (_pin_num & 31);
  // ESP_LOGD("DEBUG","restore GPIO_ENABLE_REG:%08x", *gpio_enable_reg);
            if (_gpio_enable)
            {
              *gpio_enable_reg |= pin_mask;
            }
            else
            {
              *gpio_enable_reg &= ~pin_mask;
            }
  // ESP_LOGD("DEBUG","restore GPIO_ENABLE_REG:%08x", *gpio_enable_reg);
          }
        }

      private:
        uint32_t _io_mux_gpio_reg;
        uint32_t _gpio_pin_reg;
        uint32_t _gpio_func_out_reg;
        gpio_num_t _pin_num;
        bool _gpio_enable;
      };

      static void _pin_level(int_fast16_t pin, bool level)
      {
        if (pin < 0) return;

        lgfx::pinMode(pin, lgfx::pin_mode_t::output);
        if (level) lgfx::gpio_hi(pin);
        else       lgfx::gpio_lo(pin);
      }

      static void _pin_reset(int_fast16_t pin, bool use_reset)
      {
        if (pin < 0) return;

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

      static uint32_t _read_panel_id(lgfx::IBus* bus, int32_t pin_cs, uint_fast16_t cmd = 0x04, uint8_t dummy_read_bit = 1) // 0x04 = RDDID command
      {
        size_t dlen = 8;
        uint_fast16_t read_cmd = cmd;
        if (bus->busType() == bus_type_t::bus_parallel16)
        {
          dlen = 16;
          read_cmd = getSwap16(read_cmd);
        }

        bus->beginTransaction();
        _pin_level(pin_cs, true);
        bus->writeCommand(0, dlen);
        bus->wait();

        _pin_level(pin_cs, false);
        bus->writeCommand(read_cmd, dlen);
        bus->beginRead(dummy_read_bit);
        uint32_t res = 0;
        for (size_t i = 0; i < 4; ++i)
        {
          res += ((bus->readData(dlen) >> (dlen - 8)) & 0xFF) << (i * 8);
        }
        bus->endTransaction();
        _pin_level(pin_cs, true);

        ESP_LOGV(LIBRARY_NAME, "[Autodetect] read cmd:%02x = %08x", (unsigned int)cmd, (unsigned int)res);
        return res;
      }

      static ILight* _create_pwm_backlight(int16_t pin, uint8_t ch, uint32_t freq = 12000, bool invert = false, uint8_t offset = 0)
      {
        auto bl = new lgfx::Light_PWM();
        auto cfg = bl->config();
        cfg.pin_bl = pin;
        cfg.freq   = freq;
        cfg.pwm_channel = ch;
        cfg.offset = offset;
        cfg.invert = invert;
        bl->config(cfg);
        return bl;
      }
    };

    struct _detector_t : public _detector_base_t
    {
      board_t board = board_t::board_unknown;
      uint16_t id_cmd;
      uint32_t id_mask;
      uint32_t id_value;
      uint32_t freq_write;
      uint32_t freq_read;

      constexpr _detector_t (board_t board_, uint16_t id_cmd_, uint32_t id_mask_, uint32_t id_value_, uint32_t freq_write_, uint32_t freq_read_ )
      : board      { board_      }
      , id_cmd     { id_cmd_     }
      , id_mask    { id_mask_    }
      , id_value   { id_value_   }
      , freq_write { freq_write_ }
      , freq_read  { freq_read_  }
      {}

      virtual bool judgement(IBus* bus, int pin_cs) const
      {
        bool hit = true;
        if (id_mask) {
          hit = (id_value == (_read_panel_id(bus, pin_cs, id_cmd) & id_mask));
          if (hit && id_value == 0 && id_cmd == 0x04)
          {
            hit = (0 != (_read_panel_id(bus, pin_cs, 0x09) & 0xFFFFFF));
          }
        }
        return hit;
      }
    };

    struct _detector_spi_t : public _detector_t
    {
      union
      {
        struct {
          int8_t pin_mosi;
          int8_t pin_miso;
          int8_t pin_sclk;
          int8_t pin_dc;
          int8_t pin_cs;
          int8_t pin_rst;
          int8_t pin_tfcard_cs;
        };
        int8_t pins[7];
      };
      int8_t spi_mode;
      bool spi_3wire;
      spi_host_device_t spi_host;

      constexpr _detector_spi_t
      ( board_t board_
      , uint16_t id_cmd_
      , uint32_t id_mask_
      , uint32_t id_value_
      , uint32_t freq_write_
      , uint32_t freq_read_
      , gpio_num_t pin_mosi_
      , gpio_num_t pin_miso_
      , gpio_num_t pin_sclk_
      , gpio_num_t pin_dc_
      , gpio_num_t pin_cs_
      , gpio_num_t pin_rst_
      , gpio_num_t pin_tfcard_cs_
      , int8_t spi_mode_
      , bool spi_3wire_
      , spi_host_device_t spi_host_
      )
      : _detector_t { board_, id_cmd_, id_mask_, id_value_, freq_write_, freq_read_ }
      ,  pins       { pin_mosi_, pin_miso_, pin_sclk_, pin_dc_, pin_cs_, pin_rst_, pin_tfcard_cs_ }
      ,  spi_mode   { spi_mode_   }
      ,  spi_3wire  { spi_3wire_  }
      ,  spi_host   { spi_host_   }
      {}

      bool detect(_detector_result_t* result, bool use_reset) const override
      {
        if (result->board && result->board != board)
        {
          return false;
        }
        _pin_backup_t backup[7] =
        { pins[0], pins[1], pins[2], pins[3], pins[4], pins[5], pins[6] };

        _pin_level(pin_cs, true);

        Bus_SPI bus_spi;
        auto bus_cfg = bus_spi.config();
        bus_cfg.freq_write  = 8000000;
        bus_cfg.freq_read   = 8000000;
        bus_cfg.use_lock    = true;

// パネル検出時点ではDMAを使用しない設定にしておく。
// これはバスをリリースしてもDPORT_SPI_DMA_CHAN_SEL_REGの値がクリアされず、
// 次回DMA設定時に動作に支障が出ることがあるため。
        bus_cfg.dma_channel = 0;
        bus_cfg.spi_host    = spi_host;
        bus_cfg.pin_mosi    = pin_mosi;
        bus_cfg.pin_miso    = pin_miso;
        bus_cfg.pin_sclk    = pin_sclk;
        bus_cfg.pin_dc      = pin_dc;
        bus_cfg.spi_mode    = spi_mode;
        bus_cfg.spi_3wire   = spi_3wire;
        bus_spi.config(bus_cfg);
        bus_spi.init();
        _pin_reset(pin_rst, use_reset); // LCD RST

        if ((int)pin_tfcard_cs >= 0) {
          _set_sd_spimode(spi_host, pin_tfcard_cs);
        }

        bool hit = judgement(&bus_spi, pin_cs);

        bus_spi.release();

        if (hit)
        {
          bus_cfg.dma_channel = 1;
#if defined (ESP_IDF_VERSION)
 #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 3, 0)
          bus_cfg.dma_channel = SPI_DMA_CH_AUTO;
 #endif
#endif
          bus_cfg.freq_write = freq_write;
          bus_cfg.freq_read  = freq_read;
          auto bus = new Bus_SPI();
          bus->config(bus_cfg);
          result->bus = bus;
          result->board = board;
          setup(result);
          auto p = result->panel;
          p->bus(bus);
          {
            auto cfg = p->config();
            if (pin_cs  >= 0) { cfg.pin_cs  = pin_cs;  }
            if (pin_rst >= 0) { cfg.pin_rst = pin_rst; }
            p->config(cfg);
          }
          return true;
        }

        for (auto &b : backup)
        {
          b.restore();
        }

        return false;
      }
    };

#if defined (CONFIG_IDF_TARGET_ESP32S2) || defined (CONFIG_IDF_TARGET_ESP32S3)

    struct _detector_parallel16_t : public _detector_t
    {
      union
      {
        struct {
          int8_t pin_data[16];
          int8_t pin_wr;
          int8_t pin_rd;
          int8_t pin_rs;
          int8_t pin_cs;
          int8_t pin_rst;
        };
        int8_t pins[21];
      };
      int8_t port;


      constexpr _detector_parallel16_t
      ( board_t board_
      , uint16_t id_cmd_
      , uint32_t id_mask_
      , uint32_t id_value_
      , uint32_t freq_write_
      , uint32_t freq_read_
      , gpio_num_t pin_d0_ , gpio_num_t pin_d1_ , gpio_num_t pin_d2_ , gpio_num_t pin_d3_ , gpio_num_t pin_d4_ , gpio_num_t pin_d5_ , gpio_num_t pin_d6_ , gpio_num_t pin_d7_
      , gpio_num_t pin_d8_ , gpio_num_t pin_d9_ , gpio_num_t pin_d10_, gpio_num_t pin_d11_, gpio_num_t pin_d12_, gpio_num_t pin_d13_, gpio_num_t pin_d14_, gpio_num_t pin_d15_
      , gpio_num_t pin_wr_
      , gpio_num_t pin_rd_
      , gpio_num_t pin_rs_
      , gpio_num_t pin_cs_
      , gpio_num_t pin_rst_
      , int8_t port_
      )
      : _detector_t { board_, id_cmd_, id_mask_, id_value_, freq_write_, freq_read_ }
      , pins       { pin_d0_, pin_d1_, pin_d2_, pin_d3_, pin_d4_, pin_d5_, pin_d6_, pin_d7_
                   , pin_d8_, pin_d9_, pin_d10_, pin_d11_, pin_d12_, pin_d13_, pin_d14_, pin_d15_
                   , pin_wr_, pin_rd_, pin_rs_, pin_cs_, pin_rst_ }
      , port       { port_     }
      {}

      bool detect(_detector_result_t* result, bool use_reset) const override
      {
        if (result->board && result->board != board)
        {
          return false;
        }

        _pin_backup_t backup[] =
        { pins[ 0], pins[ 1], pins[ 2], pins[ 3], pins[ 4], pins[ 5], pins[ 6], pins[ 7],
          pins[ 8], pins[ 9], pins[10], pins[11], pins[12], pins[13], pins[14], pins[15],
          pins[16], pins[17], pins[18], pins[19], pins[20]
        };

        _pin_level(pin_cs, true);

        Bus_Parallel16 bus_tmp;
        auto cfg = bus_tmp.config();
        for (size_t i = 0; i < 16; ++i)
        {
          cfg.pin_data[i] = pin_data[i];
        }
        cfg.pin_wr = pin_wr;
        cfg.pin_rd = pin_rd;
        cfg.pin_rs = pin_rs;
        cfg.freq_write = 5000000;
        // cfg.freq_read = 5000000;
        cfg.port   = port;

        bus_tmp.config(cfg);
        bus_tmp.init();
        _pin_reset(pin_rst, use_reset); // LCD RST

        bool hit = judgement(&bus_tmp, pin_cs);

        bus_tmp.release();

        if (hit)
        {
          auto bus = new Bus_Parallel16();

          cfg.freq_write = freq_write;
          // cfg.freq_read = freq_read;
          cfg.port   = port;

          bus->config(cfg);
          result->bus = bus;
          result->board = board;
          setup(result);
          auto p = result->panel;
          p->bus(bus);
          {
            auto cfg_panel = p->config();
            if (pin_cs  >= 0) { cfg_panel.pin_cs  = pin_cs;  }
            if (pin_rst >= 0) { cfg_panel.pin_rst = pin_rst; }
            p->config(cfg_panel);
          }
          return true;
        }

        for (auto &b : backup)
        {
          b.restore();
        }

        return false;
      }
    };

#endif // (CONFIG_IDF_TARGET_ESP32S2) || defined (CONFIG_IDF_TARGET_ESP32S3)

    lgfx::Panel_Device* _panel_last = nullptr;
    lgfx::ILight* _light_last = nullptr;
    lgfx::ITouch* _touch_last = nullptr;
    lgfx::IBus* _bus_last = nullptr;

    bool init_impl(bool use_reset, bool use_clear)
    {
      static constexpr char NVS_KEY[] = "AUTODETECT";
      uint32_t nvs_board = 0;
      uint32_t nvs_handle = 0;
      if (0 == nvs_open(LIBRARY_NAME, NVS_READONLY, &nvs_handle))
      {
        nvs_get_u32(nvs_handle, NVS_KEY, static_cast<uint32_t*>(&nvs_board));
        nvs_close(nvs_handle);
        ESP_LOGI(LIBRARY_NAME, "[Autodetect] load from NVS : board:%d", (int)nvs_board);
      }

#if defined ( LGFX_DEFAULT_BOARD )
      if (0 == nvs_board)
      {
        nvs_board = LGFX_DEFAULT_BOARD;
      }
#endif

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
      bool res = LGFX_Device::init_impl(false, use_clear);

      if (nvs_board != board) {
        if (0 == nvs_open(LIBRARY_NAME, NVS_READWRITE, &nvs_handle)) {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] save to NVS : board:%d", board);
          nvs_set_u32(nvs_handle, NVS_KEY, board);
          nvs_close(nvs_handle);
        }
      }
      return res;
    }

  public:

    board_t autodetect(bool use_reset = true, board_t board = board_t::board_unknown)
    {
#if defined (CONFIG_IDF_TARGET_ESP32S3)

      struct _detector_M5AtomS3_t : public _detector_spi_t
      {
        constexpr _detector_M5AtomS3_t(void) :
        _detector_spi_t
        { board_t::board_M5AtomS3
        , 0x04, 0xFFFFFF, 0x079100  // GC9107
        , 40000000, 16000000
        , GPIO_NUM_21     // MOSI
        , GPIO_NUM_13     // MISO
        , GPIO_NUM_17     // SCLK
        , GPIO_NUM_33     // DC
        , GPIO_NUM_15     // CS
        , GPIO_NUM_34     // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , SPI3_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* param) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] M5AtomS3");
          auto p = new Panel_GC9107();
          param->panel = p;
          p->light(_create_pwm_backlight(GPIO_NUM_16, 7, 256, false, 48));

          {
            auto cfg = p->config();
            cfg.panel_width = 128;
            cfg.panel_height = 128;
            cfg.offset_y = 32;
            p->config(cfg);
          }
        }
      };

      struct _detector_M5StackCoreS3_t : public _detector_spi_t
      {
        constexpr _detector_M5StackCoreS3_t(void) :
        _detector_spi_t
        { board_t::board_M5StackCoreS3
        , 0x04, 0xFF, 0xE3 // ILI9342C
        , 40000000, 20000000
        , GPIO_NUM_37     // MOSI
        , GPIO_NUM_35     // MISO
        , GPIO_NUM_36     // SCLK
        , GPIO_NUM_35     // DC
        , GPIO_NUM_3      // CS
        , (gpio_num_t)-1  // RST
        , GPIO_NUM_4      // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , SPI2_HOST       // SPI HOST
        } {}

        bool detect(_detector_result_t* result, bool use_reset) const override
        {
          using namespace m5stack;

          _pin_backup_t backup[] = { i2c_sda, i2c_scl };

          lgfx::i2c::init(i2c_port, i2c_sda, i2c_scl);

    // ESP_LOGI("DEBUG","AW 0x10 :%02x ", lgfx::i2c::readRegister8(i2c_port, aw9523_i2c_addr, 0x10, 400000).value());
    // ESP_LOGI("DEBUG","AXP0x03 :%02x ", lgfx::i2c::readRegister8(i2c_port, axp_i2c_addr, 0x03, 400000).value());

          auto chk_axp = lgfx::i2c::readRegister8(i2c_port, axp_i2c_addr, 0x03, i2c_freq);
          if (chk_axp.has_value() && chk_axp.value() == 0x4A)
          {
            auto chk_aw  = lgfx::i2c::readRegister8(i2c_port, aw9523_i2c_addr, 0x10, i2c_freq);
            if (chk_aw .has_value() && chk_aw .value() == 0x23)
            {
              static constexpr const uint8_t axp_reg_set[] = {
                0x90, 0xBF,           // LDOS ON/OFF control 0
                0x95, 0x28,           // ALDO3 set to 3.3v // for TF card slot
                0x96, (0b11110 - 7),  // BLDO1 => CAM AVDD 2.8V
                0x97, (0b00000 + 7),  // BLDO2 => CAM DVDD 1.2V
              };
              for (size_t i = 0; i < sizeof(axp_reg_set); i += 2) {
                lgfx::i2c::writeRegister8(i2c_port, axp_i2c_addr, axp_reg_set[i], axp_reg_set[i+1]);
              }

              lgfx::i2c::bitOn(i2c_port, aw9523_i2c_addr, 0x02, 0b00000101); //port0 output ctrl
              lgfx::i2c::bitOn(i2c_port, aw9523_i2c_addr, 0x03, 0b00000011); //port1 output ctrl

              static constexpr const uint8_t aw9523_reg_set[] = {
                0x04, 0b00011000,     // CONFIG_P0
                0x05, 0b00001100,     // CONFIG_P1
                0x11, 0b00010000,     // GCR P0 port is Push-Pull mode.
                0x12, 0b11111111,     // LEDMODE_P0
                0x13, 0b11111111,     // LEDMODE_P1
              };
              for (size_t i = 0; i < sizeof(aw9523_reg_set); i += 2) {
                lgfx::i2c::writeRegister8(i2c_port, aw9523_i2c_addr, aw9523_reg_set[i], aw9523_reg_set[i+1]);
              }

              if (_detector_spi_t::detect(result, use_reset))
              {
                return true;
              }
            }
          }
          lgfx::i2c::release(i2c_port);

          for (auto &b : backup)
          {
            b.restore();
          }

          return false;
        }

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] M5StackCoreS3");
          auto p = new Panel_M5StackCoreS3();
          result->panel = p;
          p->light(new Light_M5StackCoreS3());
          p->touch(new lgfx::Touch_M5StackCoreS3());
        }
      };

      struct _detector_ESP32_S3_BOX_t : public _detector_spi_t
      {
        constexpr _detector_ESP32_S3_BOX_t(void) :
        _detector_spi_t
        { board_t::board_ESP32_S3_BOX
        , 0x04, 0xFF, 0xE3 // ILI9342C
        , 40000000, 16000000
        , GPIO_NUM_6      // MOSI
        , GPIO_NUM_0      // MISO
        , GPIO_NUM_7      // SCLK
        , GPIO_NUM_4      // DC
        , GPIO_NUM_5      // CS
        , GPIO_NUM_48     // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , SPI2_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] ESP32_S3_BOX");
          auto p = new Panel_ILI9342();
          result->panel = p;
          {
            auto cfg = p->config();
            cfg.offset_rotation = 1;
            p->config(cfg);    // config設定;
            p->setRotation(1); // config設定後に向きを設定;
            p->light(_create_pwm_backlight(GPIO_NUM_45, 0, 12000));
          }

          {
            auto t = new lgfx::Touch_TT21xxx();
            auto cfg = t->config();
            cfg.pin_int  = GPIO_NUM_3;
            cfg.pin_sda  = GPIO_NUM_8;
            cfg.pin_scl  = GPIO_NUM_18;
            cfg.i2c_addr = 0x24;
            cfg.i2c_port = I2C_NUM_1;
            cfg.freq = 400000;
            cfg.x_min = 0;
            cfg.x_max = 319;
            cfg.y_min = 0;
            cfg.y_max = 239;
            cfg.bus_shared = false;
            t->config(cfg);
            p->touch(t);
            float affine[6] = { 1, 0, 0, 0, -1, 240 };
            p->setCalibrateAffine(affine);
          }
        }
      };

      struct _detector_ESP32_S3_BOX_Lite_t : public _detector_spi_t
      {
        constexpr _detector_ESP32_S3_BOX_Lite_t(void) :
        _detector_spi_t
        { board_t::board_ESP32_S3_BOX_Lite
        , 0x04, 0xFFFFFF, 0x528585 // ST7789V
        , 80000000, 16000000
        , GPIO_NUM_6      // MOSI
        , GPIO_NUM_0      // MISO
        , GPIO_NUM_7      // SCLK
        , GPIO_NUM_4      // DC
        , GPIO_NUM_5      // CS
        , GPIO_NUM_48     // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , SPI2_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] ESP32_S3_BOX_Lite");
          auto p = new Panel_ST7789();
          result->panel = p;
          {
            auto cfg = p->config();
            cfg.invert = true;
            cfg.offset_rotation = 2;
            p->config(cfg);
            p->setRotation(1);
            p->light(_create_pwm_backlight(GPIO_NUM_45, 0, 12000, true));
          }
        }
      };


      struct _detector_ESP32_S3_BOX_V3_t : public _detector_spi_t
      {
        constexpr _detector_ESP32_S3_BOX_V3_t(void) :
        _detector_spi_t
        { board_t::board_ESP32_S3_BOX_V3
          , 0x04, 0xff, 0xE3 // ILI9342C
          , 40000000, 16000000
          , GPIO_NUM_6      // MOSI
          , (gpio_num_t)-1  // MISO
          , GPIO_NUM_7      // SCLK
          , GPIO_NUM_4      // DC
          , GPIO_NUM_5      // CS
          , (gpio_num_t)-1  // RST
          , (gpio_num_t)-1  // TF CARD CS
          , 0               // SPI MODE
          , true            // SPI 3wire
          , SPI2_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] ESP32_S3_BOX_V3");
          lgfx::pinMode(GPIO_NUM_48, lgfx::pin_mode_t::input_pullup);
          auto p = new Panel_ILI9342();
          result->panel = p;
          {
            auto cfg = p->config();
            cfg.offset_rotation = 1;
            p->config(cfg);    // config設定;
            p->setRotation(1); // config設定後に向きを設定;
            p->light(_create_pwm_backlight(GPIO_NUM_47, 0, 12000));
          }

          {
            auto t = new lgfx::Touch_GT911();
            auto cfg = t->config();
            cfg.pin_int  = GPIO_NUM_3;
            cfg.pin_sda  = GPIO_NUM_8;
            cfg.pin_scl  = GPIO_NUM_18;
            cfg.i2c_addr = 0x14;
            cfg.i2c_port = I2C_NUM_0;
            cfg.x_min    = 0;
            cfg.x_max    = 319;
            cfg.y_min    = 0;
            // Max-y = 239 + 40 pixels for "red" touch point below LCD active area
            cfg.y_max    = 279;
            cfg.offset_rotation = 2;
            cfg.bus_shared = false;
            t->config(cfg);
            if (!t->init())
            {
              cfg.i2c_addr = 0x5D; // addr change (0x14 or 0x5D)
              t->config(cfg);
            }
            p->touch(t);
          }
        }
      };


      struct _detector_Makerfabs_ESP32_S3_TFT_Touch_SPI_t : public _detector_spi_t
      {
        constexpr _detector_Makerfabs_ESP32_S3_TFT_Touch_SPI_t(void) :
        _detector_spi_t
        { board_t::board_Makerfabs_ESP32_S3_TFT_Touch_SPI
        , 0x04, 0xFFFFFF, 0x668054 // ILI9488
        , 40000000, 20000000
        , GPIO_NUM_13     // MOSI
        , GPIO_NUM_12     // MISO
        , GPIO_NUM_14     // SCLK
        , GPIO_NUM_21     // DC
        , GPIO_NUM_15     // CS
        , (gpio_num_t)-1  // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , false           // SPI 3wire
        , SPI3_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] Makerfabs_ESP32_S3_TFT_Touch_SPI");

          // disable pull-up for MISO pin.
          PIN_PULLUP_DIS(IO_MUX_GPIO12_REG);

          auto p = new Panel_ILI9488();
          result->panel = p;
          {
            auto cfg = p->config();
            // cfg.pin_cs  = GPIO_NUM_15;
            cfg.bus_shared = false;
            p->config(cfg);
            p->light(_create_pwm_backlight(GPIO_NUM_48, 0, 12000));
          }

          {
            auto t = new lgfx::Touch_FT5x06();
            auto cfg = t->config();
            cfg.pin_sda  = GPIO_NUM_38;
            cfg.pin_scl  = GPIO_NUM_39;
            cfg.pin_int  = GPIO_NUM_40;
            cfg.i2c_addr = 0x38;
            cfg.i2c_port = I2C_NUM_0;
            cfg.freq = 400000;
            cfg.x_min = 0;
            cfg.x_max = 319;
            cfg.y_min = 0;
            cfg.y_max = 479;
            cfg.bus_shared = false;
            t->config(cfg);
            p->touch(t);
          }
        }
      };

      // https://github.com/Makerfabs/Makerfabs-ESP32-S3-Parallel-TFT-with-Touch/
      struct _detector_Makerfabs_ESP32_S3_TFT_Touch_Parallel16_t : public _detector_parallel16_t
      {
        constexpr _detector_Makerfabs_ESP32_S3_TFT_Touch_Parallel16_t(void) :
        _detector_parallel16_t
        { board_t::board_Makerfabs_ESP32_S3_TFT_Touch_Parallel16
        , 0x04, 0xFF, 0x54 // ILI9488
        , 40000000, 16000000
        // DATA 0 - 7
        , GPIO_NUM_47, GPIO_NUM_21, GPIO_NUM_14, GPIO_NUM_13, GPIO_NUM_12, GPIO_NUM_11, GPIO_NUM_10, GPIO_NUM_9
        // DATA 8-15
        , GPIO_NUM_3 , GPIO_NUM_8 , GPIO_NUM_16, GPIO_NUM_15, GPIO_NUM_7 , GPIO_NUM_6 , GPIO_NUM_5 , GPIO_NUM_4
        , GPIO_NUM_35     // WR
        , GPIO_NUM_48     // RD
        , GPIO_NUM_36     // RS
        , GPIO_NUM_37     // CS
        , (gpio_num_t)-1  // RST
        , 0               // peripheral port
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] Makerfabs_ESP32_S3_TFT_Touch_Parallel16");
          auto p = new Panel_ILI9488();
          result->panel = p;
          {
            auto cfg = p->config();
            // cfg.pin_cs  = GPIO_NUM_37;
            cfg.bus_shared = false;
            cfg.dlen_16bit = true;
            p->config(cfg);
            p->light(_create_pwm_backlight(GPIO_NUM_45, 0, 12000));
          }

          {
            auto t = new lgfx::Touch_FT5x06();
            auto cfg = t->config();
            cfg.pin_sda  = GPIO_NUM_38;
            cfg.pin_scl  = GPIO_NUM_39;
            cfg.pin_int  = GPIO_NUM_40;
            cfg.i2c_addr = 0x38;
            cfg.i2c_port = I2C_NUM_0;
            cfg.freq = 400000;
            cfg.x_min = 0;
            cfg.x_max = 319;
            cfg.y_min = 0;
            cfg.y_max = 479;
            cfg.bus_shared = false;
            t->config(cfg);
            p->touch(t);
          }
        }
      };

      // https://github.com/W00ng/ESP32-S3-HMI-DevKit/blob/master/components/bsp/include/bsp_board.h
      struct _detector_wywy_ESP32S3_HMI_DevKit_t : public _detector_parallel16_t
      {
        constexpr _detector_wywy_ESP32S3_HMI_DevKit_t(void) :
        _detector_parallel16_t
        { board_t::board_wywy_ESP32S3_HMI_DevKit
        , 0xDB00, 0xFF, 0x82 // RM68120
        , 30000000, 16000000
        // DATA 0 - 7
        , GPIO_NUM_1 , GPIO_NUM_9 , GPIO_NUM_2 , GPIO_NUM_10, GPIO_NUM_3 , GPIO_NUM_11, GPIO_NUM_4 , GPIO_NUM_12
        // DATA 8-15
        , GPIO_NUM_5 , GPIO_NUM_13, GPIO_NUM_6 , GPIO_NUM_14, GPIO_NUM_7 , GPIO_NUM_15, GPIO_NUM_8 , GPIO_NUM_16
        , GPIO_NUM_17     // WR
        , GPIO_NUM_21     // RD
        , GPIO_NUM_38     // RS
        , (gpio_num_t)-1  // CS
        , GPIO_NUM_18     // RST
        , 0               // peripheral port
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] wywy_ESP32S3_HMI_DevKit");
          auto p = new Panel_RM68120();
          result->panel = p;
          {
            auto cfg = p->config();
            // cfg.pin_rst = GPIO_NUM_18;
            cfg.bus_shared = false;
            cfg.dlen_16bit = true;
            cfg.rgb_order = true;
            cfg.panel_width = 480;
            cfg.panel_height = 800;
            cfg.memory_width = 480;
            cfg.memory_height = 800;
            p->config(cfg);
          }

          {
            auto t = new lgfx::Touch_FT5x06();
            auto cfg = t->config();
            cfg.pin_sda  = GPIO_NUM_40;
            cfg.pin_scl  = GPIO_NUM_39;
            cfg.pin_int  = -1;
            cfg.i2c_addr = 0x38;
            cfg.i2c_port = I2C_NUM_0;
            cfg.freq = 400000;
            cfg.x_min = 0;
            cfg.x_max = 479;
            cfg.y_min = 0;
            cfg.y_max = 799;
            cfg.bus_shared = false;
            t->config(cfg);
            p->touch(t);
          }
        }
      };

      struct _detector_Feather_ESP32_S3_TFT_t : public _detector_spi_t
      {
        constexpr _detector_Feather_ESP32_S3_TFT_t(void)
        : _detector_spi_t
        { board_t::board_Feather_ESP32_S3_TFT
        , 0, 0, 0         // ID checker disable
        , 40000000, 16000000
        , GPIO_NUM_35     // MOSI
        , (gpio_num_t)-1  // MISO
        , GPIO_NUM_36     // SCLK
        , GPIO_NUM_39     // DC
        , GPIO_NUM_7      // CS
        , GPIO_NUM_40     // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , SPI2_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] Feather_ESP32_S3_TFT");

          lgfx::pinMode(GPIO_NUM_21, lgfx::pin_mode_t::output);
          lgfx::gpio_hi(GPIO_NUM_21); // Enable power to TFT

          auto p = new Panel_ST7789();
          result->panel = p;
          {
            auto cfg = p->config();
            // cfg.pin_cs  = GPIO_NUM_7;
            // cfg.pin_rst = GPIO_NUM_40;
            cfg.panel_width  = 135;
            cfg.panel_height = 240;
            cfg.offset_x = 52;
            cfg.offset_y = 40;
            cfg.readable = false;
            cfg.rgb_order = false;
            cfg.invert = true;
            cfg.offset_rotation = 1;
            p->config(cfg);
            p->light(_create_pwm_backlight(GPIO_NUM_45, 0, 12000));
          }
        }
      };

      struct _detector_board_LoLinS3Pro_7735_t : public _detector_spi_t
      {
        constexpr _detector_board_LoLinS3Pro_7735_t(void)
        : _detector_spi_t
        { board_t::board_LoLinS3Pro
        , 0x04, 0xFF, 0x7C // ST7735
        , 27000000, 16000000
        , GPIO_NUM_11     // MOSI
        , GPIO_NUM_13     // MISO
        , GPIO_NUM_12     // SCLK
        , GPIO_NUM_47     // DC
        , GPIO_NUM_48     // CS
        , GPIO_NUM_21     // RST
        , GPIO_NUM_46     // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , SPI2_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] LoLinS3Pro (ST7735)");

          auto p = new Panel_ST7735S();
          result->panel = p;
          {
            auto cfg = p->config();
            cfg.memory_width  = 132;
            cfg.memory_height = 132;
            cfg.panel_width  = 128;
            cfg.panel_height = 128;
            cfg.offset_x = 2;
            cfg.offset_y = 1;
            p->config(cfg);
            p->light(_create_pwm_backlight(GPIO_NUM_14, 0, 12000 ));
          }
        }
      };

      struct _detector_board_LoLinS3Pro_9341_t : public _detector_spi_t
      {
        constexpr _detector_board_LoLinS3Pro_9341_t(void)
        : _detector_spi_t
        { board_t::board_LoLinS3Pro
        , 0x04, 0xFF, 0x00 // ILI9341
        , 40000000, 16000000
        , GPIO_NUM_11     // MOSI
        , GPIO_NUM_13     // MISO
        , GPIO_NUM_12     // SCLK
        , GPIO_NUM_47     // DC
        , GPIO_NUM_48     // CS
        , GPIO_NUM_21     // RST
        , GPIO_NUM_46     // TF CARD CS
        , 0               // SPI MODE
        , false           // SPI 3wire
        , SPI2_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] LoLinS3Pro (ILI9341)");

          auto p = new Panel_ILI9341();
          result->panel = p;
          p->light(_create_pwm_backlight(GPIO_NUM_14, 0, 12000 ));

          {
            auto t = new lgfx::Touch_XPT2046();
            auto cfg = t->config();
            cfg.bus_shared = true;
            cfg.spi_host = spi_host;
            cfg.pin_cs   = GPIO_NUM_45;
            cfg.pin_mosi = pin_mosi;
            cfg.pin_miso = pin_miso;
            cfg.pin_sclk = pin_sclk;
            cfg.offset_rotation = 2;
            t->config(cfg);
            p->touch(t);
          }
        }
      };

#elif defined (CONFIG_IDF_TARGET_ESP32S2)

      // https://github.com/Makerfabs/Makerfabs-ESP32-S2-Parallel-TFT-with-Touch/
      struct _detector_Makerfabs_ESP32_S2_TFT_Touch_Parallel16_t : public _detector_parallel16_t
      {
        constexpr _detector_Makerfabs_ESP32_S2_TFT_Touch_Parallel16_t(void) :
        _detector_parallel16_t
        { board_t::board_Makerfabs_ESP32_S2_TFT_Touch_Parallel16
        , 0x04, 0xFFFFFF, 0x668054 // ILI9488
        , 27000000, 16000000
        // DATA 0 - 7
        , GPIO_NUM_33, GPIO_NUM_21, GPIO_NUM_14, GPIO_NUM_13, GPIO_NUM_12, GPIO_NUM_11, GPIO_NUM_10, GPIO_NUM_9
        // DATA 8-15
        , GPIO_NUM_3 , GPIO_NUM_8 , GPIO_NUM_16, GPIO_NUM_15, GPIO_NUM_7 , GPIO_NUM_6 , GPIO_NUM_5 , GPIO_NUM_4
        , GPIO_NUM_35     // WR
        , GPIO_NUM_34     // RD
        , GPIO_NUM_36     // RS
        , GPIO_NUM_37     // CS
        , (gpio_num_t)-1  // RST
        , 0               // peripheral port
        } {}

        void setup(_detector_result_t* result) const override
        {
          static constexpr int I2C_PORT_NUM = I2C_NUM_0;
          static constexpr int I2C_PIN_SDA = GPIO_NUM_38;
          static constexpr int I2C_PIN_SCL = GPIO_NUM_39;
          static constexpr int I2C_PIN_INT = GPIO_NUM_40;

          ESP_LOGI(LIBRARY_NAME, "[Autodetect] Makerfabs_ESP32_S2_TFT_Touch_Parallel16");
          auto p = new Panel_ILI9488();
          result->panel = p;
          {
            auto cfg = p->config();
            // cfg.pin_cs  = GPIO_NUM_37;
            cfg.bus_shared = false;
            cfg.dlen_16bit = true;
            p->config(cfg);
            p->light(_create_pwm_backlight(GPIO_NUM_45, 7, 12000));
          }

          lgfx::ITouch::config_t cfg;
          lgfx::ITouch* touch = nullptr;
          lgfx::i2c::init(I2C_PORT_NUM, I2C_PIN_SDA, I2C_PIN_SCL);
          if (lgfx::i2c::beginTransaction(I2C_PORT_NUM, 0x38, 400000, false).has_value()
          && lgfx::i2c::endTransaction(I2C_PORT_NUM).has_value())
          {
            touch = new lgfx::Touch_FT5x06();
            cfg = touch->config();
            cfg.i2c_addr = 0x38;
            cfg.x_max = 319;
            cfg.y_max = 479;
          }
          else
          if (lgfx::i2c::beginTransaction(I2C_PORT_NUM, 0x48, 400000, false).has_value()
          && lgfx::i2c::endTransaction(I2C_PORT_NUM).has_value())
          {
            touch = new lgfx::Touch_NS2009();
            cfg = touch->config();
            cfg.i2c_addr = 0x48;
            cfg.x_min = 368;
            cfg.y_min = 212;
            cfg.x_max = 3800;
            cfg.y_max = 3800;
          }
          if (touch != nullptr)
          {
            cfg.i2c_port = I2C_PORT_NUM;
            cfg.pin_sda  = I2C_PIN_SDA;
            cfg.pin_scl  = I2C_PIN_SCL;
            cfg.pin_int  = I2C_PIN_INT;
            cfg.freq = 400000;
            cfg.bus_shared = false;
            touch->config(cfg);
            p->touch(touch);
          }
        }
      };

      // https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/hw-reference/esp32s2/user-guide-esp32-s2-kaluga-1-kit.html
      struct _detector_ESP32_S2_Kaluga_1_t : public _detector_spi_t
      {
        constexpr _detector_ESP32_S2_Kaluga_1_t(void) :
        _detector_spi_t
        { board_t::board_ESP32_S2_Kaluga_1
        , 0x04, 0xFF, 0x85 // ST7789
        , 80000000, 16000000
        , GPIO_NUM_9      // MOSI
        , GPIO_NUM_8      // MISO
        , GPIO_NUM_15     // SCLK
        , GPIO_NUM_13     // DC
        , GPIO_NUM_11     // CS
        , GPIO_NUM_16     // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , false           // SPI 3wire
        , SPI2_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] ESP32_S2_Kaluga_1");
          auto p = new Panel_ST7789();
          result->panel = p;
          {
            // auto cfg = p->config();
            // cfg.pin_cs  = GPIO_NUM_11;
            // cfg.pin_rst = GPIO_NUM_16;
            // p->config(cfg);
            p->light(_create_pwm_backlight(GPIO_NUM_6, 7, 12000));
          }
        }
      };

      // https://www.adafruit.com/product/5300
      struct _detector_Feather_ESP32_S2_TFT_t : public _detector_spi_t
      {
        constexpr _detector_Feather_ESP32_S2_TFT_t(void) :
        _detector_spi_t
        { board_t::board_Feather_ESP32_S2_TFT
        , 0,0,0       // ST7789 (write only)
        , 40000000, 16000000
        , GPIO_NUM_35     // MOSI
        , (gpio_num_t)-1  // MISO
        , GPIO_NUM_36     // SCLK
        , GPIO_NUM_39     // DC
        , GPIO_NUM_7      // CS
        , GPIO_NUM_40     // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , SPI2_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] Feather_ESP32_S2_TFT");
          auto p = new Panel_ST7789();
          result->panel = p;
          {
            auto cfg = p->config();
            // cfg.pin_cs  = GPIO_NUM_7;
            // cfg.pin_rst = GPIO_NUM_40;
            cfg.panel_width  = 135;
            cfg.panel_height = 240;
            cfg.offset_x = 52;
            cfg.offset_y = 40;
            cfg.readable = false;
            cfg.rgb_order = false;
            cfg.invert = true;
            cfg.offset_rotation = 1;
            p->config(cfg);
            p->light(_create_pwm_backlight(GPIO_NUM_45, 0, 12000));
          }
        }
      };

      // https://www.adafruit.com/product/4985
      struct _detector_FunHouse_t : public _detector_spi_t
      {
        constexpr _detector_FunHouse_t(void) :
        _detector_spi_t
        { board_t::board_FunHouse
        , 0,0,0       // ST7789 (write only)
        , 40000000, 16000000
        , GPIO_NUM_35     // MOSI
        , (gpio_num_t)-1  // MISO
        , GPIO_NUM_36     // SCLK
        , GPIO_NUM_39     // DC
        , GPIO_NUM_40     // CS
        , GPIO_NUM_41     // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , SPI2_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] FunHouse");
          auto p = new Panel_ST7789();
          result->panel = p;
          {
            auto cfg = p->config();
            // cfg.pin_cs  = GPIO_NUM_40;
            // cfg.pin_rst = GPIO_NUM_41;
            cfg.panel_width  = 240;
            cfg.panel_height = 240;
            cfg.readable = false;
            cfg.rgb_order = false;
            cfg.invert = true;
            cfg.offset_rotation = 2;
            p->config(cfg);
            p->light(_create_pwm_backlight(GPIO_NUM_21, 0, 12000));
          }
        }
      };

#elif defined (CONFIG_IDF_TARGET_ESP32C3)

      struct _detector_Makerfabs_ESP32_C3_TFT_Touch_SPI_t : public _detector_spi_t
      {
        constexpr _detector_Makerfabs_ESP32_C3_TFT_Touch_SPI_t(void) :
        _detector_spi_t
        { board_t::board_Makerfabs_ESP32_C3_TFT_Touch_SPI
        , 0x04, 0xFFFFFF, 0x668054 // ILI9488
        , 40000000, 20000000
        , GPIO_NUM_6      // MOSI
        , GPIO_NUM_7      // MISO
        , GPIO_NUM_5      // SCLK
        , GPIO_NUM_10     // DC
        , GPIO_NUM_4      // CS
        , (gpio_num_t)-1  // RST
        , GPIO_NUM_1      // TF CARD CS
        , 0               // SPI MODE
        , false           // SPI 3wire
        , SPI2_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] Makerfabs_ESP32_C3_TFT_Touch_SPI");

          // disable pull-up for MISO pin.
          // PIN_PULLUP_DIS(IO_MUX_GPIO7_REG);

          auto p = new Panel_ILI9488();
          result->panel = p;
          {
            auto cfg = p->config();
            // cfg.pin_cs  = GPIO_NUM_4;
            cfg.bus_shared = true;
            p->config(cfg);
            p->light(_create_pwm_backlight(GPIO_NUM_8, 0, 12000));
          }

          {
            auto t = new lgfx::Touch_FT5x06();
            auto cfg = t->config();
            cfg.pin_sda  = GPIO_NUM_2;
            cfg.pin_scl  = GPIO_NUM_3;
            cfg.pin_int  = GPIO_NUM_0;
            cfg.i2c_addr = 0x38;
            cfg.i2c_port = I2C_NUM_0;
            cfg.freq = 400000;
            cfg.x_min = 0;
            cfg.x_max = 319;
            cfg.y_min = 0;
            cfg.y_max = 479;
            cfg.bus_shared = false;
            t->config(cfg);
            p->touch(t);
          }
        }
      };

#elif defined (CONFIG_IDF_TARGET_ESP32) || !defined (CONFIG_IDF_TARGET)

      struct _detector_M5StickCPlus_t : public _detector_spi_t
      {
        constexpr _detector_M5StickCPlus_t(void) :
        _detector_spi_t
        { board_t::board_M5StickCPlus
        , 0x04, 0xFF, 0x85 // ST7789
        , 40000000, 15000000
        , GPIO_NUM_15     // MOSI
        , GPIO_NUM_14     // MISO
        , GPIO_NUM_13     // SCLK
        , GPIO_NUM_23     // DC
        , GPIO_NUM_5      // CS
        , GPIO_NUM_18     // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , HSPI_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] M5StickCPlus");

          auto p = new Panel_M5StickCPlus();
          result->panel = p;
          p->light(new Light_M5StickC());
        }
      };

      struct _detector_M5StickCPlus2_t : public _detector_spi_t
      {
        constexpr _detector_M5StickCPlus2_t(void) :
        _detector_spi_t
        { board_t::board_M5StickCPlus2
        , 0x04, 0xFF, 0x85 // ST7789
        , 40000000, 15000000
        , GPIO_NUM_15     // MOSI
        , (gpio_num_t)-1  // MISO
        , GPIO_NUM_13     // SCLK
        , GPIO_NUM_14     // DC
        , GPIO_NUM_5      // CS
        , GPIO_NUM_12     // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , HSPI_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] M5StickCPlus2");

          auto p = new Panel_M5StickCPlus();
          result->panel = p;
          p->light(_create_pwm_backlight(GPIO_NUM_27, 7, 256, false, 40));
        }
      };

      struct _detector_M5StickC_t : public _detector_spi_t
      {
        constexpr _detector_M5StickC_t(void) :
        _detector_spi_t
        { board_t::board_M5StickC
        , 0x04, 0xFF, 0x7C // ST7735S
        , 27000000, 14000000
        , GPIO_NUM_15     // MOSI
        , GPIO_NUM_14     // MISO
        , GPIO_NUM_13     // SCLK
        , GPIO_NUM_23     // DC
        , GPIO_NUM_5      // CS
        , GPIO_NUM_18     // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , HSPI_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] M5StickC");

          auto p = new Panel_M5StickC();
          result->panel = p;
          p->light(new Light_M5StickC());
        }
      };

      struct _detector_M5StackCoreInk_t : public _detector_spi_t
      {
        constexpr _detector_M5StackCoreInk_t(void)
        : _detector_spi_t
        { board_t::board_M5StackCoreInk
        , 0x70, 0xFFFFFF, 0xE00100 // e-paper GDEW0154M09
        , 40000000, 16000000
        , GPIO_NUM_23     // MOSI
        , GPIO_NUM_34     // MISO
        , GPIO_NUM_18     // SCLK
        , GPIO_NUM_15     // DC
        , GPIO_NUM_9      // CS
        , GPIO_NUM_0      // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , VSPI_HOST       // SPI HOST
        } {}

        bool detect(_detector_result_t* result, bool use_reset) const override
        {
          // ESPがスリープしていると検出に失敗するため、リセットは必須とする;
          return _detector_spi_t::detect(result, true);
        }

        void setup(_detector_result_t* result) const override
        {
          _pin_level(GPIO_NUM_12, true);  // POWER_HOLD_PIN 12

          ESP_LOGI(LIBRARY_NAME, "[Autodetect] M5StackCoreInk");

          auto p = new Panel_GDEW0154M09();
          result->panel = p;
          {
            auto cfg = p->config();
            cfg.panel_height = 200;
            cfg.panel_width  = 200;
            // cfg.pin_cs   = GPIO_NUM_9;
            // cfg.pin_rst  = GPIO_NUM_0;
            cfg.pin_busy = GPIO_NUM_4;
            p->config(cfg);
          }
        }
      };

      struct _detector_TTGO_TWristband_t : public _detector_spi_t
      {
        constexpr _detector_TTGO_TWristband_t(void)
        : _detector_spi_t
        { board_t::board_TTGO_TWristband
        , 0x04, 0xFF, 0x7C // ST7735
        , 27000000, 14000000
        , GPIO_NUM_19     // MOSI
        , (gpio_num_t)-1  // MISO
        , GPIO_NUM_18     // SCLK
        , GPIO_NUM_23     // DC
        , GPIO_NUM_5      // CS
        , GPIO_NUM_26     // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , HSPI_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] TTGO_TWristband");

          auto p = new Panel_ST7735S();
          result->panel = p;
          {
            auto cfg = p->config();
            cfg.dummy_read_pixel = 17;
            cfg.invert = true;
            // cfg.pin_cs  = GPIO_NUM_5;
            // cfg.pin_rst = GPIO_NUM_26;
            cfg.panel_width  = 80;
            cfg.panel_height = 160;
            cfg.offset_x     = 26;
            cfg.offset_y     = 1;
            cfg.offset_rotation = 2;
            cfg.bus_shared = false;
            p->config(cfg);
            p->light(_create_pwm_backlight(GPIO_NUM_27, 7, 12000));
          }
        }
      };

      // https://github.com/LilyGO/TTGO-TS
      struct _detector_TTGO_TS_t : public _detector_spi_t
      {
        constexpr _detector_TTGO_TS_t(void)
        : _detector_spi_t
        { board_t::board_TTGO_TS
        , 0x04, 0xFF, 0x7C // ST7735
        , 20000000, 14000000
        , GPIO_NUM_23     // MOSI
        , (gpio_num_t)-1  // MISO
        , GPIO_NUM_5      // SCLK
        , GPIO_NUM_17     // DC
        , GPIO_NUM_16     // CS
        , GPIO_NUM_9      // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , HSPI_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] TTGO_TWristband");

          auto p = new Panel_ST7735S();
          result->panel = p;
          {
            auto cfg = p->config();
            // cfg.pin_cs  = GPIO_NUM_16;
            // cfg.pin_rst = GPIO_NUM_9;
            cfg.panel_width  = 128;
            cfg.panel_height = 160;
            cfg.offset_x     = 2;
            cfg.offset_y     = 1;
            cfg.offset_rotation = 2;
            p->config(cfg);
            p->light(_create_pwm_backlight(GPIO_NUM_27, 7, 12000));
          }
        }
      };

      struct _detector_ODROID_GO_t : public _detector_spi_t
      {
        constexpr _detector_ODROID_GO_t(void)
        : _detector_spi_t
        { board_t::board_ODROID_GO
        , 0x04, 0xFF, 0   // ILI9341
        , 40000000, 16000000
        , GPIO_NUM_23     // MOSI
        , GPIO_NUM_19     // MISO
        , GPIO_NUM_18     // SCLK
        , GPIO_NUM_21     // DC
        , GPIO_NUM_5      // CS
        , (gpio_num_t)-1  // RST
        , GPIO_NUM_22     // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , VSPI_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] ODROID_GO");

          auto p = new Panel_ILI9341();
          result->panel = p;
          {
            // auto cfg = p->config();
            // cfg.pin_cs  = GPIO_NUM_5;
            // p->config(cfg);
            p->setRotation(1);
            p->light(_create_pwm_backlight(GPIO_NUM_14, 7));
          }
        }
      };

      struct _detector_M5Station_t : public _detector_spi_t
      {
        constexpr _detector_M5Station_t(void)
        : _detector_spi_t
        { board_t::board_M5Station
        , 0x04, 0xFF, 0x85 // ST7789
        , 40000000, 15000000
        , GPIO_NUM_23     // MOSI
        , (gpio_num_t)-1  // MISO
        , GPIO_NUM_18     // SCLK
        , GPIO_NUM_19     // DC
        , GPIO_NUM_5      // CS
        , GPIO_NUM_15     // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , HSPI_HOST       // SPI HOST
        } {}

        bool detect(_detector_result_t* result, bool use_reset) const override
        {
          using namespace m5stack;
          _pin_backup_t backup[] = { axp_i2c_sda, axp_i2c_scl };
          lgfx::i2c::init(axp_i2c_port, axp_i2c_sda, axp_i2c_scl);
          if (lgfx::i2c::readRegister8(axp_i2c_port, axp_i2c_addr, 0x03, 400000) == 0x03) // AXP192 found
          {
            if (_detector_spi_t::detect(result, use_reset))
            {
              return true;
            }
          }
          lgfx::i2c::release(axp_i2c_port);
          for (auto &b : backup)
          {
            b.restore();
          }
          return false;
        }

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] M5Station");

          // M5StationのLCDはM5StickCPlusと同じ;
          auto p = new Panel_M5StickCPlus();
          result->panel = p;
          {
            // auto cfg = p->config();
            // cfg.pin_rst = GPIO_NUM_15;
            // p->config(cfg);
            p->setRotation(1);
            // M5StationのバックライトはM5Toughと同じ;
            p->light(new Light_M5Tough());
          }
        }
      };

      struct _detector_M5StackCore2_t : public _detector_spi_t
      {
        constexpr _detector_M5StackCore2_t(void)
        : _detector_spi_t
        { board_t::board_M5StackCore2
        , 0x04, 0xFF, 0xE3 // ILI9342
        , 40000000, 16000000
        , GPIO_NUM_23     // MOSI
        , GPIO_NUM_38     // MISO
        , GPIO_NUM_18     // SCLK
        , GPIO_NUM_15     // DC
        , GPIO_NUM_5      // CS
        , (gpio_num_t)-1  // RST
        , GPIO_NUM_4      // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , VSPI_HOST       // SPI HOST
        } {}

        bool detect(_detector_result_t* result, bool use_reset) const override
        {
          if (result->board && result->board != board && result->board != board_t::board_M5Tough)
          {
            return false;
          }

          using namespace m5stack;
          _pin_backup_t backup[] = { axp_i2c_sda, axp_i2c_scl };
          lgfx::i2c::init(axp_i2c_port, axp_i2c_sda, axp_i2c_scl);

          auto chk_axp = lgfx::i2c::readRegister8(axp_i2c_port, axp_i2c_addr, 0x03, 400000);
          if (chk_axp.has_value())
          {
            uint_fast16_t axp_exists = 0;
            if (chk_axp.value() == 0x03) { // AXP192 found
              axp_exists = 192;
              ESP_LOGD(LIBRARY_NAME, "AXP192 found");
            }
            else if (chk_axp.value() == 0x4A) { // AXP2101 found
              axp_exists = 2101;
              ESP_LOGD(LIBRARY_NAME, "AXP2101 found");
            }
            if (axp_exists)
            {
              // fore Core2 1st gen (AXP192)
                // AXP192_LDO2 = LCD PWR
                // AXP192_IO4  = LCD RST
                // AXP192_DC3  = LCD BL (Core2)
                // AXP192_LDO3 = LCD BL (Tough)
                // AXP192_IO1  = TP RST (Tough)
              static constexpr uint8_t reg_data_axp192_first[] = {
                0x95, 0x84, 0x72,   // GPIO4 enable
                0x28, 0xF0, 0xFF,   // set LDO2 3300mv // LCD PWR
                0x12, 0x04, 0xFF,   // LDO2 enable
                0x92, 0x00, 0xF8,   // GPIO1 OpenDrain (M5Tough TOUCH)
                0xFF, 0xFF, 0xFF,
              };
              static constexpr uint8_t reg_data_axp192_reset[] = {
                0x96, 0x00, 0xFD,   // GPIO4 LOW (LCD RST)
                0x94, 0x00, 0xFD,   // GPIO1 LOW (M5Tough TOUCH RST)
                0xFF, 0xFF, 0xFF,
              };
              static constexpr uint8_t reg_data_axp192_second[] = {
                0x96, 0x02, 0xFF,   // GPIO4 HIGH (LCD RST)
                0x94, 0x02, 0xFF,   // GPIO1 HIGH (M5Tough TOUCH RST)
                0xFF, 0xFF, 0xFF,
              };

              // for Core2 v1.1 (AXP2101)
                // ALDO2 == LCD+TOUCH RST
                // ALDO3 == SPK EN
                // ALDO4 == TF, TP, LCD PWR
                // BLDO1 == LCD BL
                // BLDO2 == Boost EN
                // DLDO1 == Vibration Motor
              static constexpr uint8_t reg_data_axp2101_first[] = {
                0x90, 0x08, 0x7B,   // ALDO4 ON / ALDO3 OFF, DLDO1 OFF
                0x80, 0x05, 0xFF,   // DCDC1 + DCDC3 ON
                0x82, 0x12, 0x00,   // DCDC1 3.3V
                0x84, 0x6A, 0x00,   // DCDC3 3.3V
                0xFF, 0xFF, 0xFF,
              };
              static constexpr uint8_t reg_data_axp2101_reset[] = {
                0x90, 0x00, 0xFD,   // ALDO2 OFF
                0xFF, 0xFF, 0xFF,
              };
              static constexpr uint8_t reg_data_axp2101_second[] = {
                0x90, 0x02, 0xFF,   // ALDO2 ON
                0xFF, 0xFF, 0xFF,
              };

              _pin_level(GPIO_NUM_5, true);

              bool isAxp192 = axp_exists == 192;

              i2c_write_register8_array(axp_i2c_port, axp_i2c_addr, isAxp192 ? reg_data_axp192_first : reg_data_axp2101_first, axp_i2c_freq);
              if (use_reset) {
                i2c_write_register8_array(axp_i2c_port, axp_i2c_addr, isAxp192 ? reg_data_axp192_reset : reg_data_axp2101_reset, axp_i2c_freq);
                lgfx::delay(1);
              }
              i2c_write_register8_array(axp_i2c_port, axp_i2c_addr, isAxp192 ? reg_data_axp192_second : reg_data_axp2101_second, axp_i2c_freq);
              lgfx::delay(1);

              result->board = board_t::board_unknown;
              if (_detector_spi_t::detect(result, use_reset))
              {
                return true;
              }
            }
          }
          lgfx::i2c::release(axp_i2c_port);
          for (auto &b : backup)
          {
            b.restore();
          }
          return false;
        }

        void setup(_detector_result_t* result) const override
        {
          using namespace m5stack;

          auto p = new Panel_M5StackCore2();
          result->panel = p;
          ITouch* t;
          // Tough のタッチコントローラ有無をチェックする;
          // Core2/Tough 判別条件としてCore2のTP(0x38)の有無を用いた場合、以下の問題が生じる;
          // ・Core2のTPがスリープしている場合は反応が得られない;
          // ・ToughにGoPlus2を組み合わせると0x38に反応がある;
          // 上記のことから、ここではToughのTP(0x2E)の有無によって判定する;
          if ( lgfx::i2c::readRegister8(axp_i2c_port, 0x2E, 0, 400000).has_value()) // 0x2E:M5Tough TOUCH
          {
            ESP_LOGI(LIBRARY_NAME, "[Autodetect] M5Tough");
            result->board = board_t::board_M5Tough;
            p->light(new Light_M5Tough());
            t = new lgfx::Touch_M5Tough();
            p->touch(t);
          }
          else
          {
            ESP_LOGI(LIBRARY_NAME, "[Autodetect] M5StackCore2");

            auto chk_axp = lgfx::i2c::readRegister8(axp_i2c_port, axp_i2c_addr, 0x03, 400000);
            p->light( chk_axp.value() == 0x4A // AXP2101 found
                    ? (lgfx::ILight*)(new Light_M5StackCore2_AXP2101())
                    : (lgfx::ILight*)(new Light_M5StackCore2())
                    );
            t = new lgfx::Touch_FT5x06();
            auto cfg = t->config();
            cfg.x_min = 0;
            cfg.x_max = 319;
            cfg.y_min = 0;
            cfg.y_max = 279;
            t->config(cfg);
            p->touch(t);
            // Touch 登録時に計算される標準変換式を上書きする;
            // 標準式では表示領域外の仮想ボタンの高さ分だけずれてしまう;
            float affine[6] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f };
            p->setCalibrateAffine(affine);
          }
          auto cfg = t->config();
          cfg.pin_int  = GPIO_NUM_39;   // INT pin number
          cfg.pin_sda  = axp_i2c_sda;   // I2C SDA pin number
          cfg.pin_scl  = axp_i2c_scl;   // I2C SCL pin number
          cfg.i2c_port = I2C_NUM_1;// I2C port number
          cfg.freq = 400000;   // I2C freq
          cfg.bus_shared = false;
          t->config(cfg);
        }
      };

      // https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library
      struct _detector_TTGO_TWatch_t : public _detector_spi_t
      {
        constexpr _detector_TTGO_TWatch_t(void)
        : _detector_spi_t
        { board_t::board_TTGO_TWatch
        , 0x04, 0xFF, 0x85 // ST7789
        , 80000000, 20000000
        , GPIO_NUM_19     // MOSI
        , (gpio_num_t)-1  // MISO
        , GPIO_NUM_18     // SCLK
        , GPIO_NUM_27     // DC
        , GPIO_NUM_5      // CS
        , GPIO_NUM_26     // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , HSPI_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] TTGO_TWatch");

          auto p = new Panel_ST7789();
          result->panel = p;
          {
            auto cfg = p->config();
            cfg.invert = true;
            // cfg.pin_cs  = GPIO_NUM_5;
            // cfg.pin_rst = GPIO_NUM_26;
            cfg.panel_width  = 240;
            cfg.panel_height = 240;
            p->config(cfg);
            p->setRotation(1);
            p->light(new Light_TWatch());
          }

    /// 2020 v1 : GPIO12 & AXP202 LDO2
    /// 2020 v2 : GPIO25 & AXP202 LDO2
    /// 2020 v3 : GPIO15 & AXP202 LDO2
    // 年式によってバックライト用のGPIOが異なるので一通りHIGHに設定しておく
          _pin_level(GPIO_NUM_15, true);
          _pin_level(GPIO_NUM_25, true);

          {
            auto t = new lgfx::Touch_FT5x06();
            auto cfg = t->config();
            cfg.pin_int  = GPIO_NUM_38;   // INT pin number
            cfg.pin_sda  = GPIO_NUM_23;   // I2C SDA pin number
            cfg.pin_scl  = GPIO_NUM_32;   // I2C SCL pin number
            cfg.i2c_addr = 0x38; // I2C device addr
            cfg.i2c_port = I2C_NUM_1;// I2C port number
            cfg.freq = 400000;   // I2C freq
            cfg.x_min = 0;
            cfg.y_min = 0;
            // cfg.x_max = 319;
            // cfg.y_max = 319;
            cfg.x_max = 239;
            cfg.y_max = 239;
            cfg.bus_shared = false;
            t->config(cfg);
            p->touch(t);
          }
        }
      };

      struct _detector_TTGO_TDisplay_t : public _detector_spi_t
      {
        constexpr _detector_TTGO_TDisplay_t(void)
        : _detector_spi_t
        { board_t::board_TTGO_TDisplay
        , 0x04, 0xFF, 0x85 // ST7789
        , 40000000, 14000000
        , GPIO_NUM_19     // MOSI
        , (gpio_num_t)-1  // MISO
        , GPIO_NUM_18     // SCLK
        , GPIO_NUM_16     // DC
        , GPIO_NUM_5      // CS
        , GPIO_NUM_23     // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , HSPI_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] TTGO_TDisplay");

          auto p = new Panel_ST7789();
          result->panel = p;
          {
            auto cfg = p->config();
            cfg.invert = true;
            cfg.panel_width  = 135;
            cfg.panel_height = 240;
            cfg.offset_x     = 52;
            cfg.offset_y     = 40;
            cfg.dummy_read_pixel = 16;
            cfg.dummy_read_bits  =  1;
            p->config(cfg);
            p->light(_create_pwm_backlight(GPIO_NUM_4, 7, 44100));
          }
        }
      };

      struct _detector_TTGO_T4_Display_t : public _detector_spi_t
      {
        constexpr _detector_TTGO_T4_Display_t(void)
        : _detector_spi_t
        { board_t::board_TTGO_T4_Display
        , 0x04, 0xFF, 0x00 // ili9341
        , 40000000, 16000000
        , GPIO_NUM_23     // MOSI
        , GPIO_NUM_12     // MISO
        , GPIO_NUM_18     // SCLK
        , GPIO_NUM_32     // DC
        , GPIO_NUM_27     // CS
        , GPIO_NUM_5      // RST
        , GPIO_NUM_13     // TF CARD CS
        , 0               // SPI MODE
        , false           // SPI 3wire
        , VSPI_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] TTGO_T4_Display");

          auto p = new Panel_ILI9341();
          result->panel = p;
          {
            auto cfg = p->config();
            cfg.pin_cs  = GPIO_NUM_27;
            cfg.pin_rst = GPIO_NUM_5;
            cfg.pin_busy     =  -1;
            cfg.panel_width  = 240;
            cfg.panel_height = 320;
            cfg.offset_x     = 0;
            cfg.offset_y     = 0;
            cfg.offset_rotation  =  2;
            cfg.dummy_read_pixel =  8;
            cfg.dummy_read_bits  =  1;
            cfg.readable         =  true;
            cfg.invert           = false;
            cfg.rgb_order        = false;
            cfg.dlen_16bit       = false;
            cfg.bus_shared       =  true;

            p->config(cfg);
            p->light(_create_pwm_backlight(GPIO_NUM_4, 7, 44100));
          }
        }
      };

      struct _detector_WiFiBoy_Mini_t : public _detector_spi_t
      {
        constexpr _detector_WiFiBoy_Mini_t(void)
        : _detector_spi_t
        { board_t::board_WiFiBoy_Mini
        , 0x04, 0xFF, 0x7C // ST7735
        , 20000000, 8000000
        , GPIO_NUM_13     // MOSI
        , GPIO_NUM_12     // MISO
        , GPIO_NUM_14     // SCLK
        , GPIO_NUM_4      // DC
        , GPIO_NUM_15     // CS
        , (gpio_num_t)-1  // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , HSPI_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] WiFiBoy_Mini");

          auto p = new Panel_ST7735S();
          result->panel = p;
          {
            auto cfg = p->config();
            cfg.panel_width  = 128;
            cfg.panel_height = 128;
            cfg.memory_width  = 132;
            cfg.memory_height = 132;
            cfg.offset_x     = 2;
            cfg.offset_y     = 1;
            cfg.offset_rotation = 2;
            p->config(cfg);
            p->light(_create_pwm_backlight(GPIO_NUM_27, 7, 12000));
          }
        }
      };

      struct _detector_WiFiBoy_Pro_t : public _detector_spi_t
      {
        constexpr _detector_WiFiBoy_Pro_t(void)
        : _detector_spi_t
        { board_t::board_WiFiBoy_Pro
        , 0x04, 0xFF, 0x00 // ILI9341
        , 40000000, 16000000
        , GPIO_NUM_13     // MOSI
        , GPIO_NUM_12     // MISO
        , GPIO_NUM_14     // SCLK
        , GPIO_NUM_4      // DC
        , GPIO_NUM_15     // CS
        , (gpio_num_t)-1  // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , false           // SPI 3wire
        , HSPI_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] WiFiBoy_Pro");

          auto p = new Panel_ILI9341();
          result->panel = p;
          {
            auto cfg = p->config();
            cfg.offset_rotation = 2;
            p->config(cfg);
            p->light(_create_pwm_backlight(GPIO_NUM_27, 7, 12000));
          }
        }
      };

      struct _detector_Makerfabs_MakePython_t : public _detector_spi_t
      {
        constexpr _detector_Makerfabs_MakePython_t(void)
        : _detector_spi_t
        { board_t::board_Makerfabs_MakePython
        , 0x04, 0xFF, 0x85 // ST7789
        , 80000000, 14000000
        , GPIO_NUM_13     // MOSI
        , GPIO_NUM_12     // MISO
        , GPIO_NUM_14     // SCLK
        , GPIO_NUM_22     // DC
        , GPIO_NUM_15     // CS
        , GPIO_NUM_21     // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , HSPI_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] Makerfabs_MakePython");

          auto p = new Panel_ST7789();
          result->panel = p;
          {
            auto cfg = p->config();
            cfg.invert = true;
            cfg.panel_height = 240;
            p->config(cfg);
            p->light(_create_pwm_backlight(GPIO_NUM_5, 7, 12000));
          }
        }
      };

      struct _detector_M5Stack_t : public _detector_spi_t
      {
        constexpr _detector_M5Stack_t(void)
        : _detector_spi_t
        { board_t::board_M5Stack
        , 0x04, 0xFF, 0xE3 // ILI9342
        , 40000000, 16000000
        , GPIO_NUM_23     // MOSI
        , GPIO_NUM_19     // MISO
        , GPIO_NUM_18     // SCLK
        , GPIO_NUM_27     // DC
        , GPIO_NUM_14     // CS
        , GPIO_NUM_33     // RST
        , GPIO_NUM_4      // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , VSPI_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] M5Stack");

          auto p = new Panel_M5Stack();
          result->panel = p;
          p->light(_create_pwm_backlight(GPIO_NUM_32, 7, 44100));
        }
      };

      struct _detector_M5Paper_t : public _detector_spi_t
      {
        constexpr _detector_M5Paper_t(void)
        : _detector_spi_t
        { board_t::board_M5Paper
        , 0,0,0 // IT8951 独自判定ルールのため無設定;
        , 40000000, 20000000
        , GPIO_NUM_12     // MOSI
        , GPIO_NUM_13     // MISO
        , GPIO_NUM_14     // SCLK
        , (gpio_num_t)-1  // DC
        , GPIO_NUM_15     // CS
        , GPIO_NUM_23     // RST
        , GPIO_NUM_4      // TF CARD CS
        , 0               // SPI MODE
        , false           // SPI 3wire
        , VSPI_HOST       // SPI HOST
        } {}

        bool detect(_detector_result_t* result, bool use_reset) const override
        {
          constexpr gpio_num_t pin_power = GPIO_NUM_2;
          constexpr gpio_num_t pin_busy = GPIO_NUM_27;

          if (result->board && result->board != board)
          {
            return false;
          }

          _pin_backup_t backup[] = { pin_power, pin_busy, pin_rst };

          _pin_reset(pin_rst, true);
          lgfx::pinMode(pin_busy, lgfx::pin_mode_t::input_pullup); // M5Paper EPD busy pin
          if (!lgfx::gpio_in(pin_busy))
          {
            _pin_level(GPIO_NUM_2, true);  // M5EPD_MAIN_PWR_PIN 2
            lgfx::pinMode(GPIO_NUM_27, lgfx::pin_mode_t::input);
            if (_detector_spi_t::detect(result, use_reset))
            {
              return true;
            }
          }
          for (auto &b : backup)
          {
            b.restore();
          }
          return false;
        }

        bool judgement(IBus* bus, int) const override
        {
          uint32_t id = lgfx::millis();
          do
          {
            vTaskDelay(1);
            if (lgfx::millis() - id > 1024) { id = 0; break; }
          } while (!lgfx::gpio_in(GPIO_NUM_27));
          if (id)
          {
            bus->beginTransaction();
            lgfx::gpio_lo(GPIO_NUM_15);
            bus->writeData(getSwap16(0x6000), 16);
            bus->writeData(getSwap16(0x0302), 16);  // read DevInfo
            id = lgfx::millis();
            bus->wait();
            lgfx::gpio_hi(GPIO_NUM_15);
            do
            {
              vTaskDelay(1);
              if (lgfx::millis() - id > 192) { break; }
            } while (!lgfx::gpio_in(GPIO_NUM_27));
            lgfx::gpio_lo(GPIO_NUM_15);
            bus->writeData(getSwap16(0x1000), 16);
            bus->writeData(getSwap16(0x0000), 16);
            uint8_t buf[40];
            bus->beginRead();
            bus->readBytes(buf, 40, false);
            bus->endRead();
            bus->endTransaction();
            lgfx::gpio_hi(GPIO_NUM_15);
            id = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
            // ESP_LOGV(LIBRARY_NAME, "[Autodetect] panel size :%08x", (unsigned int)id);
            if (id == 0x03C0021C)
            {
              return true;
            }
          }
          return false;
        }

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] M5Paper");

          auto p = new Panel_IT8951();
          result->panel = p;
          {
            auto cfg = p->config();
            cfg.panel_height = 540;
            cfg.panel_width  = 960;
            cfg.pin_busy = GPIO_NUM_27;
            cfg.offset_rotation = 3;
            p->config(cfg);
          }
          {
            auto t = new lgfx::Touch_GT911();
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
            cfg.x_max = 539;
            cfg.y_max = 959;
            cfg.offset_rotation = 1;
            cfg.bus_shared = false;
            t->config(cfg);
            if (!t->init())
            {
              cfg.i2c_addr = 0x5D; // addr change (0x14 or 0x5D)
              t->config(cfg);
            }
            p->touch(t);
          }
        }
      };

      // https://github.com/Makerfabs/Project_Touch-Camera-ILI9341
      struct _detector_Makerfabs_TouchCamera_9341_t : public _detector_spi_t
      {
        constexpr _detector_Makerfabs_TouchCamera_9341_t(void)
        : _detector_spi_t
        { board_t::board_Makerfabs_TouchCamera
        , 0x04, 0xFF, 0x00 // ILI9341
        , 40000000, 16000000
        , GPIO_NUM_13     // MOSI
        , GPIO_NUM_12     // MISO
        , GPIO_NUM_14     // SCLK
        , GPIO_NUM_33     // DC
        , GPIO_NUM_15     // CS
        , (gpio_num_t)-1  // RST
        , GPIO_NUM_4      // TF CARD CS
        , 0               // SPI MODE
        , false           // SPI 3wire
        , VSPI_HOST       // SPI HOST
        } {}

        bool detect(_detector_result_t* result, bool use_reset) const override
        {
          // ESP32_2432S028 とSPIピンやパネル種類に共通点が多く誤判定しやすい。
          // そこで ESP32_2432S028 のD/CピンであるGPIO2をHIGHにすることで誤判定を防止する
          _pin_backup_t backup = { GPIO_NUM_2 };
          _pin_level(GPIO_NUM_2, true);
          bool res = _detector_spi_t::detect(result, use_reset);
          backup.restore();
          return res;
        }

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] Makerfabs_TouchCamera (ILI9341)");

          auto p = new Panel_ILI9341();
          result->panel = p;

          {
            auto t = new lgfx::Touch_STMPE610();
            auto cfg = t->config();
            cfg.bus_shared = true;
            cfg.spi_host = spi_host;
            cfg.pin_int  = GPIO_NUM_0;
            cfg.pin_cs   = GPIO_NUM_2;
            cfg.pin_mosi = pin_mosi;
            cfg.pin_miso = pin_miso;
            cfg.pin_sclk = pin_sclk;
            cfg.offset_rotation = 2;
            t->config(cfg);
            p->touch(t);
          }
        }
      };

      // https://github.com/Makerfabs/Project_Touch-Screen-Camera
      struct _detector_Makerfabs_TouchCamera_9488_t : public _detector_spi_t
      {
        constexpr _detector_Makerfabs_TouchCamera_9488_t(void)
        : _detector_spi_t
        { board_t::board_Makerfabs_TouchCamera
        , 0x04, 0xFF, 0x54 // ILI9488
        , 40000000, 16000000
        , GPIO_NUM_13     // MOSI
        , GPIO_NUM_12     // MISO
        , GPIO_NUM_14     // SCLK
        , GPIO_NUM_33     // DC
        , GPIO_NUM_15     // CS
        , (gpio_num_t)-1  // RST
        , GPIO_NUM_4      // TF CARD CS
        , 0               // SPI MODE
        , false           // SPI 3wire
        , VSPI_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] Makerfabs_TouchCamera (ILI9488)");

          auto p = new Panel_ILI9488();
          result->panel = p;

          if (lgfx::i2c::init(I2C_NUM_1, GPIO_NUM_26, GPIO_NUM_27).has_value())
          {
            ITouch* t = nullptr;
            ITouch::config_t cfg;
            if (!lgfx::i2c::readRegister8(I2C_NUM_1, 0x38, 0, 400000).has_value()) // 0x48:NS2009
            {
              t = new lgfx::Touch_NS2009();
              cfg = t->config();
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
              cfg.x_max = 319;
              cfg.y_max = 479;
            // cfg.pin_int  = GPIO_NUM_0;   // INT pin number (board ver1.1 doesn't work)
            }
            cfg.bus_shared = false;
            cfg.pin_sda  = GPIO_NUM_26;   // I2C SDA pin number
            cfg.pin_scl  = GPIO_NUM_27;   // I2C SCL pin number
            cfg.i2c_port = I2C_NUM_1;// I2C port number
            t->config(cfg);
            p->touch(t);
          }
        }
      };

      struct _detector_board_LoLinD32_7735_t : public _detector_spi_t
      {
        constexpr _detector_board_LoLinD32_7735_t(void)
        : _detector_spi_t
        { board_t::board_LoLinD32
        , 0x04, 0xFF, 0x7C // ST7735
        , 27000000, 16000000
        , GPIO_NUM_23     // MOSI
        , GPIO_NUM_19     // MISO
        , GPIO_NUM_18     // SCLK
        , GPIO_NUM_27     // DC
        , GPIO_NUM_14     // CS
        , GPIO_NUM_33     // RST
        , GPIO_NUM_4      // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , VSPI_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] LoLinD32 (ST7735)");

          auto p = new Panel_ST7735S();
          result->panel = p;
          {
            auto cfg = p->config();
            cfg.memory_width  = 132;
            cfg.memory_height = 132;
            cfg.panel_width  = 128;
            cfg.panel_height = 128;
            cfg.offset_x = 2;
            cfg.offset_y = 1;
            p->config(cfg);
            p->light(_create_pwm_backlight(GPIO_NUM_32, 7, 44100));
          }
        }
      };

      struct _detector_board_LoLinD32_9341_t : public _detector_spi_t
      {
        constexpr _detector_board_LoLinD32_9341_t(void)
        : _detector_spi_t
        { board_t::board_LoLinD32
        , 0x04, 0xFF, 0x00 // ILI9341
        , 40000000, 16000000
        , GPIO_NUM_23     // MOSI
        , GPIO_NUM_19     // MISO
        , GPIO_NUM_18     // SCLK
        , GPIO_NUM_27     // DC
        , GPIO_NUM_14     // CS
        , GPIO_NUM_33     // RST
        , GPIO_NUM_4      // TF CARD CS
        , 0               // SPI MODE
        , false           // SPI 3wire
        , VSPI_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] LoLinD32 (ILI9341)");

          auto p = new Panel_ILI9341();
          result->panel = p;
          p->light(_create_pwm_backlight(GPIO_NUM_32, 7, 44100));

          {
            auto t = new lgfx::Touch_XPT2046();
            auto cfg = t->config();
            cfg.bus_shared = true;
            cfg.spi_host = spi_host;
            cfg.pin_cs   = GPIO_NUM_12;
            cfg.pin_mosi = pin_mosi;
            cfg.pin_miso = pin_miso;
            cfg.pin_sclk = pin_sclk;
            cfg.offset_rotation = 2;
            t->config(cfg);
            p->touch(t);
          }
        }
      };

      struct _detector_ESP_WROVER_KIT_9341_t : public _detector_spi_t
      {
        constexpr _detector_ESP_WROVER_KIT_9341_t(void)
        : _detector_spi_t
        { board_t::board_ESP_WROVER_KIT
        , 0x04, 0xFF, 0x00 // ILI9341
        , 40000000, 20000000
        , GPIO_NUM_23     // MOSI
        , GPIO_NUM_25     // MISO
        , GPIO_NUM_19     // SCLK
        , GPIO_NUM_21     // DC
        , GPIO_NUM_22     // CS
        , GPIO_NUM_18     // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , false           // SPI 3wire
        , VSPI_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] ESP_WROVER_KIT (ILI9341)");

          auto p = new Panel_ILI9341();
          result->panel = p;
          p->light(_create_pwm_backlight(GPIO_NUM_5, 7, 12000, true));
        }
      };

      struct _detector_ESP_WROVER_KIT_7789_t : public _detector_spi_t
      {
        constexpr _detector_ESP_WROVER_KIT_7789_t(void)
        : _detector_spi_t
        { board_t::board_ESP_WROVER_KIT
        , 0x04, 0xFF, 0x85 // ST7789
        , 80000000, 16000000
        , GPIO_NUM_23     // MOSI
        , GPIO_NUM_25     // MISO
        , GPIO_NUM_19     // SCLK
        , GPIO_NUM_21     // DC
        , GPIO_NUM_22     // CS
        , GPIO_NUM_18     // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , false           // SPI 3wire
        , VSPI_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] ESP_WROVER_KIT (ST7789)");

          auto p = new Panel_ST7789();
          result->panel = p;
          {
            auto cfg = p->config();
            cfg.offset_rotation = 2;
            p->config(cfg);
            p->light(_create_pwm_backlight(GPIO_NUM_5, 7, 12000, true));
          }
        }
      };

      struct _detector_Sunton_ESP32_2432S028_t : public _detector_spi_t
      {
        constexpr _detector_Sunton_ESP32_2432S028_t
        ( board_t board_
        , uint16_t id_cmd_
        , uint32_t id_mask_
        , uint32_t id_value_
        , uint32_t freq_write_
        , uint32_t freq_read_
        , gpio_num_t pin_mosi_
        , gpio_num_t pin_miso_
        , gpio_num_t pin_sclk_
        , gpio_num_t pin_dc_
        , gpio_num_t pin_cs_
        , gpio_num_t pin_rst_
        , gpio_num_t pin_tfcard_cs_
        , int8_t spi_mode_
        , bool spi_3wire_
        , spi_host_device_t spi_host_
        ) :
        _detector_spi_t { board_, id_cmd_, id_mask_, id_value_, freq_write_, freq_read_,
                          pin_mosi_, pin_miso_, pin_sclk_, pin_dc_, pin_cs_, pin_rst_, pin_tfcard_cs_,
                          spi_mode_, spi_3wire_, spi_host_ }
        {}

        bool detect(_detector_result_t* result, bool use_reset) const override
        {
          // Makerfabs_TouchCamera とSPIピンやパネル種類に共通点が多く誤判定しやすい。
          // そこで Makerfabs_TouchCamera のD/CピンであるGPIO33をHIGHにすることで誤判定を防止する
          _pin_backup_t backup = { GPIO_NUM_33 };
          _pin_level(GPIO_NUM_33, true);
          bool res = _detector_spi_t::detect(result, use_reset);
          backup.restore();
          return res;
        }

        void setup(_detector_result_t* result) const override
        {
          auto p = result->panel;
          {
            auto cfg = p->config();
            cfg.bus_shared = false;
            p->config(cfg);
            p->light(_create_pwm_backlight(GPIO_NUM_21, 7));
          }

          {
            auto t = new lgfx::Touch_XPT2046();
            auto cfg = t->config();
            cfg.x_min      =  300;
            cfg.x_max      = 3900;
            cfg.y_min      = 3700;
            cfg.y_max      =  200;
            cfg.pin_int    = -1;
            cfg.bus_shared = false;
            cfg.spi_host = -1; // -1:use software SPI for XPT2046
            cfg.pin_sclk = GPIO_NUM_25;
            cfg.pin_mosi = GPIO_NUM_32;
            cfg.pin_miso = GPIO_NUM_39;
            cfg.pin_cs   = GPIO_NUM_33;
            t->config(cfg);
            p->touch(t);
          }
        }
      };

      struct _detector_Sunton_2432S028_9341_t : public _detector_Sunton_ESP32_2432S028_t
      {
        constexpr _detector_Sunton_2432S028_9341_t(void)
        : _detector_Sunton_ESP32_2432S028_t
        { board_t::board_Sunton_ESP32_2432S028
        , 0x04, 0xFF, 0x00 // ILI9341
        , 40000000, 16000000
        , GPIO_NUM_13     // MOSI
        , GPIO_NUM_12     // MISO
        , GPIO_NUM_14     // SCLK
        , GPIO_NUM_2      // DC
        , GPIO_NUM_15     // CS
        , (gpio_num_t)-1  // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , false           // SPI 3wire
        , HSPI_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] Sunton_2432S028 (ILI9341)");

          auto p = new Panel_ILI9341();
          result->panel = p;
          {
            auto cfg = p->config();
            cfg.offset_rotation = 2;
            p->config(cfg);
          }
          _detector_Sunton_ESP32_2432S028_t::setup(result);
        }
      };

      struct _detector_Sunton_2432S028_7789_t : public _detector_Sunton_ESP32_2432S028_t
      {
        constexpr _detector_Sunton_2432S028_7789_t(void)
        : _detector_Sunton_ESP32_2432S028_t
        { board_t::board_Sunton_ESP32_2432S028
        , 0x04, 0xFF, 0x85 // ST7789
        , 80000000, 16000000
        , GPIO_NUM_13     // MOSI
        , GPIO_NUM_12     // MISO
        , GPIO_NUM_14     // SCLK
        , GPIO_NUM_2      // DC
        , GPIO_NUM_15     // CS
        , (gpio_num_t)-1  // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , false           // SPI 3wire
        , HSPI_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] Sunton_2432S028 (ST7789)");

          result->panel = new Panel_ST7789();
          _detector_Sunton_ESP32_2432S028_t::setup(result);
          auto t = result->panel->getTouch();
          {
            auto cfg = t->config();
            cfg.offset_rotation = 2;
            t->config(cfg);
          }
        }
      };

      struct _detector_Guition_JC2432W328C_t : public _detector_Sunton_ESP32_2432S028_t
      {
        constexpr _detector_Guition_JC2432W328C_t(void)
        : _detector_Sunton_ESP32_2432S028_t
        { board_t::board_Guition_ESP32_2432W328
        , 0x04, 0xFFFF00, 0xb38100
        , 55000000, 20000000
        , GPIO_NUM_13     // MOSI
        , GPIO_NUM_12     // MISO
        , GPIO_NUM_14     // SCLK
        , GPIO_NUM_2      // DC
        , GPIO_NUM_15     // CS
        , (gpio_num_t)-1  // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , false           // SPI 3wire
        , HSPI_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] Guition_JC2432W328C");

          result->panel = new Panel_ILI9341_2();

          auto p = result->panel;
          {
            auto cfg = p->config();
            cfg.offset_x         =     0;
            cfg.offset_y         =     0;
            cfg.offset_rotation  =     6;
            cfg.dummy_read_pixel =     8;
            cfg.dummy_read_bits  =     1;
            cfg.readable         = true;
            cfg.invert           = true;
            cfg.rgb_order        = false;
            cfg.dlen_16bit       = false;
            cfg.bus_shared       = false;
            p->config(cfg);

            p->light(_create_pwm_backlight(GPIO_NUM_27, 7));
          }
          {
              auto t = new lgfx::Touch_CST816S();

              auto cfg = t->config();
              cfg.i2c_port = I2C_NUM_0;
              cfg.pin_sda = GPIO_NUM_33;
              cfg.pin_scl = GPIO_NUM_32;
              cfg.pin_rst = GPIO_NUM_25;
              cfg.pin_int = -1;
              cfg.offset_rotation = 6;
              cfg.freq = 400000;
              cfg.x_max = 240;
              cfg.y_max = 320;
              t->config(cfg);
              p->touch(t);
          }
        }
      };

      struct _detector_WT32_SC01_t : public _detector_spi_t
      {
        constexpr _detector_WT32_SC01_t(void)
        : _detector_spi_t
        { board_t::board_WT32_SC01
        , 0,0,0       // (write only)
        , 40000000, 16000000
        , GPIO_NUM_13     // MOSI
        , (gpio_num_t)-1  // MISO
        , GPIO_NUM_14     // SCLK
        , GPIO_NUM_21     // DC
        , GPIO_NUM_15     // CS
        , GPIO_NUM_22     // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , true            // SPI 3wire
        , HSPI_HOST       // SPI HOST
        } {}

        bool judgement(IBus* bus, int pin_cs_) const override
        {
          // タッチパネルの有無をチェックする;
          _pin_backup_t backup[] = { GPIO_NUM_18, GPIO_NUM_19 };
          lgfx::i2c::init(I2C_NUM_1, GPIO_NUM_18, GPIO_NUM_19);
          // I2C通信でタッチパネルコントローラが存在するかチェックする
          if (0x11 == lgfx::i2c::readRegister8(I2C_NUM_1, 0x38, 0xA8, 400000))
          { /// FocalTech's Panel ID reg=0xA8  value=0x11
            return true;
          }
          lgfx::i2c::release(I2C_NUM_1);
          for (auto &b : backup)
          {
            b.restore();
          }
          return false;
        }

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] WT32_SC01");

          auto p = new Panel_ST7796();
          result->panel = p;
          {
            auto cfg = p->config();
            // cfg.pin_cs  = GPIO_NUM_15;
            // cfg.pin_rst = GPIO_NUM_22;
            cfg.readable  = false;
            cfg.bus_shared = false;
            p->config(cfg);
            p->light(_create_pwm_backlight(GPIO_NUM_23, 7));
          }

          {
            auto t = new lgfx::Touch_FT5x06();
            auto cfg = t->config();
            cfg.pin_int  = GPIO_NUM_39;   // INT pin number
            cfg.pin_sda  = GPIO_NUM_18;   // I2C SDA pin number
            cfg.pin_scl  = GPIO_NUM_19;   // I2C SCL pin number
            cfg.i2c_port = I2C_NUM_1;// I2C port number
            cfg.x_max = 319;
            cfg.y_max = 479;
            cfg.bus_shared = false;
            t->config(cfg);
            p->touch(t);
          }
        }
      };

      struct _detector_DDUINO32_XS_t : public _detector_spi_t
      {
        constexpr _detector_DDUINO32_XS_t(void)
        : _detector_spi_t
        { board_t::board_DDUINO32_XS
        , 0,0,0       // ST7789 (write only)
        , 80000000, 16000000
        , GPIO_NUM_26     // MOSI
        , (gpio_num_t)-1  // MISO
        , GPIO_NUM_27     // SCLK
        , GPIO_NUM_23     // DC
        , (gpio_num_t)-1  // CS
        , GPIO_NUM_32     // RST
        , (gpio_num_t)-1  // TF CARD CS
        , 0               // SPI MODE
        , false           // SPI 3wire
        , HSPI_HOST       // SPI HOST
        } {}

        void setup(_detector_result_t* result) const override
        {
          ESP_LOGI(LIBRARY_NAME, "[Autodetect] DDUINO32_XS");

          auto p = new Panel_ST7789();
          result->panel = p;
          {
            auto cfg = p->config();
            cfg.panel_height = 240;
            cfg.invert = true;
            cfg.readable  = false;
            cfg.bus_shared = false;
            p->config(cfg);
            p->light(_create_pwm_backlight(GPIO_NUM_22, 7));
          }
        }
      };

#endif

      const _detector_t* const* detectors = nullptr;

#if defined (CONFIG_IDF_TARGET_ESP32S3)

      static constexpr const _detector_M5AtomS3_t                                detector_M5AtomS3;
      static constexpr const _detector_M5StackCoreS3_t                           detector_M5StackCoreS3;
      static constexpr const _detector_ESP32_S3_BOX_t                            detector_ESP32_S3_BOX;
      static constexpr const _detector_ESP32_S3_BOX_Lite_t                       detector_ESP32_S3_BOX_Lite;
      static constexpr const _detector_ESP32_S3_BOX_V3_t                         detector_ESP32_S3_BOX_V3;
      static constexpr const _detector_Makerfabs_ESP32_S3_TFT_Touch_SPI_t        detector_Makerfabs_ESP32_S3_TFT_Touch_SPI;
      static constexpr const _detector_Makerfabs_ESP32_S3_TFT_Touch_Parallel16_t detector_Makerfabs_ESP32_S3_TFT_Touch_Parallel16;
      static constexpr const _detector_wywy_ESP32S3_HMI_DevKit_t                 detector_wywy_ESP32S3_HMI_DevKit;
      static constexpr const _detector_Feather_ESP32_S3_TFT_t                    detector_Feather_ESP32_S3_TFT;
      static constexpr const _detector_board_LoLinS3Pro_7735_t                   detector_board_LoLinS3Pro_7735;
      static constexpr const _detector_board_LoLinS3Pro_9341_t                   detector_board_LoLinS3Pro_9341;

      static constexpr const _detector_t* detector_list[] =
      {

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5ATOMS3 ) || defined ( LGFX_M5ATOM_S3LCD )
        &detector_M5AtomS3,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK_CORES3 )
        &detector_M5StackCoreS3,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_ESP32_S3_BOX )
        &detector_ESP32_S3_BOX,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_ESP32_S3_BOX_LITE )
        &detector_ESP32_S3_BOX_Lite,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_ESP32_S3_BOX_V3 )
        &detector_ESP32_S3_BOX_V3,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_MAKERFABS_TFT_TOUCH_SPI )
        &detector_Makerfabs_ESP32_S3_TFT_Touch_SPI,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_MAKERFABS_TFT_TOUCH_PARALLEL16 )
        &detector_Makerfabs_ESP32_S3_TFT_Touch_Parallel16,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_WYWY_ESP32S3_HMI_DEVKIT )
        &detector_wywy_ESP32S3_HMI_DevKit,
#endif

// Feather S3 TFT screen is write-only, no LGFX_AUTODETECT
#if defined ( LGFX_FEATHER_ESP32_S3_TFT )
        &detector_Feather_ESP32_S3_TFT,
#endif

#if defined( LGFX_LOLIN_S3_PRO )
        &detector_board_LoLinS3Pro_7735,
        &detector_board_LoLinS3Pro_9341,
#endif

        nullptr // terminator
      };

      detectors = detector_list;

#elif defined (CONFIG_IDF_TARGET_ESP32S2)

      static constexpr const _detector_ESP32_S2_Kaluga_1_t                        detector_ESP32_S2_Kaluga_1;
      static constexpr const _detector_Makerfabs_ESP32_S2_TFT_Touch_Parallel16_t  detector_Makerfabs_ESP32_S2_TFT_Touch_Parallel16;
      static constexpr const _detector_Feather_ESP32_S2_TFT_t                     detector_Feather_ESP32_S2_TFT;
      static constexpr const _detector_FunHouse_t                                 detector_FunHouse;

      static constexpr const _detector_t* detector_list[] =
      {

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_ESP32_S2_KALUGA_1 )
        &detector_ESP32_S2_Kaluga_1,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_MAKERFABS_TFT_TOUCH_PARALLEL16 )
        &detector_Makerfabs_ESP32_S2_TFT_Touch_Parallel16,
#endif

// Feather S2 TFT screen is write-only, no LGFX_AUTODETECT
#if defined ( LGFX_FEATHER_ESP32_S2_TFT )
        &detector_Feather_ESP32_S2_TFT,
#endif
// FunHouse screen is write-only, no LGFX_AUTODETECT
#if defined ( LGFX_FUNHOUSE )
        &detector_FunHouse,
#endif

        nullptr // terminator
      };

      detectors = detector_list;

#elif defined (CONFIG_IDF_TARGET_ESP32C3)

      static constexpr const _detector_Makerfabs_ESP32_C3_TFT_Touch_SPI_t        detector_Makerfabs_ESP32_C3_TFT_Touch_SPI;

      static constexpr const _detector_t* detector_list[] =
      {

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_MAKERFABS_TFT_TOUCH_SPI )
        &detector_Makerfabs_ESP32_C3_TFT_Touch_SPI,
#endif

        nullptr // terminator
      };

      detectors = detector_list;

#elif defined (CONFIG_IDF_TARGET_ESP32) || !defined (CONFIG_IDF_TARGET)

      static constexpr const _detector_M5StickC_t              detector_M5StickC;
      static constexpr const _detector_M5StickCPlus_t          detector_M5StickCPlus;
      static constexpr const _detector_M5StickCPlus2_t         detector_M5StickCPlus2;
      static constexpr const _detector_M5StackCoreInk_t        detector_M5StackCoreInk;
      static constexpr const _detector_TTGO_TWristband_t       detector_TTGO_TWristband;
      static constexpr const _detector_TTGO_TS_t               detector_TTGO_TS;

      static constexpr const _detector_M5Station_t             detector_M5Station;
      static constexpr const _detector_M5StackCore2_t          detector_M5StackCore2; // and M5Tough
      static constexpr const _detector_TTGO_TWatch_t           detector_TTGO_TWatch;
      static constexpr const _detector_TTGO_TDisplay_t         detector_TTGO_TDisplay;
      static constexpr const _detector_TTGO_T4_Display_t       detector_TTGO_T4_Display;
      static constexpr const _detector_WiFiBoy_Mini_t          detector_WiFiBoy_Mini;
      static constexpr const _detector_WiFiBoy_Pro_t           detector_WiFiBoy_Pro;
      static constexpr const _detector_Makerfabs_MakePython_t  detector_Makerfabs_MakePython;
      static constexpr const _detector_M5Stack_t               detector_M5Stack;
      static constexpr const _detector_M5Paper_t               detector_M5Paper;

      static constexpr const _detector_Makerfabs_TouchCamera_9341_t detector_Makerfabs_TouchCamera_9341;
      static constexpr const _detector_Makerfabs_TouchCamera_9488_t detector_Makerfabs_TouchCamera_9488;
      static constexpr const _detector_board_LoLinD32_7735_t   detector_board_LoLinD32_7735;
      static constexpr const _detector_board_LoLinD32_9341_t   detector_board_LoLinD32_9341;
      static constexpr const _detector_ESP_WROVER_KIT_7789_t   detector_ESP_WROVER_KIT_7789;
      static constexpr const _detector_ESP_WROVER_KIT_9341_t   detector_ESP_WROVER_KIT_9341;
      static constexpr const _detector_Guition_JC2432W328C_t   detector_Guition_JC2432W328C;
      static constexpr const _detector_Sunton_2432S028_9341_t  detector_Sunton_2432S028_9341;
      static constexpr const _detector_Sunton_2432S028_7789_t  detector_Sunton_2432S028_7789;
      static constexpr const _detector_ODROID_GO_t             detector_ODROID_GO;
      static constexpr const _detector_WT32_SC01_t             detector_WT32_SC01;

      static constexpr const _detector_DDUINO32_XS_t           detector_DDUINO32_X;

      static constexpr const _detector_t* detector_list_PICO_V3[] =
      {

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STICK_C ) || defined ( LGFX_M5STICKC )
        &detector_M5StickCPlus2,
#endif
        nullptr // terminator
      };

      static constexpr const _detector_t* detector_list_PICO_D4[] =
      {

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK_COREINK )
        &detector_M5StackCoreInk,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STICK_C ) || defined ( LGFX_M5STICKC )
        &detector_M5StickCPlus,
        &detector_M5StickC,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_TTGO_TWRISTBAND )
        &detector_TTGO_TWristband,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_TTGO_TS )
        &detector_TTGO_TS,
#endif
        nullptr // terminator
      };

      static constexpr const _detector_t* detector_list_D0WDQ5[] =
      {
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_MAKERFABS_TOUCHCAMERA )
        &detector_Makerfabs_TouchCamera_9488,
        &detector_Makerfabs_TouchCamera_9341,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_LOLIN_D32_PRO )
        &detector_board_LoLinD32_7735,
        &detector_board_LoLinD32_9341,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_ESP_WROVER_KIT )
        &detector_ESP_WROVER_KIT_7789,
        &detector_ESP_WROVER_KIT_9341,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_ESP32_2432W328 ) || defined ( LGFX_GUITION_ESP32_2432W328 )
        &detector_Guition_JC2432W328C,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_ESP32_2432S028 ) || defined ( LGFX_SUNTON_ESP32_2432S028 )
        &detector_Sunton_2432S028_9341,
        &detector_Sunton_2432S028_7789,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_ODROID_GO )
        &detector_ODROID_GO,
#endif

  // WT32_SC01 は読出しが出来ない製品だがタッチパネルの有無で判定する。;
  // LGFX_AUTO_DETECTでは機能しないようにしておく。;
#if defined ( LGFX_WT32_SC01 )
        &detector_WT32_SC01,
#endif
  // DSTIKE D-Duino32XS については読出しが出来ないため無条件設定となる。;
  // そのためLGFX_AUTO_DETECTでは機能しないようにしておく。;
#if defined ( LGFX_DDUINO32_XS )
        &detector_DDUINO32_X,
#endif

        nullptr // terminator
      };

      static constexpr const _detector_t* detector_list_D0WDQ6[] =
      {
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STATION )
        &detector_M5Station,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK_CORE2 ) || defined ( LGFX_M5STACKCORE2 ) || defined ( LGFX_M5TOUGH )
        &detector_M5StackCore2,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_TTGO_TWATCH )
        &detector_TTGO_TWatch,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_TTGO_TDISPLAY )
        &detector_TTGO_TDisplay,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_TTGO_T4_DISPLAY )
        &detector_TTGO_T4_Display,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_WIFIBOY_MINI )
        &detector_WiFiBoy_Mini,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_WIFIBOY_PRO )
        &detector_WiFiBoy_Pro,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_MAKERFABS_MAKEPYTHON )
        &detector_Makerfabs_MakePython,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK )
        &detector_M5Stack,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5PAPER )
        &detector_M5Paper,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_ODROID_GO )
        &detector_ODROID_GO,
#endif
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_ESP32_2432S028 ) || defined ( LGFX_SUNTON_ESP32_2432S028 )
        &detector_Sunton_2432S028_9341,
        &detector_Sunton_2432S028_7789,
#endif

        nullptr // terminator
      };


      uint32_t pkg_ver = lgfx::get_pkg_ver();
      ESP_LOGV("LGFX", "pkg: %lu", (unsigned long)pkg_ver);

      switch (pkg_ver)
      {
      case EFUSE_RD_CHIP_VER_PKG_ESP32D0WDQ5:
        detectors = detector_list_D0WDQ5;
        break;

      case EFUSE_RD_CHIP_VER_PKG_ESP32D0WDQ6:
        detectors = detector_list_D0WDQ6;
        break;

      default:
      case EFUSE_RD_CHIP_VER_PKG_ESP32PICOD4:
        detectors = detector_list_PICO_D4;
        break;

      case 6:
        detectors = detector_list_PICO_V3;
        break;
      }

#endif

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
      if (_bus_last)
      {
        _bus_last->release();
        delete _bus_last;
        _bus_last = nullptr;
      }

      _detector_result_t result;
      while (*detectors != nullptr)
      {
        auto detector = *detectors++;

        result.board = board;
        if (detector->detect(&result, use_reset))
        {
          _bus_last = result.bus;
          auto p = result.panel;
          _touch_last = p->touch();
          _light_last = p->light();
          _panel_last = p;
          setPanel(p);
          return result.board;
        }
      }

      return board_t::board_unknown;
    }
  };

//----------------------------------------------------------------------------
 }
}

using LGFX = lgfx::LGFX;
