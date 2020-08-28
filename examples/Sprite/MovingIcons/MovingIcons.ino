#include <LovyanGFX.hpp>

#include "Alert.h"
#include "Close.h"
#include "Info.h"

static uint32_t sec, psec;
static size_t fps = 0, frame_count = 0;
static uint32_t lcd_width ;
static uint32_t lcd_height;

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
    } else if (x >= lcd_width) {
      x = lcd_width -1;
      if (dx > 0) dx = - dx;
    }
    y += dy;
    if (y < 0) {
      y = 0;
      if (dy < 0) dy = - dy;
    } else if (y >= lcd_height) {
      y = lcd_height - 1;
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
  lcd.init();

  lcd_width = lcd.width();
  lcd_height = lcd.height();
  obj_info_t *a;
  for (size_t i = 0; i < obj_count; ++i) {
    a = &objects[i];
    a->img = i % 3;
    a->x = random(lcd_width);
    a->y = random(lcd_height);
    a->dx = random(1, 4) * (i & 1 ? 1 : -1);
    a->dy = random(1, 4) * (i & 2 ? 1 : -1);
    a->dr = random(1, 4) * (i & 2 ? 1 : -1);
    a->r = 0;
    a->z = (float)random(10, 20) / 10;
    a->dz = (float)(random(1, 10)) / 100;
  }

  uint32_t div = 2;
  for (;;) {
    sprite_height = (lcd_height + div - 1) / div;
    bool fail = false;
    for (std::uint32_t i = 0; !fail && i < 2; ++i)
    {
      sprites[i].setColorDepth(lcd.getColorDepth());
      sprites[i].setFont(&fonts::Font2);
      fail = !sprites[i].createSprite(lcd_width, sprite_height);
    }
    if (!fail) break;
    for (std::uint32_t i = 0; i < 2; ++i)
    {
      sprites[i].deleteSprite();
    }
    ++div;
  }

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
  lcd.setAddrWindow(0, 0, lcd_width, lcd_height);
}

void loop(void)
{
  static uint8_t flip = 0;

  obj_info_t *a;
  for (int i = 0; i != obj_count; i++) {
    objects[i].move();
  }
  for (int_fast16_t y = 0; y < lcd_height; y += sprite_height) {
    flip = flip ? 0 : 1;
    sprites[flip].clear();
    for (size_t i = 0; i != obj_count; i++) {
      a = &objects[i];
      icons[a->img].pushRotateZoom(&sprites[flip], a->x, a->y - y, a->r, a->z, a->z, 0);
    }

    if (y == 0) {
      sprites[flip].setCursor(0,0);
      sprites[flip].setFont(&fonts::Font4);
      sprites[flip].setTextColor(0xFFFFFFU);
      sprites[flip].printf("obj:%d  fps:%d", obj_count, fps);
    }
    size_t len = sprite_height * lcd_width;
    if (y + sprite_height > lcd_height) {
      len = (lcd_height - y) * lcd_width;
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

