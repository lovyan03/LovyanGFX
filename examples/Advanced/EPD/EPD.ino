// 使用するボードに応じた define を記述します。
// （ボードマネージャで該当ボードを選択している場合は省略しても構いません）
// #define LGFX_M5PAPER
// #define LGFX_M5STACK_COREINK
#define LGFX_AUTODETECT      // 自動検出を使用する場合はこちらの記述だけで動作します。

// 使用ボードのdefineより後にLovyanGFX.hppをincludeします。
#include <LovyanGFX.hpp>

LGFX gfx;
LGFX_Sprite sp(&gfx);

int w;
int h;

void setup(void)
{
  gfx.init();   // 初期化を行います。LCDもEPDも共通です。

  gfx.setRotation(0);    // 回転方向の設定はLCDもEPDも共通です。

  w = gfx.width();
  h = gfx.height();

  gfx.setBrightness(50); // バックライトの輝度設定はEPDでは効果を持ちません。


// EPDの動作モードを設定できます。描画用途に応じて事前に設定してください。
// ※ 現在、M5Stack CoreInk は設定の効果はありません。
  gfx.setEpdMode(epd_mode_t::epd_fastest);  // 最速更新、白黒反転なし、残像が残りやすい
  gfx.setEpdMode(epd_mode_t::epd_fast);     // 高速更新、白黒反転なし、残像が残りやすい
  gfx.setEpdMode(epd_mode_t::epd_quality);  // 高品質更新、白黒反転が一瞬起きる

// M5Paperでの描画更新に必要な時間は以下の通りです。
// epd_quality = 450msec
// epd_fast    = 260msec
// epd_fastest = 120msec


// 描画関数はすべてLCDと同様に利用可能です。
// 色指定もLCDと同様に指定できますが、自動的にグレースケールに変換されます。
// （グレースケール変換時の比率は R1:G2:B1 です。緑色がやや明るく表現されます。）
  int rectwidth = std::min(w, h) / 2;
  gfx.fillCircle(w/2, h/2, rectwidth, TFT_BLUE);
  gfx.fillRect((w-rectwidth)/2, (h-rectwidth)/2, rectwidth, rectwidth, TFT_YELLOW);
  gfx.drawLine(0, 0, w-1, h-1, TFT_GREEN);
  gfx.drawLine(0, h-1, w-1, 0, TFT_RED);

  gfx.setTextColor(TFT_BLACK);
  gfx.setFont(&fonts::Font4);
  gfx.drawString("Hello World !!", 0, 0);
  gfx.qrcode("Hello world !", (w-rectwidth)/2, (h-rectwidth)/2, rectwidth);

  delay(3000);

// EPDの場合はstartWrite/endWriteで囲むことで、endWrite() や display() のタイミングでまとめて画面に反映できます。

  gfx.startWrite(); // 描画内容の即時反映を抑止。
  for (int i = 0; i < 10; i++)
  {
    gfx.fillTriangle(random(w), random(h), random(w), random(h), random(w), random(h), random(65535));
  }
  gfx.endWrite();  // ここで画面に反映される。

  gfx.waitDisplay(); // EPDの描画更新の完了待機をしたい場合はwaitDisplay()を使用します。

  delay(3000);

  gfx.startWrite(); // 描画内容の即時反映を抑止。
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 10; j++)
    {
      gfx.fillRect(random(w-20), random(h-20), 20, 20, random(65535));
    }
    gfx.display(); // display()関数を呼び出すことでも画面に反映できる
  }
  gfx.endWrite();  // すでに画面に反映済の場合は、ここでの更新は行われない。

  delay(3000);


// M5Paperでは epd_quality を使用するとグレースケール16諧調が使用できますが、
// epd_fast/epd_fastestを使用した場合は白黒２諧調に制限されます。
// LovyanGFXのタイルパターン処理により疑似的に16諧調を表現できます。

  gfx.setEpdMode(epd_mode_t::epd_quality); // 高品質更新モードに設定（以後の描画はグレースケールを使用する）
  gfx.startWrite();
  for (int i = 0; i < 16; i++)
  {
    gfx.fillRect(i * w / 16, 0, w / 16 + 1, h / 2, gfx.color888(i*17, i*17, i*17));
  }
  gfx.endWrite();  // ここで画面に反映される。高品質モードでの描画更新となる。


  gfx.setEpdMode(epd_mode_t::epd_fast);  // 高速更新モードに設定（以後の描画は白黒２値を使用する）
  gfx.startWrite();
  for (int i = 0; i < 16; i++)
  {
    gfx.fillRect(i * w / 16, h / 2, w / 16+1, h / 2, gfx.color888(i*17, i*17, i*17));
  }
  gfx.endWrite();  // ここで画面に反映される。

  delay(3000);

  // M5Paperの高品質モードではグレースケール16諧調＋タイルパターン処理による241諧調が表現できます。
  gfx.setEpdMode(epd_mode_t::epd_quality);
  gfx.startWrite();
  for (int i = 0; i < 256; ++i)
  {
    gfx.fillRect(0, (i * h) >> 8, w, 4, gfx.color888(i, i, i));
  }
  gfx.endWrite();

  delay(3000);


  sp.setColorDepth(4);
  sp.createSprite(w / 2, h / 2);
  sp.setFont(&fonts::Font8);
}

void loop(void)
{
  static int count;
  if (0 == (count & 15))
  {
    switch ((count >> 4) & 1)
    {
    case 0:  gfx.setEpdMode(epd_mode_t::epd_fast   );  break;
    case 1:  gfx.setEpdMode(epd_mode_t::epd_quality);  break;
    }
  }

  for (int y = 0; y < sp.height(); y += 4) {
    for (int x = 0; x < sp.width(); x += 4) {
      std::uint32_t val = ((x+count*4)|(y+count*4)) >> 4;
      sp.fillRect(x, y, 4, 4, val);
    }
  }

  int x = 2;
  int y = 2;
  sp.setTextColor(TFT_BLACK);
  sp.drawNumber(count, x-2, y-2);
  sp.drawNumber(count, x+2, y-2);
  sp.drawNumber(count, x+2, y+2);
  sp.drawNumber(count, x-2, y+2);
  sp.setTextColor(TFT_WHITE);
  sp.drawNumber(count, x, y);

  sp.pushSprite(count & 1 ? w / 2 : 0, count & 2 ? h / 2 : 0);
  delay(2000);
  ++count;
}
