#define LGFX_USE_V1
#include <LovyanGFX.hpp>

static LGFX lcd;
static LGFX_Sprite canvas(&lcd);       // オフスクリーン描画用バッファ
static LGFX_Sprite base(&canvas);      // 文字盤パーツ
static LGFX_Sprite needle(&canvas);    // 針パーツ

static int32_t width = 239;             // 画像サイズ
static int32_t halfwidth = width >> 1;  // 中心座標
static auto transpalette = 0;           // 透過色パレット番号
static float zoom;                      // 表示倍率

#ifdef min
#undef min
#endif

void setup(void)
{
  Serial.begin(115200);

  lcd.init();
  int lw = std::min(lcd.width(), lcd.height());

  zoom = (float)lw / width; // 表示が画面にフィットするよう倍率を調整

  int px = lcd.width() >> 1;
  int py = lcd.height() >> 1;
  lcd.setPivot(px, py); // 描画時の中心を画面中心に合わせる

  lcd.setColorDepth(24);
  for (int i = 0; i < 180; i+=2) { // 外周を描画する
    lcd.setColor(lcd.color888(i*1.4,i*1.4+2,i*1.4+4));
    lcd.fillArc(px, py, (lw>>1)       , (lw>>1)-zoom*3,  90+i,  92+i);
    lcd.fillArc(px, py, (lw>>1)       , (lw>>1)-zoom*3,  88-i,  90-i);
  }
  for (int i = 0; i < 180; i+=2) { // 外周を描画する
    lcd.setColor(lcd.color888(i*1.4,i*1.4+2,i*1.4+4));
    lcd.fillArc(px, py, (lw>>1)-zoom*4, (lw>>1)-zoom*7, 270+i, 272+i);
    lcd.fillArc(px, py, (lw>>1)-zoom*4, (lw>>1)-zoom*7, 268-i, 270-i);
  }
  lcd.setColorDepth(16);

  canvas.setColorDepth(lgfx::palette_2bit);  // 各部品を２ビットパレットモードで準備する
  base  .setColorDepth(lgfx::palette_2bit);
  needle.setColorDepth(lgfx::palette_2bit);

  canvas.createSprite(width, width); // メモリ確保
  base  .createSprite(width, width);
  needle.createSprite(3, 11);

  base.setFont(&fonts::Orbitron_Light_24);    // フォント種類を変更(盤の文字用)
//base.setFont(&fonts::Roboto_Thin_24);       // フォント種類を変更(盤の文字用)

  base.setTextDatum(lgfx::middle_center);

  base.fillCircle(halfwidth, halfwidth, halfwidth - 8, 1);
  base.fillArc(halfwidth, halfwidth, halfwidth - 10, halfwidth - 11, 135,  45, 3);
  base.fillArc(halfwidth, halfwidth, halfwidth - 20, halfwidth - 23,   2,  43, 2);
  base.fillArc(halfwidth, halfwidth, halfwidth - 20, halfwidth - 23, 317, 358, 2);
  base.setTextColor(3);

  for (int i = -5; i <= 25; ++i) {
    bool flg = 0 == (i % 5);      // ５目盛り毎フラグ
    if (flg) {
      // 大きい目盛り描画
      base.fillArc(halfwidth, halfwidth, halfwidth - 10, halfwidth - 24, 179.8 + i*9, 180.2 + i*9, 3);
      base.fillArc(halfwidth, halfwidth, halfwidth - 10, halfwidth - 20, 179.4 + i*9, 180.6 + i*9, 3);
      base.fillArc(halfwidth, halfwidth, halfwidth - 10, halfwidth - 14, 179   + i*9, 181   + i*9, 3);
      float rad = i * 9 * 0.0174532925;
      float ty = - sin(rad) * (halfwidth * 10 / 15);
      float tx = - cos(rad) * (halfwidth * 10 / 17);
      base.drawFloat((float)i/10, 1, halfwidth + tx, halfwidth + ty); // 数値描画
    } else {
      // 小さい目盛り描画
      base.fillArc(halfwidth, halfwidth, halfwidth - 10, halfwidth - 17, 179.5 + i*9, 180.5 + i*9, 3);
    }
  }

  needle.setPivot(1, 9);  // 針パーツの回転中心座標を設定する
  needle.drawRect(0, 0, 3, 11, 2);

  canvas.setPaletteColor(1, 0, 0, 15);
  canvas.setPaletteColor(2, 255, 31, 31);
  canvas.setPaletteColor(3, 255, 255, 191);

  lcd.startWrite();
}


void draw(float value)
{
  base.pushSprite(0, 0);  // 描画用バッファに盤の画像を上書き

  float angle = 270 + value * 90.0;
  needle.pushRotateZoom( angle, 3.0, 10.0, transpalette); // 針をバッファに描画する
  canvas.fillCircle(halfwidth, halfwidth, 7, 3);
  canvas.pushRotateZoom(0, zoom, zoom, transpalette);    // 完了した盤をLCDに描画する
  if (value >= 1.5)
  lcd.fillCircle(lcd.width()>>1, (lcd.height()>>1) + width * 4/10, 5, 0x007FFFU);
}

void loop(void)
{
  float value = 0;
  do {
    draw(value);
  } while (1.9 > (value += 0.005 + value * 0.05));
  do {
    draw(value);
  } while (-0.5 < (value -= 0.1));
  do {
    draw(value);
  } while (0.0 > (value += 0.05));
}

