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


#if defined ( ARDUINO_ESP32_S3_BOX )
  #define LGFX_ESP32_S3_BOX
#endif


namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  static constexpr char LIBRARY_NAME[] = "LovyanGFX";

  namespace m5stack
  {
    static constexpr int32_t i2c_freq = 400000;
    static constexpr int_fast16_t aw9523_i2c_addr = 0x58;  // AW9523B
    static constexpr int_fast16_t axp_i2c_addr = 0x34;     // AXP2101
    static constexpr int_fast16_t i2c_port = I2C_NUM_1;
    static constexpr int_fast16_t i2c_sda = GPIO_NUM_34;
    static constexpr int_fast16_t i2c_scl = GPIO_NUM_33;
  }

  struct Panel_M5StackCoreS3 : public lgfx::Panel_ILI9342
  {
    Panel_M5StackCoreS3(void)
    {
      _cfg.pin_cs = GPIO_NUM_36;
      _cfg.invert = true;
      _cfg.offset_rotation = 3;

      _rotation = 1; // default rotation
    }

    void reset(void) override
    {
      using namespace m5stack;
      // AXP192 reg 0x96 = GPIO3&4 control
      lgfx::i2c::writeRegister8(i2c_port, aw9523_i2c_addr, 0x03, 0, ~(1<<5), i2c_freq);  // LCD_RST
      lgfx::delay(4);
      lgfx::i2c::writeRegister8(i2c_port, aw9523_i2c_addr, 0x03, (1<<5), ~0, i2c_freq);  // LCD_RST
    }
  };

  struct Touch_M5StackCoreS3 : public lgfx::Touch_FT5x06
  {
    Touch_M5StackCoreS3(void)
    {
      using namespace m5stack;
      _cfg.pin_int  = GPIO_NUM_35;
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
#if defined ( ARDUINO_ESP32_S3_BOX )

        nvs_board = board_t::board_ESP32_S3_BOX;

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
      bus_cfg.spi_host = SPI3_HOST;
      bus_cfg.spi_mode = 0;
      bus_cfg.dma_channel = SPI_DMA_CH_AUTO;
      bus_cfg.use_lock = true;

      uint32_t id;
      (void)id;  // suppress warning

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5ATOM_S3LCD )

      if (board == 0 || board == board_t::board_M5AtomS3LCD)
      {
        bus_cfg.pin_mosi = GPIO_NUM_21;
        bus_cfg.pin_miso = GPIO_NUM_13;
        bus_cfg.pin_sclk = GPIO_NUM_16;
        bus_cfg.pin_dc   = GPIO_NUM_33;
        bus_cfg.spi_mode = 0;
        bus_cfg.spi_3wire = true;
        _bus_spi.config(bus_cfg);
        _bus_spi.init();
        _pin_reset(GPIO_NUM_34, use_reset); // LCD RST
        id = _read_panel_id(&_bus_spi, GPIO_NUM_15);
        if ((id & 0xFFFFFF) == 0x079100)
        {  //  check panel (GC9107)
          board = board_t::board_M5AtomS3LCD;
          ESP_LOGW(LIBRARY_NAME, "[Autodetect] board_M5AtomS3LCD");
          bus_cfg.freq_write = 40000000;
          bus_cfg.freq_read  = 16000000;
          _bus_spi.config(bus_cfg);
          auto p = new Panel_GC9107();
          p->bus(&_bus_spi);
          {
            auto cfg = p->config();
            cfg.pin_cs  = GPIO_NUM_15;
            cfg.pin_rst = GPIO_NUM_34;
            cfg.panel_width = 128;
            cfg.panel_height = 128;
            cfg.offset_y = 32;
            p->config(cfg);
          }
          _panel_last = p;
          _set_pwm_backlight(GPIO_NUM_18, 7, 240); /// AtomS3LCDのバックライトはPWM周期が速いと点灯しない;

          goto init_clear;
        }
        lgfx::pinMode(GPIO_NUM_48, lgfx::pin_mode_t::input); // LCD RST
        _bus_spi.release();
      }
#endif

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK_CORES3 )

      if (board == 0 || board == board_t::board_M5StackCoreS3)
      {
        using namespace m5stack;
        lgfx::i2c::init(i2c_port, i2c_sda, i2c_scl);

// ESP_LOGI("DEBUG","AW 0x10 :%02x ", lgfx::i2c::readRegister8(i2c_port, aw9523_i2c_addr, 0x10, 400000).value());
// ESP_LOGI("DEBUG","AXP0x03 :%02x ", lgfx::i2c::readRegister8(i2c_port, axp_i2c_addr, 0x03, 400000).value());

        auto chk_axp = lgfx::i2c::readRegister8(i2c_port, axp_i2c_addr, 0x03, i2c_freq);
        if (chk_axp.has_value() && chk_axp.value() == 0x4A)
        {
          auto chk_aw  = lgfx::i2c::readRegister8(i2c_port, aw9523_i2c_addr, 0x10, i2c_freq);
          if (chk_aw .has_value() && chk_aw .value() == 0x23)
          {
            lgfx::i2c::writeRegister8(i2c_port, aw9523_i2c_addr, 0x05, 0b01011000);
            lgfx::i2c::bitOn(i2c_port, aw9523_i2c_addr, 0x03, 0b10100000);

            bus_cfg.pin_mosi = GPIO_NUM_11;
            bus_cfg.pin_miso = GPIO_NUM_12;
            bus_cfg.pin_sclk = GPIO_NUM_13;
            bus_cfg.pin_dc   = GPIO_NUM_3;
            bus_cfg.spi_mode = 0;
            bus_cfg.spi_3wire = true;
            _bus_spi.config(bus_cfg);
            _bus_spi.init();

            _set_sd_spimode(bus_cfg.spi_host, GPIO_NUM_37);

            id = _read_panel_id(&_bus_spi, GPIO_NUM_36);
            if ((id & 0xFF) == 0xE3)
            {  //  check panel (ILI9342)
              board = board_t::board_M5StackCoreS3;
              ESP_LOGW(LIBRARY_NAME, "[Autodetect] board_M5StackCoreS3");
              bus_cfg.freq_write = 40000000;
              bus_cfg.freq_read  = 16000000;
              _bus_spi.config(bus_cfg);
              auto p = new Panel_M5StackCoreS3();
              p->bus(&_bus_spi);
              _panel_last = p;

              _set_backlight(new Light_M5StackCoreS3());

              {
                lgfx::i2c::bitOn(i2c_port, aw9523_i2c_addr, 0x03, 0b10100000);

                auto t = new lgfx::Touch_M5StackCoreS3();
                _touch_last = t;
                _panel_last->touch(t);
              }

              goto init_clear;
            }
            _bus_spi.release();
          }
        }
        lgfx::i2c::release(i2c_port);
      }
#endif

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_ESP32_S3_BOX )

      if (board == 0 || board == board_t::board_ESP32_S3_BOX)
      {
        bus_cfg.pin_mosi = GPIO_NUM_6;
        bus_cfg.pin_miso = GPIO_NUM_0;
        bus_cfg.pin_sclk = GPIO_NUM_7;
        bus_cfg.pin_dc   = GPIO_NUM_4;
        bus_cfg.spi_mode = 0;
        bus_cfg.spi_3wire = true;
        _bus_spi.config(bus_cfg);
        _bus_spi.init();
        _pin_reset(GPIO_NUM_48, use_reset); // LCD RST
        id = _read_panel_id(&_bus_spi, GPIO_NUM_5);
        if ((id & 0xFF) == 0xE3)
        {  //  check panel (ILI9342)
          board = board_t::board_ESP32_S3_BOX;
          ESP_LOGW(LIBRARY_NAME, "[Autodetect] board_ESP32_S3_BOX");
          bus_cfg.freq_write = 40000000;
          bus_cfg.freq_read  = 16000000;
          _bus_spi.config(bus_cfg);
          auto p = new Panel_ILI9342();
          p->bus(&_bus_spi);
          {
            auto cfg = p->config();
            cfg.pin_cs  = GPIO_NUM_5;
            cfg.pin_rst = GPIO_NUM_48;
            cfg.offset_rotation = 1;
            p->config(cfg);
            p->setRotation(1);
          }
          _panel_last = p;
          _set_pwm_backlight(GPIO_NUM_45, 0, 12000);

          {
            auto t = new lgfx::Touch_TT21xxx();
            _touch_last = t;
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

          goto init_clear;
        }
        lgfx::pinMode(GPIO_NUM_48, lgfx::pin_mode_t::input); // LCD RST
        _bus_spi.release();
      }
#endif

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
