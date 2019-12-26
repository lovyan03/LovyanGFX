
#include <SD.h>
#include <LovyanGFX.hpp>

/*
#include <M5Stack.h>

static M5Display &tft (M5.Lcd);
static TFT_eSprite sprite(&tft);
*/

struct LGFX_Config
{
  static constexpr spi_host_device_t spi_host = VSPI_HOST;
  static constexpr int spi_mosi = 23;
  static constexpr int spi_miso = 19;
  static constexpr int spi_sclk = 18;
  static constexpr int spi_cs   = 14;
  static constexpr int spi_dc   = 27;
  static constexpr int panel_rst = 33;
  static constexpr int panel_bl  = 32;
  static constexpr int freq_write = 40000000;
  static constexpr int freq_read  = 16000000;
  static constexpr int freq_fill  = 40000000;
  static constexpr bool spi_half_duplex = true;
  const lgfx::Panel_M5Stack panel;
};

static LGFX<LGFX_Config> tft;
static LGFXSprite sprite;

void setup()
{
  bool lcdver = false;
#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE)
  pinMode(LGFX_Config::panel_rst, INPUT);
  delay(100);
  lcdver = digitalRead(LGFX_Config::panel_rst);
#endif

  Serial.begin(115200);
  SD.begin(4, SPI, 20000000);  // 4=TFCARD_CS_PIN

  tft.init();
  tft.invertDisplay(lcdver);
  tft.setRotation(1);

  const int BLK_PWM_CHANNEL = 7;
  ledcSetup(BLK_PWM_CHANNEL, 12000, 8);
  ledcAttachPin(LGFX_Config::panel_bl, BLK_PWM_CHANNEL);
  ledcWrite(BLK_PWM_CHANNEL, 128);

// sprite使う例
//  sprite.createSprite(100,100);
//  sprite.drawBmpFile(SD, "/m5stack_24bpp.bmp", 0, 0);
}

void loop() {
  tft.setColorDepth( ( tft.getColorDepth()==24 ) ? 16 : 24);
  tft.drawBmpFile(SD, "/m5stack_mono.bmp",      0,  10);
  tft.drawBmpFile(SD, "/m5stack_16color.bmp", 110,  10);
  tft.drawBmpFile(SD, "/m5stack_256color.bmp",220,  10);
  tft.drawBmpFile(SD, "/m5stack_16bpp.bmp",     0, 130);
  tft.drawBmpFile(SD, "/m5stack_24bpp.bmp",   110, 130);
  tft.drawBmpFile(SD, "/m5stack_32bpp.bmp",   220, 130);
}

/*

  // sprite使う例
void loop_sprite() {
  uint8_t* spbuf = sprite.getDevice()->buffer(); // (機能未実装のため暫定措置)
  tft.pushImage(random(-70, 300), random(-70, 200), 100, 100, spbuf);
}
*/
