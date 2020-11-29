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

  gfx.setRotation(0);  // 回転方向の設定はLCDもEPDも共通です。0～3で右回りに90度ずつ回転します。4～7は上下反転になります。

  w = gfx.width();
  h = gfx.height();

  gfx.setBrightness(50); // バックライトの輝度設定はEPDでは効果を持ちません。


// EPDの動作モードを設定できます。描画用途に応じて事前に設定してください。
// ※ 現在、M5Stack CoreInk には設定の効果はありません。
  gfx.setEpdMode(epd_mode_t::epd_fastest);  // 最速更新、白黒反転なし、残像が残りやすい
  gfx.setEpdMode(epd_mode_t::epd_fast);     // 高速更新、白黒反転なし、残像が残りやすい
  gfx.setEpdMode(epd_mode_t::epd_quality);  // 高品質更新、白黒反転が一瞬起きる

// M5Paper (IT8951)での描画更新に必要な時間は以下の通りです。
// epd_fastest = 120msec
// epd_fast    = 260msec
// epd_quality = 450msec


// 描画関数はすべてLCDと同様に利用可能ですが、画面への反映は display(); を呼び出すまで行われません。
// 色指定もLCDと同様に指定できますが、自動的にグレースケールに変換されます。
// （グレースケール変換時の比率は R1:G2:B1 です。緑色がやや明るく表現されます。）

  int rectwidth = std::min(w, h) / 2;
  gfx.fillCircle(w/2, h/2, rectwidth, TFT_BLUE);
  gfx.fillRect((w-rectwidth)/2, (h-rectwidth)/2, rectwidth, rectwidth, TFT_YELLOW);
  gfx.drawLine(0, 0, w-1, h-1, TFT_GREEN);
  gfx.drawLine(0, h-1, w-1, 0, TFT_RED);

  gfx.display();     // ここで表示が更新される。

  gfx.waitDisplay(); // EPDの表示更新の完了待機。（省略可）

  gfx.setTextColor(TFT_BLACK);
  gfx.setFont(&fonts::Font4);
  gfx.drawString("Hello World !!", 0, 0);
  gfx.qrcode("Hello world !", (w-rectwidth)/2, (h-rectwidth)/2, rectwidth);

  gfx.display();    // ここで表示が更新される。

  delay(3000);

  gfx.startWrite();
  for (int i = 0; i < 10; i++)
  {
    gfx.fillTriangle(random(w), random(h), random(w), random(h), random(w), random(h), random(65535));
  }
  gfx.endWrite();

  gfx.display();    // ここで表示が更新される。

  delay(3000);

  gfx.startWrite();
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 10; j++)
    {
      gfx.fillRect(random(w-20), random(h-20), 20, 20, random(65535));
    }
    gfx.display();    // ここで表示が更新される。
  }
  gfx.endWrite();

  delay(3000);

// M5Paper (IT8951)では epd_quality を使用するとグレースケール16諧調が使用できますが、
// epd_fast/epd_fastestを使用した場合は白黒２諧調に制限されます。
// この場合でもLovyanGFXのタイルパターン処理により疑似的に17諧調を表現できます。

  gfx.setEpdMode(epd_mode_t::epd_quality); // 高品質更新モードに設定（以後の描画はグレースケールを使用する）
  gfx.startWrite();
  for (int i = 0; i < 17; i++)
  {
    gfx.fillRect(i * w / 17, 0, w / 17 + 1, h / 2, gfx.color888(i*15.9, i*15.9, i*15.9));
  }
  gfx.display();  // ここでの表示更新は高品質モードとなる。

  delay(100);
  //gfx.waitDisplay(); // EPDの表示更新の完了待機。

  gfx.setEpdMode(epd_mode_t::epd_fast);  // 高速更新モードに設定（以後の描画は白黒２値を使用する）
  for (int i = 0; i < 17; i++)
  {
    gfx.fillRect(i * w / 17, h / 2, w / 17+1, h / 2, gfx.color888(i*15.9, i*15.9, i*15.9));
  }
  gfx.display();  // ここでの描画は高速モードとなる。

  delay(3000);

  // M5Paper (IT8951)の高品質モードではグレースケール16諧調＋タイルパターン処理により241諧調が表現できます。
  gfx.setEpdMode(epd_mode_t::epd_quality);

  for (int i = 0; i < 241; ++i)
  {
    gfx.fillRect(0, (i * h) / 241, w, 4, gfx.color888(i*1.065, i*1.065, i*1.065));
  }
  gfx.display();

  gfx.endWrite();

  delay(3000);

  // M5Paper (IT8951)の表示更新は複数個所を同時に行う事が可能です。
  // ただし表示更新中の範囲への変更は避けることをお勧めします。
  // また、なるべく表示更新の範囲が同時に重ならないようにすることをお勧めします。
  for (int i = 0; i < 16; ++i)
  {
    for (int j = 0; j < 16; ++j)
    {
      gfx.fillRect((j * w) / 16, (i * h) / 16, w/16, h/16, ~gfx.color888(i*16+j, i*16+j, i*16+j));
      gfx.display();
    }
  }

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
  gfx.display();
  delay(1000);

  ++count;
}
