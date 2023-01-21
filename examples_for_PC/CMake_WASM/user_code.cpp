
// If you write this, you can use drawBmpFile / drawJpgFile / drawPngFile
// #include <stdio.h>

// If you write this, you can use drawBmpUrl / drawJpgUrl / drawPngUrl ( for Windows )
// #include <windows.h>
// #include <winhttp.h>
// #pragma comment (lib, "winhttp.lib")

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>

#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

LGFX lcd(320, 240);

int32_t target_x = 160 * 256;
int32_t target_y = 120 * 256;
int32_t current_x = 0;
int32_t current_y = 0;
int32_t add_x = 0;
int32_t add_y = 0;

void setup()
{
  lcd.init();
  lcd.clear(TFT_PINK);
  lcd.setFont(&fonts::DejaVu24);
}

void loop()
{
  static int i;
  ++i;
  lcd.fillCircle(current_x >> 8, current_y >> 8, 5, i);
  current_x += add_x;
  current_y += add_y;
  add_x += (current_x < target_x) ? 1 : -1;
  add_y += (current_y < target_y) ? 1 : -1;
  lgfx::touch_point_t new_tp;
  if (lcd.getTouch(&new_tp))
  {
    target_x = new_tp.x * 256;
    target_y = new_tp.y * 256;
    lcd.drawCircle(new_tp.x, new_tp.y, 5, TFT_WHITE);
  }
  lcd.drawCenterString("LGFX & WASM", 160, 100);
  lgfx::delay(5);
}

/// exported funcitons
void lgfx_clear(int color)
{
  lcd.clear(color);
}

void lgfx_drawString(const std::string &s, uint32_t x, uint32_t y)
{
  lcd.drawString(s.c_str(), x, y);
}

void lgfx_drawRect(int32_t x, int32_t y, int32_t w, int32_t h, int color)
{
  lcd.drawRect(x, y, w, h, color);
}

void lgfx_drawTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int color)
{
  lcd.drawTriangle(x0, y0, x1, y1, x2, y2, color);
}

void lgfx_drawCircle(int32_t x, int32_t y, int32_t r, int color)
{
  lcd.drawCircle(x, y, r, color);
}

/// binding C++ functions to JavaScript
EMSCRIPTEN_BINDINGS(lgfx_module)
{
  function("clear", &lgfx_clear);
  function("drawString", &lgfx_drawString);
  function("drawRect", &lgfx_drawRect);
  function("drawTriangle", &lgfx_drawTriangle);
  function("drawCircle", &lgfx_drawCircle);
}
