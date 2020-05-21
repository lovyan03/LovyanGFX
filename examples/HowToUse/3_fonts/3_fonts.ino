#include <LovyanGFX.hpp>

static LGFX lcd;

void drawGradation(void)
{
  // 背景にグラデーションを描画する
  lcd.startWrite();
  for (int y = 0; y < 512; ++y) {
    for (int x = 0; x < 512; ++x) {
      lcd.writePixel(x, y, lcd.color888(x>>1, (x + y) >> 2, y>>1));
    }
  }
  lcd.endWrite();
}

void setup(void)
{
  lcd.init();

  drawGradation();

  // 文字の描画方法には大きく分けて２通り、print 系の関数と drawString 系の関数があります。

  // drawString 関数では、第１引数で文字列を指定し、第２引数でX座標、第３引数でY座標を指定します。
  lcd.drawString("string!", 10, 10);

  // drawNumber 関数では、第１引数が数値になります。
  lcd.drawNumber(123, 100, 10);

  // drawFloat 関数では、第１引数が数値、第２引数が小数点以下の桁数になり、第３引数がX座標、第４引数がY座標になります。
  lcd.drawFloat(3.14, 2, 150, 10);

  // print 関数では、setCursor関数で指定した座標 (またはprint関数で最後に描画した文字の続き)に描画します。
  lcd.setCursor(10, 20);
  lcd.print("print!");

  // printf関数で、第２引数以降の内容を描画できます。(C言語のprintf準拠ですので文字列や浮動小数も描画できます)
  int value = 123;
  lcd.printf("test %d", value);

  // println関数で、文字列を描画後に改行できます。print("\n");と同じ効果です。
  lcd.println("println");

  // フォントを変更するには、setFont関数を使用します。
  // TFT_eSPIのsetTextFont関数と同じフォントは Font0 ～ Font8 になります。
  // ※ エディタの入力支援が使える場合、引数に&fonts::まで入力する事でフォント一覧が表示されます
  lcd.setFont(&fonts::Font4);
  lcd.println("TestFont4");

  // TFT_eSPIとの互換性のためにsetTextFont関数による番号でのフォント変更にも対応しています。
  // 引数に指定できる数字は 0, 2, 4, 6, 7, 8 です。(TFT_eSPI準拠です)
  // ※ ただし この方法は、他の番号のフォントも強制的にバイナリに含まれサイズが膨らむため、非推奨です。
  lcd.setTextFont(2);
  lcd.println("TestFont2");


  // setTextColorで色を変更できます。
  // １つ目の引数が文字色、２つ目の引数が背景色になります。
  lcd.setTextColor(0x00FFFFU, 0xFF0000U);
  lcd.print("CyanText RedBack");
  // ※ 同じ場所に文字を繰り返し描画し直したい場合、背景色を指定して重ね書きすることを推奨します。
  //    fillRect等で消去してから書き直すと、ちらつきが発生する可能性があります。


  // setTextColorで第１引数のみを指定し第２引数を省略した場合は、
  // 背景を塗り潰さず文字だけを描画します。
  lcd.setTextColor(0xFFFF00U);
  lcd.print("YellowText ClearBack");


  // Font6は時計用の文字のみが収録されています。
  lcd.setFont(&fonts::Font6);
  lcd.print("apm.:-0369");

  // Font7は７セグメント液晶風のフォントが収録されています。
  lcd.setFont(&fonts::Font7);
  lcd.print(".:-147");

  // Font8は数字のみが収録されています。
  lcd.setFont(&fonts::Font8);
  lcd.print(".:-258");


  delay(3000);
  drawGradation();


  // LovyanGFXでは AdafruitGFX フォントも setFont 関数で使用できます。
  // (TFT_eSPIとの互換性のために setFreeFont関数も用意しています)
  lcd.setFont(&fonts::FreeSerif9pt7b);


  // 右揃え や 中央揃え で描画したい場合は、setTextDatum 関数で基準位置を指定します。
  // 縦方向が top、middle、baseline、bottomの4通り、横方向が left、center、rightの3通りです。
  // 縦と横の指定を組み合わせた12通りの中から指定します。
  lcd.setTextDatum( textdatum_t::top_left        );
  lcd.setTextDatum( textdatum_t::top_center      );
  lcd.setTextDatum( textdatum_t::top_right       );
  lcd.setTextDatum( textdatum_t::middle_left     );
  lcd.setTextDatum( textdatum_t::middle_center   );
  lcd.setTextDatum( textdatum_t::middle_right    );
  lcd.setTextDatum( textdatum_t::baseline_left   );
  lcd.setTextDatum( textdatum_t::baseline_center );
  lcd.setTextDatum( textdatum_t::baseline_right  );
  lcd.setTextDatum( textdatum_t::bottom_left     );
  lcd.setTextDatum( textdatum_t::bottom_center   );
  lcd.setTextDatum( textdatum_t::bottom_right    );
  // ※  "textdatum_t::" は省略可能です
  // ※ print系関数には縦方向の指定のみ効果があり、横方向の指定は効果がありません。

  // 右下揃え
  lcd.setTextDatum( bottom_right );
  lcd.drawString("bottom_right",  lcd.width() / 2,  lcd.height() / 2);

  // 左下揃え
  lcd.setTextDatum( bottom_left );
  lcd.drawString("bottom_left",  lcd.width() / 2,  lcd.height() / 2);

  // 右上揃え
  lcd.setTextDatum( top_right );
  lcd.drawString("top_right",  lcd.width() / 2,  lcd.height() / 2);

  // 左上揃え
  lcd.setTextDatum( top_left );
  lcd.drawString("top_left",  lcd.width() / 2,  lcd.height() / 2);


  // 基準座標に中心線を描画
  lcd.drawFastVLine(lcd.width() / 2, 0, lcd.height(), 0xFFFFFFU);
  lcd.drawFastHLine(0, lcd.height() / 2, lcd.width(), 0xFFFFFFU);


  delay(3000);
  drawGradation();

  lcd.setFont(&Font2);
  lcd.setCursor(0, 0);


  lcd.drawRect(8, 8, lcd.width() - 16, lcd.height() - 16, 0xFFFFFFU);

  // setClipRect関数で描画する範囲を限定できます。指定した範囲外には描画されなくなります。
  // ※ テキスト系のみならず、すべての描画関数に影響します。
  lcd.setClipRect(10, 10, lcd.width() - 20, lcd.height() - 20);


  // setTextSize 関数で 文字の拡大率を指定します。
  // 第１引数で横方向の倍率、第２引数で縦方向の倍率を指定します。
  // 第２引数を省略した場合は、第１引数の倍率が縦と横の両方に反映されます。
  lcd.setTextSize(3, 4);
  lcd.println("Size 3 x 4");

  lcd.setTextSize(2);
  lcd.println("Size 2 x 2");

  lcd.setTextSize(1, 2);
  lcd.println("Size 1 x 2");

  lcd.setTextSize(1);

  // setTextWrap 関数で、print 関数が画面端(描画範囲端)に到達した時の折り返し動作を指定します。
  // 第１引数をtrueにすると、右端到達後に左端へ移動します。
  // 第２引数をtrueにすると、下端到達後に上端へ移動します。(省略時:false)
  lcd.setTextWrap(false);
  lcd.setTextColor(0x00FFFFU, 0);
  lcd.println("setTextWrap(false) testing... long long long long string wrap test string ");
  // false指定時は位置調整されず、描画範囲外にはみ出した部分は描画されません。

  lcd.setTextWrap(true);
  lcd.setTextColor(0xFFFF00U, 0);
  lcd.println("setTextWrap(true) testing... long long long long string wrap test string ");
  // true指定時は描画範囲内に収まるよう座標を自動調整します。

  delay(1000);

  // 第２引数にtrue指定時は、画面下端に到達すると続きを上端から描画します。
  lcd.setTextColor(0xFFFFFFU, 0);
  lcd.setTextWrap(true, true);
  lcd.println("setTextWrap(true, true) testing...");
  for (int i = 0; i < 100; ++i) {
    lcd.printf("wrap test %03d ", i);
    delay(50);
  }


  drawGradation();

  // setTextScroll 関数で、画面下端に到達した時のスクロール動作を指定します。
  // setScrollRect 関数でスクロールする矩形範囲を指定します。(未指定時は画面全体がスクロールします)
  // ※ スクロール機能は、LCDが画素読出しに対応している必要があります。
  lcd.setTextScroll(true);

  // 第１～第４引数で X Y Width Height の矩形範囲を指定し、第５引数でスクロール後の色を指定します。第５引数は省略可(省略時は変更なし)
  lcd.setScrollRect(10, 10, lcd.width() - 20, lcd.height() - 20, 0x00001FU);

  for (int i = 0; i < 50; ++i) {
    lcd.printf("scroll test %d \n", i);
  }


  // setClipRectの範囲指定を解除します。
  lcd.clearClipRect();

  // setScrollRectの範囲指定を解除します。
  lcd.clearScrollRect();


  lcd.setTextSize(1);
  lcd.setTextColor(0xFFFFFFU, 0);


  // setTextPadding 関数で、drawString 系関数で背景塗り潰し時の最小幅を指定できます。
  lcd.setTextPadding(100);


  drawGradation();
}

void drawNumberTest(const lgfx::IFont* font)
{
  lcd.setFont(font);

  lcd.fillScreen(0x0000FF);

  lcd.setColor(0xFFFF00U);
  lcd.drawFastVLine( 80, 0, 240);
  lcd.drawFastVLine(160, 0, 240);
  lcd.drawFastVLine(240, 0, 240);
  lcd.drawFastHLine(0,  45, 320);
  lcd.drawFastHLine(0,  95, 320);
  lcd.drawFastHLine(0, 145, 320);
  lcd.drawFastHLine(0, 195, 320);

  for (int i = 0; i < 200; ++i) {
    lcd.setTextDatum( bottom_right    );     lcd.drawNumber(i,  80,  45);
    lcd.setTextDatum( bottom_center   );     lcd.drawNumber(i, 160,  45);
    lcd.setTextDatum( bottom_left     );     lcd.drawNumber(i, 240,  45);
    lcd.setTextDatum( baseline_right  );     lcd.drawNumber(i,  80,  95);
    lcd.setTextDatum( baseline_center );     lcd.drawNumber(i, 160,  95);
    lcd.setTextDatum( baseline_left   );     lcd.drawNumber(i, 240,  95);
    lcd.setTextDatum( middle_right    );     lcd.drawNumber(i,  80, 145);
    lcd.setTextDatum( middle_center   );     lcd.drawNumber(i, 160, 145);
    lcd.setTextDatum( middle_left     );     lcd.drawNumber(i, 240, 145);
    lcd.setTextDatum( top_right       );     lcd.drawNumber(i,  80, 195);
    lcd.setTextDatum( top_center      );     lcd.drawNumber(i, 160, 195);
    lcd.setTextDatum( top_left        );     lcd.drawNumber(i, 240, 195);
  }
}

void loop(void)
{
// ※ 名前が"Free"で始まるフォントは 9pt 12pt 18pt 24ptの４種類があります。
  drawNumberTest(&Font0);
  drawNumberTest(&Font2);
  drawNumberTest(&Font4);
  drawNumberTest(&Font6);
  drawNumberTest(&Font7);
  drawNumberTest(&Font8);
  drawNumberTest(&TomThumb);
  drawNumberTest(&FreeMono9pt7b);
  drawNumberTest(&FreeMonoBold9pt7b);
  drawNumberTest(&FreeMonoOblique9pt7b);
  drawNumberTest(&FreeMonoBoldOblique9pt7b);
  drawNumberTest(&FreeSans9pt7b);
  drawNumberTest(&FreeSansBold9pt7b);
  drawNumberTest(&FreeSansOblique9pt7b);
  drawNumberTest(&FreeSansBoldOblique9pt7b);
  drawNumberTest(&FreeSerif9pt7b);
  drawNumberTest(&FreeSerifBold9pt7b);
  drawNumberTest(&FreeSerifItalic9pt7b);
  drawNumberTest(&FreeSerifBoldItalic9pt7b);
  drawNumberTest(&Orbitron_Light_24);
  drawNumberTest(&Roboto_Thin_24);
  drawNumberTest(&Satisfy_24);
  drawNumberTest(&Yellowtail_32);
}
