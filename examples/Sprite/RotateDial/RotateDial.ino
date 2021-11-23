#define LGFX_USE_V1
#include <LovyanGFX.hpp>

static LGFX lcd;
static LGFX_Sprite sp;
static LGFX_Sprite sprites[2];
static bool flip;
static int sprite_height;

inline uint16_t getBackColor(int x, int y)
{
  return lcd.swap565(abs((x&31)-16)<<3, 0, abs((y&31)-16)<<3);
//return lcd.swap565(x, 0, y);
}

void setup(void)
{
  lcd.init();

  if (lcd.width() < lcd.height()) { lcd.setRotation(lcd.getRotation()^1); }

  for (int i = 0; i < 2; i++)
  {
    sprites[i].setColorDepth(lcd.getColorDepth());
    sprites[i].setFont(&fonts::Font2);
    sprites[i].setTextColor(TFT_WHITE);
    sprites[i].setTextDatum(textdatum_t::top_right);
  }

  int div = 1;
  for (;;)
  {
    sprite_height = (lcd.height() + div - 1) / div;
    bool fail = false;
    for (int i = 0; !fail && i < 2; ++i)
    {
      fail = !sprites[i].createSprite(lcd.width(), sprite_height);
    }
    if (!fail) break;
    for (int i = 0; i < 2; ++i)
    {
      sprites[i].deleteSprite();
    }
    div++;
  }

  sp.setColorDepth(8);
  sp.setFont(&fonts::Font2);
  sp.setTextDatum(textdatum_t::middle_center);

  lcd.startWrite();
}

void loop(void)
{
  static int count;
  static int prev_sec;
  static int prev_fps;
  static int fps;
  ++fps;
  int sec = millis() / 1000;
  if (prev_sec != sec)
  {
    prev_sec = sec;
    prev_fps = fps;
    fps = 0;
  }
  count++;

  float y_center = (std::max(lcd.width(), lcd.height()));
  float distance;
  float zoom_x, zoom_y;

  for (int div_y = 0; div_y < lcd.height(); div_y += sprite_height)
  {
    float angle = (float)(count * 36) / 100;
//  sprites[flip].clear();
//* // draw background
    if (lcd.getColorDepth() == 16)
    {
      std::uint16_t* rawbuf = (std::uint16_t*)sprites[flip].getBuffer();
      for (int y = 0; y < sprite_height; y++)
      {
        std::uint16_t* rawline = &rawbuf[y * lcd.width()];
        for (int x = 0; x < lcd.width(); x++)
        {
          rawline[x] = lcd.swap565(abs((x&31)-16)<<3, 0, abs(((div_y + y)&31)-16)<<3);
        }
      }
    }
    else
    if (lcd.getColorDepth() > 16)
    {
      lgfx::bgr888_t* rawbuf = (lgfx::bgr888_t*)sprites[flip].getBuffer();
      for (int y = 0; y < sprite_height; y++)
      {
        lgfx::bgr888_t* rawline = &rawbuf[y * lcd.width()];
        for (int x = 0; x < lcd.width(); x++)
        {
          rawline[x] = lcd.swap888(abs((x&31)-16)<<3, 0, abs(((div_y + y)&31)-16)<<3);
        }
      }
    }

    if (div_y == 0)
    {
      // draw fps and counter
      sprites[flip].setCursor(0,0);
      sprites[flip].printf("fps:%03d", prev_fps);
      sprites[flip].drawNumber(count, lcd.width(), 0);

      distance = y_center;
      zoom_x = distance / 512.0f;
      zoom_y = distance / 256.0f;
      sp.createSprite(21, 11);
      sp.fillTriangle(0, 0, 20, 0, 10, 10, TFT_WHITE);
      sp.setPivot(10, distance / zoom_y - 0.5);
      sp.pushRotateZoomWithAA(&sprites[flip], lcd.width() / 2, y_center - div_y, 0, zoom_x, zoom_y, 0);
    }


    // draw outer dial
    distance = y_center * 0.95;
    zoom_y = distance / 30;
    sp.createSprite(1, 1);
    sp.clear(TFT_WHITE);
    for (int i = 0; i < 100; i++)
    {
      float a = fmodf(360.0f + angle - i * 3.6f, 360.0f);
      if (a > 40 && a < 320) continue;

      zoom_x = distance / (i%5 ? 50 : 30);
      zoom_y = distance / (i%5 ? 30 : ((i % 10) ? 20 : 15));
      sp.setPivot(0, distance / zoom_y - 0.5);
      sp.pushRotateZoomWithAA(&sprites[flip], lcd.width() / 2, y_center - div_y, a, zoom_x, zoom_y, 0);
    }

    // draw outer number
    distance *= 0.92;
    zoom_x = distance / 128.0f;
    zoom_y = distance / 128.0f;
    sp.createSprite(18, 14);
    sp.setPivot((float)sp.width() / 2, distance / zoom_y);
    sp.setTextColor(TFT_WHITE, TFT_BLACK);
    for (int i = 0; i < 100; i += 10)
    {
      float a = fmodf(360.0f + angle - i * 3.6f, 360.0f);
      if (a > 50 && a < 310) continue;
      sp.drawNumber(i, sp.width()>>1, sp.height()>>1);
      sp.pushRotateZoomWithAA(&sprites[flip], lcd.width() / 2, y_center - div_y, a, zoom_x, zoom_y, 0);
    }


    // draw inner dial
    angle /= 10;
    distance *= 0.85;
    sp.createSprite(1, 1);
    sp.clear(TFT_YELLOW);
    for (int i = 0; i < 100; i++)
    {
      float a = fmodf(360.0f + angle - i * 3.6f, 360.0f);
      if (a > 60 && a < 300) continue;

      zoom_x = distance / (i%5 ? 50 : 30);
      zoom_y = distance / (i%5 ? 30 : ((i % 10) ? 20 : 15));
      sp.setPivot(0, distance / zoom_y - 0.5);
      sp.pushRotateZoomWithAA(&sprites[flip], lcd.width() / 2, y_center - div_y, a, zoom_x, zoom_y, 0);
    }

    // draw inner number
    distance *= 0.9;
    zoom_x = distance / 128.0f;
    zoom_y = distance / 128.0f;
    sp.createSprite(21, 14);
    sp.setPivot((float)sp.width() / 2, distance / zoom_y);
    sp.setTextColor(TFT_YELLOW, TFT_BLACK);
    for (int i = 0; i < 100; i += 10)
    {
      float a = fmodf(360.0f + angle - i * 3.6f, 360.0f);
      if (a > 70 && a < 290) continue;
      sp.drawFloat(float(i)/100, 1, sp.width()>>1, sp.height()>>1);
      sp.pushRotateZoomWithAA(&sprites[flip], lcd.width() / 2, y_center - div_y, a, zoom_x, zoom_y, 0);
    }

    sprites[flip].pushSprite(&lcd, 0, div_y);
    flip = !flip;
  }
}
