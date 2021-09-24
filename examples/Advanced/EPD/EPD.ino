
// M5PaperやCoreInkのライブラリと同時に利用する場合はLovyanGFX.hppより前にincludeします。
// If you use it with M5Paper or CoreInk libraries, include it before LovyanGFX.hpp.
// #include <M5EPD.h>
// #include <M5CoreInk.h>

// 使用するボードに応じた define を記述します。
// （ボードマネージャで該当ボードを選択している場合は省略しても構いません）
// #define LGFX_M5PAPER
// #define LGFX_M5STACK_COREINK
   #define LGFX_AUTODETECT      // 自動検出を使用する場合はこちらの記述だけで動作します。

// 使用ボードのdefineより後にLovyanGFX.hppをincludeします。
#include <LovyanGFX.hpp>

LGFX gfx;
LGFX_Sprite sp(&gfx);

int w = 200;
int h = 200;

#ifdef min
#undef min
#endif

void setup(void)
{
// M5.begin();

  gfx.init();   // 初期化を行います。LCDもEPDも共通です。

  gfx.setRotation(0);  // 回転方向の設定はLCDもEPDも共通です。0～3で右回りに90度ずつ回転します。4～7は上下反転になります。

  w = gfx.width();
  h = gfx.height();

  gfx.setBrightness(128); // バックライトの輝度設定はEPDでは効果を持ちません。

// EPDの動作モードを設定できます。描画用途に応じて 都度、変更してください。
// ※ M5Stack CoreInk では epd_quality以外は差はありません。
  gfx.setEpdMode(epd_mode_t::epd_fastest);  // 最速更新、白黒反転なし、残像が残りやすい
  gfx.setEpdMode(epd_mode_t::epd_fast);     // 高速更新、白黒反転なし、残像が残りやすい
  gfx.setEpdMode(epd_mode_t::epd_text);     // 高品質更新、白黒反転が一瞬起きる（白背景用）
  gfx.setEpdMode(epd_mode_t::epd_quality);  // 高品質更新、白黒反転が一瞬起きる

// M5Paper (IT8951)での各モードの特徴は以下の通りです。
// epd_fastest = DU4  更新時間 120msec  完全な白と黒のみ描画でき、中間階調は描画されない。また、中間階調で表示されている箇所を更新できない。
// epd_fast    = DU   更新時間 260msec  完全な白と黒のみ描画でき、中間階調は描画されない。
// epd_text    = GL16 更新時間 450msec  グレースケール16階調で描画できる。白背景・黒文字 用途
// epd_quality = GC16 更新時間 450msec  グレースケール16階調で描画できる。画像用途


// 描画関数はすべてLCDと同様に利用可能です。
// 色指定もLCDと同様に指定できますが、自動的にグレースケールに変換されます。
// （グレースケール変換時の比率は R1:G2:B1 です。緑色がやや明るく表現されます。）

  int rectwidth = std::min(w, h) / 2;
  gfx.fillTriangle( w / 2, 0, 0, h - 1, w - 1, h - 1, TFT_RED);
  gfx.fillCircle(w/2, h/2, rectwidth, TFT_GREEN);
  gfx.fillRect((w-rectwidth)/2, (h-rectwidth)/2, rectwidth, rectwidth, TFT_BLUE);

  delay(3000);


// 描画処理をstartWrite/endWriteで囲むと、endWrite() のタイミングでまとめて画面に反映できます。
// または、display() を呼んだ時点でも画面に反映できます。

  gfx.startWrite(); // 描画内容の即時反映を抑止。

  for (int i = 0; i < 20; ++i)
  {
    gfx.drawLine(i * w / 20, 0, w - 1, i * h / 20, TFT_BLACK);  // この時点では画面に反映されない。
    gfx.drawLine(0, i * h / 20, i * w / 20, h - 1, TFT_BLACK);
  }

  gfx.endWrite();   // ここで画面に反映される。

  delay(3000);

  gfx.startWrite(); // 描画内容の即時反映を抑止。

  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 20; j++)
    {
      gfx.fillRect(random(w-20), random(h-20), 20, 20, random(65535));  // この時点では画面に反映されない。
    }
    gfx.display();  // ここで画面に反映される。
  }

  gfx.endWrite();   // すでに画面に反映済みの場合は、この時点では何も起きない。

  delay(3000);

// ※ 正確には、  display() を呼んだ時にのみ 画面に反映される仕組みなのですが、
// SPIバス解放のタイミングで display() を呼ぶ仕組みを用意しており、初期値で有効にしています。
// この仕組みにより、描画関数やendWriteを呼んだ直後に自動で画面に反映されています。
// なお、 setAutoDisplay(bool) でこの自動display呼出しの有効／無効を変更できます。

  gfx.setAutoDisplay(false);  // 自動表示更新を無効にする。（以降は display()を呼ぶまで画面に反映されない。）

  gfx.setFont(&fonts::Font4);
  gfx.setTextColor(TFT_BLACK, TFT_WHITE); // 文字色を黒、背景色を白に指定
  gfx.setTextDatum(textdatum_t::top_center);
  for (int i = 0; i < 10; i++)
  {
    gfx.drawString("Hello World !!", w / 2, i * h / 10);
  }
  gfx.qrcode("Hello world !", (w-rectwidth)/2, (h-rectwidth)/2, rectwidth);

  gfx.display();   // ここで画面に反映される。

  gfx.setAutoDisplay(true);  // 自動表示更新を有効にする。

  delay(3000);

  gfx.fillScreen(TFT_WHITE);

// M5Paper (IT8951)では epd_quality/epd_text を使用するとグレースケール16階調の表示が可能ですが、
// epd_fast/epd_fastestを使用した場合は白黒２階調のみに制限されます。
// この場合でもLovyanGFXのタイルパターン処理により疑似的に17階調を表現できます。

  gfx.setEpdMode(epd_mode_t::epd_quality); // 高品質更新モードに設定（以後の描画はグレースケールを使用する）
  gfx.startWrite();
  for (int i = 0; i < 16; i++)
  {
    int level = 8 + i * 16;
    gfx.fillRect(i * w / 16, 0, w / 16 + 1, h / 2, gfx.color888(level, level, level));
  }
  gfx.display();  // ここでの表示更新は高品質モードとなる。

  gfx.waitDisplay(); // EPDの表示更新の完了待機。
  // ※ 待機せずに表示更新中かどうかを調べたい場合は displayBusy() を使用します。
  //    他の処理の合間に描画を行いたい場合などにご利用ください。
  // 例 : while (gfx.displayBusy()) delay(10); // delayの代わりに何か他の処理を指定


  gfx.setEpdMode(epd_mode_t::epd_fast);  // 高速更新モードに設定（以後の描画は白黒２値を使用する）
  for (int i = 0; i < 17; i++)
  {
    int level = std::min(255, i * 16);
    gfx.fillRect(i * w / 17, h / 2, w / 17+1, h / 2, gfx.color888(level, level, level));
  }
  gfx.display();  // ここでの描画は高速モードとなる。

  delay(3000);

  // M5Paper (IT8951)の高品質モードではグレースケール16階調＋タイルパターン処理により241階調が表現できます。
  gfx.setEpdMode(epd_mode_t::epd_quality);

  for (int i = 0; i < 256; ++i)
  {
    gfx.fillRect(0, (i * h) / 256, w, 4, gfx.color888(i, i, i));
  }
  gfx.display();

  gfx.endWrite();

  delay(3000);

  // M5Paper (IT8951)の表示更新は複数個所を同時に行う事が可能です。
  gfx.startWrite();
  for (int i = 0; i < 16; ++i)
  {
    std::int32_t y1 = (i * h) / 16, y2 = ((i + 1) * h) / 16;
    std::int32_t x1 = 0, x2 = 0;
    for (int j = 0; j < 16; ++j)
    {
      x2 = ((j + 1) * w) / 16;
      std::int_fast8_t l = 255 - (i * 16 + j);
      gfx.fillRect(x1, y1, x2 - x1, y2 - y1, gfx.color888(l, l, l));
      x1 = x2;
      if ((j & 7) == 7)  gfx.display(); // ８回に１回 表示更新を行う
    }
  }
  gfx.endWrite();

  // ※ 表示更新中の範囲への描画をしないように注意してください。
  //    表示更新の途中で内容が変更されると正しく描画されなくなります。

  delay(3000);

  gfx.fillScreen(TFT_WHITE);

  // 前回の表示更新範囲と重なる範囲に描画する場合、
  // LovyanGFX内部で表示更新が完了するのを待機する仕組みになっています。
  // そのため特に意識しなくても表示が乱れることがありません。
  gfx.setEpdMode(epd_mode_t::epd_quality);
  gfx.fillRect(0, 0, w/2, h/2, TFT_BLUE);  // これらの描画は範囲が重なっているが、
  gfx.fillRect(0, 0, w/3, h/3, TFT_YELLOW);// 表示更新を待機する仕組みが機能するため
  gfx.fillRect(0, 0, w/4, h/4, TFT_BLUE);  // 特に意識しなくても描画が乱れることがない
  gfx.fillRect(0, 0, w/5, h/5, TFT_YELLOW);
  gfx.fillRect(0, 0, w/6, h/6, TFT_BLUE);
  gfx.fillRect(0, 0, w/7, h/7, TFT_YELLOW);

  gfx.waitDisplay();

  // しかし自動チェックは完全ではありません。「前回の表示更新範囲」と比較する仕組みのため、
  // 別の範囲への描画を間に挟むと範囲チェックが十分に機能しなくなります。
  gfx.fillRect(w/2, 0, w/2, h/2, TFT_BLUE);  // 範囲が重ならない描画を交互に行うと、
  gfx.drawPixel(0, 0);                       // 表示更新を待機する仕組みが機能せず、
  gfx.fillRect(w/2, 0, w/3, h/3, TFT_YELLOW);// 正しく描画されないようになる。
  gfx.drawPixel(0, 0);
  gfx.fillRect(w/2, 0, w/4, h/4, TFT_BLUE);  // ※ 先の例と同じ色で描画しているのに
  gfx.drawPixel(0, 0);                       // 表示される階調が違っていたり、
  gfx.fillRect(w/2, 0, w/5, h/5, TFT_YELLOW);// 正しい描画が行われていないことを確認してください
  gfx.drawPixel(0, 0);
  gfx.fillRect(w/2, 0, w/6, h/6, TFT_BLUE);
  gfx.drawPixel(0, 0);
  gfx.fillRect(w/2, 0, w/7, h/7, TFT_YELLOW);

  gfx.waitDisplay();

  // また、表示更新モードが epd_fastest モードの場合は、レスポンスを最優先とするため、
  // 表示更新範囲のチェック処理を省略するようになります。
  gfx.setEpdMode(epd_mode_t::epd_fastest);    // 最速更新モードに設定する。
  gfx.fillRect(0, h/2, w/2, h/2, TFT_BLUE);   // 以後の描画は表示更新範囲と重複していても、
  gfx.fillRect(0, h/2, w/3, h/3, TFT_YELLOW); // 一切待機せずに描画するようになる。
  gfx.fillRect(0, h/2, w/4, h/4, TFT_BLUE);   // そのため範囲の重なった描画を続けて行うと、
  gfx.fillRect(0, h/2, w/5, h/5, TFT_YELLOW); // 表示更新中に内容を書き換えてしまい
  gfx.fillRect(0, h/2, w/6, h/6, TFT_BLUE);   // 意図した描画結果にならない。
  gfx.fillRect(0, h/2, w/7, h/7, TFT_YELLOW);

  gfx.fillRect(w/2, h/2, w/2, h/2, TFT_BLUE);
  gfx.waitDisplay();                           // 必要に応じてwaitDisplayで待機させる。
  gfx.fillRect(w/2, h/2, w/3, h/3, TFT_YELLOW);// 正しく待機することで、
  gfx.waitDisplay();                           // 描画結果の乱れを防止できる。
  gfx.fillRect(w/2, h/2, w/4, h/4, TFT_BLUE);
  gfx.waitDisplay();                           // 表示結果が先の例と違うことを確認してください
  gfx.fillRect(w/2, h/2, w/5, h/5, TFT_YELLOW);
  gfx.waitDisplay();
  gfx.fillRect(w/2, h/2, w/6, h/6, TFT_BLUE);
  gfx.waitDisplay();
  gfx.fillRect(w/2, h/2, w/7, h/7, TFT_YELLOW);

  delay(3000);


  gfx.setEpdMode(epd_mode_t::epd_quality);

  gfx.fillScreen(TFT_WHITE);

  if (gfx.touch())  // touch関数の戻り値がnullかどうかでタッチコントローラの有無を判定できます。
  {
    gfx.startWrite();
    for (int i = 0; i < 1024; i++)
    {
      if ((i & 255) == 0)
      {
        gfx.setEpdMode(epd_mode_t::epd_fast);
        gfx.fillScreen(TFT_WHITE);
        gfx.setTextColor(TFT_BLACK, TFT_WHITE);
        gfx.setTextSize(3, 3);
        gfx.drawString("Touch Test", w / 2, h / 2);
        gfx.display();
        delay(100);
        gfx.waitDisplay();
        gfx.setEpdMode(epd_mode_t::epd_fastest);
      }
      delay(15);

      std::int32_t x, y, number = 0;
      while (gfx.getTouch(&x, &y, number))  // getTouch関数でタッチ中の座標を取得できます。
      {
        gfx.fillCircle(x, y, 5, (std::uint32_t)(number * 0x333333u));
        gfx.display();
        ++number;
      }
    }
    gfx.endWrite();
  }

  gfx.setEpdMode(epd_mode_t::epd_fast);
  gfx.fillScreen(TFT_WHITE);

  sp.setColorDepth(4);
  if (!sp.createSprite(w / 2, h / 2)) sp.createSprite(w / 4, h / 4);
  sp.setFont(&fonts::Font8);
}

void loop(void)
{
  static int count = 0;

  if (0 == (count & 15))
  {
    switch ((count >> 4) & 3)
    {
    case 0:  gfx.setEpdMode(epd_mode_t::epd_quality);  break;
    case 1:  gfx.setEpdMode(epd_mode_t::epd_text   );  break;
    case 2:  gfx.setEpdMode(epd_mode_t::epd_fast   );  break;
    case 3:  gfx.setEpdMode(epd_mode_t::epd_fastest);  break;
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

  gfx.waitDisplay();
  sp.pushSprite(count & 1 ? w / 2 : 0, count & 2 ? h / 2 : 0);
  ++count;
}
