//#define LGFX_AUTODETECT

#include <LovyanGFX.hpp>

#include "320x240x16.h"

static LGFX lcd;
static LGFX_Sprite sprite;

static std::uint32_t count = 0;

void setup()
{
  lcd.init();
  if (lcd.width() < lcd.height()) { lcd.setRotation(lcd.getRotation() ^ 1); }

// setBufferを使用することで、予め用意されたデータを用いてスプライトを使用できるようにする。
  sprite.setBuffer(const_cast<std::uint8_t*>(bmp320x240x16), 320, 240, 16);

//  sprite.setPsram(true);
//  sprite.setBuffer(const_cast<std::uint8_t*>(bmp320x240x16),  640, 240, 8);
//  sprite.setBuffer(const_cast<std::uint8_t*>(bmp320x240x16),  640, 480, 4);
//  sprite.setBuffer(const_cast<std::uint8_t*>(bmp320x240x16), 1280, 480, 2);
//  sprite.setBuffer(const_cast<std::uint8_t*>(bmp320x240x16), 1280, 960, 1);
//  sprite.createPalette();


// ※ 本来const(書換不可能)なデータをconst_castにより非constと偽って使用しています。
// このため、このスプライトに対して描画を行うとプログラムはクラッシュします。
// LCDや他のスプライトに描画する関数など、読出しを行う関数のみが使用できます。

// OK)  sprite.pushSprite 
// OK)  sprite.pushRotated
// OK)  sprite.pushRotateZoom
// OK)  sprite.readRect

// NG)  sprite.drawPixel
// NG)  sprite.fillRect
// NG)  sprite.scroll

}

void loop() {
  if (++count == 360) count = 0;
  float zoom = (((float)count) / 200) + 0.1;
  sprite.pushRotateZoom(&lcd, lcd.width() >> 1, lcd.height() >> 1, count, zoom, zoom);
}

