#include <stdio.h>  // for drawBmpFile / drawJpgFile / drawPngFile

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>

#define SCREEN_X 320
#define SCREEN_Y 240
#define DEVICE_NAME "/dev/fb0"

// 引数1 画面の幅
// 引数2 画面の高さ
// 引数3 操作対象とするフレームバッファのパス名、またはデバイス名称 ("st7789") 等の文字列
LGFX lcd ( SCREEN_X, SCREEN_Y, DEVICE_NAME );

int32_t target_x = (SCREEN_X / 2) * 256;
int32_t target_y = (SCREEN_Y / 2) * 256;
int32_t current_x = 0;
int32_t current_y = 0;
int32_t add_x = 0;
int32_t add_y = 0;

void setup()
{
  lcd.init();
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
  lgfx::delay(1);
}
