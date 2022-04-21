#if !defined LGFX_AUTODETECT
  #define LGFX_AUTODETECT
#endif

#if !defined LGFX_USE_V1
  #define LGFX_USE_V1
#endif

#if defined ( ARDUINO ) && defined ( ESP32 )
  #include <SPIFFS.h>
#endif

#include <LovyanGFX.hpp>

#if defined DUMMY_DISPLAY
  // AUTODETECT will fail, so let's build a dummy object
  // with arbitraty display for the sakes of testing
  class LGFX : public lgfx::LGFX_Device
  {
    lgfx::Panel_ST7735S _panel_instance;
    lgfx::Bus_SPI       _bus_instance;

   public:
    LGFX(void)
    {
      {
        auto cfg = _bus_instance.config();
        _bus_instance.config(cfg);
        _panel_instance.setBus(&_bus_instance);
      }
      {
        auto cfg = _panel_instance.config();
        _panel_instance.config(cfg);
      }
      setPanel(&_panel_instance);
    }
  };
#endif

  class LGFX_I2C : public lgfx::LGFX_Device
  {
    lgfx::Bus_I2C       _bus_instance;
    lgfx::Panel_SSD1306 _panel_instance;
    lgfx::Touch_FT5x06  _touch_instance;

   public:
    LGFX_I2C(void)
    {
      {
        auto cfg = _bus_instance.config();
        _bus_instance.config(cfg);
        _panel_instance.setBus(&_bus_instance);
      }
      {
        auto cfg = _panel_instance.config();
        _panel_instance.config(cfg);
      }
      {
        auto cfg = _touch_instance.config();
        _touch_instance.config(cfg);
        _panel_instance.setTouch(&_touch_instance);
      }
      setPanel(&_panel_instance);
    }
  };


static LGFX display1;
static LGFX_I2C display2;
static LGFX_Sprite sprite(&display1);

void test(LGFX_Device &lcd)
{
  lcd.init();
  lcd.setRotation(1);
  lcd.setBrightness(128);
  lcd.setColorDepth(24);
  lcd.drawPixel(0, 0, 0xFFFF);
  lcd.drawFastVLine(2, 0, 100, lcd.color888(255,   0,   0));
  lcd.drawFastVLine(4, 0, 100, lcd.color565(  0, 255,   0));
  lcd.drawFastVLine(6, 0, 100, lcd.color332(  0,   0, 255));
  uint32_t red = 0xFF0000;
  lcd.drawFastHLine(0, 2, 100, red);
  lcd.drawFastHLine(0, 4, 100, 0x00FF00U);
  lcd.drawFastHLine(0, 6, 100, (uint32_t)0xFF);
  uint16_t green = 0x07E0;
  lcd.drawRect(10, 10, 50, 50, 0xF800);
  lcd.drawRect(12, 12, 50, 50, green);
  lcd.drawRect(14, 14, 50, 50, (uint16_t)0x1F);
  uint8_t blue = 0x03;
  lcd.fillRect(20, 20, 20, 20, (uint8_t)0xE0);
  lcd.fillRect(30, 30, 20, 20, (uint8_t)0x1C);
  lcd.fillRect(40, 40, 20, 20, blue);
  lcd.setColor(0xFF0000U);
  lcd.fillCircle ( 40, 80, 20    );
  lcd.fillEllipse( 80, 40, 10, 20);
  lcd.fillArc    ( 80, 80, 20, 10, 0, 90);
  lcd.fillTriangle(80, 80, 60, 80, 80, 60);
  lcd.setColor(0x0000FFU);
  lcd.drawCircle ( 40, 80, 20    );
  lcd.drawEllipse( 80, 40, 10, 20);
  lcd.drawArc    ( 80, 80, 20, 10, 0, 90);
  lcd.drawTriangle(60, 80, 80, 80, 80, 60);
  lcd.setColor(0x00FF00U);
  lcd.drawBezier( 60, 80, 80, 80, 80, 60);
  lcd.drawBezier( 60, 80, 80, 20, 20, 80, 80, 60);
  lcd.drawGradientLine( 0, 80, 80, 0, 0xFF0000U, 0x0000FFU);

  lcd.fillScreen(0xFFFFFFu);
  lcd.setColor(0x00FF00u);
  lcd.fillScreen();
  lcd.clear(0xFFFFFFu);
  lcd.setBaseColor(0x000000u);
  lcd.clear();
  lcd.drawLine(0, 1, 39, 40, red);
  lcd.drawLine(1, 0, 40, 39, blue);
  lcd.startWrite();
  lcd.drawLine(38, 0, 0, 38, 0xFFFF00U);
  lcd.drawLine(39, 1, 1, 39, 0xFF00FFU);
  lcd.drawLine(40, 2, 2, 40, 0x00FFFFU);
  lcd.endWrite();
  lcd.startWrite();
  lcd.startWrite();
  lcd.startWrite();
  lcd.endWrite();
  lcd.endWrite();
  lcd.endWrite();
  lcd.endWrite();
  lcd.startWrite();
  lcd.startWrite();
  lcd.drawPixel(0, 0);
  lcd.endTransaction();
  lcd.beginTransaction();
  lcd.drawPixel(0, 0);
  lcd.endWrite();
  lcd.endWrite();
  lcd.startWrite();
  for (uint32_t x = 0; x < 128; ++x) {
    for (uint32_t y = 0; y < 128; ++y) {
      lcd.writePixel(x, y, lcd.color888( x*2, x + y, y*2));
    }
  }
  lcd.endWrite();

  sprite.setColorDepth(24);
  sprite.createSprite(65, 65);

  for (uint32_t x = 0; x < 64; ++x) {
    for (uint32_t y = 0; y < 64; ++y) {
      sprite.drawPixel(x, y, lcd.color888(3 + x*4, (x + y)*2, 3 + y*4));
    }
  }
  sprite.drawRect(0, 0, 65, 65, 0xFFFF);
  sprite.pushSprite(64, 0);
  sprite.pushSprite(&lcd, 0, 64);


  sprite.setPivot(32, 32);
  int32_t center_x = lcd.width()/2;
  int32_t center_y = lcd.height()/2;
  lcd.startWrite();
  for (int angle = 0; angle <= 360; ++angle) {
    sprite.pushRotateZoom(center_x, center_y, angle, 2.5, 3);
    if ((angle % 36) == 0) lcd.display();
  }
  lcd.endWrite();

  sprite.deleteSprite();

  sprite.setColorDepth(4);
  sprite.createSprite(65, 65);
  sprite.setPaletteColor(1, 0x0000FFU);
  sprite.setPaletteColor(2, 0x00FF00U);
  sprite.setPaletteColor(3, 0xFF0000U);

  sprite.fillRect(10, 10, 45, 45, 1);
  sprite.fillCircle(32, 32, 22, 2);
  sprite.fillTriangle(32, 12, 15, 43, 49, 43, 3);

  sprite.pushSprite(&lcd, 0,  0, 0);

  lcd.drawBmp((uint8_t*)nullptr, 0, 0, 0);
  lcd.drawPng((uint8_t*)nullptr, 0, 0, 0);
  lcd.drawJpg((uint8_t*)nullptr, 0, 0, 0);
  lcd.drawQoi((uint8_t*)nullptr, 0, 0, 0);

#if defined ( ARDUINO ) && defined ( ESP32 )
  lcd.drawBmpFile(SPIFFS, "/test.bmp");
  lcd.drawPngFile(SPIFFS, "/test.png");
  lcd.drawJpgFile(SPIFFS, "/test.jpg");
  lcd.drawQoiFile(SPIFFS, "/test.qoi");
#endif
}

void setup()
{
  test(display1);
  test(display2);
}

void loop(void)
{
  lgfx::delay(1000);
}



#if !defined ARDUINO
extern "C" {
  void loopTask(void*)
  {
    setup();
    for(;;) {
      loop();
    }
  }
  void app_main()
  {
    xTaskCreatePinnedToCore( loopTask, "loopTask", 8192, NULL, 1, NULL, 1 );
  }

}
#endif

