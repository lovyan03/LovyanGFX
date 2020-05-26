
// このサンプルの実行にはArduino-misakiUTF16が必要です。
// need misakifont to run this example.
//
// URL : https://github.com/Tamakichi/Arduino-misakiUTF16
//

// Arduino-misakiUTF16のフォントデータをincludeします。
// Include the font data of Arduino-misakiUTF16.
#include "misakiUTF16FontData.h"

#include <LovyanGFX.hpp>

static LGFX lcd;

// lgfx::BDFfontクラスを使って、Arduino-misakiUTF16を使用できるように設定します。
static constexpr lgfx::BDFfont misaki_font = 
  { fdata             // 第１引数 フォントのビットマップデータ
  , ftable            // 第２引数 unicodeフォントテーブル
  , sizeof(ftable)>>1 // 第３引数 フォントテーブルのサイズ
  , 8                 // 第４引数 フォント幅
  , 4                 // 第５引数 フォント幅 (半角)
  , 7                 // 第６引数 フォント高さ
  , 6                 // 第７引数 ベースライン高さ
  , 8                 // 第８引数 改行時の縦方向カーソル移動量
  };


void setup()
{
  lcd.init();

  // 先ほど作成した misaki_font を setFont 関数の引数に指定することで、print や drawString 等で使用できます。
  lcd.setFont(&misaki_font);

  lcd.setTextWrap(true, true);
}

void loop()
{
  lcd.setTextColor(0x808080U|random(0xFFFFFF), 0x7F7F7FU&random(0x10000));

  lcd.print("美さきフォントは8x8のコンパクトなフォントです。");
  lcd.print("Arduino-misakiUTF16は、教育漢字1,006字(小学校で習う漢字）＋ひらがな・カタカナ・記号・半角等の1,710字にしぼって収録されています。");
  lcd.print("Hello");
  lcd.print("ＨＥＬＬＯ");
  lcd.print("こんにちは");
  delay(1000);
}
