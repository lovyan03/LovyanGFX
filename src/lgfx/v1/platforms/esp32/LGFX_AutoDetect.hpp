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

#include "../../init.hpp"

#include <nvs.h>
#include <memory>
#include <esp_log.h>
#include <driver/i2c.h>

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  static constexpr char LIBRARY_NAME[] = "LovyanGFX";

  struct Panel_M5Stack : public Panel_ILI9342
  {
    Panel_M5Stack(void)
    {
      _cfg.pin_cs  = 14;
      _cfg.pin_rst = 33;
      _cfg.rotation = 1;
      _cfg.offset_rotation = 3;
    }

    void init(bool use_reset) override
    {
/*
      static constexpr char NVS_KEY[] = "M5Stack_IPS";
      std::uint32_t nvs_handle = 0;
      std::uint8_t readbuf = 0x80u;
      if (0 == nvs_open(LIBRARY_NAME, NVS_READONLY, &nvs_handle)) {
        nvs_get_u8(nvs_handle, NVS_KEY, &readbuf);
        nvs_close(nvs_handle);
      }
      if (readbuf != 0x80u)
      {
        _cfg.invert = readbuf;
      }
      else
      {
        nvs_handle = 0;
//*/
        lgfx::gpio_hi(_cfg.pin_rst);
        lgfx::pinMode(_cfg.pin_rst, pin_mode_t::input_pulldown);
        _cfg.invert = lgfx::gpio_in(_cfg.pin_rst);       // get panel type (IPS or TN)
        lgfx::pinMode(_cfg.pin_rst, pin_mode_t::output);
/*
        if (0 == nvs_open(LIBRARY_NAME, NVS_READWRITE, &nvs_handle)) {
          nvs_set_u8(nvs_handle, NVS_KEY, _cfg.invert);
          nvs_close(nvs_handle);
        }
      }
//*/

      Panel_ILI9342::init(use_reset);
    }
  };

  namespace m5stack
  {
    static constexpr std::int32_t axp_i2c_freq = 400000;
    static constexpr std::uint_fast8_t axp_i2c_addr = 0x34;
    static constexpr std::int_fast16_t axp_i2c_port = I2C_NUM_1;
    static constexpr std::int_fast16_t axp_i2c_sda = 21;
    static constexpr std::int_fast16_t axp_i2c_scl = 22;
  }

  struct Panel_M5StackCore2 : public Panel_ILI9342
  {
    Panel_M5StackCore2(void)
    {
      _cfg.pin_cs = 5;
      _cfg.rotation = 1;
      _cfg.offset_rotation = 3;
      _cfg.invert = true;
    }

    void reset(void) override
    {
      using namespace m5stack;
      // AXP192 reg 0x96 = GPIO3&4 control
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x96, 0, ~0x02); // GPIO4 LOW (LCD RST)
      delay(4);
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x96, 2, ~0); // GPIO4 HIGH (LCD RST)
    }
  };

  struct Light_M5StackCore2 : public ILight
  {
    void init(void) override
    {
      setBrightness(_brightness);
    }

    void setBrightness(std::uint8_t brightness) override
    {
      using namespace m5stack;
      _brightness = brightness;
      if (brightness)
      {
        brightness = (brightness >> 3) + 72;
        lgfx::i2c::bitOn(axp_i2c_port, axp_i2c_addr, 0x12, 0x02); // DC3 enable
      }
      else
      {
        lgfx::i2c::bitOff(axp_i2c_port, axp_i2c_addr, 0x12, 0x02); // DC3 disable
      }
    // AXP192 reg 0x27 = DC3
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x27, brightness, 0x80);
    }
  };

  struct Panel_M5StickC : public Panel_ST7735S
  {
    Panel_M5StickC(void)
    {
      _cfg.invert = true;
      _cfg.pin_cs  = 5;
      _cfg.pin_rst = 18;
      _cfg.panel_width  = 80;
      _cfg.panel_height = 160;
      _cfg.offset_x     = 26;
      _cfg.offset_y     = 1;
      _cfg.offset_rotation = 2;
    }
/*
    void init(bool use_reset) override
    {
      using namespace m5stack;
      lgfx::i2c::init(axp_i2c_port, axp_i2c_sda, axp_i2c_scl, axp_i2c_freq);
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x12, 0x4D, ~0);
      Panel_ST7735S::init(use_reset);
    }
*/
  protected:

    const std::uint8_t* getInitCommands(std::uint8_t listno) const override
    {
      static constexpr std::uint8_t list[] = {
          CMD_GAMMASET, 1, 0x08,  // Gamma set, curve 4
          0xFF,0xFF, // end
      };
      if (listno == 2)  return list;
      return Panel_ST7735S::getInitCommands(listno);
    }
  };

  struct Light_M5StickC : public ILight
  {
    void init(void) override
    {
      using namespace m5stack;
      lgfx::i2c::init(axp_i2c_port, axp_i2c_sda, axp_i2c_scl, axp_i2c_freq);
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x12, 0x4D, ~0);
      setBrightness(_brightness);
    }

    void setBrightness(std::uint8_t brightness) override
    {
      using namespace m5stack;
      _brightness = brightness;
      if (brightness)
      {
        brightness = (((brightness >> 1) + 8) / 13) + 5;
        lgfx::i2c::bitOn(axp_i2c_port, axp_i2c_addr, 0x12, 1 << 2);
      }
      else
      {
        lgfx::i2c::bitOff(axp_i2c_port, axp_i2c_addr, 0x12, 1 << 2);
      }
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x28, brightness << 4, 0x0F);
    }
  };


  struct Panel_M5StickCPlus : public Panel_ST7789
  {
    Panel_M5StickCPlus(void)
    {
      _cfg.invert = true;
      _cfg.pin_cs  = 5;
      _cfg.pin_rst = 18;
      _cfg.panel_width  = 135;
      _cfg.panel_height = 240;
      _cfg.offset_x     = 52;
      _cfg.offset_y     = 40;
    }
  };

  class LGFX : public LGFX_Device
  {
    lgfx::Bus_SPI _bus_spi;
    lgfx::IPanel* _panel_last;
    lgfx::ILight* _light_last;
    lgfx::ITouch* _touch_last;

    void _pin_level(std::int_fast16_t pin, bool level)
    {
      lgfx::pinMode(pin, lgfx::pin_mode_t::output); // M5Stack TF card CS
      if (level) lgfx::gpio_hi(pin);
      else       lgfx::gpio_lo(pin);
    }

    void _pin_reset(std::int_fast16_t pin, bool use_reset)
    {
      lgfx::gpio_hi(pin);
      lgfx::pinMode(pin, lgfx::pin_mode_t::output);
      if (!use_reset) return;
      lgfx::gpio_lo(pin);
      auto time = millis();
      do
      {
        delay(1);
      } while (millis() - time < 2);
      lgfx::gpio_hi(pin);
      time = millis();
      do
      {
        delay(1);
      } while (millis() - time < 10);
    }

    std::uint32_t _read_panel_id(std::int32_t pin_cs, std::uint32_t cmd = 0x04, std::uint8_t dummy_read_bit = 1) // 0x04 = RDDID command
    {
      _bus_spi.beginTransaction();
      _pin_level(pin_cs, false);
      _bus_spi.writeCommand(cmd, 8);
      if (dummy_read_bit) _bus_spi.writeData(0, dummy_read_bit);  // dummy read bit
      _bus_spi.beginRead();
      std::uint32_t res = _bus_spi.readData(32);
      _bus_spi.endTransaction();
      _pin_level(pin_cs, true);

      ESP_LOGW(LIBRARY_NAME, "[Autodetect] read cmd:%02x = %08x", cmd, res);
      return res;
    }

    void _set_backlight(ILight* bl)
    {
      if (_light_last) { delete _light_last; }
      _light_last = bl;
      light(bl);
    }

    void _set_pwm_backlight(std::int16_t pin, std::uint8_t ch, std::uint32_t freq)
    {
      auto bl = new lgfx::Light_PWM();
      auto cfg = bl->config();
      cfg.pin_bl = pin;
      cfg.freq   = freq;
      cfg.pwm_channel = ch;
      bl->config(cfg);
      _set_backlight(bl);
    }

    void init_impl(bool use_reset)
    {
      int retry = 3;
      while (!autodetect(use_reset) && --retry);
      if (retry == 0)
      {
        retry = 3;
        while (!autodetect(true) && --retry);
      }
      LGFX_Device::init_impl(use_reset);
    }

  public:

    bool autodetect(bool use_reset = true)
    {
      auto bus_cfg = _bus_spi.config();
      if (bus_cfg.pin_mosi != -1 && bus_cfg.pin_sclk != -1) return true;

      panel(nullptr);
      light(nullptr);
      touch(nullptr);

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
      bus_cfg.dma_channel = 1;
      bus_cfg.use_lock = true;

      std::uint32_t id;
      (void)id;  // suppress warning

      static constexpr char NVS_KEY[] = "AUTODETECT";
      std::uint32_t nvs_board = 0;
      std::uint32_t nvs_handle = 0;
      if (0 == nvs_open(LIBRARY_NAME, NVS_READONLY, &nvs_handle))
      {
        nvs_get_u32(nvs_handle, NVS_KEY, static_cast<uint32_t*>(&nvs_board));
        nvs_close(nvs_handle);
        ESP_LOGW(LIBRARY_NAME, "[Autodetect] load from NVS : board:%d", nvs_board);
      }

#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK ) || defined ( LGFX_LOLIN_D32_PRO )

      if (nvs_board == 0 || nvs_board == lgfx::board_t::board_M5Stack || nvs_board == lgfx::board_t::board_LoLinD32)
      {
        _pin_level(14, true);     // LCD CS;
        bus_cfg.pin_mosi = 23;
        bus_cfg.pin_miso = 19;
        bus_cfg.pin_sclk = 18;
        bus_cfg.pin_dc   = 27;

        #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK )
        _pin_level( 4, true);  // M5Stack TF card CS
        #endif
        #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_LOLIN_D32_PRO )
        _pin_level( 5, true);  // LoLinD32 TF card CS
        _pin_level(12, true);  // LoLinD32 TouchScreen CS
        #endif

        #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK )
        if (nvs_board == 0 || nvs_board == lgfx::board_t::board_M5Stack)
        {
          bus_cfg.spi_3wire = true;
          _bus_spi.config(bus_cfg);
          _bus_spi.init();
          _pin_reset(33, use_reset); // LCD RST;
          id = _read_panel_id(14);
          if ((id & 0xFF) == 0xE3)
          {   // ILI9342c
            ESP_LOGW(LIBRARY_NAME, "[Autodetect] M5Stack");
            _board = lgfx::board_t::board_M5Stack;

            #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_LOLIN_D32_PRO )
            lgfx::pinMode( 5, pin_mode_t::input); // LoLinD32 TF card CS
            lgfx::pinMode(12, pin_mode_t::input); // LoLinD32 TouchScreen CS
            #endif

            bus_cfg.freq_write = 40000000;
            bus_cfg.freq_read  = 16000000;
            _bus_spi.config(bus_cfg);

            auto p = new lgfx::Panel_M5Stack();
            p->bus(&_bus_spi);
            _panel_last = p;
            _set_pwm_backlight(32, 7, 44100);
            goto init_clear;
          }
          _bus_spi.release();
        }
        lgfx::pinMode( 4, pin_mode_t::input); // M5Stack TF card CS
        #endif

        #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_LOLIN_D32_PRO )
        if (nvs_board == 0 || nvs_board == lgfx::board_t::board_LoLinD32)
        {
          bus_cfg.spi_3wire = false;
          _bus_spi.config(bus_cfg);
          _bus_spi.init();
          _pin_reset(33, use_reset); // LCD RST;
          id = _read_panel_id(14);
          if ((id & 0xFF) == 0 && _read_panel_id(14, 0x09) != 0)
          {   // check panel (ILI9341) panelIDが0なのでステータスリード0x09を併用する
            ESP_LOGW(LIBRARY_NAME, "[Autodetect] LoLinD32Pro ILI9341");
            _board = lgfx::board_t::board_LoLinD32;

            #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK )
            lgfx::pinMode( 4, pin_mode_t::input); // M5Stack TF card CS
            #endif

            bus_cfg.freq_write = 40000000;
            bus_cfg.freq_read  = 16000000;
            _bus_spi.config(bus_cfg);

            auto p = new lgfx::Panel_ILI9341();
            {
              auto cfg = p->config();
              cfg.pin_cs  = 14;
              cfg.pin_rst = 33;
              p->config(cfg);
            }
            p->bus(&_bus_spi);
            _panel_last = p;
            _set_pwm_backlight(32, 7, 44100);

/*          auto t = new lgfx::Touch_XPT2046();
            t->spi_mosi = 23;
            t->spi_miso = 19;
            t->spi_sclk = 18;
            t->spi_cs   = 12;
            t->spi_host = VSPI_HOST;
            t->bus_shared = true;
            t->freq     = 2500000;
            touch(t);
*/
            goto init_clear;
          }
          _bus_spi.release();
        }
        lgfx::pinMode( 5, pin_mode_t::input); // LoLinD32 TF card CS
        lgfx::pinMode(12, pin_mode_t::input); // LoLinD32 TouchScreen CS
        #endif

        lgfx::pinMode(14, pin_mode_t::input); // LCD CS
        lgfx::pinMode(33, pin_mode_t::input); // LCD RST
      }
#endif

// M5StickC / CPlus 判定
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STICK_C ) || defined ( LGFX_M5STICKC )
      if (nvs_board == 0 || nvs_board == lgfx::board_t::board_M5StickC || nvs_board == lgfx::board_t::board_M5StickCPlus)
      {
        bus_cfg.pin_mosi = 15;
        bus_cfg.pin_miso = 14;
        bus_cfg.pin_sclk = 13;
        bus_cfg.pin_dc   = 23;
        bus_cfg.spi_3wire = true;
        _bus_spi.config(bus_cfg);
        _bus_spi.init();
        _pin_reset(18, use_reset); // LCD RST
        id = _read_panel_id(5);
        if ((id & 0xFF) == 0x85)
        {  //  check panel (ST7789)
          _board = lgfx::board_t::board_M5StickCPlus;
          ESP_LOGW(LIBRARY_NAME, "[Autodetect] M5StickCPlus");
          bus_cfg.freq_write = 80000000;
          bus_cfg.freq_read  = 16000000;
          _bus_spi.config(bus_cfg);
          auto p = new lgfx::Panel_M5StickCPlus();
          p->bus(&_bus_spi);
          _panel_last = p;
          _set_backlight(new Light_M5StickC());
          goto init_clear;
        }
        if ((id & 0xFF) == 0x7C)
        {  //  check panel (ST7735)
          _board = lgfx::board_t::board_M5StickC;
          ESP_LOGW(LIBRARY_NAME, "[Autodetect] M5StickC");
          bus_cfg.freq_write = 27000000;
          bus_cfg.freq_read  = 14000000;
          _bus_spi.config(bus_cfg);
          {
            auto p = new lgfx::Panel_M5StickC();
            p->bus(&_bus_spi);
            _panel_last = p;
          }
          _set_backlight(new Light_M5StickC());



          goto init_clear;
        }
        lgfx::pinMode(18, pin_mode_t::input); // LCD RST
        lgfx::pinMode( 5, pin_mode_t::input); // LCD CS
        _bus_spi.release();
      }
#endif

// M5Stack CoreInk 判定
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK_COREINK )
      if (nvs_board == 0 || nvs_board == lgfx::board_t::board_M5Stack_CoreInk)
      {
        bus_cfg.pin_mosi = 23;
        bus_cfg.pin_miso = -1;
        bus_cfg.pin_sclk = 18;
        bus_cfg.pin_dc   = 15;
        bus_cfg.spi_3wire = true;
        _bus_spi.config(bus_cfg);
        _bus_spi.init();
        _pin_reset( 0, true); // EPDがDeepSleepしていると自動認識に失敗するためRST制御は必須とする
        id = _read_panel_id(9, 0x70, 0);
        if (id == 0x00F00000)
        {  //  check panel (e-paper GDEW0154M09)
          _pin_level(12, true);  // POWER_HOLD_PIN 12
          _board = lgfx::board_t::board_M5Stack_CoreInk;
          ESP_LOGW(LIBRARY_NAME, "[Autodetect] M5Stack_CoreInk");
          bus_cfg.freq_write = 40000000;
          bus_cfg.freq_read  = 16000000;
          _bus_spi.config(bus_cfg);
          auto p = new lgfx::Panel_GDEW0154M09();
          p->bus(&_bus_spi);
          _panel_last = p;
          auto cfg = p->config();
          cfg.panel_height = 200;
          cfg.panel_width  = 200;
          cfg.pin_cs   = 9;
          cfg.pin_rst  = 0;
          cfg.pin_busy = 4;
          p->config(cfg);
          goto init_clear;
        }
        lgfx::pinMode( 0, pin_mode_t::input); // RST
        lgfx::pinMode( 9, pin_mode_t::input); // CS
        _bus_spi.release();
      }
#endif

// M5Stack Paper 判定
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5PAPER )
      if (nvs_board == 0 || nvs_board == lgfx::board_t::board_M5Paper)
      {
        _pin_reset(23, true);
        lgfx::pinMode(27, lgfx::pin_mode_t::input_pullup); // M5Paper EPD busy pin
        if (!lgfx::gpio_in(27))
        {
          lgfx::pinMode(27, lgfx::pin_mode_t::input);
//ESP_LOGW("LovyanGFX", "check M5Paper");
          bus_cfg.pin_mosi = 12;
          bus_cfg.pin_miso = 13;
          bus_cfg.pin_sclk = 14;
          bus_cfg.pin_dc   = -1;
          bus_cfg.spi_3wire = false;
          _bus_spi.config(bus_cfg);
          id = millis();
          do
          {
            if (millis() - id > 1000) { id = 0; break; }
            delay(1);
          } while (!lgfx::gpio_in(27));
          if (id)
          {
//ESP_LOGW("LovyanGFX", "ms:%d", millis() - id);
            _pin_level( 2, true);  // M5EPD_MAIN_PWR_PIN 2
            _pin_level( 4, true);  // M5Paper TF card CS
            _bus_spi.init();
            _bus_spi.beginTransaction();
            _pin_level(15, false); // M5Paper CS;
            _bus_spi.writeData(__builtin_bswap16(0x6000), 16);
            _bus_spi.writeData(__builtin_bswap16(0x0302), 16);  // read DevInfo
            _bus_spi.wait();
            lgfx::gpio_hi(15);
            id = millis();
            while (!lgfx::gpio_in(27))
            {
              if (millis() - id > 150) { break; }
              delay(1);
            };
//ESP_LOGW("LovyanGFX", "ms:%d", millis() - id);
            lgfx::gpio_lo(15);
            _bus_spi.writeData(__builtin_bswap16(0x1000), 16);
            _bus_spi.writeData(__builtin_bswap16(0x0000), 16);
            std::uint8_t buf[40];
            _bus_spi.beginRead();
            _bus_spi.readBytes(buf, 40, false);
            _bus_spi.endRead();
            _bus_spi.endTransaction();
            lgfx::gpio_hi(15);
            id = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
            ESP_LOGW("LovyanGFX", "[Autodetect] panel size :%08x", id);
            if (id == 0x03C0021C)
            {  //  check panel ( panel size 960(0x03C0) x 540(0x021C) )
//*
              _board = lgfx::board_t::board_M5Paper;
              ESP_LOGW(LIBRARY_NAME, "[Autodetect] M5Paper");
              bus_cfg.freq_write = 40000000;
              bus_cfg.freq_read  = 20000000;
              _bus_spi.config(bus_cfg);
              auto p = new lgfx::Panel_IT8951();
              p->bus(&_bus_spi);
              _panel_last = p;
              auto cfg = p->config();
              cfg.panel_height = 540;
              cfg.panel_width  = 960;
              cfg.pin_cs   = 15;
              cfg.pin_rst  = 23;
              cfg.pin_busy = 27;
              cfg.offset_rotation = 3;
              p->config(cfg);

            {
              auto t = new lgfx::Touch_GT911();
              _touch_last = t;
              touch(t);
              auto cfg = t->config();
              cfg.pin_int  = 36;   // INT pin number
              cfg.pin_sda  = 21;   // I2C SDA pin number
              cfg.pin_scl  = 22;   // I2C SCL pin number
              cfg.i2c_addr = 0x14; // I2C device addr
              cfg.i2c_port = I2C_NUM_0;// I2C port number
              cfg.freq = 400000;   // I2C freq
              cfg.x_min = 0;
              cfg.x_max = 959;
              cfg.y_min = 0;
              cfg.y_max = 539;
              t->config(cfg);
              if (!t->init())
              {
                cfg.i2c_addr = 0x5D; // addr change (0x14 or 0x5D)
                t->config(cfg);
              }
            }

              goto init_clear;
//*/
            }
            _bus_spi.release();
            lgfx::pinMode( 2, pin_mode_t::input); // M5EPD_MAIN_PWR_PIN 2
            lgfx::pinMode( 4, pin_mode_t::input); // M5Paper TF card CS
            lgfx::pinMode(15, pin_mode_t::input); // CS
          }
        }
        lgfx::pinMode(27, pin_mode_t::input); // BUSY
        lgfx::pinMode(23, pin_mode_t::input); // RST
      }
#endif

// M5Stack Core2 判定
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK_CORE2 ) || defined ( LGFX_M5STACKCORE2 )

      if (nvs_board == 0 || nvs_board == lgfx::board_t::board_M5StackCore2)
      {
        using namespace m5stack;

        lgfx::i2c::init(m5stack::axp_i2c_port, m5stack::axp_i2c_sda, m5stack::axp_i2c_scl, m5stack::axp_i2c_freq);
        // I2C addr 0x34 = AXP192
        if (lgfx::i2c::writeRegister8(m5stack::axp_i2c_port, m5stack::axp_i2c_addr, 0x95, 0x84, 0x72))
        { // GPIO4 enable
          // AXP192_LDO2 = LCD PWR
          // AXP192_DC3  = LCD BL
          // AXP192_IO4  = LCD RST
          if (use_reset) lgfx::i2c::writeRegister8(m5stack::axp_i2c_port, m5stack::axp_i2c_addr, 0x96, 0, ~0x02); // GPIO4 LOW (LCD RST)
          lgfx::i2c::writeRegister8(m5stack::axp_i2c_port, m5stack::axp_i2c_addr, 0x28, 0xF0, ~0);   // set LDO2 3300mv // LCD PWR
          lgfx::i2c::writeRegister8(m5stack::axp_i2c_port, m5stack::axp_i2c_addr, 0x12, 0x06, ~0);   // LDO2 and DC3 enable (DC3 = LCD BL)
          lgfx::i2c::writeRegister8(m5stack::axp_i2c_port, m5stack::axp_i2c_addr, 0x96, 2, ~0);      // GPIO4 HIGH (LCD RST)

          bus_cfg.pin_mosi = 23;
          bus_cfg.pin_miso = 38;
          bus_cfg.pin_sclk = 18;
          bus_cfg.pin_dc   = 15;
          bus_cfg.spi_3wire = true;
          _bus_spi.config(bus_cfg);
          _bus_spi.init();

          _pin_level( 4, true);   // TF card CS
          id = _read_panel_id(5);
          if ((id & 0xFF) == 0xE3)
          {   // ILI9342c
            ESP_LOGW(LIBRARY_NAME, "[Autodetect] M5StackCore2");
            _board = lgfx::board_t::board_M5StackCore2;

            bus_cfg.freq_write = 40000000;
            bus_cfg.freq_read  = 16000000;
            _bus_spi.config(bus_cfg);

            auto p = new Panel_M5StackCore2();
            p->bus(&_bus_spi);
            _panel_last = p;

            _set_backlight(new Light_M5StackCore2());

            {
              auto t = new lgfx::Touch_FT5x06();
              auto cfg = t->config();
              cfg.pin_int  = 39;   // INT pin number
              cfg.pin_sda  = 21;   // I2C SDA pin number
              cfg.pin_scl  = 22;   // I2C SCL pin number
              cfg.i2c_addr = 0x38; // I2C device addr
              cfg.i2c_port = I2C_NUM_1;// I2C port number
              cfg.freq = 400000;   // I2C freq
              cfg.x_min = 0;
              cfg.x_max = 319;
              cfg.y_min = 0;
              cfg.y_max = 279;
              t->config(cfg);
              touch(t);
              float affine[6] = { 0, -1, 240, 1, 0, 0};
              t->setCalibrateAffine(affine);
              _touch_last = t;
            }

            goto init_clear;
          }
          lgfx::pinMode( 4, pin_mode_t::input); // TF card CS
          lgfx::pinMode( 5, pin_mode_t::input); // LCD CS
          _bus_spi.release();
        }
      }

#endif

      goto init_clear;
  init_clear:

      panel(_panel_last);

      if (nvs_board != _board) {
        if (0 == nvs_open(LIBRARY_NAME, NVS_READWRITE, &nvs_handle)) {
          ESP_LOGW(LIBRARY_NAME, "[Autodetect] save to NVS : board:%d", _board);
          nvs_set_u32(nvs_handle, NVS_KEY, _board);
          nvs_close(nvs_handle);
        }
      }
      return (_board != lgfx::board_t::board_unknown);
    }
  };

//----------------------------------------------------------------------------
 }
}

using LGFX = lgfx::LGFX;