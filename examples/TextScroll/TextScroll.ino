
#include <SD.h>
#include <LGFX_TFT_eSPI.hpp>

TFT_eSPI tft;
TFT_eSprite sprite(&tft);

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE) // M5Stack
 #define BUTTON_A_PIN 39
 #define BUTTON_B_PIN 38
 #define BUTTON_C_PIN 37
#else
 #define BUTTON_A_PIN 37
 #define BUTTON_B_PIN 39
 #define BUTTON_C_PIN -1

 #include <AXP192.h>
 AXP192 axp;
#endif

void cursor_init(lgfx::LovyanGFX& tft)
{
  tft.setTextScroll(true);
  tft.setCursor(0,0);
  tft.fillScreen(0);

  tft.fillScreen(0xF800);
#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE) // M5Stack
  tft.fillRect(19,19,202,202,0);
  tft.drawRect(18,18,204,204,0x001F);
  tft.setScrollRect(20,20,200,200);
#else
  tft.fillRect(9,9,62,142,0);
  tft.drawRect(8,8,64,144,0x001F);
  tft.setScrollRect(10,10,60,140);
#endif
//*/
}


int16_t textsize_x = 1;
int16_t textsize_y = 1;
int16_t textfont = 0;

void exec_text(lgfx::LovyanGFX& tft)
{
#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE) // M5Stack
  if (lgfx::TPin<BUTTON_B_PIN>::get() == 0) {
    while (lgfx::TPin<BUTTON_B_PIN>::get() == 0) delay(1);
    textsize_x = (textsize_x & 7) + 1;
    tft.setTextSize(textsize_x, textsize_y);
  }
  if (lgfx::TPin<BUTTON_C_PIN>::get() == 0) {
    while (lgfx::TPin<BUTTON_C_PIN>::get() == 0) delay(1);
    textsize_y = (textsize_y & 7) + 1;
    tft.setTextSize(textsize_x, textsize_y);
  }
#else
  if (lgfx::TPin<BUTTON_B_PIN>::get() == 0) {
    while (lgfx::TPin<BUTTON_B_PIN>::get() == 0) delay(1);
    textsize_x = (textsize_x & 7) + 1;
    textsize_y = (textsize_y & 7) + 1;
    tft.setTextSize(textsize_x, textsize_y);
  }
#endif

  if (lgfx::TPin<BUTTON_A_PIN>::get() == 0) {
    while (lgfx::TPin<BUTTON_A_PIN>::get() == 0) delay(1);
    textfont = (textfont < 51) ? textfont + 1 : 0;
    switch (textfont) {
    case  0: tft.setTextFont(1); break;
    case  1: tft.setTextFont(2); break;
    case  2: tft.setTextFont(4); break;
    case  3: tft.setTextFont(6); break;
    case  4: tft.setTextFont(7); break;
    case  5: tft.loadFont("/Ud36.vlw", SD); break;
    case  6: tft.loadFont("/RictyDiminished16.vlw", SD); break;
    case  7: tft.setFreeFont(&TomThumb); break;
    case  8: tft.setFreeFont(&FreeMono9pt7b); break;
    case  9: tft.setFreeFont(&FreeMono12pt7b); break;
    case 10: tft.setFreeFont(&FreeMono18pt7b); break;
    case 11: tft.setFreeFont(&FreeMono24pt7b); break;
    case 12: tft.setFreeFont(&FreeMonoOblique9pt7b); break;
    case 13: tft.setFreeFont(&FreeMonoOblique12pt7b); break;
    case 14: tft.setFreeFont(&FreeMonoOblique18pt7b); break;
    case 15: tft.setFreeFont(&FreeMonoOblique24pt7b); break;
    case 16: tft.setFreeFont(&FreeMonoBold9pt7b); break;
    case 17: tft.setFreeFont(&FreeMonoBold12pt7b); break;
    case 18: tft.setFreeFont(&FreeMonoBold18pt7b); break;
    case 19: tft.setFreeFont(&FreeMonoBold24pt7b); break;
    case 20: tft.setFreeFont(&FreeSans9pt7b); break;
    case 21: tft.setFreeFont(&FreeSans12pt7b); break;
    case 22: tft.setFreeFont(&FreeSans18pt7b); break;
    case 23: tft.setFreeFont(&FreeSans24pt7b); break;
    case 24: tft.setFreeFont(&FreeSansOblique9pt7b); break;
    case 25: tft.setFreeFont(&FreeSansOblique12pt7b); break;
    case 26: tft.setFreeFont(&FreeSansOblique18pt7b); break;
    case 27: tft.setFreeFont(&FreeSansOblique24pt7b); break;
    case 28: tft.setFreeFont(&FreeSansBold9pt7b); break;
    case 29: tft.setFreeFont(&FreeSansBold12pt7b); break;
    case 30: tft.setFreeFont(&FreeSansBold18pt7b); break;
    case 31: tft.setFreeFont(&FreeSansBold24pt7b); break;
    case 32: tft.setFreeFont(&FreeSansBoldOblique9pt7b); break;
    case 33: tft.setFreeFont(&FreeSansBoldOblique12pt7b); break;
    case 34: tft.setFreeFont(&FreeSansBoldOblique18pt7b); break;
    case 35: tft.setFreeFont(&FreeSansBoldOblique24pt7b); break;
    case 36: tft.setFreeFont(&FreeSerif9pt7b); break;
    case 37: tft.setFreeFont(&FreeSerif12pt7b); break;
    case 38: tft.setFreeFont(&FreeSerif18pt7b); break;
    case 39: tft.setFreeFont(&FreeSerif24pt7b); break;
    case 40: tft.setFreeFont(&FreeSerifItalic9pt7b); break;
    case 41: tft.setFreeFont(&FreeSerifItalic12pt7b); break;
    case 42: tft.setFreeFont(&FreeSerifItalic18pt7b); break;
    case 43: tft.setFreeFont(&FreeSerifItalic24pt7b); break;
    case 44: tft.setFreeFont(&FreeSerifBold9pt7b); break;
    case 45: tft.setFreeFont(&FreeSerifBold12pt7b); break;
    case 46: tft.setFreeFont(&FreeSerifBold18pt7b); break;
    case 47: tft.setFreeFont(&FreeSerifBold24pt7b); break;
    case 48: tft.setFreeFont(&FreeSerifBoldItalic9pt7b); break;
    case 49: tft.setFreeFont(&FreeSerifBoldItalic12pt7b); break;
    case 50: tft.setFreeFont(&FreeSerifBoldItalic18pt7b); break;
    case 51: tft.setFreeFont(&FreeSerifBoldItalic24pt7b); break;
    }
    cursor_init(tft);
  }
  static uint8_t count = 0;
  tft.printf("%02x ", count);
//  tft.print((char)++count);
//  tft.print("  ");
/*  tft.printf("%d", ++count);
  if (7 == (count & 31))
    tft.printf("N\nN");
*/
  tft.print((char)count);
  if (++count == 0) {
    tft.print("  ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺ");
    tft.print("あいうえおかきくけこさしすせそたちつてとなにぬねのはひふへほまみむめもやゆよわをん");
    tft.print("漢字入力テスト");
  }
}

void setup(void) {
  lgfx::TPin<BUTTON_A_PIN>::init(GPIO_MODE_INPUT);
  lgfx::TPin<BUTTON_B_PIN>::init(GPIO_MODE_INPUT);
  lgfx::TPin<BUTTON_C_PIN>::init(GPIO_MODE_INPUT);

  Serial.begin(115200);

  SD.begin(4, SPI, 20000000);
  tft.init();

  sprite.setColorDepth(3);
  sprite.createSprite(tft.width(), tft.height());

  //tft.setColorDepth(24);

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE) // M5Stack
  const int BLK_PWM_CHANNEL = 7;
  ledcSetup(BLK_PWM_CHANNEL, 12000, 8);
  ledcAttachPin(32, BLK_PWM_CHANNEL);
  ledcWrite(BLK_PWM_CHANNEL, 128);
#else
  axp.begin();
#endif

  cursor_init(sprite);
}


void loop(void)
{
  static uint64_t ms = 0;
  exec_text(sprite);
  if (ms + 16000 < micros()) {
    sprite.pushSprite(0,0);
    ms = micros();
  }
}

