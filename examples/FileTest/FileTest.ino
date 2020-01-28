
#include <SD.h>

/*
#include <M5Stack.h>
static M5Display &tft (M5.Lcd);
static TFT_eSprite sprite(&tft);
/*/
#include <LGFX_TFT_eSPI.hpp>
static TFT_eSPI tft;
static TFT_eSprite sprite(&tft);
//*/

void setup()
{
  Serial.begin(115200);
  SD.begin(4, SPI, 20000000);  // 4=TFCARD_CS_PIN

  //sprite.setPSRAM(true); // PSRAM使う場合
  sprite.setColorDepth(8); // 8 , 16 , 24
  sprite.createSprite(320,240);

  sprite.drawBmpFile(SD, "/m5stack_mono.bmp",      0,  10);
  sprite.drawBmpFile(SD, "/m5stack_16color.bmp", 110,  10);
  sprite.drawBmpFile(SD, "/m5stack_256color.bmp",220,  10);

  tft.init();
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
//tft.fillScreen(++count<<4);

  //sprite.pushSprite(random(-20, 20), random(-20, 20), 0xFFFFFFFF);
  //sprite.pushSprite(random(-20, 20), random(-20, 20));
  sprite.pushSprite(0,0);
/*
  tft.drawBmpFile(SD, "/m5stack_mono.bmp",      0,  10);
  tft.drawBmpFile(SD, "/m5stack_16color.bmp", 110,  10);
  tft.drawBmpFile(SD, "/m5stack_256color.bmp",220,  10);
  tft.drawBmpFile(SD, "/m5stack_16bpp.bmp",     0, 130);
  tft.drawBmpFile(SD, "/m5stack_24bpp.bmp",   110, 130);
  tft.drawBmpFile(SD, "/m5stack_32bpp.bmp",   220, 130);
//*/
  static uint32_t count, sec, psec=-1;
  count++;
  sec = millis()/1000;
  if (psec != sec) {
    psec = sec;
    Serial.printf("%d \r\n", (float)count);
    count = 0;
    tft.setRotation((sec&1) ? 1:3);
    tft.setColorDepth( ( tft.getColorDepth()==24 ) ? 16 : 24);
    tft.setAddrWindow(0, 0, 320, 240);
  }
}
