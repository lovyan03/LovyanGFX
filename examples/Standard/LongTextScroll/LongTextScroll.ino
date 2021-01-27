#define LGFX_AUTODETECT
#include <LovyanGFX.hpp>

LGFX lcd;
LGFX_Sprite canvas;

static constexpr char text[] = "Hello world ! こんにちは世界！ this is long long string sample. 寿限無、寿限無、五劫の擦り切れ、海砂利水魚の、水行末・雲来末・風来末、喰う寝る処に住む処、藪ら柑子の藪柑子、パイポ・パイポ・パイポのシューリンガン、シューリンガンのグーリンダイ、グーリンダイのポンポコピーのポンポコナの、長久命の長助";
static constexpr size_t textlen = sizeof(text) / sizeof(text[0]);
size_t textpos = 0;

void setup(void) 
{
  lcd.init();

  // 画面が横長になるように回転
  if (lcd.width() < lcd.height()) lcd.setRotation(lcd.getRotation() ^ 1);

  canvas.setColorDepth(8);
  canvas.setFont(&fonts::lgfxJapanMinchoP_32);
  canvas.setTextWrap(false);        // 右端到達時のカーソル折り返しを禁止
  canvas.createSprite(lcd.width() + 36, 36); // 画面幅+１文字分の横幅を用意
}

void loop(void)
{
  int32_t cursor_x = canvas.getCursorX() - 1;   // 現在のカーソル位置を取得し、1ドット左に移動
  if (cursor_x <= 0) // カーソル位置が左端に到達している場合は一周したと判断
  {
    textpos = 0;            // 文字列の読取り位置をリセット
    cursor_x = lcd.width(); // 新たな文字が画面右端に描画されるようにカーソル位置を変更
  }

  canvas.setCursor(cursor_x, 0); // カーソル位置を更新
  canvas.scroll(-1, 0);          // キャンバスの内容を1ドット左にスクロール
  while (textpos < textlen && cursor_x <= lcd.width()) // 画面右端に文字が書けるか判定
  {
    canvas.print(text[textpos++]);   // 1バイトずつ出力 (マルチバイト文字でもこの処理で動作します)
    cursor_x = canvas.getCursorX();  // 出力後のカーソル位置を取得
  }

  canvas.pushSprite(&lcd, 0, 0);
}
