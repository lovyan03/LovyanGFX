
#if defined(ARDUINO_M5Stick_C)
#include <AXP192.h>
static AXP192 axp;
#endif

#include <LovyanGFX.hpp>

static LGFX tft;
static LGFX_Sprite offscreen;
static LGFX_Sprite clockbase;
static LGFX_Sprite needle1(&offscreen);
static LGFX_Sprite shadow1(&offscreen);
static LGFX_Sprite needle2(&offscreen);
static LGFX_Sprite shadow2(&offscreen);
static uint64_t count = 0;

static auto transp = lgfx::color888(255,255,0);

void setup() {
  Serial.begin(115200);

#if defined(ARDUINO_M5Stick_C)
  axp.begin();
#endif

  tft.init();

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
    clockbase.fillCircle(halfwidth + sinx + 1, halfwidth + cosy + 1, flg * 3 + 1, 1);
    clockbase.fillCircle(halfwidth + sinx    , halfwidth + cosy    , flg * 3 + 1, 3);
    if (flg) {
      cosy = - cos(rad) * (halfwidth * 10 / 13);
      sinx = - sin(rad) * (halfwidth * 10 / 13);
      clockbase.setTextColor(1);
      clockbase.drawNumber(i/5, halfwidth + sinx + 1, halfwidth + cosy + 4);
      clockbase.setTextColor(3);
      clockbase.drawNumber(i/5, halfwidth + sinx    , halfwidth + cosy + 3);
    }
  }
  if (width > 160) {
    clockbase.setTextColor(1);
    clockbase.setTextFont(7);
    clockbase.setCursor(clockbase.getPivotX() - 70 , clockbase.getPivotY() - 22);
    clockbase.print("88:88");
  }
  needle1.setColorDepth(lgfx::palette_4bit);
  shadow1.setColorDepth(lgfx::palette_4bit);
  needle2.setColorDepth(lgfx::palette_2bit);
  shadow2.setColorDepth(lgfx::palette_2bit);
  needle1.createSprite(9, 119);
  shadow1.createSprite(9, 119);
  needle2.createSprite(3, 119);
  shadow2.createSprite(3, 119);
  needle1.setPivot(4, 100);
  shadow1.setPivot(4, 100);
  needle2.setPivot(1, 100);
  shadow2.setPivot(1, 100);

  needle2.setPaletteColor(3, 239, 239, 191);
  needle2.setPaletteColor(2, 207, 207, 159);
  shadow2.setPaletteColor(2, 128, 128, 128);

  for (int i = 7; i >= 0; --i) {
    needle1.fillTriangle(4, - 24 - (i<<1), 8, needle1.height() - (i<<1), 0, needle1.height() - (i<<1), 15 - i);
    shadow1.fillTriangle(4, - 24 - (i<<1), 8, shadow1.height() - (i<<1), 0, shadow1.height() - (i<<1),  1 + i);
  }
  for (int i = 0; i < 8; ++i) {
    needle1.fillTriangle(4, 16 + i<<1, 8, needle1.height() + 40 + (i<<1), 0, needle1.height() + 40 + (i<<1), 15 - i);
    shadow1.fillTriangle(4, 16 + i<<1, 8, shadow1.height() + 40 + (i<<1), 0, shadow1.height() + 40 + (i<<1),  1 + i);
  }
  needle1.fillTriangle(4, 48, 8, needle1.height() + 72, 0, needle1.height() + 72, 0);
  shadow1.fillTriangle(4, 48, 8, shadow1.height() + 72, 0, shadow1.height() + 72, 0);

  needle1.fillCircle(4, 100, 4, 127);
  shadow1.fillCircle(4, 100, 4,   1);
  needle1.drawCircle(4, 100, 4, 255);
  shadow1.drawCircle(4, 100, 4,  65);
  needle1.drawFastHLine(1, 116, 7, 191);
  shadow1.drawFastHLine(1, 116, 7,  65);
  needle1.drawRect(0, 117, 9, 2, 255);
  shadow1.drawRect(0, 117, 9, 2,   1);
  needle1.drawPixel(4,100,0);

  needle2.clear(2);
  shadow2.clear(2);
  needle2.drawFastVLine(1, 0, 119, 3);
  shadow2.drawFastVLine(1, 0, 119, 1);

  offscreen.setTextFont(7);
  offscreen.setTextDatum(lgfx::middle_center);
  offscreen.setTextColor(lgfx::color888(255,255,200));

  tft.startWrite();

//  shadow1.pushSprite(&tft,  0, 0);
//  needle1.pushSprite(&tft, 10, 0);
//  shadow2.pushSprite(&tft, 20, 0);
//  needle2.pushSprite(&tft, 25, 0);
}

void drawClock(uint64_t time)
{
  clockbase.pushSprite(&offscreen, 0, 0, 0);

  if (offscreen.width() > 160) {
    offscreen.setCursor(offscreen.getPivotX() - 70 , offscreen.getPivotY() - 22);
    int hour = time / 36000;
    int min  = (time / 600) % 60;
    offscreen.printf("%02d:%02d", hour, min);
  }

  shadow1.pushRotateZoom(offscreen.getPivotX()+2, offscreen.getPivotY()+2, (float)time / 1200.0, 1.0, 0.7, 0);
  shadow1.pushRotateZoom(offscreen.getPivotX()+3, offscreen.getPivotY()+3, (float)time /  100.0, 1.0, 1.0, 0);
  shadow2.pushRotateZoom(offscreen.getPivotX()+4, offscreen.getPivotY()+4, (float)time * 6/10.0, 1.0, 1.0, 0);
  needle1.pushRotateZoom((float)time / 1200.0, 1.0, 0.7, 0);
  needle1.pushRotated((float)time /  100.0, 0);
  needle2.pushRotated((float)time * 6/10.0, 0);

  offscreen.pushSprite(&tft, tft.width() - offscreen.width() >> 1, tft.height() - offscreen.height() >> 1, transp);
}

void loop(void)
{
  static uint32_t p_sec10 = 0;
  uint32_t sec10;
  do {
    sec10 = (millis() / 100) % 10;
  } while (p_sec10 == sec10);
  if (p_sec10 > sec10) count += sec10 - p_sec10 + 10;
  else                 count += sec10 - p_sec10;
  p_sec10 = sec10;
  while ( count > 864000 ) { count -= 864000; }
  drawClock(count);
}

