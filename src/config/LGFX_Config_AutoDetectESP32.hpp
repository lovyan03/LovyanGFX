#ifndef LOVYANGFX_CONFIG_HPP_
#define LOVYANGFX_CONFIG_HPP_

namespace lgfx
{
  struct LGFX_Config
  {
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int dma_channel = 1;
  };

  struct Panel_Dummy : public Panel_ST7735S
  {
    Panel_Dummy() {
      spi_3wire  = true;
      freq_read  = 8000000;
      panel_width  = 0;
      panel_height = 0;
    }
  };

  struct Panel_TTGO_TWatch : public Panel_ST7789
  {
    Panel_TTGO_TWatch() : Panel_ST7789() {
      freq_write = 80000000;
      freq_read  = 20000000;
      freq_fill  = 80000000;
      panel_height = 240;
      invert    = true;
      spi_3wire = true;
      spi_cs    =  5;
      spi_dc    = 27;
      gpio_bl   = 12;
      pwm_ch_bl = 7;
    }
  };

  struct Panel_TTGO_TWristband : public lgfx::Panel_ST7735S
  {
    Panel_TTGO_TWristband() {
      len_dummy_read_pixel = 17;
      spi_3wire  = true;
      invert     = true;
      spi_cs     =  5;
      spi_dc     = 23;
      gpio_rst   = 26;
      gpio_bl    = 27;
      pwm_ch_bl  = 7;
      panel_width  = 80;
      panel_height = 160;
      offset_x     = 26;
      offset_y     = 1;
      offset_rotation = 2;
    }
  };

  struct Panel_TTGO_TS : public Panel_ST7735S
  {
    Panel_TTGO_TS(void) {
      freq_write = 20000000;
      panel_width  = 128;
      panel_height = 160;
      offset_x     = 2;
      offset_y     = 1;
      offset_rotation = 2;
      rgb_order = true;
      spi_3wire = true;
      spi_cs    = 16;
      spi_dc    = 17;
      gpio_rst  = 9;
      gpio_bl   = 27;
      pwm_ch_bl = 7;
    }
  };

  struct Panel_M5StickC : public Panel_ST7735S
  {
    Panel_M5StickC() {
      spi_3wire  = true;
      invert     = true;
      spi_cs     =  5;
      spi_dc     = 23;
      gpio_rst   = 18;
      panel_width  = 80;
      panel_height = 160;
      offset_x     = 26;
      offset_y     = 1;
      offset_rotation = 2;
    }
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
    Panel_M5StickCPlus() {
      spi_3wire  = true;
      invert     = true;
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
  };

  struct Panel_M5Stack : public Panel_ILI9342
  {
    bool isIPS = false;

    Panel_M5Stack(void) {
      spi_3wire = true;
      spi_cs = 14;
      spi_dc = 27;
      rotation = 1;
      offset_rotation = 3;
      gpio_rst = 33;
      gpio_bl  = 32;
      pwm_ch_bl = 7;
    }

    void init(void) override {
      gpio_lo(gpio_rst);
      lgfxPinMode(gpio_rst, pin_mode_t::input);
      delay(1);
      isIPS = gpio_in(gpio_rst);       // get panel type (IPS or TN)

      Panel_ILI9342::init();
    }

  protected:

    const std::uint8_t* getInvertDisplayCommands(std::uint8_t* buf, bool invert) override {
      if (!isIPS) return Panel_ILI9342::getInvertDisplayCommands(buf, invert);
      this->invert = invert;
      buf[2] = buf[0] = invert ? CommandCommon::INVOFF : CommandCommon::INVON;
      buf[3] = buf[1] = 0;
      buf[4] = CMD::GAMMASET;
      buf[5] = 1;
    //buf[6] = 0x08;  // Gamma set, curve 8
    //buf[6] = 0x04;  // Gamma set, curve 4
      buf[6] = 0x02;  // Gamma set, curve 2
    //buf[6] = 0x01;  // Gamma set, curve 1
      buf[8] = buf[7] = 0xFF;
      return buf;
    }
  };

  struct Panel_ODROID_GO : public Panel_ILI9341
  {
    Panel_ODROID_GO(void) {
      freq_fill  = 80000000;
      spi_3wire = true;
      spi_cs =  5;
      spi_dc = 21;
      rotation = 1;
      gpio_bl = 14;
      pwm_ch_bl = 7;
    }
  };

  struct Panel_DDUINO32_XS : public Panel_ST7789
  {
    Panel_DDUINO32_XS() : Panel_ST7789() {
      freq_write = 40000000;
      freq_fill  = 40000000;
      panel_height = 240;
      invert    = true;
      spi_3wire = true;
      spi_read  = false;
      spi_cs    = -1;
      spi_dc    = 23;
      gpio_rst  = 32;
      gpio_bl   = 22;
      pwm_ch_bl = 7;
    }
  };
}

class LGFX : public lgfx::LGFX_SPI<lgfx::LGFX_Config>
{
public:
  LGFX(void) : lgfx::LGFX_SPI<lgfx::LGFX_Config>()
  {
  }

  enum board_t
  { board_unknown
  , board_M5Stack
  , board_M5StickC
  , board_M5StickCPlus
  , board_TTGO_TS
  , board_TTGO_TWatch
  , board_TTGO_TWristband
  , board_ODROID_GO
  , board_DDUINO32_XS
  , board_ESP_WROVER_KIT
  , board_LoLinD32
  };

  board_t getBoard(void) const { return board; }

  void init(void) override
  {
    static lgfx::Panel_Dummy panel_dummy;
    board = board_unknown;
    std::uint32_t id;


// TTGO T-Watch 判定 (GPIO33を使う判定を先に行うと振動モーターが作動する事に注意)
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_TTGO_TWATCH )
    releaseBus();
    _spi_mosi = 19;
    _spi_miso = -1;
    _spi_sclk = 18;
    initBus();

    panel_dummy.spi_cs   =  5;
    panel_dummy.spi_dc   = 27;
    panel_dummy.gpio_rst = -1;
    setPanel(&panel_dummy);

    id = readPanelID();
    ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);
    if ((id & 0xFF) == 0x85) {  //  check panel (ST7789)
      ESP_LOGW("LovyanGFX", "[Autodetect] TWatch");
      board = board_TTGO_TWatch;
      releaseBus();
      _spi_host = HSPI_HOST;
      initBus();
      static lgfx::Panel_TTGO_TWatch panel;
      setPanel(&panel);
      goto init_clear;
    }
    lgfx::gpio_lo(panel_dummy.spi_cs);
    lgfx::gpio_lo(panel_dummy.spi_dc);
#endif


// TTGO T-Wristband 判定
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_TTGO_TWRISTBAND )
    releaseBus();
    _spi_mosi = 19;
    _spi_miso = -1;
    _spi_sclk = 18;
    initBus();

    panel_dummy.spi_cs   =  5;
    panel_dummy.spi_dc   = 23;
    panel_dummy.gpio_rst = 26;
    setPanel(&panel_dummy);
    _reset();

    id = readPanelID();
    ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);
    if ((id & 0xFF) == 0x7C) {  //  check panel (ST7735)
      ESP_LOGW("LovyanGFX", "[Autodetect] TWristband");
      board = board_TTGO_TWristband;
      static lgfx::Panel_TTGO_TWristband panel;
      setPanel(&panel);
      goto init_clear;
    }
    lgfx::gpio_lo(panel_dummy.spi_cs);
    lgfx::gpio_lo(panel_dummy.spi_dc);
    lgfx::gpio_lo(panel_dummy.gpio_rst);
#endif


// M5Stack/LoLinD32Pro 判定 (GPIO15を使う判定を先に行うとM5GO bottomのLEDが点灯する事に注意)
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK ) || defined ( LGFX_LOLIN_D32_PRO )
    releaseBus();
    _spi_mosi = 23;
    _spi_miso = 19;
    _spi_sclk = 18;
    initBus();

    panel_dummy.spi_cs   = 14;
    panel_dummy.spi_dc   = 27;
    panel_dummy.gpio_rst = 33;
    setPanel(&panel_dummy);
    _reset();

    id = readPanelID();
    ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);

// LoLinD32Pro 判定
 #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_LOLIN_D32_PRO )
    if ((id & 0xFF) == 0x7C) {  //  check panel (ST7735)
      ESP_LOGW("LovyanGFX", "[Autodetect] LoLinD32Pro ST7735");
      board = board_LoLinD32;
      static lgfx::Panel_ST7735S panel;
      panel.panel_width  = 128;
      panel.panel_height = 128;
      panel.memory_width  = 132;
      panel.memory_height = 132;
      panel.offset_x  = 2;
      panel.offset_y  = 1;
      panel.spi_3wire = true;
      panel.spi_cs    = 14;
      panel.spi_dc    = 27;
      panel.gpio_rst  = 33;
      panel.gpio_bl   = 32;
      panel.pwm_ch_bl = 7;
      setPanel(&panel);
      goto init_clear;
    }

    if ((id & 0xFF) == 0x7F) {  //  check panel (ILI9341)
      ESP_LOGW("LovyanGFX", "[Autodetect] LoLinD32Pro ILI9341");
      board = board_LoLinD32;
      static lgfx::Panel_ILI9341 panel;
      panel.spi_3wire = true;
      panel.spi_cs    = 14;
      panel.spi_dc    = 27;
      panel.gpio_rst  = 33;
      panel.gpio_bl   = 32;
      panel.pwm_ch_bl = 7;
      setPanel(&panel);
      goto init_clear;
    }
 #endif

// M5Stack 判定
 #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK )
    if (id != 0 && id != ~0) {   // M5Stack
      ESP_LOGW("LovyanGFX", "[Autodetect] M5Stack");
      board = board_M5Stack;
      static lgfx::Panel_M5Stack panel;
      setPanel(&panel);
      goto init_clear;
    }
 #endif

    lgfx::gpio_lo(panel_dummy.spi_cs);
    lgfx::gpio_lo(panel_dummy.spi_dc);
    lgfx::gpio_lo(panel_dummy.gpio_rst);
#endif


// ODROID_GO 判定 (ボードマネージャでM5StickCを選択していると判定失敗する事に注意)
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_ODROID_GO )
    releaseBus();
    _spi_mosi = 23;
    _spi_miso = 19;
    _spi_sclk = 18;
    initBus();

    panel_dummy.spi_cs   =  5;
    panel_dummy.spi_dc   = 21;
    panel_dummy.gpio_rst = -1;
    setPanel(&panel_dummy);

    id = readPanelID();
    if (id == 0 && readCommand32(0x09) != 0) {   // ODROID_GOはpanelIDが0なのでステータスリードを併用する
      ESP_LOGW("LovyanGFX", "[Autodetect] ODROID_GO");
      board = board_ODROID_GO;
      static lgfx::Panel_ODROID_GO panel;
      setPanel(&panel);
      goto init_clear;
    }
    lgfx::gpio_lo(panel_dummy.spi_cs);
    lgfx::gpio_lo(panel_dummy.spi_dc);
#endif

// M5StickC / CPlus 判定
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STICKC )
    releaseBus();
    _spi_mosi = 15;
    _spi_miso = 14;
    _spi_sclk = 13;
    initBus();

    panel_dummy.spi_cs   =  5;
    panel_dummy.spi_dc   = 23;
    panel_dummy.gpio_rst = 18;
    setPanel(&panel_dummy);
    _reset();

    id = readPanelID();
    ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);
    if ((id & 0xFF) == 0x85) {  //  check panel (ST7789)
      ESP_LOGW("LovyanGFX", "[Autodetect] M5StickCPlus");
      board = board_M5StickCPlus;
      static lgfx::Panel_M5StickCPlus panel;
      setPanel(&panel);
      goto init_clear;
    }

    if ((id & 0xFF) == 0x7C) {  //  check panel (ST7735)
      ESP_LOGW("LovyanGFX", "[Autodetect] M5StickC");
      board = board_M5StickC;
      static lgfx::Panel_M5StickC panel;
      setPanel(&panel);
      goto init_clear;
    }
    lgfx::gpio_lo(panel_dummy.spi_cs);
    lgfx::gpio_lo(panel_dummy.spi_dc);
    lgfx::gpio_lo(panel_dummy.gpio_rst);
#endif

// ESP-WROVER-KIT 判定
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_ESP_WROVER_KIT )
    releaseBus();
    _spi_mosi = 23;
    _spi_miso = 25;
    _spi_sclk = 19;
    initBus();

    panel_dummy.spi_3wire = false;
    panel_dummy.spi_cs   = 22;
    panel_dummy.spi_dc   = 21;
    panel_dummy.gpio_rst = 18;
    setPanel(&panel_dummy);
    _reset();

    id = readPanelID();
    ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);
    if ((id & 0xFF) == 0x85) {  //  check panel (ST7789)
      ESP_LOGW("LovyanGFX", "[Autodetect] ESP-WROVER-KIT ST7789");
      board = board_ESP_WROVER_KIT;
      static lgfx::Panel_ST7789 panel;
      panel.spi_3wire = false;
      panel.spi_cs   = 22;
      panel.spi_dc   = 21;
      panel.gpio_rst = 18;
      panel.gpio_bl  = 5;
      panel.pwm_ch_bl = 7;
      panel.freq_write = 80000000;
      panel.freq_read  = 16000000;
      panel.freq_fill  = 80000000;
      panel.backlight_level = false;
      panel.offset_rotation = 2;
      panel.spi_mode_read = 1;
      panel.len_dummy_read_pixel = 16;

      setPanel(&panel);
      goto init_clear;
    }

    if (id == 0 && readCommand32(0x09) != 0) {   // ILI9341モデルはpanelIDが0なのでステータスリードを併用する
      ESP_LOGW("LovyanGFX", "[Autodetect] ESP-WROVER-KIT ILI9341");
      board = board_ESP_WROVER_KIT;
      static lgfx::Panel_ILI9341 panel;
      panel.spi_3wire = false;
      panel.spi_cs   = 22;
      panel.spi_dc   = 21;
      panel.gpio_rst = 18;
      panel.gpio_bl  = 5;
      panel.pwm_ch_bl = 7;
      panel.freq_write = 40000000;
      panel.freq_read  = 20000000;
      panel.freq_fill  = 80000000;
      panel.backlight_level = false;

      setPanel(&panel);
      goto init_clear;
    }
    lgfx::gpio_lo(panel_dummy.spi_cs);
    lgfx::gpio_lo(panel_dummy.spi_dc);
    lgfx::gpio_lo(panel_dummy.gpio_rst);
    panel_dummy.spi_3wire = true;
#endif



#if defined ( LGFX_TTGO_TS )
    releaseBus();
    _spi_mosi = 23;
    _spi_miso = -1;
    _spi_sclk =  5;
    initBus();

    panel_dummy.spi_cs   = 16;
    panel_dummy.spi_dc   = 17;
    panel_dummy.gpio_rst =  9;
    setPanel(&panel_dummy);

    id = readPanelID();
    ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);
    if (id != 0 && id != ~0) {
      ESP_LOGW("LovyanGFX", "[Autodetect] TTGO TS");
      board = board_TTGO_TS;
      static lgfx::Panel_TTGO_TS panel;
      setPanel(&panel);
      goto init_clear;
    }
#endif

#if defined ( LGFX_DDUINO32_XS )
    releaseBus();
    _spi_mosi = 26;
    _spi_miso = -1;
    _spi_sclk = 27;
    initBus();
    {
      ESP_LOGW("LovyanGFX", "[Autodetect] D-Duino32 XS");
      board = board_DDUINO32_XS;
      static lgfx::Panel_DDUINO32_XS panel;
      setPanel(&panel);
      goto init_clear;
    }
#endif

    releaseBus();

    ESP_LOGW("LovyanGFX", "[Autodetect] detect fail.");
    return;

init_clear:
    initPanel();
    startWrite();
    clear();
    setWindow(0,0,0,0);
    endWrite();
  }

private:
  board_t board = board_unknown;

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
