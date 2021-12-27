#ifndef LOVYANGFX_CONFIG_HPP_
#define LOVYANGFX_CONFIG_HPP_

#include <nvs.h>

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------
  struct LGFX_Config
  {
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int dma_channel = 1;
  };

  struct Panel_M5StickC : public Panel_ST7735S
  {
  private:

    static constexpr int32_t freq = 400000;
    static constexpr uint_fast8_t axp_i2c_addr = 0x34;
    static constexpr int_fast16_t axp_i2c_port = I2C_NUM_1;
    static constexpr int_fast16_t axp_i2c_sda = 21;
    static constexpr int_fast16_t axp_i2c_scl = 22;

  public:

    Panel_M5StickC() {
      reverse_invert = true;
      spi_3wire  = true;
      spi_cs     =  5;
      spi_dc     = 23;
      gpio_rst   = 18;
      panel_width  = 80;
      panel_height = 160;
      offset_x     = 26;
      offset_y     = 1;
      offset_rotation = 2;
    }

    void init(bool use_reset) override
    {
      (void)use_reset;
      lgfx::i2c::init(axp_i2c_port, axp_i2c_sda, axp_i2c_scl, freq);

      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x12, 0x4D, ~0);

      Panel_ST7735S::init(use_reset);
    }

    void setBrightness(uint8_t brightness) override
    {
      this->brightness = brightness;
      if (brightness)
      {
        brightness = (((brightness >> 1) + 8) / 13) + 5;
        wakeup(nullptr);
      }
      else
      {
        sleep(nullptr);
      }
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x28, brightness << 4, 0x0F);
    }

    void sleep(LGFX_Device*) override { lgfx::i2c::bitOff(axp_i2c_port, axp_i2c_addr, 0x12, 1 << 2); }

    void wakeup(LGFX_Device*) override { lgfx::i2c::bitOn(axp_i2c_port, axp_i2c_addr, 0x12, 1 << 2); }

  protected:
    const uint8_t* getInitCommands(uint8_t listno) const override {
      static constexpr uint8_t list[] = {
          CMD::GAMMASET, 1, 0x08,  // Gamma set, curve 4
          0xFF,0xFF, // end
      };
      if (listno == 2)  return list;
      return Panel_ST7735S::getInitCommands(listno);
    }
  };

  struct Panel_M5StickCPlus: public Panel_ST7789
  {
  private:

    static constexpr int32_t freq = 400000;
    static constexpr uint_fast8_t axp_i2c_addr = 0x34;
    static constexpr int_fast16_t axp_i2c_port = I2C_NUM_1;
    static constexpr int_fast16_t axp_i2c_sda = 21;
    static constexpr int_fast16_t axp_i2c_scl = 22;

  public:

    Panel_M5StickCPlus() {
      reverse_invert = true;
      spi_3wire  = true;
      spi_cs     =  5;
      spi_dc     = 23;
      gpio_rst   = 18;
      freq_write = 40000000;
      freq_read  = 15000000;
      freq_fill  = 40000000;
      spi_mode_read = 1;
      len_dummy_read_pixel = 16;
      panel_width  = 135;
      panel_height = 240;
      offset_x = 52;
      offset_y = 40;
    }

    void init(bool use_reset) override
    {
      (void)use_reset;
      lgfx::i2c::init(axp_i2c_port, axp_i2c_sda, axp_i2c_scl, freq);

      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x12, 0x4D, ~0);

      Panel_ST7789::init(use_reset);
    }

    void setBrightness(uint8_t brightness) override
    {
      this->brightness = brightness;
      if (brightness)
      {
        brightness = (((brightness >> 1) + 8) / 13) + 5;
        wakeup(nullptr);
      }
      else
      {
        sleep(nullptr);
      }
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x28, brightness << 4, 0x0F);
    }

    void sleep(LGFX_Device*) override { lgfx::i2c::bitOff(axp_i2c_port, axp_i2c_addr, 0x12, 1 << 2); }

    void wakeup(LGFX_Device*) override { lgfx::i2c::bitOn(axp_i2c_port, axp_i2c_addr, 0x12, 1 << 2); }
  };

  struct Panel_M5Stack : public Panel_ILI9342
  {
    Panel_M5Stack(void)
    {
      spi_3wire = true;
      spi_cs = 14;
      spi_dc = 27;
      rotation = 1;
      offset_rotation = 3;
      gpio_rst = 33;
      gpio_bl  = 32;
      pwm_ch_bl = 7;
      pwm_freq  = 44100;
    }

    void init(bool use_reset) override
    {
      static constexpr char NVS_NAME[] = "LovyanGFX";
      static constexpr char NVS_KEY[] = "M5Stack_IPS";
      uint32_t nvs_handle = 0;
      uint8_t readbuf = 0x80u;
      if (ESP_OK == nvs_open(NVS_NAME, NVS_READONLY, &nvs_handle)) {
        nvs_get_u8(nvs_handle, NVS_KEY, &readbuf);
        nvs_close(nvs_handle);
      }
      if (readbuf != 0x80u)
      {
        reverse_invert = readbuf;
      }
      else
      {
        nvs_handle = 0;
        gpio_lo(gpio_rst);
        lgfxPinMode(gpio_rst, pin_mode_t::input);
        delay(1);
        reverse_invert = gpio_in(gpio_rst);       // get panel type (IPS or TN)
        if (ESP_OK == nvs_open(NVS_NAME, NVS_READWRITE, &nvs_handle)) {
          nvs_set_u8(nvs_handle, NVS_KEY, reverse_invert);
          nvs_close(nvs_handle);
        }
      }

      Panel_ILI9342::init(use_reset);
    }
  };

  struct Panel_M5StackCore2 : public Panel_ILI9342
  {
  private:

    static constexpr int32_t freq = 400000;
    static constexpr uint_fast8_t axp_i2c_addr = 0x34;
    static constexpr int_fast16_t axp_i2c_port = I2C_NUM_1;
    static constexpr int_fast16_t axp_i2c_sda = 21;
    static constexpr int_fast16_t axp_i2c_scl = 22;

  public:

    Panel_M5StackCore2(void) {
      reverse_invert = true;
      spi_3wire = true;
      spi_cs =  5;
      spi_dc = 15;
      rotation = 1;
      offset_rotation = 3;
    }

    void resetPanel(void)
    {
      // AXP192 reg 0x96 = GPIO3&4 control
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x96, 0, ~0x02); // GPIO4 LOW (LCD RST)
      delay(10);
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x96, 2, ~0); // GPIO4 HIGH (LCD RST)
    }

    void init(bool use_reset) override
    {
      if (use_reset) resetPanel();

      Panel_ILI9342::init(use_reset);
    }

    void setBrightness(uint8_t brightness) override
    {
      this->brightness = brightness;
      if (brightness)
      {
        brightness = (brightness >> 3) + 72;
        wakeup(nullptr);
      }
      else
      {
        sleep(nullptr);
      }
      // AXP192 reg 0x27 = DC3
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x27, brightness, 0x80);
    }

    void sleep(LGFX_Device*) override { lgfx::i2c::bitOff(axp_i2c_port, axp_i2c_addr, 0x12, 0x02); } // DC3 disable

    void wakeup(LGFX_Device*) override { lgfx::i2c::bitOn(axp_i2c_port, axp_i2c_addr, 0x12, 0x02); } // DC3 enable

  };


  class LGFX : public lgfx::LGFX_SPI<lgfx::LGFX_Config>
  {
  public:
    LGFX(void) : lgfx::LGFX_SPI<lgfx::LGFX_Config>()
    {
    }

    void init_without_reset(void)
    {
      int retry = 3;
      while (!autodetect(false) && --retry);
      lgfx::LGFX_SPI<lgfx::LGFX_Config>::init_impl(false);
    }

    bool autodetect(bool use_reset = true)
    {
      if (_spi_mosi != -1 && _spi_sclk != -1) {
        return true;
      }
      preInit();

      if (_panel_last) {
        delete _panel_last;
        _panel_last = nullptr;
      }

      lgfx::PanelIlitekCommon p_tmp;
      p_tmp.spi_3wire  = true;
      p_tmp.freq_write = 8000000;
      p_tmp.freq_read  = 8000000;
      p_tmp.freq_fill  = 8000000;
      board = lgfx::board_t::board_unknown;
      uint32_t id;
      (void)id; // Suppressing Compiler Warnings

      static constexpr char NVS_NAME[] = "LovyanGFX";
      static constexpr char NVS_KEY[] = "AUTODETECT";
      uint32_t nvs_board = 0;
      uint32_t nvs_handle = 0;
      if (ESP_OK == nvs_open(NVS_NAME, NVS_READONLY, &nvs_handle)) {
        nvs_get_u32(nvs_handle, NVS_KEY, static_cast<uint32_t*>(&nvs_board));
        nvs_close(nvs_handle);
        ESP_LOGW("LovyanGFX", "[Autodetect] load from NVS : board:%d", nvs_board);
      }

  // TTGO T-Watch 判定 (GPIO33を使う判定を先に行うと振動モーターが作動する事に注意)
  #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_TTGO_TWATCH )
      if (nvs_board == 0 || nvs_board == lgfx::board_t::board_TTGO_TWatch) {
        releaseBus();
        _spi_mosi = 19;
        _spi_miso = -1;
        _spi_sclk = 18;
        initBus();

        p_tmp.spi_cs   =  5;
        p_tmp.spi_dc   = 27;
        p_tmp.gpio_rst = -1;
        setPanel(&p_tmp);

        auto id = readPanelID();
        ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);
        if ((id & 0xFF) == 0x85) {  //  check panel (ST7789)
          ESP_LOGW("LovyanGFX", "[Autodetect] TWatch");
          board = lgfx::board_t::board_TTGO_TWatch;
          releaseBus();
          _spi_host = HSPI_HOST;
          initBus();
          auto p = new lgfx::Panel_ST7789();
          p->reverse_invert = true;
          p->freq_write = 80000000;
          p->freq_read  = 20000000;
          p->freq_fill  = 80000000;
          p->panel_height = 240;
          p->spi_3wire = true;
          p->spi_cs    =  5;
          p->spi_dc    = 27;
          p->gpio_bl   = 12;
          p->pwm_ch_bl = 7;
          p->pwm_freq  = 1200;
          setPanel(p);

          auto t = new lgfx::Touch_FT5x06();
          t->gpio_int = 38;   // INT pin number
          t->i2c_sda  = 23;   // I2C SDA pin number
          t->i2c_scl  = 32;   // I2C SCL pin number
          t->i2c_addr = 0x38; // I2C device addr
          t->i2c_port = I2C_NUM_1;// I2C port number
          t->freq = 400000;   // I2C freq
          t->x_min = 0;
          t->x_max = 319;
          t->y_min = 0;
          t->y_max = 319;
          touch(t);

          goto init_clear;
        }
        lgfx::gpio_lo(p_tmp.spi_cs);
        lgfx::gpio_lo(p_tmp.spi_dc);
      }
  #endif

  // TTGO T-Wristband 判定
  #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_TTGO_TWRISTBAND )
      if (nvs_board == 0 || nvs_board == lgfx::board_t::board_TTGO_TWristband) {
        releaseBus();
        _spi_mosi = 19;
        _spi_miso = -1;
        _spi_sclk = 18;
        initBus();

        p_tmp.spi_cs   =  5;
        p_tmp.spi_dc   = 23;
        p_tmp.gpio_rst = 26;
        setPanel(&p_tmp);
        _reset(use_reset);

        auto id = readPanelID();
        ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);
        if ((id & 0xFF) == 0x7C) {  //  check panel (ST7735)
          ESP_LOGW("LovyanGFX", "[Autodetect] TWristband");
          board = lgfx::board_t::board_TTGO_TWristband;
          auto p = new lgfx::Panel_ST7735S();
          p->reverse_invert = true;
          p->len_dummy_read_pixel = 17;
          p->spi_3wire  = true;
          p->spi_cs     =  5;
          p->spi_dc     = 23;
          p->gpio_rst   = 26;
          p->gpio_bl    = 27;
          p->pwm_ch_bl  = 7;
          p->panel_width  = 80;
          p->panel_height = 160;
          p->offset_x     = 26;
          p->offset_y     = 1;
          p->offset_rotation = 2;
          setPanel(p);
          goto init_clear;
        }
        lgfx::gpio_lo(p_tmp.spi_cs);
        lgfx::gpio_lo(p_tmp.spi_dc);
        lgfx::gpio_lo(p_tmp.gpio_rst);
      }
  #endif

  // TTGO T-Display
  #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_TTGO_TDISPLAY )
      if (nvs_board == 0 || nvs_board == lgfx::board_t::board_TTGO_TDisplay) {
        releaseBus();
        _spi_mosi = 19;
        _spi_miso = -1;
        _spi_sclk = 18;
        initBus();

        p_tmp.spi_cs   =  5;
        p_tmp.spi_dc   = 16;
        p_tmp.gpio_rst = 23;
        setPanel(&p_tmp);

        auto id = readPanelID();
        ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);
        if ((id & 0xFF) == 0x85) {  //  check panel (ST7789)
          ESP_LOGW("LovyanGFX", "[Autodetect] TDisplay");
          board = lgfx::board_t::board_TTGO_TDisplay;
          releaseBus();
          _spi_host = HSPI_HOST;
          initBus();
          auto p = new lgfx::Panel_ST7789();
          p->reverse_invert = true;
          p->freq_write = 40000000;
          p->freq_read  = 6000000;
          p->freq_fill  = 40000000;
          p->panel_width  = 135;
          p->panel_height = 240;
          p->offset_x     = 52;
          p->offset_y     = 40;         
          p->spi_3wire = true;
          p->spi_cs    =  5;
          p->spi_dc    = 16;
          p->gpio_bl   = 4;
          p->pwm_ch_bl = 7;
          p->pwm_freq  = 1200;
          setPanel(p);

          goto init_clear;
        }
        lgfx::gpio_lo(p_tmp.spi_cs);
        lgfx::gpio_lo(p_tmp.spi_dc);
      }
  #endif

  // M5Stack/LoLinD32Pro 判定 (GPIO15を使う判定を先に行うとM5GO bottomのLEDが点灯する事に注意)
  #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK ) || defined ( LGFX_LOLIN_D32_PRO )
      if (nvs_board == 0 || nvs_board == lgfx::board_t::board_M5Stack || nvs_board == lgfx::board_t::board_LoLinD32) {

        releaseBus();
        _spi_mosi = 23;
        _spi_miso = 19;
        _spi_sclk = 18;
        initBus();

  #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK )
        lgfx::lgfxPinMode(4, lgfx::pin_mode_t::output); // M5Stack TF card CS
        lgfx::gpio_hi(4);
  #endif
  #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_LOLIN_D32_PRO )
        lgfx::lgfxPinMode(5, lgfx::pin_mode_t::output); // LoLinD32 TF card CS
        lgfx::gpio_hi(5);
        lgfx::lgfxPinMode(12, lgfx::pin_mode_t::output); // LoLinD32 TouchScreen CS
        lgfx::gpio_hi(12);
  #endif

        p_tmp.spi_cs   = 14;
        p_tmp.spi_dc   = 27;
        p_tmp.gpio_rst = 33;

  #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_LOLIN_D32_PRO )
        if (nvs_board == 0 || nvs_board == lgfx::board_t::board_LoLinD32) {

          p_tmp.spi_3wire = false;
          setPanel(&p_tmp);
          _reset(use_reset);

          id = readPanelID();

          ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);

      // LoLinD32Pro (ILI9341) 判定
          if ((id & 0xFF) == 0 && readCommand32(0x09) != 0) {   // check panel (ILI9341) panelIDが0なのでステータスリードを併用する
    #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK )
            lgfx::gpio_lo(4);
    #endif
            ESP_LOGW("LovyanGFX", "[Autodetect] LoLinD32Pro ILI9341");
            board = lgfx::board_t::board_LoLinD32;
            auto p = new lgfx::Panel_ILI9341();
            p->spi_3wire = false;
            p->spi_cs    = 14;
            p->spi_dc    = 27;
            p->gpio_rst  = 33;
            p->gpio_bl   = 32;
            p->pwm_ch_bl = 7;
            setPanel(p);

            auto t = new lgfx::Touch_XPT2046();
            t->spi_mosi = 23;
            t->spi_miso = 19;
            t->spi_sclk = 18;
            t->spi_cs   = 12;
            t->spi_host = VSPI_HOST;
            t->bus_shared = true;
            t->freq     = 2500000;

            touch(t);

            goto init_clear;
          }

          p_tmp.spi_3wire = true;
        }

  #endif

        setPanel(&p_tmp);
        _reset(use_reset);

        id = readPanelID();

        ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);

    // LoLinD32Pro (ST7735) 判定
  #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_LOLIN_D32_PRO )
        if (nvs_board == 0 || nvs_board == lgfx::board_t::board_LoLinD32) {

          if ((id & 0xFF) == 0x7C) {  //  check panel (ST7735)
    #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK )
            lgfx::gpio_lo(4);
    #endif
            ESP_LOGW("LovyanGFX", "[Autodetect] LoLinD32Pro ST7735");
            board = lgfx::board_t::board_LoLinD32;
            auto p = new lgfx::Panel_ST7735S();
            p->panel_width  = 128;
            p->panel_height = 128;
            p->memory_width  = 132;
            p->memory_height = 132;
            p->offset_x  = 2;
            p->offset_y  = 1;
            p->spi_3wire = true;
            p->spi_cs    = 14;
            p->spi_dc    = 27;
            p->gpio_rst  = 33;
            p->gpio_bl   = 32;
            p->pwm_ch_bl = 7;
            setPanel(p);
            goto init_clear;
          }
        }
  #endif

    // M5Stack 判定
  #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK )
        if (nvs_board == 0 || nvs_board == lgfx::board_t::board_M5Stack) {
          if ((id & 0xFF) == 0xE3) {   // ILI9342c

      #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_LOLIN_D32_PRO )
            lgfx::gpio_lo(5);
            lgfx::gpio_lo(12);
      #endif
            ESP_LOGW("LovyanGFX", "[Autodetect] M5Stack");
            board = lgfx::board_t::board_M5Stack;
            auto p = new lgfx::Panel_M5Stack();
            setPanel(p);
            goto init_clear;
          }
    #endif
        }
        lgfx::gpio_lo(p_tmp.spi_cs);
        lgfx::gpio_lo(p_tmp.spi_dc);
        lgfx::gpio_lo(p_tmp.gpio_rst);
        lgfx::gpio_lo(4);
        lgfx::gpio_lo(5);
        lgfx::gpio_lo(12);
      }
  #endif

  // ODROID_GO 判定 (ボードマネージャでM5StickCを選択していると判定失敗する事に注意)
  #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_ODROID_GO )
      if (nvs_board == 0 || nvs_board == lgfx::board_t::board_ODROID_GO) {
        releaseBus();
        _spi_mosi = 23;
        _spi_miso = 19;
        _spi_sclk = 18;
        initBus();

        p_tmp.spi_cs   =  5;
        p_tmp.spi_dc   = 21;
        p_tmp.gpio_rst = -1;
        setPanel(&p_tmp);

        lgfx::lgfxPinMode(22, lgfx::pin_mode_t::output); // ODROID-GO TF card CS
        lgfx::gpio_hi(22);

        id = readPanelID();
        if ((id & 0xFF) == 0 && readCommand32(0x09) != 0) {   // check panel (ILI9341) panelIDが0なのでステータスリードを併用する
          ESP_LOGW("LovyanGFX", "[Autodetect] ODROID_GO");
          board = lgfx::board_t::board_ODROID_GO;
          auto p = new lgfx::Panel_ILI9341();
          p->freq_fill  = 80000000;
          p->spi_3wire = true;
          p->spi_cs =  5;
          p->spi_dc = 21;
          p->rotation = 1;
          p->gpio_bl = 14;
          p->pwm_ch_bl = 7;
          setPanel(p);
          goto init_clear;
        }
        lgfx::gpio_lo(p_tmp.spi_cs);
        lgfx::gpio_lo(p_tmp.spi_dc);
        lgfx::gpio_lo(22);
      }
  #endif

  // M5StickC / CPlus 判定
  #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STICK_C ) || defined ( LGFX_M5STICKC )
      if (nvs_board == 0 || nvs_board == lgfx::board_t::board_M5StickC || nvs_board == lgfx::board_t::board_M5StickCPlus) {
        releaseBus();
        _spi_mosi = 15;
        _spi_miso = 14;
        _spi_sclk = 13;
        initBus();

        p_tmp.spi_cs   =  5;
        p_tmp.spi_dc   = 23;
        p_tmp.gpio_rst = 18;
        setPanel(&p_tmp);
        _reset(use_reset);

        id = readPanelID();
        ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);
        if ((id & 0xFF) == 0x85) {  //  check panel (ST7789)
          ESP_LOGW("LovyanGFX", "[Autodetect] M5StickCPlus");
          board = lgfx::board_t::board_M5StickCPlus;
          auto p = new lgfx::Panel_M5StickCPlus();
          setPanel(p);
          goto init_clear;
        }

        if ((id & 0xFF) == 0x7C) {  //  check panel (ST7735)
          ESP_LOGW("LovyanGFX", "[Autodetect] M5StickC");
          board = lgfx::board_t::board_M5StickC;
          auto p = new lgfx::Panel_M5StickC();
          setPanel(p);
          goto init_clear;
        }
        lgfx::gpio_lo(p_tmp.spi_cs);
        lgfx::gpio_lo(p_tmp.spi_dc);
        lgfx::gpio_lo(p_tmp.gpio_rst);
      }
  #endif

  // ESP-WROVER-KIT 判定
  #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_ESP_WROVER_KIT )
      if (nvs_board == 0 || nvs_board == lgfx::board_t::board_ESP_WROVER_KIT) {
        releaseBus();
        _spi_mosi = 23;
        _spi_miso = 25;
        _spi_sclk = 19;
        initBus();

        p_tmp.spi_3wire = false;
        p_tmp.spi_cs   = 22;
        p_tmp.spi_dc   = 21;
        p_tmp.gpio_rst = 18;
        setPanel(&p_tmp);
        _reset(use_reset);

        id = readPanelID();
        ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);
        if ((id & 0xFF) == 0x85) {  //  check panel (ST7789)
          ESP_LOGW("LovyanGFX", "[Autodetect] ESP-WROVER-KIT ST7789");
          board = lgfx::board_t::board_ESP_WROVER_KIT;
          auto p = new lgfx::Panel_ST7789();
          p->spi_3wire = false;
          p->spi_cs   = 22;
          p->spi_dc   = 21;
          p->gpio_rst = 18;
          p->gpio_bl  = 5;
          p->pwm_ch_bl = 7;
          p->freq_write = 80000000;
          p->freq_read  = 16000000;
          p->freq_fill  = 80000000;
          p->backlight_level = false;
          p->offset_rotation = 2;
          p->spi_mode_read = 1;
          p->len_dummy_read_pixel = 16;

          setPanel(p);
          goto init_clear;
        }

        if ((id & 0xFF) == 0 && readCommand32(0x09) != 0) {   // check panel (ILI9341) panelIDが0なのでステータスリードを併用する
          ESP_LOGW("LovyanGFX", "[Autodetect] ESP-WROVER-KIT ILI9341");
          board = lgfx::board_t::board_ESP_WROVER_KIT;
          auto p = new lgfx::Panel_ILI9341();
          p->spi_3wire = false;
          p->spi_cs   = 22;
          p->spi_dc   = 21;
          p->gpio_rst = 18;
          p->gpio_bl  = 5;
          p->pwm_ch_bl = 7;
          p->freq_write = 40000000;
          p->freq_read  = 20000000;
          p->freq_fill  = 80000000;
          p->backlight_level = false;

          setPanel(p);
          goto init_clear;
        }
        lgfx::gpio_lo(p_tmp.spi_cs);
        lgfx::gpio_lo(p_tmp.spi_dc);
        lgfx::gpio_lo(p_tmp.gpio_rst);
        p_tmp.spi_3wire = true;
      }
  #endif

  // M5Stack CoreInk 判定
  #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK_COREINK )
      if (nvs_board == 0 || nvs_board == lgfx::board_t::board_M5Stack_CoreInk) {
        releaseBus();
        _spi_mosi = 23;
        _spi_miso = -1;
        _spi_sclk = 18;
        initBus();

        p_tmp.spi_cs   = 9;
        p_tmp.spi_dc   = 15;
        p_tmp.gpio_rst = 0;
        setPanel(&p_tmp);
        _reset(true);  // EPDがDeepSleepしていると自動認識も失敗するためRST制御は必須とする

        auto id = readCommand32(0x70);

        ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);
        if (id == 0x0001e000) {  //  check panel (e-paper GDEW0154M09)
          ESP_LOGW("LovyanGFX", "[Autodetect] M5Stack_CoreInk");
          board = lgfx::board_t::board_M5Stack_CoreInk;

          lgfx::lgfxPinMode(12, lgfx::pin_mode_t::output); // POWER_HOLD_PIN 12
          lgfx::gpio_hi(12);

          auto p = new lgfx::Panel_GDEW0154M09();
          p->freq_write = 40000000;
          p->freq_read  = 16000000;
          p->freq_fill  = 40000000;
          p->spi_3wire = true;
          p->panel_width = 200;
          p->panel_height = 200;
          p->spi_cs    = 9;
          p->spi_dc    = 15;
          p->gpio_bl   = -1;
          p->gpio_rst  = 0;
          p->gpio_busy = 4;
          setPanel(p);

          goto init_clear;
        }
        lgfx::gpio_lo(p_tmp.spi_cs);
        lgfx::gpio_lo(p_tmp.spi_dc);
        lgfx::gpio_lo(p_tmp.gpio_rst);
      }
  #endif

  // M5Stack Paper 判定
  #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5PAPER )
      if (nvs_board == 0 || nvs_board == lgfx::board_t::board_M5Paper)
      {
        lgfx::lgfxPinMode(27, lgfx::pin_mode_t::input); // M5Paper EPD busy pin
        lgfx::lgfxPinMode(4, lgfx::pin_mode_t::output); // M5Paper TF card CS
        lgfx::gpio_hi(4);

        releaseBus();
        _spi_mosi = 12;
        _spi_miso = 13;
        _spi_sclk = 14;
        initBus();

        p_tmp.spi_3wire= false;
        p_tmp.spi_cs   = 15;
        p_tmp.spi_dc   = -1;
        p_tmp.gpio_rst = 23;
        setPanel(&p_tmp);
        _reset(true);

        if (!lgfx::gpio_in(27))
        {
          id = millis();
          while (!lgfx::gpio_in(27))
          {
            if (millis() - id > 1000) { id = 0; break; }
            delay(1);
          };
          if (id)
          {
            startWrite();
            cs_l();
            writeData16(0x6000);
            writeData16(0x0302);  // read DevInfo
            cs_h();
            id = millis();
            while (!lgfx::gpio_in(27))
            {
              if (millis() - id > 150) { break; }
              delay(1);
            };
            cs_l();
            writeData16(0x1000);
            writeData16(0x0000);
            uint8_t buf[40];
            readBytes(buf, 40);
            cs_h();
            endWrite();
            id = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];

            ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);
            if (id == 0x03C0021C) {  //  check panel ( panel size 960(0x03C0) x 540(0x021C) )
              ESP_LOGW("LovyanGFX", "[Autodetect] M5Paper");

              lgfx::lgfxPinMode( 2, lgfx::pin_mode_t::output); // M5EPD_MAIN_PWR_PIN 2
              lgfx::gpio_hi( 2);

              board = lgfx::board_t::board_M5Paper;
              auto p = new lgfx::Panel_IT8951();
              p->freq_write = 40000000;
              p->freq_read  = 20000000;
              p->freq_fill  = 40000000;
              p->spi_3wire = false;
              p->spi_cs    = 15;
              p->spi_dc    = -1;
              p->gpio_busy = 27;
              p->gpio_bl   = -1;
              p->gpio_rst  = 23;
              p->rotation  = 0;
              p->offset_rotation = 3;
              setPanel(p);

              auto t = new lgfx::Touch_GT911();
              t->gpio_int = 36;   // INT pin number
              t->i2c_sda  = 21;   // I2C SDA pin number
              t->i2c_scl  = 22;   // I2C SCL pin number
              t->i2c_addr = 0x14; // I2C device addr
              t->i2c_port = I2C_NUM_0;// I2C port number
              t->freq = 400000;   // I2C freq
              t->x_min = 0;
              t->x_max = 959;
              t->y_min = 0;
              t->y_max = 539;
              touch(t);
              if (!t->init()) { t->i2c_addr = 0x5D; } // addr change (0x14 or 0x5D)
              goto init_clear;
            }
          }
        }
        lgfx::gpio_lo(p_tmp.spi_cs);
        lgfx::gpio_lo(p_tmp.gpio_rst);
        lgfx::gpio_lo(4);
      }
  #endif

  // TTGO TS判定
  #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_TTGO_TS )
      if (nvs_board == 0 || nvs_board == lgfx::board_t::board_TTGO_TS) {
        releaseBus();
        _spi_mosi = 23;
        _spi_miso = -1;
        _spi_sclk =  5;
        initBus();

        p_tmp.spi_cs   = 16;
        p_tmp.spi_dc   = 17;
        p_tmp.gpio_rst =  9;
        setPanel(&p_tmp);
        _reset(use_reset);

        id = readPanelID();
        ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);
        if ((id & 0xFF) == 0x7C) {  //  check panel (ST7735)
          ESP_LOGW("LovyanGFX", "[Autodetect] TTGO TS");
          board = lgfx::board_t::board_TTGO_TS;
          auto p = new lgfx::Panel_ST7735S();
          p->freq_write = 20000000;
          p->panel_width  = 128;
          p->panel_height = 160;
          p->offset_x     = 2;
          p->offset_y     = 1;
          p->offset_rotation = 2;
          p->rgb_order = true;
          p->spi_3wire = true;
          p->spi_cs    = 16;
          p->spi_dc    = 17;
          p->gpio_rst  =  9;
          p->gpio_bl   = 27;
          p->pwm_ch_bl = 7;
          setPanel(p);
          goto init_clear;
        }
        lgfx::gpio_lo(p_tmp.spi_cs);
        lgfx::gpio_lo(p_tmp.spi_dc);
        lgfx::gpio_lo(p_tmp.gpio_rst);
      }
  #endif

  // WiFiBoy mini
  #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_WIFIBOY_MINI )
      if (nvs_board == 0
      || nvs_board == lgfx::board_t::board_WiFiBoy_Mini
      ) {
        releaseBus();
        _spi_mosi = 13;
        _spi_miso = 12;
        _spi_sclk = 14;
        initBus();

        p_tmp.spi_3wire = true;
        p_tmp.spi_cs   = 15;
        p_tmp.spi_dc   = 4;
        p_tmp.gpio_rst = -1;
        setPanel(&p_tmp);

        auto id = readPanelID();
        ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);
        if ((id & 0xFF) == 0x7C) {  //  check panel (ST7735)
          ESP_LOGW("LovyanGFX", "[Autodetect] WiFiBoy mini");
          board = lgfx::board_t::board_WiFiBoy_Mini;
          releaseBus();
          _spi_host = HSPI_HOST;
          initBus();
          auto p = new lgfx::Panel_ST7735S();
          p->freq_read  = 8000000;
          p->panel_width  = 128;
          p->panel_height = 128;
          p->memory_width  = 132;
          p->memory_height = 132;
          p->spi_3wire = true;
          p->offset_x     = 2;
          p->offset_y     = 1;
          p->spi_cs    = 15;
          p->spi_dc    = 4;
          p->gpio_bl   = 27;
          p->pwm_ch_bl = 7;
          p->offset_rotation = 2;
          setPanel(p);

          goto init_clear;
        }
        lgfx::gpio_lo(p_tmp.spi_cs);
        lgfx::gpio_lo(p_tmp.spi_dc);
      }
  #endif

  // WiFiBoy Pro
  #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_WIFIBOY_PRO )
      if (nvs_board == 0 || nvs_board == lgfx::board_t::board_WiFiBoy_Pro) {
        releaseBus();
        _spi_mosi = 13;
        _spi_miso = 12;
        _spi_sclk = 14;
        initBus();

        p_tmp.spi_3wire = false;
        p_tmp.spi_cs   = 15;
        p_tmp.spi_dc   =  4;
        p_tmp.gpio_rst = -1;
        setPanel(&p_tmp);

        auto id = readPanelID();
        ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);
        if (id == 0 && readCommand32(0x09) != 0) {   // check panel (ILI9341) panelIDが0なのでステータスリードを併用する
          ESP_LOGW("LovyanGFX", "[Autodetect] WiFiBoy Pro");
          board = lgfx::board_t::board_WiFiBoy_Pro;
          releaseBus();
          _spi_host = HSPI_HOST;
          initBus();
          auto p = new lgfx::Panel_ILI9341();
          p->freq_write = 40000000;
          p->freq_read  = 16000000;
          p->freq_fill  = 40000000;
          p->spi_3wire = false;
          p->spi_cs    = 15;
          p->spi_dc    =  4;
          p->gpio_bl   = 27;
          p->offset_rotation = 2;
          setPanel(p);

          goto init_clear;
        }
        lgfx::gpio_lo(p_tmp.spi_cs);
        lgfx::gpio_lo(p_tmp.spi_dc);
        p_tmp.spi_3wire = true;
      }
  #endif

  // Makerfabs TouchScreen_Camera
  #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_MAKERFABS_TOUCHCAMERA )
      if (nvs_board == 0 || nvs_board == lgfx::board_t::board_Makerfabs_TouchCamera) {
        releaseBus();
        _spi_mosi = 13;
        _spi_miso = 12;
        _spi_sclk = 14;
        initBus();

        lgfx::lgfxPinMode(4, lgfx::pin_mode_t::output); // Makerfabs TouchCamera TF card CS
        lgfx::gpio_hi(4);

        p_tmp.spi_3wire = false;
        p_tmp.spi_cs   = 15;
        p_tmp.spi_dc   = 33;
        p_tmp.gpio_rst = -1;
        setPanel(&p_tmp);

        auto id = readPanelID();
        ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);
        if ((id & 0xFF) == 0x54) {   // check panel (ILI9488)
          ESP_LOGW("LovyanGFX", "[Autodetect] Makerfabs_TouchCamera");
          board = lgfx::board_t::board_Makerfabs_TouchCamera;
          auto p = new lgfx::Panel_ILI9488();
          p->freq_write = 40000000;
          p->freq_read  = 16000000;
          p->freq_fill  = 40000000;
          p->spi_3wire = false;
          p->spi_cs    = 15;
          p->spi_dc    = 33;
          p->gpio_bl   = -1;
          setPanel(p);

          lgfx::i2c::init(I2C_NUM_1, 26, 27, 400000);
          uint8_t tmp[2];
          if (lgfx::i2c::readRegister(I2C_NUM_1, 0x38, 0xA8, tmp, 1))
          {
            ESP_LOGW("LovyanGFX", "[Autodetect] touch id:%08x", tmp[1]);
            auto t = new lgfx::Touch_FT5x06();
            t->gpio_int = 38;   // INT pin number
            t->i2c_sda  = 26;   // I2C SDA pin number
            t->i2c_scl  = 27;   // I2C SCL pin number
            t->i2c_addr = 0x38; // I2C device addr
            t->i2c_port = I2C_NUM_1;// I2C port number
            t->freq = 400000;   // I2C freq
            t->x_min = 0;
            t->x_max = 319;
            t->y_min = 0;
            t->y_max = 479;
            touch(t);
          }
          goto init_clear;
        }
        lgfx::gpio_lo(p_tmp.spi_cs);
        lgfx::gpio_lo(p_tmp.spi_dc);
        lgfx::gpio_lo(4);
        p_tmp.spi_3wire = true;
      }
  #endif

  // Makerfabs MakePython
  #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_MAKERFABS_MAKEPYTHON )
      if (nvs_board == 0 || nvs_board == lgfx::board_t::board_Makerfabs_MakePython) {
        releaseBus();
        _spi_mosi = 13;
        _spi_miso = 12;
        _spi_sclk = 14;
        initBus();

        p_tmp.spi_cs   = 15;
        p_tmp.spi_dc   = 22;
        p_tmp.gpio_rst = 21;
        setPanel(&p_tmp);
        _reset(use_reset);

        auto id = readPanelID();
        ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);
        if ((id & 0xFF) == 0x85) {  //  check panel (ST7789)
          ESP_LOGW("LovyanGFX", "[Autodetect] Makerfabs_Makepython");
          board = lgfx::board_t::board_Makerfabs_MakePython;
          releaseBus();
          _spi_host = HSPI_HOST;
          initBus();
          auto p = new lgfx::Panel_ST7789();
          p->reverse_invert = true;
          p->freq_write = 80000000;
          p->freq_read  = 16000000;
          p->freq_fill  = 80000000;
          p->spi_3wire = true;
          p->panel_height = 240;
          p->spi_cs    = 15;
          p->spi_dc    = 22;
          p->gpio_bl   = 5;
          p->gpio_rst  = 21;
          p->pwm_ch_bl = 7;
          setPanel(p);

          goto init_clear;
        }
        lgfx::gpio_lo(p_tmp.spi_cs);
        lgfx::gpio_lo(p_tmp.spi_dc);
        lgfx::gpio_lo(p_tmp.gpio_rst);
      }
  #endif

  // M5Stack Core2 判定
  #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK_CORE2 ) || defined ( LGFX_M5STACKCORE2 )
      if (nvs_board == 0 || nvs_board == lgfx::board_t::board_M5StackCore2) {
        lgfx::i2c::init(I2C_NUM_1, 21, 22, 400000);
        // I2C addr 0x34 = AXP192
        if (lgfx::i2c::writeRegister8(I2C_NUM_1, 0x34, 0x95, 0x84, 0x72)) { // GPIO4 enable
          // AXP192_LDO2 = LCD PWR
          // AXP192_DC3  = LCD BL
          // AXP192_IO4  = LCD RST
          if (use_reset) lgfx::i2c::writeRegister8(I2C_NUM_1, 0x34, 0x96, 0, ~0x02); // GPIO4 LOW (LCD RST)
          lgfx::i2c::writeRegister8(I2C_NUM_1, 0x34, 0x28, 0xF0, ~0);   // set LDO2 3300mv // LCD PWR
          lgfx::i2c::writeRegister8(I2C_NUM_1, 0x34, 0x12, 0x06, ~0);   // LDO2 and DC3 enable (DC3 = LCD BL)
          lgfx::i2c::writeRegister8(I2C_NUM_1, 0x34, 0x96, 2, ~0);      // GPIO4 HIGH (LCD RST)

          releaseBus();
          delay(100);
          _spi_mosi = 23;
          _spi_miso = 38;
          _spi_sclk = 18;
          initBus();

          p_tmp.spi_cs   =  5;
          p_tmp.spi_dc   = 15;
          p_tmp.gpio_rst = -1;
          setPanel(&p_tmp);

          lgfx::lgfxPinMode(4, lgfx::pin_mode_t::output); // TF card CS
          lgfx::gpio_hi(4);

          id = readPanelID();
          ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);

          if ((id & 0xFF) == 0xE3) {   // ILI9342c
            ESP_LOGW("LovyanGFX", "[Autodetect] M5StackCore2");
            board = lgfx::board_t::board_M5StackCore2;
            auto p = new lgfx::Panel_M5StackCore2();
            setPanel(p);

            auto t = new lgfx::Touch_FT5x06();
            t->gpio_int = 39;   // INT pin number
            t->i2c_sda  = 21;   // I2C SDA pin number
            t->i2c_scl  = 22;   // I2C SCL pin number
            t->i2c_addr = 0x38; // I2C device addr
            t->i2c_port = I2C_NUM_1;// I2C port number
            t->freq = 400000;   // I2C freq
            t->x_min = 0;
            t->x_max = 319;
            t->y_min = 0;
            t->y_max = 279;
            touch(t);
            _touch_affine[0] =  0;
            _touch_affine[1] = -1;
            _touch_affine[2] = 240;
            _touch_affine[3] =  1;
            _touch_affine[4] =  0;
            _touch_affine[5] =  0;

            goto init_clear;
          }
        }
        lgfx::gpio_lo(p_tmp.spi_cs);
        lgfx::gpio_lo(p_tmp.spi_dc);
        lgfx::gpio_lo(4);
      }
  #endif

  #if defined ( LGFX_WT32_SC01 )
      if (nvs_board == 0 || nvs_board == lgfx::board_t::board_WT32_SC01) {
        releaseBus();
        _spi_mosi = 13;
        _spi_miso = -1;
        _spi_sclk = 14;
        initBus();

        p_tmp.spi_cs   = 15;
        p_tmp.spi_dc   = 21;
        p_tmp.gpio_rst = -1;
        setPanel(&p_tmp);

        /* if ( determination here ) */ {
          ESP_LOGW("LovyanGFX", "[Autodetect] WT32-SC01");
          board = lgfx::board_t::board_WT32_SC01;
          auto p = new lgfx::Panel_ST7796();
          releaseBus();
          _spi_host = HSPI_HOST;
          initBus();
          p->spi_3wire = true;
          p->spi_read  = false;
          p->spi_cs    = 15;
          p->spi_dc    = 21;
          p->gpio_rst  = 22;
          p->gpio_bl   = 23;
          p->pwm_ch_bl = 7;
          p->rotation  = 1;
          setPanel(p);

          auto t = new lgfx::Touch_FT5x06();
          t->gpio_int = 39;   // INT pin number
          t->i2c_sda  = 18;   // I2C SDA pin number
          t->i2c_scl  = 19;   // I2C SCL pin number
          t->i2c_addr = 0x38; // I2C device addr
          t->i2c_port = I2C_NUM_1;// I2C port number
          t->freq = 400000;   // I2C freq
          t->x_min = 0;
          t->x_max = 319;
          t->y_min = 0;
          t->y_max = 479;
          touch(t);
          _touch_affine[0] =  1;
          _touch_affine[1] =  0;
          _touch_affine[2] =  0;
          _touch_affine[3] =  0;
          _touch_affine[4] =  1;
          _touch_affine[5] =  0;

          goto init_clear;
        }
      }
  #endif

  // DSTIKE D-Duino32XS については読出しが出来ないため無条件設定となる。
  // そのためLGFX_AUTO_DETECTでは機能しないようにしておく。
  #if defined ( LGFX_DDUINO32_XS )
      if (nvs_board == 0 || nvs_board == lgfx::board_t::board_DDUINO32_XS) {
        releaseBus();
        _spi_mosi = 26;
        _spi_miso = -1;
        _spi_sclk = 27;
        initBus();
        {
          ESP_LOGW("LovyanGFX", "[Autodetect] D-Duino32 XS");
          board = lgfx::board_t::board_DDUINO32_XS;
          auto p = new lgfx::Panel_ST7789();
          p->reverse_invert = true;
          p->freq_write = 40000000;
          p->freq_fill  = 40000000;
          p->panel_height = 240;
          p->spi_3wire = true;
          p->spi_read  = false;
          p->spi_cs    = -1;
          p->spi_dc    = 23;
          p->gpio_rst  = 32;
          p->gpio_bl   = 22;
          p->pwm_ch_bl = 7;
          setPanel(p);
          goto init_clear;
        }
      }
  #endif

      releaseBus();
      {
        ESP_LOGW("LovyanGFX", "[Autodetect] detect fail.");
        auto p = new lgfx::PanelIlitekCommon();
        p->panel_width  = 0;
        p->panel_height = 0;
        setPanel(p);
      }
  // LGFX_NON_PANELはパネル無しの環境を対象とする。(M5ATOM等)
  #if defined ( LGFX_NON_PANEL )
      board = lgfx::board_t::board_Non_Panel;
  #else
      board = lgfx::board_t::board_unknown;
  #endif

      goto init_clear;
  init_clear:
      _panel_last = getPanel();

      if (nvs_board != board) {
        if (ESP_OK == nvs_open(NVS_NAME, NVS_READWRITE, &nvs_handle)) {
          ESP_LOGW("LovyanGFX", "[Autodetect] save to NVS : board:%d", board);
          nvs_set_u32(nvs_handle, NVS_KEY, board);
          nvs_close(nvs_handle);
        }
      }
      return (board != lgfx::board_t::board_unknown);
    }

  private:
    lgfx::PanelCommon* _panel_last = nullptr;

    void init_impl(bool use_reset) override
    {
      int retry = 3;
      while (!autodetect(use_reset) && --retry);
      lgfx::LGFX_SPI<lgfx::LGFX_Config>::init_impl(use_reset);
    }

    void _reset(bool use_reset) {
      auto pin = _panel->gpio_rst;
      lgfx::gpio_hi(pin);
      lgfx::lgfxPinMode(pin, lgfx::pin_mode_t::output);
      if (!use_reset) return;
      lgfx::gpio_lo(pin);
      auto time = millis();
      do {
        delay(1);
      } while (millis() - time < 2);
      lgfx::gpio_hi(pin);
      time = millis();
      do {
        delay(1);
      } while (millis() - time < 10);
    }
  };
//----------------------------------------------------------------------------
 }
}

using lgfx::LGFX;

#endif
