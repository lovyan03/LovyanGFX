# LovyanGFX

SPI LCD graphics library (for ESP32). (open beta edition.)

ベータ版の注意
----------------
LovyanGFXベータ版は動作テストおよび意見公募を目的として公開しているものです。ベータ版ソフトウェアは現在も開発途中のものであり、何らかの障害を引き起こすことがあります。また、大きな仕様変更を行う場合があります。予めご了承ください。  
The LovyanGFX beta version is open to the public for the purpose of testing and gathering feedback. This software is still under development and may fail in different ways. Additionally, major specification changes may occur. Thank you for your understanding.  

概要 Overview.
----------------
ESP32とSPI接続のLCDの組み合わせで動作するグラフィックライブラリです。  
This is a graphics library that runs on ESP32 connected to a SPI LCD (see compatibility list below).

 [AdafruitGFX](https://github.com/adafruit/Adafruit-GFX-Library) や [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) と互換性をある程度持ちつつ、より高機能・高速動作を目標としています。  

This library mimics [AdafruitGFX](https://github.com/adafruit/Adafruit-GFX-Library) and [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) APIs while aiming for higher functional coverage and performances.
  
既存のライブラリに対して、以下のアドバンテージがあります。  
  - ArduinoESP32 / ESP-IDF 両対応  
  - 16bit / 24bitカラーモード両対応(実際の色数はLCDの仕様によります)  
  - DMA転送を用いた通信動作中の別処理実行  
  - オフスクリーンバッファ（スプライト）の高速な回転/拡縮描画  
  - 複数LCDの同時利用  

This library has the following advantages.
  - Both ArduinoESP32 and ESP-IDF are supported.
  - Both 16bit and 24bit color modes are supported. (actual number of colors depends on LCD specifications)
  - Execute another process during communication operation using DMA transfer.
  - Fast rotation/expansion of the off-screen buffer (sprite).
  - Simultaneous use of multiple LCDs.


対応環境 support environments
---------------
  - プラットフォーム Platform
    - ESP-IDF
    - Arduino ESP32
    - Platformio

  - ディスプレイ Displays
    - ILI9342 (M5Stack Basic,Gray,Fire,Go)
    - ILI9341 (ODROID-GO, ESP-WROVER-KIT v4.1)
    - ILI9163
    - ST7735 (M5StickC)
    - ST7789 (TTGO T-Watch)
    - SSD1351


対応機種については[src/lgfx/panel](src/lgfx/panel)をご参照ください。  
接続するピンの初期設定は[src/LovyanGFX.hpp](src/LovyanGFX.hpp)にあります。  
上記対応機種とコマンド体系の類似した液晶パネルであれば対応可能ですが、当方で入手し動作確認が取れたもののみ正式対応としています。  
対応要望を頂けた機種には優先的に対応を検討致します。  
  
This library is also compatible with the above models and LCD panels with a similar command system,
 but only those that have been obtained and confirmed to work are officially supported.  

使い方 How to use
----------------
[examples](examples/)に具体的なサンプルがあります。
### 基本的な使い方
```c
// ヘッダをincludeします。
#include <LovyanGFX.hpp>

// 対応機種をArduino環境で使う場合は、特別な設定は不要です。
static LGFX lcd;                // LGFXのインスタンスを作成。
static LGFX_Sprite sprite(&lcd); // スプライトを使う場合はLGFX_Spriteのインスタンスを作成。

// ESP-IDF環境で使う場合や、Arduino環境でもSPIバスやパネルを設定したい場合は、
// examples/HowToUse/2_spi_setting.ino を参照してください。


// もし現在 TFT_eSPI を使用中で、ソースをなるべく変更したくない場合は、こちらのヘッダを利用できます。
// #include <LGFX_TFT_eSPI.hpp>
// static TFT_eSPI lcd;               // TFT_eSPIがLGFXの別名として定義されます。
// static TFT_eSprite sprite(&lcd);   // TFT_eSpriteがLGFX_Spriteの別名として定義されます。


void setup(void)
{
// 最初に初期化関数を呼び出します。
  lcd.init();


// 回転方向を 0～3 の4方向から設定します。
  lcd.setRotation(1);


// バックライトの輝度を 0～255 の範囲で設定します。
  lcd.setBrightness(255); // の範囲で設定
// M5Stick-Cのバックライト調整は現在非対応です。
// AXP192ライブラリを別途includeして設定してください。


// 必要に応じてカラーモードを設定します。（初期値は16）
// 16の方がSPI通信量が少なく高速に動作しますが、赤と青の諧調が5bitになります。
// 24の方がSPI通信量が多くなりますが、諧調表現が綺麗になります。
//lcd.setColorDepth(16);  // RGB565の16ビットに設定
  lcd.setColorDepth(24);  // RGB888の24ビットに設定(表示される色数はパネル性能によりRGB666の18ビットになります)


// clearまたはfillScreenで画面全体を塗り潰します。
// どちらも同じ動作をしますが、clearは引数を省略でき、その場合は黒で塗り潰します。
  lcd.fillScreen(0);  // 黒で塗り潰し
  lcd.clear(0xFFFF);  // 白で塗り潰し
  lcd.clear();        // 黒で塗り潰し


// 基本的な図形の描画関数は以下の通りです。
/*
  drawPixel     ( x, y         , color);  // 点
  drawFastVLine ( x, y   , h   , color);  // 垂直線
  drawFastHLine ( x, y, w      , color);  // 水平線
  drawRect      ( x, y, w, h   , color);  // 矩形の外周
  fillRect      ( x, y, w, h   , color);  // 矩形の塗り
  drawRoundRect ( x, y, w, h, r, color);  // 角丸の矩形の外周
  fillRoundRect ( x, y, w, h, r, color);  // 角丸の矩形の塗り
  drawCircle    ( x, y      , r, color);  // 円の外周
  fillCircle    ( x, y      , r, color);  // 円の塗り
  drawEllipse   ( x, y, rx, ry , color);  // 楕円の外周
  fillEllipse   ( x, y, rx, ry , color);  // 楕円の塗り
  drawLine      ( x0, y0, x1, y1        , color); // ２点間の直線
  drawTriangle  ( x0, y0, x1, y1, x2, y2, color); // ３点間の三角形の外周
  fillTriangle  ( x0, y0, x1, y1, x2, y2, color); // ３点間の三角形の塗り
*/


// 例えばdrawPixelで点を書く場合は、引数は X座標,Y座標,色 の３つ。
  lcd.drawPixel(0, 0, 0xFFFF); // 座標0:0に白の点を描画


// カラーコードを生成する関数が用意されており、色の指定に使用できます。
// 引数は、赤,緑,青をそれぞれ 0～255で指定します。
// 色情報の欠落を防ぐため、color888を使う事を推奨します。
  lcd.drawFastVLine(2, 0, 100, lcd.color888(255,   0,   0)); // 赤で垂直の線を描画
  lcd.drawFastVLine(4, 0, 100, lcd.color565(  0, 255,   0)); // 緑で垂直の線を描画
  lcd.drawFastVLine(6, 0, 100, lcd.color332(  0,   0, 255)); // 青で垂直の線を描画


// カラーコード生成関数を使用しない場合は以下のようになります。
// RGB888 24ビットで指定 uint32_t型
// RGB565 16ビットで指定 uint16_t型、int32_t型
// RGB332  8ビットで指定 uint8_t型

// uint32_t型を使用すると、RGB888の24ビットとして扱われます。
// 16進数2桁で赤緑青の順に記述できます。
// uint32_t型の変数を使うか、末尾にUを付けるか、uint32_t型にキャストして使用します。
  uint32_t red = 0xFF0000;
  lcd.drawFastHLine(0, 2, 100, red);            // 赤で水平の線を描画
  lcd.drawFastHLine(0, 4, 100, 0x00FF00U);      // 緑で水平の線を描画
  lcd.drawFastHLine(0, 6, 100, (uint32_t)0xFF); // 青で水平の線を描画


// uint16_t型およびint32_t型を使用すると、RGB565の16ビットとして扱われます。
// 特別な書き方をしない場合はint32_t型として扱われるので、この方式になります。
// （AdafruitGFX や TFT_eSPI との互換性のために、このようにしています。）
  uint16_t green = 0x07E0;
  lcd.drawRect(10, 10, 50, 50, 0xF800);         // 赤で矩形の外周を描画
  lcd.drawRect(12, 12, 50, 50, green);          // 緑で矩形の外周を描画
  lcd.drawRect(14, 14, 50, 50, (uint16_t)0x1F); // 青で矩形の外周を描画


// int8_t型、uint8_t型を使用すると、RGB332の8ビットとして扱われます。
  uint8_t blue = 0x03;
  lcd.fillRect(20, 20, 20, 20, (uint8_t)0xE0);  // 赤で矩形の塗りを描画
  lcd.fillRect(30, 30, 20, 20, (uint8_t)0x1C);  // 緑で矩形の塗りを描画
  lcd.fillRect(40, 40, 20, 20, blue);           // 青で矩形の塗りを描画


// 描画関数の引数の色は省略できます。
// 省略した場合、setColor関数で設定した色 または最後に使用した色で描画できます。
// 同じ色で繰り返し描画する場合は、省略した方がわずかに速く動作します。
  lcd.setColor(0xFF0000U);           // 赤色を指定
  lcd.fillEllipse( 80, 40, 10, 20);  // 赤色で楕円の塗り
  lcd.fillCircle ( 40, 80, 20    );  // 赤色で円の塗り
  lcd.setColor(0x0000FFU);           // 青色を指定
  lcd.drawEllipse( 80, 40, 10, 20);  // 青色で楕円の外周
  lcd.drawCircle ( 40, 80, 20    );  // 青色で円の外周


// SPIバスの確保と解放は描画関数を呼び出した時に自動的に行われます。
// 描画スピードを重視する場合は、描画処理の前後に startWriteとendWriteを使用します。
// SPIバスの確保と解放が抑制され、速度が向上します。
  lcd.drawPixel(0, 0);  // SPIバス確保、描画、SPIバス解放
  lcd.drawPixel(0, 0);  // SPIバス確保、描画、SPIバス解放
  lcd.startWrite();     // SPIバス確保
  lcd.drawPixel(0, 0);  // 描画
  lcd.drawPixel(0, 0);  // 描画
  lcd.drawPixel(0, 0);  // 描画
  lcd.endWrite();       // SPIバス解放


// startWriteとendWriteは呼出し回数を内部でカウントしており、
// 繰り返し呼び出した場合は最初と最後のみ動作します。
// startWriteとendWriteは必ず対になるように使用してください。
// (SPIバスを占有して構わない場合は、最初にstartWriteを一度呼び、endWriteしない使い方も可能です。)
  lcd.startWrite();     // カウント+1、SPIバス確保
  lcd.startWrite();     // カウント+1
  lcd.startWrite();     // カウント+1
  lcd.endWrite();       // カウント-1
  lcd.endWrite();       // カウント-1
  lcd.endWrite();       // カウント-1、SPIバス解放
  lcd.endWrite();       // 何もしない
// なお過剰にendWriteを呼び出した場合は何も行わず、カウントがマイナスになることもありません。


// startWriteのカウントの状態に依らず、強制的にSPIバスを解放・確保したい場合は、
// endTransaction・beginTransactionを使用します。
// カウントはクリアされないので、辻褄が合わなくならないよう注意してください。
  lcd.startWrite();       // カウント+1、SPIバス確保
  lcd.startWrite();       // カウント+1
  lcd.drawPixel(0, 0);    // 描画
  lcd.endTransaction();   // SPIバス解放
  // ここで他のSPIデバイスの使用が可能
  // 同じSPIバスの別のデバイス(SDカード等)を使う場合、
  // 必ずSPIバスが解放された状態で行ってください。
  lcd.beginTransaction(); // SPIバスの確保
  lcd.drawPixel(0, 0);    // 描画
  lcd.endWrite();         // カウント-1
  lcd.endWrite();         // カウント-1、SPIバス解放


  delay(1000);


// drawPixelとは別に、writePixelという点を描画する関数があります。
// drawPixelは必要に応じてSPIバスの確保を行うのに対し、
// writePixelはSPIバスの状態をチェックしません。
  lcd.startWrite();  // SPIバス確保
  for (uint32_t x = 0; x < 128; ++x) {
    for (uint32_t y = 0; y < 128; ++y) {
      lcd.writePixel(x, y, lcd.color888(255 - x, x + y, 255 - y));
    }
  }
  lcd.endWrite();    // SPIバス解放
// 名前が write～ で始まる関数は全て明示的にstartWriteを呼び出しておく必要があります。
// writePixel、writeFastVLine、writeFastHLine、writeFillRect が該当します。


  delay(1000);
}

void loop(void)
{
  lcd.fillRect(0, 0, random(lcd.width()), random(lcd.height()), lcd.color888(random(256),random(256),random(256)));
}
```


### スプライトの使い方
（作成中 under construction. )
カラーパレットを作成した場合はカラーコードではなくパレットのColorIndexを指定します。（例、4bitの場合は0～15を指定。）  


# 注意・制限事項
## M5Stack.h(M5StickC.h)との共存は不可  
TFT_eSPIと定義が重複するため利用できません。  


作成動機 Motivation behind this library
----------------
TFT_eSPIは素晴らしいライブラリです。しかし、複数のアーキテクチャを対象とするため構造的に複雑となっており、ESP-IDFへの対応や18bitカラーへの対応など、求める機能の追加を行う事が非常に困難となっていました。  
LovyanGFX はこれらの機能の追加とパフォーマンスの最適化を実現するために作成しました。  

TFT_eSPI is a great library. However, it is structurally complex because it targets multiple architectures, making it very difficult to add required functions such as ESP-IDF support and 18-bit color support.  
LovyanGFX has been created to add these features and optimize performance.  


謝辞 Acknowledgements
----------------
このライブラリを作成するにあたり、インスピレーションを頂いた[TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)ライブラリの作者[Bodmer](https://github.com/Bodmer/)氏へ感謝いたします。  
TFT_eSPIのベースとなった、[AdafruitGFX](https://github.com/adafruit/Adafruit-GFX-Library)を公開している[Adafruit Industries](https://github.com/adafruit/)へ感謝いたします。  
[TJpgDec](http://elm-chan.org/fsw/tjpgd/00index.html) (Tiny JPEG Decompressor) の作者 [ChaN](http://elm-chan.org/)氏へ感謝いたします。  
[Pngle](https://github.com/kikuchan/pngle) (PNG Loader for Embedding) の作者 [kikuchan](https://github.com/kikuchan/)氏へ感謝いたします。  

Thanks to [Bodmer](https://github.com/Bodmer/), author of the [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) library, for the inspiration to create this library.  
Thanks to [Adafruit Industries](https://github.com/adafruit/) for publishing [AdafruitGFX](https://github.com/adafruit/Adafruit-GFX-Library), which is the basis for TFT_eSPI.  
Thanks to [ChaN](http://elm-chan.org/), author of [TJpgDec](http://elm-chan.org/fsw/tjpgd/00index.html) (Tiny JPEG Decompressor).  
Thanks to [kikuchan](https://github.com/kikuchan/), author of [Pngle](https://github.com/kikuchan/pngle) (PNG Loader for Embedding).  


使用ライブラリ included library
----------------
[TJpgDec](http://elm-chan.org/fsw/tjpgd/00index.html)  [ChaN](http://elm-chan.org/)  
[Pngle](https://github.com/kikuchan/pngle)  [kikuchan](https://github.com/kikuchan/)  


クレジット Credits
----------------
  - Inspiration: [Bodmer](https://github.com/Bodmer)
  - Author: [lovyan03](https://github.com/lovyan03)
  - Contributors:
    - [ciniml](https://github.com/ciniml)
    - [mongonta0716](https://github.com/mongonta0716)
    - [tobozo](https://github.com/tobozo)


ライセンス License
----------------
main : [MIT](https://github.com/lovyan03/LovyanGFX/blob/master/LICENSE)  

GFX font and GLCD font : [BSD](https://github.com/adafruit/Adafruit-GFX-Library/blob/master/license.txt) Adafruit Industries  




