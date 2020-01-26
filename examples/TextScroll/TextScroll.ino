
#include <LGFX_TFT_eSPI.hpp>

TFT_eSPI tft;

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

void setup() {
  lgfx::TPin<BUTTON_A_PIN>::init(GPIO_MODE_INPUT);
  lgfx::TPin<BUTTON_B_PIN>::init(GPIO_MODE_INPUT);
  lgfx::TPin<BUTTON_C_PIN>::init(GPIO_MODE_INPUT);

  Serial.begin(115200);

  tft.init();

  //tft.setColorDepth(24);
  tft.setTextScroll(true);
  tft.fillScreen(0xF800);

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE) // M5Stack
  const int BLK_PWM_CHANNEL = 7;
  ledcSetup(BLK_PWM_CHANNEL, 12000, 8);
  ledcAttachPin(32, BLK_PWM_CHANNEL);
  ledcWrite(BLK_PWM_CHANNEL, 128);

  tft.fillRect(19,19,202,202,0);
  tft.drawRect(18,18,204,204,0x001F);
  tft.setScrollRect(20,20,200,200);
#else
  tft.fillRect(9,9,62,142,0);
  tft.drawRect(8,8,64,144,0x001F);
  tft.setScrollRect(10,10,60,140);

  axp.begin();
#endif
}

int16_t textsize_x = 1;
int16_t textsize_y = 1;
int16_t textfont = 0;
void loop()
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
    textfont = ((textfont+1) & 7);
    switch (textfont) {
    case 0: tft.setTextFont(1); break;
    case 1: tft.setTextFont(2); break;
    case 2: tft.setTextFont(4); break;
    case 3: tft.setTextFont(6); break;
    case 4: tft.setTextFont(7); break;
    case 5: tft.setFreeFont(&FreeSansBoldOblique9pt7b); break;
    case 6: tft.setFreeFont(&FreeMonoOblique9pt7b); break;
    case 7: tft.setFreeFont(&FreeMonoBold9pt7b); break;
    }
  }

  static uint8_t count = 0;
  tft.printf("%02x", ++count);
  tft.print((char)count);
  tft.print("  ");
/*  tft.printf("%d", ++count);
  if (7 == (count & 31))
    tft.printf("N\nN");
*/
}

