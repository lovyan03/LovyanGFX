#define LGFX_USE_V1
#include <LovyanGFX.hpp>

static LGFX lcd;
static LGFX_Sprite canvas(&lcd);       // オフスクリーン描画用バッファ
static LGFX_Sprite clockbase(&canvas); // 時計の文字盤パーツ
static LGFX_Sprite needle1(&canvas);   // 長針・短針パーツ
static LGFX_Sprite shadow1(&canvas);   // 長針・短針の影パーツ
static LGFX_Sprite needle2(&canvas);   // 秒針パーツ
static LGFX_Sprite shadow2(&canvas);   // 秒針の影パーツ

static constexpr uint64_t oneday = 86400000; // 1日 = 1000msec x 60sec x 60min x 24hour = 86400000
static uint64_t count = rand() % oneday;    // 現在時刻 (ミリ秒カウンタ)
static int32_t width = 239;             // 時計の縦横画像サイズ
static int32_t halfwidth = width >> 1;  // 時計盤の中心座標
static auto transpalette = 0;           // 透過色パレット番号
static float zoom;                      // 表示倍率

#ifdef min
#undef min
#endif

void setup(void)
{
  lcd.init();

  zoom = (float)(std::min(lcd.width(), lcd.height())) / width; // 表示が画面にフィットするよう倍率を調整

  lcd.setPivot(lcd.width() >> 1, lcd.height() >> 1); // 時計描画時の中心を画面中心に合わせる

  canvas.setColorDepth(lgfx::palette_4bit);  // 各部品を４ビットパレットモードで準備する
  clockbase.setColorDepth(lgfx::palette_4bit);
  needle1.setColorDepth(lgfx::palette_4bit);
  shadow1.setColorDepth(lgfx::palette_4bit);
  needle2.setColorDepth(lgfx::palette_4bit);
  shadow2.setColorDepth(lgfx::palette_4bit);
// パレットの初期色はグレースケールのグラデーションとなっており、
// 0番が黒(0,0,0)、15番が白(255,255,255)
// 1番～14番は黒から白へ段階的に明るさが変化している
//
// パレットを使う場合、描画関数は色の代わりに0～15のパレット番号を指定する

  canvas.createSprite(width, width); // メモリ確保
  clockbase.createSprite(width, width);
  needle1.createSprite(9, 119);
  shadow1.createSprite(9, 119);
  needle2.createSprite(3, 119);
  shadow2.createSprite(3, 119);

  canvas.fillScreen(transpalette); // 透過色で背景を塗り潰す (create直後は0埋めされているので省略可能)
  clockbase.fillScreen(transpalette);
  needle1.fillScreen(transpalette);
  shadow1.fillScreen(transpalette);

  clockbase.setTextFont(4);           // フォント種類を変更(時計盤の文字用)
  clockbase.setTextDatum(lgfx::middle_center);
  clockbase.fillCircle(halfwidth, halfwidth, halfwidth    ,  6); // 時計盤の背景の円を塗る
  clockbase.drawCircle(halfwidth, halfwidth, halfwidth - 1, 15);
  for (int i = 1; i <= 60; ++i) {
    float rad = i * 6 * - 0.0174532925;              // 時計盤外周の目盛り座標を求める
    float cosy = - cos(rad) * (halfwidth * 10 / 11);
    float sinx = - sin(rad) * (halfwidth * 10 / 11);
    bool flg = 0 == (i % 5);      // ５目盛り毎フラグ
    clockbase.fillCircle(halfwidth + sinx + 1, halfwidth + cosy + 1, flg * 3 + 1,  4); // 目盛りを描画
    clockbase.fillCircle(halfwidth + sinx    , halfwidth + cosy    , flg * 3 + 1, 12);
    if (flg) {                    // 文字描画
      cosy = - cos(rad) * (halfwidth * 10 / 13);
      sinx = - sin(rad) * (halfwidth * 10 / 13);
      clockbase.setTextColor(1);
      clockbase.drawNumber(i/5, halfwidth + sinx + 1, halfwidth + cosy + 4);
      clockbase.setTextColor(15);
      clockbase.drawNumber(i/5, halfwidth + sinx    , halfwidth + cosy + 3);
    }
  }
  clockbase.setTextFont(7);

  needle1.setPivot(4, 100);  // 針パーツの回転中心座標を設定する
  shadow1.setPivot(4, 100);
  needle2.setPivot(1, 100);
  shadow2.setPivot(1, 100);

  for (int i = 6; i >= 0; --i) {  // 針パーツの画像を作成する
    needle1.fillTriangle(4, - 16 - (i<<1), 8, needle1.height() - (i<<1), 0, needle1.height() - (i<<1), 15 - i);
    shadow1.fillTriangle(4, - 16 - (i<<1), 8, shadow1.height() - (i<<1), 0, shadow1.height() - (i<<1),  1 + i);
  }
  for (int i = 0; i < 7; ++i) {
    needle1.fillTriangle(4, 16 + (i<<1), 8, needle1.height() + 32 + (i<<1), 0, needle1.height() + 32 + (i<<1), 15 - i);
    shadow1.fillTriangle(4, 16 + (i<<1), 8, shadow1.height() + 32 + (i<<1), 0, shadow1.height() + 32 + (i<<1),  1 + i);
  }
  needle1.fillTriangle(4, 32, 8, needle1.height() + 64, 0, needle1.height() + 64, 0);
  shadow1.fillTriangle(4, 32, 8, shadow1.height() + 64, 0, shadow1.height() + 64, 0);
  needle1.fillRect(0, 117, 9, 2, 15);
  shadow1.fillRect(0, 117, 9, 2,  1);
  needle1.drawFastHLine(1, 117, 7, 12);
  shadow1.drawFastHLine(1, 117, 7,  4);

  needle1.fillCircle(4, 100, 4, 15);
  shadow1.fillCircle(4, 100, 4,  1);
  needle1.drawCircle(4, 100, 4, 14);

  needle2.fillScreen(9);
  shadow2.fillScreen(3);
  needle2.drawFastVLine(1, 0, 119, 8);
  shadow2.drawFastVLine(1, 0, 119, 1);
  needle2.fillRect(0, 99, 3, 3, 8);

  lcd.startWrite();

//  shadow1.pushSprite(&lcd,  0, 0); // デバッグ用、パーツを直接LCDに描画する
//  needle1.pushSprite(&lcd, 10, 0);
//  shadow2.pushSprite(&lcd, 20, 0);
//  needle2.pushSprite(&lcd, 25, 0);
}

void update7Seg(int32_t hour, int32_t min)
{ // 時計盤のデジタル表示部の描画
  int x = clockbase.getPivotX() - 69;
  int y = clockbase.getPivotY();
  clockbase.setCursor(x, y);
  clockbase.setTextColor(5);  // 消去色で 88:88 を描画する
  clockbase.print("88:88");
  clockbase.setCursor(x, y);
  clockbase.setTextColor(12); // 表示色で時:分を描画する
  clockbase.printf("%02d:%02d", hour, min);
}

void drawDot(int pos, int palette)
{
  bool flg = 0 == (pos % 5);      // ５目盛り毎フラグ
  float rad = pos * 6 * - 0.0174532925;              // 時計盤外周の目盛り座標を求める
  float cosy = - cos(rad) * (halfwidth * 10 / 11);
  float sinx = - sin(rad) * (halfwidth * 10 / 11);
  canvas.fillCircle(halfwidth + sinx, halfwidth + cosy, flg * 3 + 1, palette);
}

void drawClock(uint64_t time)
{ // 時計の描画
  static int32_t p_min = -1;
  int32_t sec = time / 1000;
  int32_t min = sec / 60;
  if (p_min != min) { // 分の値が変化していれば時計盤のデジタル表示部分を更新
    p_min = min;
    update7Seg(min / 60, min % 60);
  }
  clockbase.pushSprite(0, 0);  // 描画用バッファに時計盤の画像を上書き

  drawDot(sec % 60, 14);
  drawDot(min % 60, 15);
  drawDot(((min/60)*5)%60, 15);

  float fhour = (float)time / 120000;  // 短針の角度
  float fmin  = (float)time /  10000;  // 長針の角度
  float fsec  = (float)time*6 / 1000;  // 秒針の角度
  int px = canvas.getPivotX();
  int py = canvas.getPivotY();
  shadow1.pushRotateZoom(px+2, py+2, fhour, 1.0, 0.7, transpalette); // 針の影を右下方向にずらして描画する
  shadow1.pushRotateZoom(px+3, py+3, fmin , 1.0, 1.0, transpalette);
  shadow2.pushRotateZoom(px+4, py+4, fsec , 1.0, 1.0, transpalette);
  needle1.pushRotateZoom(            fhour, 1.0, 0.7, transpalette); // 針を描画する
  needle1.pushRotateZoom(            fmin , 1.0, 1.0, transpalette);
  needle2.pushRotateZoom(            fsec , 1.0, 1.0, transpalette);

  canvas.pushRotateZoom(0, zoom, zoom, transpalette);    // 完了した時計盤をLCDに描画する
  lcd.display();
}

void loop(void)
{
  static uint32_t p_milli = 0;
  uint32_t milli = lgfx::millis() % 1000;
  if (p_milli > milli) count += 1000 + (milli - p_milli);
  else                 count +=        (milli - p_milli);
  p_milli = milli;

  int32_t tmp = (count % 1000) >> 3;
  canvas.setPaletteColor(8, 255 - (tmp>>1), 255 - (tmp>>1), 200 - tmp); // 秒針の描画色を変化させる
//count += 60000;
  if ( count > oneday ) { count -= oneday; }
  drawClock(count);
}

