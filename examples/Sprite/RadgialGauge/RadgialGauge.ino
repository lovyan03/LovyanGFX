#if defined(ARDUINO_M5Stick_C)
 #include <AXP192.h>
#endif

#include <LovyanGFX.hpp>

static LGFX tft;
static LGFX_Sprite sprite(&tft);

struct meter_t
{
  int32_t pivot_x;
  int32_t pivot_y;

  float angle = 0;
  float add = 0;

  void advance(void) {
    if ((angle >= 256 && add > 0.0)
    ||  (angle <=   0 && add < 0.0)) add = -add;
    angle += add;
  }

  void drawGauge(uint32_t color)
  {
    tft.setPivot(pivot_x, pivot_y);
    sprite.setPaletteColor(1, color);
    sprite.pushRotated(angle - 127);
    advance();
  }
};

struct meter_t meter1, meter2, meter3;

void setup()
{
#if defined(ARDUINO_M5Stick_C)
  AXP192 axp;
  axp.begin();
#endif

  tft.init();

  if (tft.width() < tft.height()) {
    tft.setRotation((tft.getRotation() + 1) & 3);
  }

  sprite.setColorDepth(2);

  sprite.createSprite(3,tft.height() / 8);
  sprite.setPivot(1, tft.height() / 4);

  sprite.drawFastVLine(1, 0, sprite.height()    , 3);
  sprite.drawFastVLine(0, 2, sprite.height() - 4, 1);

  meter1.pivot_x = tft.width() >> 1;
  meter1.pivot_y = (tft.height() >> 2) + 2;
  meter1.add = 0.01;

  meter2.pivot_x = tft.width() >> 2;
  meter2.pivot_y = (tft.height() * 3) >>2;
  meter2.add = 0.11;

  meter3.pivot_x = (tft.width() * 3) >> 2;
  meter3.pivot_y = (tft.height() * 3) >> 2;
  meter3.add = 0.57;

  tft.fillScreen(tft.color565(0,0,127));

  for (int i = 0; i < 20; i++) {
    tft.drawFastHLine(0,  (i*2+1) * tft.height() / 40, tft.width(), 0xFFFF);
    tft.drawFastVLine((i*2+1) * tft.width() / 40, 0, tft.height() , 0xFFFF);
  }
}

void loop(void)
{
  meter1.drawGauge(sprite.color888(255, meter1.angle, 127));
  meter2.drawGauge(sprite.color888(255 - meter2.angle, meter2.angle, 127));
  meter3.drawGauge(sprite.color888(0, 127, meter3.angle));
}

