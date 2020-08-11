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

    auto panel = new lgfx::PanelIlitekCommon();
    panel->spi_3wire  = true;
    panel->freq_read  = 8000000;
    panel->panel_width  = 0;
    panel->panel_height = 0;
    board = board_unknown;
    std::uint32_t id;


// TTGO T-Watch 判定 (GPIO33を使う判定を先に行うと振動モーターが作動する事に注意)
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_TTGO_TWATCH )
    releaseBus();
    _spi_mosi = 19;
    _spi_miso = -1;
    _spi_sclk = 18;
    initBus();

    panel->spi_cs   =  5;
    panel->spi_dc   = 27;
    panel->gpio_rst = -1;
    setPanel(panel);

    id = readPanelID();
    ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);
    if ((id & 0xFF) == 0x85) {  //  check panel (ST7789)
      ESP_LOGW("LovyanGFX", "[Autodetect] TWatch");
      board = board_TTGO_TWatch;
      releaseBus();
      _spi_host = HSPI_HOST;
      initBus();
      auto p = new lgfx::Panel_ST7789();
      p->freq_write = 80000000;
      p->freq_read  = 20000000;
      p->freq_fill  = 80000000;
      p->panel_height = 240;
      p->invert    = true;
      p->spi_3wire = true;
      p->spi_cs    =  5;
      p->spi_dc    = 27;
      p->gpio_bl   = 12;
      p->pwm_ch_bl = 7;

      setPanel(p);
      goto init_clear;
    }
    lgfx::gpio_lo(panel->spi_cs);
    lgfx::gpio_lo(panel->spi_dc);
#endif


// TTGO T-Wristband 判定
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_TTGO_TWRISTBAND )
    releaseBus();
    _spi_mosi = 19;
    _spi_miso = -1;
    _spi_sclk = 18;
    initBus();

    panel->spi_cs   =  5;
    panel->spi_dc   = 23;
    panel->gpio_rst = 26;
    setPanel(panel);
    _reset();

    id = readPanelID();
    ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);
    if ((id & 0xFF) == 0x7C) {  //  check panel (ST7735)
      ESP_LOGW("LovyanGFX", "[Autodetect] TWristband");
      board = board_TTGO_TWristband;
      auto p = new lgfx::Panel_ST7735S();
      p->len_dummy_read_pixel = 17;
      p->spi_3wire  = true;
      p->invert     = true;
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
    lgfx::gpio_lo(panel->spi_cs);
    lgfx::gpio_lo(panel->spi_dc);
    lgfx::gpio_lo(panel->gpio_rst);
#endif


// M5Stack/LoLinD32Pro 判定 (GPIO15を使う判定を先に行うとM5GO bottomのLEDが点灯する事に注意)
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK ) || defined ( LGFX_LOLIN_D32_PRO )
    releaseBus();
    _spi_mosi = 23;
    _spi_miso = 19;
    _spi_sclk = 18;
    initBus();

    panel->spi_cs   = 14;
    panel->spi_dc   = 27;
    panel->gpio_rst = 33;
    setPanel(panel);
    _reset();

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
    id = readPanelID();

    ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);

// LoLinD32Pro 判定
 #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_LOLIN_D32_PRO )
    if ((id & 0xFF) == 0x7C) {  //  check panel (ST7735)
 #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK )
      lgfx::gpio_lo(4);
 #endif
      ESP_LOGW("LovyanGFX", "[Autodetect] LoLinD32Pro ST7735");
      board = board_LoLinD32;
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

    if ((id & 0xFF) == 0x7F) {  //  check panel (ILI9341)
 #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK )
      lgfx::gpio_lo(4);
 #endif
      ESP_LOGW("LovyanGFX", "[Autodetect] LoLinD32Pro ILI9341");
      board = board_LoLinD32;
      auto p = new lgfx::Panel_ILI9341();
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
    if (id != 0 && id != ~0) {   // M5Stack
 #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_LOLIN_D32_PRO )
      lgfx::gpio_lo(5);
      lgfx::gpio_lo(12);
 #endif
      ESP_LOGW("LovyanGFX", "[Autodetect] M5Stack");
      board = board_M5Stack;
      auto p = new lgfx::Panel_M5Stack();
      setPanel(p);
      goto init_clear;
    }
 #endif

    lgfx::gpio_lo(panel->spi_cs);
    lgfx::gpio_lo(panel->spi_dc);
    lgfx::gpio_lo(panel->gpio_rst);
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

    panel->spi_cs   =  5;
    panel->spi_dc   = 21;
    panel->gpio_rst = -1;
    setPanel(panel);

    lgfx::lgfxPinMode(22, lgfx::pin_mode_t::output); // ODROID-GO TF card CS
    lgfx::gpio_hi(22);

    id = readPanelID();
    if (id == 0 && readCommand32(0x09) != 0) {   // ODROID_GOはpanelIDが0なのでステータスリードを併用する
      ESP_LOGW("LovyanGFX", "[Autodetect] ODROID_GO");
      board = board_ODROID_GO;
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
    lgfx::gpio_lo(panel->spi_cs);
    lgfx::gpio_lo(panel->spi_dc);
    lgfx::gpio_lo(22);
#endif

// M5StickC / CPlus 判定
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STICKC )
    releaseBus();
    _spi_mosi = 15;
    _spi_miso = 14;
    _spi_sclk = 13;
    initBus();

    panel->spi_cs   =  5;
    panel->spi_dc   = 23;
    panel->gpio_rst = 18;
    setPanel(panel);
    _reset();

    id = readPanelID();
    ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);
    if ((id & 0xFF) == 0x85) {  //  check panel (ST7789)
      ESP_LOGW("LovyanGFX", "[Autodetect] M5StickCPlus");
      board = board_M5StickCPlus;
      auto p = new lgfx::Panel_M5StickCPlus();
      setPanel(p);
      goto init_clear;
    }

    if ((id & 0xFF) == 0x7C) {  //  check panel (ST7735)
      ESP_LOGW("LovyanGFX", "[Autodetect] M5StickC");
      board = board_M5StickC;
      auto p = new lgfx::Panel_M5StickC();
      setPanel(p);
      goto init_clear;
    }
    lgfx::gpio_lo(panel->spi_cs);
    lgfx::gpio_lo(panel->spi_dc);
    lgfx::gpio_lo(panel->gpio_rst);
#endif

// ESP-WROVER-KIT 判定
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_ESP_WROVER_KIT )
    releaseBus();
    _spi_mosi = 23;
    _spi_miso = 25;
    _spi_sclk = 19;
    initBus();

    panel->spi_3wire = false;
    panel->spi_cs   = 22;
    panel->spi_dc   = 21;
    panel->gpio_rst = 18;
    setPanel(panel);
    _reset();

    id = readPanelID();
    ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);
    if ((id & 0xFF) == 0x85) {  //  check panel (ST7789)
      ESP_LOGW("LovyanGFX", "[Autodetect] ESP-WROVER-KIT ST7789");
      board = board_ESP_WROVER_KIT;
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

    if (id == 0 && readCommand32(0x09) != 0) {   // ILI9341モデルはpanelIDが0なのでステータスリードを併用する
      ESP_LOGW("LovyanGFX", "[Autodetect] ESP-WROVER-KIT ILI9341");
      board = board_ESP_WROVER_KIT;
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
    lgfx::gpio_lo(panel->spi_cs);
    lgfx::gpio_lo(panel->spi_dc);
    lgfx::gpio_lo(panel->gpio_rst);
    panel->spi_3wire = true;
#endif

// TTGO TS判定
#if defined ( LGFX_AUTODETECT ) || defined ( LGFX_TTGO_TS )
    releaseBus();
    _spi_mosi = 23;
    _spi_miso = -1;
    _spi_sclk =  5;
    initBus();

    panel->spi_cs   = 16;
    panel->spi_dc   = 17;
    panel->gpio_rst =  9;
    setPanel(panel);
    _reset();

    id = readPanelID();
    ESP_LOGW("LovyanGFX", "[Autodetect] panel id:%08x", id);
    if ((id & 0xFF) == 0x7C) {  //  check panel (ST7735)
      ESP_LOGW("LovyanGFX", "[Autodetect] TTGO TS");
      board = board_TTGO_TS;
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
    lgfx::gpio_lo(panel->spi_cs);
    lgfx::gpio_lo(panel->spi_dc);
    lgfx::gpio_lo(panel->gpio_rst);
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
      board = board_DDUINO32_XS;
      auto p = new lgfx::Panel_ST7789();
      p->freq_write = 40000000;
      p->freq_fill  = 40000000;
      p->panel_height = 240;
      p->invert    = true;
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
    panel_last = getPanel();
    return;

init_clear:
    delete panel;
    panel_last = getPanel();
    initPanel();
    startWrite();
    clear();
    setWindow(0,0,0,0);
    endWrite();
  }

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
