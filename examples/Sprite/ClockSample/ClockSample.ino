#if defined(ARDUINO_M5Stick_C)
#include <AXP192.h>
static AXP192 axp;
#endif

#include <LovyanGFX.hpp>

static LGFX lcd;
static LGFX_Sprite offscreen(&lcd);
static LGFX_Sprite clockbase(&offscreen);
static LGFX_Sprite needle1(&offscreen);
static LGFX_Sprite shadow1(&offscreen);
static LGFX_Sprite needle2(&offscreen);
static LGFX_Sprite shadow2(&offscreen);
//static uint64_t count = 4524000;
static uint64_t count = 0;
static auto transpalette = 0;
static float zoom = 1.0;

void setup(void) {
  Serial.begin(115200);

#if defined(ARDUINO_M5Stick_C)
  axp.begin();
#endif

  lcd.init();

  int32_t width = 239;
  int32_t halfwidth = width >> 1;

  zoom = (float)(std::min(lcd.width(), lcd.height())) / width;
  lcd.setPivot(lcd.width() >> 1, lcd.height() >> 1);

  offscreen.setColorDepth(lgfx::palette_4bit);
  clockbase.setColorDepth(lgfx::palette_4bit);
  needle1.setColorDepth(lgfx::palette_4bit);
  shadow1.setColorDepth(lgfx::palette_4bit);
  needle2.setColorDepth(lgfx::palette_4bit);
  shadow2.setColorDepth(lgfx::palette_4bit);

  offscreen.createSprite(width, width);
  clockbase.createSprite(width, width);
  needle1.createSprite(9, 119);
  shadow1.createSprite(9, 119);
  needle2.createSprite(3, 119);
  shadow2.createSprite(3, 119);

  offscreen.fillScreen(transpalette);
  clockbase.fillScreen(transpalette);
  needle1.fillScreen(transpalette);
  shadow1.fillScreen(transpalette);
  needle2.fillScreen(transpalette);
  shadow2.fillScreen(transpalette);

  clockbase.setTextFont(4);
  clockbase.setTextDatum(lgfx::middle_center);
  clockbase.fillCircle(halfwidth, halfwidth, halfwidth    ,  6);
  clockbase.drawCircle(halfwidth, halfwidth, halfwidth - 1, 15);
  for (int i = 1; i <= 60; ++i) {
    float rad = i * 6 * - 0.0174532925;
    float cosy = - cos(rad) * (halfwidth * 10 / 11);
    float sinx = - sin(rad) * (halfwidth * 10 / 11);
    bool flg = 0 == (i % 5);
    clockbase.fillCircle(halfwidth + sinx + 1, halfwidth + cosy + 1, flg * 3 + 1,  4);
    clockbase.fillCircle(halfwidth + sinx    , halfwidth + cosy    , flg * 3 + 1, 15);
    if (flg) {
      cosy = - cos(rad) * (halfwidth * 10 / 13);
      sinx = - sin(rad) * (halfwidth * 10 / 13);
      clockbase.setTextColor(1);
      clockbase.drawNumber(i/5, halfwidth + sinx + 1, halfwidth + cosy + 4);
      clockbase.setTextColor(15);
      clockbase.drawNumber(i/5, halfwidth + sinx    , halfwidth + cosy + 3);
    }
  }

  clockbase.setTextColor(5);
  clockbase.setTextFont(7);
  clockbase.setCursor(clockbase.getPivotX() - 70 , clockbase.getPivotY() - 22);
  clockbase.print("88:88");

  needle1.setPivot(4, 100);
  shadow1.setPivot(4, 100);
  needle2.setPivot(1, 100);
  shadow2.setPivot(1, 100);

  for (int i = 6; i >= 0; --i) {
    needle1.fillTriangle(4, - 24 - (i<<1), 8, needle1.height() - (i<<1), 0, needle1.height() - (i<<1), 15 - i);
    shadow1.fillTriangle(4, - 24 - (i<<1), 8, shadow1.height() - (i<<1), 0, shadow1.height() - (i<<1),  1 + i);
  }
  for (int i = 0; i < 7; ++i) {
    needle1.fillTriangle(4, 16 + (i<<1), 8, needle1.height() + 40 + (i<<1), 0, needle1.height() + 40 + (i<<1), 15 - i);
    shadow1.fillTriangle(4, 16 + (i<<1), 8, shadow1.height() + 40 + (i<<1), 0, shadow1.height() + 40 + (i<<1),  1 + i);
  }
  needle1.fillTriangle(4, 48, 8, needle1.height() + 72, 0, needle1.height() + 72, 0);
  shadow1.fillTriangle(4, 48, 8, shadow1.height() + 72, 0, shadow1.height() + 72, 0);

  needle1.fillCircle(4, 100, 4,  7);
  shadow1.fillCircle(4, 100, 4,  1);
  needle1.drawCircle(4, 100, 4, 15);
  shadow1.drawCircle(4, 100, 4,  1);
  needle1.fillRect(0, 117, 9, 2, 11);
  shadow1.fillRect(0, 117, 9, 2,  4);
  needle1.drawFastHLine(1, 116, 7, 15);
  shadow1.drawFastHLine(1, 116, 7,  1);

  needle2.clear(7);
  shadow2.clear(3);
  needle2.drawFastVLine(1, 0, 119, 8);
  shadow2.drawFastVLine(1, 0, 119, 1);

  offscreen.setTextFont(7);
  offscreen.setTextDatum(lgfx::middle_center);
  offscreen.setTextColor(12);

  lcd.startWrite();

//  shadow1.pushSprite(&lcd,  0, 0);
//  needle1.pushSprite(&lcd, 10, 0);
//  shadow2.pushSprite(&lcd, 20, 0);
//  needle2.pushSprite(&lcd, 25, 0);
}

void drawClock(uint64_t time)
{
  clockbase.pushSprite(0, 0, 0);

  offscreen.setCursor(offscreen.getPivotX() - 70 , offscreen.getPivotY() - 22);
  int hour = time / 360000;
  int min  = (time / 6000) % 60;
  offscreen.printf("%02d:%02d", hour, min);

  int tmp = (time % 100);
  offscreen.setPaletteColor(8, 255 - (tmp>>1), 255 - (tmp>>1), 200 - tmp);

  shadow1.pushRotateZoom(offscreen.getPivotX()+2, offscreen.getPivotY()+2, (float)time / 12000.0, 1.0, 0.7, transpalette);
  shadow1.pushRotateZoom(offscreen.getPivotX()+3, offscreen.getPivotY()+3, (float)time /  1000.0, 1.0, 1.0, transpalette);
  shadow2.pushRotateZoom(offscreen.getPivotX()+4, offscreen.getPivotY()+4, (float)time * 6/100.0, 1.0, 1.0, transpalette);
  needle1.pushRotateZoom((float)time / 12000.0, 1.0, 0.7, transpalette);
  needle1.pushRotated((float)time /  1000.0, transpalette);
  needle2.pushRotated((float)time * 6/100.0, transpalette);

  offscreen.pushRotateZoom(0, zoom, zoom, transpalette);
}

void loop(void)
{
  static uint32_t p_sec100 = 0;
  uint32_t sec100;
  do {
    sec100 = (millis() / 10) % 100;
  } while (p_sec100 == sec100);
  if (p_sec100 > sec100) count += sec100 - p_sec100 + 100;
  else                   count += sec100 - p_sec100;
  p_sec100 = sec100;
  while ( count > 8640000 ) { count -= 8640000; }
  drawClock(count);
}

