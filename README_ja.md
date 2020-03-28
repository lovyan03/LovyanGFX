# LovyanGFX
（画像と動画を挿入）
youtubeにあげるとリンク可能

# ベータ版の注意
  LovyanGFXベータ版は動作テストをおもな目的として公開しているものです。ベータ版ソフトウェアは現在も開発途中のものであり、何らかの障害を引き起こすことがあります。また、大きな仕様変更がある場合もありますのでその点はご了承ください。

# 概要
＜上手く説明できなかったのでらびやん足してくださいm(_ _)m＞
ESP32とSPIで接続したLCDの性能を引き出すために作成したGFXライブラリです。  
Spriteやカラーパレット機能もあり今までのTFT_eSPIライブラリよりも使いやすくなっています。

# 対応機種
- M5Stackシリーズ
    - M5Stack Basic,Gray,Fire,Go
    - M5StickC
- ODROID-GO
- TTGO T-Watch
## 対応機種について
  その他ESP32ベースの開発ボードと液晶パネルでも利用可能ですが、種類も多く動作確認が煩雑になるため一部機種のみとしています。
  
  対応パネルについては[src/lgfx/panel](src/lgfx/panel)を参照してください。接続するピンの初期設定は[src/LovyanGFX.hpp](src/LovyanGFX.hpp)にあります。

# 使い方
[example](example)に具体的なサンプルがあります。
## LGFX
LovyanGFX.hppをincludeしてください。 
### 使用例その１
```
#include <LovyanGFX.hpp>

LGFX tft;
LGFXSprite sprite(&tft);

void setup() {
  tft.init();
  tft.fillScreen(0x000000U);

}
```

## TFT_eSPIとの互換性を重視する場合  
LGFX_TFT_eSPI.hppをincludeしてください。

TFT_eSpriteも利用できます。
### 使用例その２
```
#include <LGFX_TFT_eSPI.hpp>

TFT_eSPI tft;
TFT_eSprite sprite(&tft);
```

# 

# API
　（APIドキュメントのリンクを入れる。もしくはソース参照？）

# 注意・制限事項
## M5Stack.h(M5StickC.h)との共存は不可
TFT_eSPIと定義が重複するため利用できません。

## カラーパレットを使った場合の色指定

カラーパレットを作成した場合はカラーコードではなくパレットのColorIndexを指定します。（例、4bitだったら0～15を指定。）


# Credit
  Bodmer
  Tobozo
  ...

# License
[MIT]()

# Author
[@lovyan03](https://twitter.com/lovyan03)