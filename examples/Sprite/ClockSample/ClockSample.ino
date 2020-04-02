
#if defined(ARDUINO_M5Stick_C)
 #include <AXP192.h>
#endif

#include <LovyanGFX.hpp>

static LGFX tft;
static LGFX_Sprite offscreen(&tft);
static LGFX_Sprite clockbase(&offscreen);
static LGFX_Sprite needle(&offscreen);
static LGFX_Sprite shadow(&offscreen);
static uint64_t count = 0;

static auto transp = lgfx::color888(255,255,0);

void setup() {
  Serial.begin(115200);

#if defined(ARDUINO_M5Stick_C)
  AXP192 axp;
  axp.begin();
#endif

  tft.init();
  tft.setBrightness(255);
  tft.setRotation(0);
//tft.setPivot(tft.width()>>1, tft.height()>>1);

  int32_t width = tft.width() - 1;
  if (width > tft.height() - 1) width = tft.height() - 1;

  int32_t halfwidth = width >> 1;

  offscreen.setColorDepth(lgfx::rgb565_2Byte);
  offscreen.createSprite(width, width);
  offscreen.fillScreen(transp);

  clockbase.setColorDepth(lgfx::palette_2bit);
  clockbase.createSprite(width, width);
  clockbase.setTextFont(4);
  clockbase.setTextDatum(lgfx::middle_center);
  clockbase.fillCircle(halfwidth, halfwidth, halfwidth    , 2);
  clockbase.drawCircle(halfwidth, halfwidth, halfwidth - 1, 3);
  for (int i = 1; i <= 60; ++i) {
    float rad = i * 6 * - 0.0174532925;
    float cosy = - cos(rad) * (halfwidth * 10 / 11);
    float sinx = - sin(rad) * (halfwidth * 10 / 11);
    bool flg = 0 == (i % 5);
    clockbase.fillCircle(halfwidth + sinx + 2, halfwidth + cosy + 2, flg * 3 + 1, 1);
    clockbase.fillCircle(halfwidth + sinx    , halfwidth + cosy    , flg * 3 + 1, 3);
    if (flg) {
      cosy = - cos(rad) * (halfwidth * 10 / 13);
      sinx = - sin(rad) * (halfwidth * 10 / 13);
      clockbase.setTextColor(1);
      clockbase.drawNumber(i/5, halfwidth + sinx + 2, halfwidth + cosy + 5);
      clockbase.setTextColor(3);
      clockbase.drawNumber(i/5, halfwidth + sinx    , halfwidth + cosy + 3);
    }
  }
  if (width > 160) {
    clockbase.setTextColor(1);
    clockbase.setTextFont(7);
    clockbase.setCursor(clockbase.getPivotX() - 70 , clockbase.getPivotY() - 24);
    clockbase.print("88:88");
  }
  needle.setColorDepth(lgfx::rgb332_1Byte);
  shadow.setColorDepth(lgfx::rgb332_1Byte);
  needle.createPalette();
  shadow.createPalette();
  needle.createSprite(9,119);
  shadow.createSprite(9,119);
  needle.setPivot(4, 104);
  shadow.setPivot(4, 104);

  for (int i = 15; i >= 0; --i) {
    needle.fillTriangle(4, - 24 - (i<<1), 8, needle.height() - (i<<1), 0, needle.height() - (i<<1), 255 - (i << 3));
    shadow.fillTriangle(4, - 24 - (i<<1), 8, shadow.height() - (i<<1), 0, shadow.height() - (i<<1),   1 + (i << 3));
  }
  for (int i = 0; i < 16; ++i) {
    needle.fillTriangle(4, i<<1, 8, needle.height() + 24 + (i<<1), 0, needle.height() + 24 + (i<<1), 255 - (i << 3));
    shadow.fillTriangle(4, i<<1, 8, shadow.height() + 24 + (i<<1), 0, shadow.height() + 24 + (i<<1),   1 + (i << 3));
  }
  needle.fillTriangle(4, 32, 8, needle.height() + 56, 0, needle.height() + 56, 0);
  shadow.fillTriangle(4, 32, 8, shadow.height() + 56, 0, shadow.height() + 56, 0);

  needle.fillCircle(4, 104, 4, 127);
  shadow.fillCircle(4, 104, 4, 127);
  needle.drawCircle(4, 104, 4, 255);
  shadow.drawCircle(4, 104, 4, 255);
  needle.drawFastHLine(0,118,9, 255);
  shadow.drawFastHLine(0,118,9, 1);
  needle.drawPixel(4,104,0);

  offscreen.setTextFont(7);
  offscreen.setTextDatum(lgfx::middle_center);
  offscreen.setTextColor(lgfx::color888(255,255,200));

  tft.startWrite();
}

void loop(void)
{
  if ( ++count == 86400 ) { count = 0; }

  clockbase.pushSprite(0,0,0);

  if (offscreen.width() > 160) {
    offscreen.setCursor(offscreen.getPivotX() - 70 , offscreen.getPivotY() - 24);
    int hour = count / 3600;
    int min  = (count / 60) % 60;
    offscreen.printf("%02d:%02d", hour, min);
  }

  shadow.pushRotateZoom(&offscreen, offscreen.getPivotX()+2, offscreen.getPivotY()+2, (float)count / 120.0, 1.0, 0.7, 0);
  shadow.pushRotateZoom(&offscreen, offscreen.getPivotX()+3, offscreen.getPivotY()+3, (float)count /  10.0, 1.0, 1.0, 0);
  shadow.pushRotateZoom(&offscreen, offscreen.getPivotX()+4, offscreen.getPivotY()+4, (float)count *   6.0, 0.4, 1.0, 0);
  needle.pushRotateZoom(&offscreen, offscreen.getPivotX()  , offscreen.getPivotY()  , (float)count / 120.0, 1.0, 0.7, 0);
  needle.pushRotateZoom(&offscreen, offscreen.getPivotX()  , offscreen.getPivotY()  , (float)count /  10.0, 1.0, 1.0, 0);
  needle.pushRotateZoom(&offscreen, offscreen.getPivotX()  , offscreen.getPivotY()  , (float)count *   6.0, 0.4, 1.0, 0);

  offscreen.pushSprite(tft.width() - offscreen.width() >> 1, tft.height() - offscreen.height() >> 1, transp);
}

