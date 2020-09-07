#ifndef LOVYANGFX_CONFIG_HPP_
#define LOVYANGFX_CONFIG_HPP_

namespace lgfx
{
  struct LGFX_Config
  {
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int dma_channel = 1;
  };

  struct Panel_M5StickC : public Panel_ST7735S
  {
  private:

    static constexpr std::int32_t freq = 400000;
    static constexpr std::uint_fast8_t axp_i2c_addr = 0x34;
    static constexpr std::int_fast16_t axp_i2c_port = I2C_NUM_1;
    static constexpr std::int_fast16_t axp_i2c_sda = 21;
    static constexpr std::int_fast16_t axp_i2c_scl = 22;

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

    void init(void) override
    {
      lgfx::i2c::init(axp_i2c_port, axp_i2c_sda, axp_i2c_scl, freq);

      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x12, 0x4D, ~0);

      Panel_ST7735S::init();
    }

    void setBrightness(std::uint8_t brightness) override
    {
      this->brightness = brightness;
      brightness = (((brightness >> 1) + 8) / 13) + 5;
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x28, brightness << 4, 0x0F);
    }

    void sleep(void) override { lgfx::i2c::bitOff(axp_i2c_port, axp_i2c_addr, 0x12, 1 << 2); }

    void wakeup(void) override { lgfx::i2c::bitOn(axp_i2c_port, axp_i2c_addr, 0x12, 1 << 2); }

  protected:
    const std::uint8_t* getInitCommands(std::uint8_t listno) const override {
      static constexpr std::uint8_t list[] = {
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

    static constexpr std::int32_t freq = 400000;
    static constexpr std::uint_fast8_t axp_i2c_addr = 0x34;
    static constexpr std::int_fast16_t axp_i2c_port = I2C_NUM_1;
    static constexpr std::int_fast16_t axp_i2c_sda = 21;
    static constexpr std::int_fast16_t axp_i2c_scl = 22;

  public:

    Panel_M5StickCPlus() {
      reverse_invert = true;
      spi_3wire  = true;
      spi_cs     =  5;
      spi_dc     = 23;
      gpio_rst   = 18;
      freq_write = 80000000;
      freq_read  = 16000000;
      freq_fill  = 80000000;
      spi_mode_read = 1;
      len_dummy_read_pixel = 16;
      panel_width  = 135;
      panel_height = 240;
      offset_x = 52;
      offset_y = 40;
    }

    void init(void) override
    {
      lgfx::i2c::init(axp_i2c_port, axp_i2c_sda, axp_i2c_scl, freq);

      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x12, 0x4D, ~0);

      Panel_ST7789::init();
    }

    void setBrightness(std::uint8_t brightness) override
    {
      this->brightness = brightness;
      brightness = (((brightness >> 1) + 8) / 13) + 5;
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x28, brightness << 4, 0x0F);
    }

    void sleep(void) override { lgfx::i2c::bitOff(axp_i2c_port, axp_i2c_addr, 0x12, 1 << 2); }

    void wakeup(void) override { lgfx::i2c::bitOn(axp_i2c_port, axp_i2c_addr, 0x12, 1 << 2); }
  };

  struct Panel_M5Stack : public Panel_ILI9342
  {
    Panel_M5Stack(void) {
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

    void init(void) override {
      gpio_lo(gpio_rst);
      lgfxPinMode(gpio_rst, pin_mode_t::input);
      delay(1);
      reverse_invert = gpio_in(gpio_rst);       // get panel type (IPS or TN)

      Panel_ILI9342::init();
    }
  };

  struct Panel_M5StackCore2 : public Panel_ILI9342
  {
  private:

    static constexpr std::int32_t freq = 400000;
    static constexpr std::uint_fast8_t axp_i2c_addr = 0x34;
    static constexpr std::int_fast16_t axp_i2c_port = I2C_NUM_1;
    static constexpr std::int_fast16_t axp_i2c_sda = 21;
    static constexpr std::int_fast16_t axp_i2c_scl = 22;

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
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x96, 0, ~0x02);
      delay(10);
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x96, 2, ~0);
    }

    void init(void) override
    {
      resetPanel();

      Panel_ILI9342::init();
    }

    void setBrightness(std::uint8_t brightness) override
    {
      this->brightness = brightness;
      brightness = (brightness >> 3) + 72;
      // AXP192 reg 0x27 = DC3
      lgfx::i2c::writeRegister8(axp_i2c_port, axp_i2c_addr, 0x27, brightness, 0x80);
    }

    void sleep(void) override { lgfx::i2c::bitOff(axp_i2c_port, axp_i2c_addr, 0x12, 0x02); } // DC3 disable

    void wakeup(void) override { lgfx::i2c::bitOn(axp_i2c_port, axp_i2c_addr, 0x12, 0x02); } // DC3 enable

  };
}

class LGFX : public lgfx::LGFX_SPI<lgfx::LGFX_Config>
{
public:
  LGFX(void) : lgfx::LGFX_SPI<lgfx::LGFX_Config>()
  {
  }

private:
  void init_impl(void) override
  {
    if (_spi_mosi != -1 && _spi_sclk != -1) {
      lgfx::LGFX_SPI<lgfx::LGFX_Config>::init_impl();
      return;
    }

    static lgfx::PanelCommon* panel_last = nullptr;
    if (panel_last) {
      delete panel_last;
      panel_last = nullptr;
    }

    lgfx::PanelIlitekCommon p_tmp;
    p_tmp.spi_3wire  = true;
    p_tmp.freq_read  = 8000000;
    board = lgfx::board_t::board_unknown;
    std::uint32_t id;
    (void)id; // Suppressing Compiler Warnings

// TTGO T-Watch 判定 (GPIO33を使う判定を先に行うと振動モーターが作動する事に注意)
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_TTGO_TWATCH )
    releaseBus();
    _spi_mosi = 19;
    _spi_miso = -1;
    _spi_sclk = 18;
    initBus();

    p_tmp.spi_cs   =  5;
    p_tmp.spi_dc   = 27;
    p_tmp.gpio_rst = -1;
    setPanel(&p_tmp);

    id = readPanelID();
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
#endif


// TTGO T-Wristband 判定
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_TTGO_TWRISTBAND )
    releaseBus();
    _spi_mosi = 19;
    _spi_miso = -1;
    _spi_sclk = 18;
    initBus();

    p_tmp.spi_cs   =  5;
    p_tmp.spi_dc   = 23;
    p_tmp.gpio_rst = 26;
    setPanel(&p_tmp);
    _reset();

    id = readPanelID();
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
#endif


// M5Stack/LoLinD32Pro 判定 (GPIO15を使う判定を先に行うとM5GO bottomのLEDが点灯する事に注意)
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK ) || defined ( LGFX_LOLIN_D32_PRO )
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

    p_tmp.spi_3wire = false;
    setPanel(&p_tmp);
    _reset();

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

 #endif

    setPanel(&p_tmp);
    _reset();

    id = readPanelID();

    ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);

// LoLinD32Pro (ST7735) 判定
 #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_LOLIN_D32_PRO )

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

 #endif

// M5Stack 判定
 #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK )
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

    lgfx::gpio_lo(p_tmp.spi_cs);
    lgfx::gpio_lo(p_tmp.spi_dc);
    lgfx::gpio_lo(p_tmp.gpio_rst);
    lgfx::gpio_lo(4);
    lgfx::gpio_lo(5);
    lgfx::gpio_lo(12);
#endif

// ODROID_GO 判定 (ボードマネージャでM5StickCを選択していると判定失敗する事に注意)
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_ODROID_GO )
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
#endif

// M5StickC / CPlus 判定
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STICKC )
    releaseBus();
    _spi_mosi = 15;
    _spi_miso = 14;
    _spi_sclk = 13;
    initBus();

    p_tmp.spi_cs   =  5;
    p_tmp.spi_dc   = 23;
    p_tmp.gpio_rst = 18;
    setPanel(&p_tmp);
    _reset();

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
#endif

// ESP-WROVER-KIT 判定
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_ESP_WROVER_KIT )
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
    _reset();

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
#endif

// TTGO TS判定
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_TTGO_TS )
    releaseBus();
    _spi_mosi = 23;
    _spi_miso = -1;
    _spi_sclk =  5;
    initBus();

    p_tmp.spi_cs   = 16;
    p_tmp.spi_dc   = 17;
    p_tmp.gpio_rst =  9;
    setPanel(&p_tmp);
    _reset();

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
#endif


// M5StackCore2 判定
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK )
    lgfx::i2c::init(I2C_NUM_1, 21, 22, 400000);
    if (lgfx::i2c::writeRegister8(I2C_NUM_1, 0x34, 0x96, 0, ~0x02)) { // GPIO4 LOW (LCD RST)
      // AXP192_LDO2 = LCD PWR
      // AXP192_DC3  = LCD BL
      // AXP192_IO4  = LCD RST
      lgfx::i2c::writeRegister8(I2C_NUM_1, 0x34, 0x95, 0x84, 0x72); // GPIO4 enable
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
        t->y_max = 319;
        touch(t);
        _touch_affin[0] =  0;
        _touch_affin[1] = -1;
        _touch_affin[2] = 240;
        _touch_affin[3] =  1;
        _touch_affin[4] =  0;
        _touch_affin[5] =  0;

        goto init_clear;
      }
      lgfx::gpio_lo(p_tmp.spi_cs);
      lgfx::gpio_lo(p_tmp.spi_dc);
      lgfx::gpio_lo(4);
    }
#endif


// DSTIKE D-Duino32XS については読出しが出来ないため無条件設定となる。
// そのためLGFX_AUTO_DETECTでは機能しないようにしておく。
#if defined ( LGFX_DDUINO32_XS )
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
#endif

    releaseBus();

    ESP_LOGW("LovyanGFX", "[Autodetect] detect fail.");
    panel_last = new lgfx::PanelIlitekCommon();
    panel_last->panel_width  = 0;
    panel_last->panel_height = 0;
    return;

    goto init_clear;
init_clear:
    panel_last = getPanel();
    lgfx::LGFX_SPI<lgfx::LGFX_Config>::init_impl();
  }

  void _reset(void) {
    auto pin = _panel->gpio_rst;
    lgfx::lgfxPinMode(pin, lgfx::pin_mode_t::output);
    lgfx::gpio_lo(pin);
    delay(1);
    lgfx::gpio_hi(pin);
    delay(10);
  }
};

#endif
