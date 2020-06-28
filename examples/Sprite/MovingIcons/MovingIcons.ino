#if defined(ARDUINO_M5Stick_C)
#include <AXP192.h>
#endif

#include <LovyanGFX.hpp>

#include "Alert.h"
#include "Close.h"
#include "Info.h"

static uint32_t sec, psec;
static size_t fps = 0, frame_count = 0;
static uint32_t tft_width ;
static uint32_t tft_height;

struct obj_info_t {
  int_fast16_t x;
  int_fast16_t y;
  int_fast16_t dx;
  int_fast16_t dy;
  int_fast8_t img;
  float r;
  float z;
  float dr;
  float dz;

  void move()
  {
    r += dr;
    x += dx;
    if (x < 0) {
      x = 0;
      if (dx < 0) dx = - dx;
    } else if (x >= tft_width) {
      x = tft_width -1;
      if (dx > 0) dx = - dx;
    }
    y += dy;
    if (y < 0) {
      y = 0;
      if (dy < 0) dy = - dy;
    } else if (y >= tft_height) {
      y = tft_height - 1;
      if (dy > 0) dy = - dy;
    }
    z += dz;
    if (z < .5) {
      z = .5;
      if (dz < .0) dz = - dz;
    } else if (z >= 2.0) {
      z = 2.0;
      if (dz > .0) dz = - dz;
    }
  }
};

static constexpr size_t obj_count = 50;
static obj_info_t objects[obj_count];

static LGFX lcd;
static LGFX_Sprite sprites[2];
static LGFX_Sprite icons[3];
static int_fast16_t sprite_height;

void setup(void)
{
#if defined(ARDUINO_M5Stick_C)
  AXP192 axp;
  axp.begin();
#endif

  lcd.init();

  tft_width = lcd.width();
  tft_height = lcd.height();
  obj_info_t *a;
  for (size_t i = 0; i < obj_count; ++i) {
    a = &objects[i];
    a->img = i % 3;
    a->x = random(tft_width);
    a->y = random(tft_height);
    a->dx = random(1, 4) * (i & 1 ? 1 : -1);
    a->dy = random(1, 4) * (i & 2 ? 1 : -1);
    a->dr = random(1, 4) * (i & 2 ? 1 : -1);
    a->r = 0;
    a->z = (float)random(10, 20) / 10;
    a->dz = (float)(random(1, 10)) / 100;
  }

  sprite_height = (tft_height + 2) / 3;
  sprites[0].createSprite(tft_width, sprite_height);
  sprites[1].createSprite(tft_width, sprite_height);

  icons[0].createSprite(infoWidth, infoHeight);
  icons[1].createSprite(alertWidth, alertHeight);
  icons[2].createSprite(closeWidth, closeHeight);

  icons[0].setSwapBytes(true);
  icons[1].setSwapBytes(true);
  icons[2].setSwapBytes(true);

  icons[0].pushImage(0, 0, infoWidth,   infoHeight,  info);
  icons[1].pushImage(0, 0, alertWidth,  alertHeight, alert);
  icons[2].pushImage(0, 0, closeWidth,  closeHeight, closeX);

  lcd.startWrite();
  lcd.setAddrWindow(0, 0, tft_width, tft_height);
}

void loop(void)
{
  static uint8_t flip = 0;

  obj_info_t *a;
  for (int i = 0; i != obj_count; i++) {
    objects[i].move();
  }
  for (int_fast16_t y = 0; y < tft_height; y += sprite_height) {
    flip = flip ? 0 : 1;
    sprites[flip].clear();
    for (size_t i = 0; i != obj_count; i++) {
      a = &objects[i];
      icons[a->img].pushRotateZoom(&sprites[flip], a->x, a->y - y, a->r, a->z, a->z, 0);
    }

    if (y == 0) {
      sprites[flip].setCursor(0,0);
      sprites[flip].setTextFont(4);
      sprites[flip].setTextColor(0xFFFFFFU);
      sprites[flip].printf("obj:%d  fps:%d", obj_count, fps);
    }
    size_t len = sprites[flip].bufferLength() >> 1;
    if (y + sprite_height > tft_height) {
      len = (tft_height - y) * tft_width * lcd.getColorConverter()->bytes;
    }
    lcd.pushPixelsDMA(sprites[flip].getBuffer(), len);
  }

  ++frame_count;
  sec = millis() / 1000;
  if (psec != sec) {
    psec = sec;
    fps = frame_count;
    frame_count = 0;
    lcd.setAddrWindow(0, 0, lcd.width(), lcd.height());
  }
}

