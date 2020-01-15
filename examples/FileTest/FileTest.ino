
#include <SD.h>

#include <LGFX_TFT_eSPI.hpp>

/*
#include <M5Stack.h>

static M5Display &tft (M5.Lcd);
static TFT_eSprite sprite(&tft);
*/

static TFT_eSPI tft;
static TFT_eSprite sprite(&tft);

void setup()
{
  bool lcdver = false;
#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE)
  pinMode(33, INPUT);
  delay(100);
  lcdver = digitalRead(33);
  pinMode(33, OUTPUT);
  digitalWrite(33, HIGH);
#endif

  Serial.begin(115200);
  SD.begin(4, SPI, 20000000);  // 4=TFCARD_CS_PIN

  //sprite.setPSRAM(true); // PSRAM使う場合
  sprite.setColorDepth(8); // 8 , 16 , 24
  sprite.createSprite(320,240);

  sprite.drawBmpFile(SD, "/m5stack_mono.bmp",      0,  10);
  sprite.drawBmpFile(SD, "/m5stack_16color.bmp", 110,  10);
  sprite.drawBmpFile(SD, "/m5stack_256color.bmp",220,  10);

  tft.initPanel();
  tft.invertDisplay(lcdver);
  tft.setRotation(1);

  const int BLK_PWM_CHANNEL = 7;
  ledcSetup(BLK_PWM_CHANNEL, 12000, 8);
  ledcAttachPin(32, BLK_PWM_CHANNEL);
  ledcWrite(BLK_PWM_CHANNEL, 128);


  sprite.drawBmpFile(SD, "/m5stack_16bpp.bmp",     0, 130);
  sprite.drawBmpFile(SD, "/m5stack_24bpp.bmp",   110, 130);
  sprite.drawBmpFile(SD, "/m5stack_32bpp.bmp",   220, 130);

  tft.fillRect(  0, 10,50,50,0x8410);
  tft.fillRect(110, 10,50,50,0x8410);
  tft.fillRect(220, 10,50,50,0x8410);
  tft.fillRect(  0,130,50,50,0x8410);
  tft.fillRect(110,130,50,50,0x8410);
  tft.fillRect(220,130,50,50,0x8410);

}

void loop() {
uint16_t count;
tft.fillScreen(++count<<4);
  tft.setColorDepth( ( tft.getColorDepth()==24 ) ? 16 : 24);

  sprite.pushSprite(random(-20, 20), random(-20, 20));

  tft.drawBmpFile(SD, "/m5stack_mono.bmp",      0,  10);
  tft.drawBmpFile(SD, "/m5stack_16color.bmp", 110,  10);
  tft.drawBmpFile(SD, "/m5stack_256color.bmp",220,  10);
  tft.drawBmpFile(SD, "/m5stack_16bpp.bmp",     0, 130);
  tft.drawBmpFile(SD, "/m5stack_24bpp.bmp",   110, 130);
  tft.drawBmpFile(SD, "/m5stack_32bpp.bmp",   220, 130);
}
