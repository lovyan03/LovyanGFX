
#define LGFX_AUTODETECT

#include <LovyanGFX.hpp>

static constexpr int image_width = 32;
static constexpr int image_height = 30;
//----------------------------------------------------------------------------
#define R 0x00,0x00,0xFF,
#define G 0x00,0xFF,0x00,
#define B 0xFF,0x00,0x00,
#define C 0xFF,0xFF,0x00,
#define M 0xFF,0x00,0xFF,
#define Y 0x00,0xFF,0xFF,
#define W 0xFF,0xFF,0xFF,
#define _ 0x00,0x00,0x00,
static uint8_t rgb888[] = {
#include "image.h"
};
#undef R
#undef G
#undef B
#undef C
#undef M
#undef Y
#undef W
#undef _
//----------------------------------------------------------------------------
#define R 0xFF,0x00,0x00,
#define G 0x00,0xFF,0x00,
#define B 0x00,0x00,0xFF,
#define C 0x00,0xFF,0xFF,
#define M 0xFF,0x00,0xFF,
#define Y 0xFF,0xFF,0x00,
#define W 0xFF,0xFF,0xFF,
#define _ 0x00,0x00,0x00,
uint8_t bgr888[] = {
#include "image.h"
};
#undef R
#undef G
#undef B
#undef C
#undef M
#undef Y
#undef W
#undef _
//----------------------------------------------------------------------------
#define R 0x00F8,
#define G 0xE007,
#define B 0x1F00,
#define C 0xFF07,
#define M 0x1FF8,
#define Y 0xE0FF,
#define W 0xFFFF,
#define _ 0x0000,
uint16_t swap565[] = {
#include "image.h"
};
#undef R
#undef G
#undef B
#undef C
#undef M
#undef Y
#undef W
#undef _
//----------------------------------------------------------------------------
#define R 0xF800,
#define G 0x07E0,
#define B 0x001F,
#define C 0x07FF,
#define M 0xF81F,
#define Y 0xFFE0,
#define W 0xFFFF,
#define _ 0x0000,
uint16_t rgb565[] = {
#include "image.h"
};
#undef R
#undef G
#undef B
#undef C
#undef M
#undef Y
#undef W
#undef _
//----------------------------------------------------------------------------
#define R 0xE0,
#define G 0x1C,
#define B 0x03,
#define C 0x1F,
#define M 0xE3,
#define Y 0xFC,
#define W 0xFF,
#define _ 0x00,
uint8_t rgb332[] = {
#include "image.h"
};
#undef R
#undef G
#undef B
#undef C
#undef M
#undef Y
#undef W
#undef _
//----------------------------------------------------------------------------

static LGFX lcd;

void setup(void)
{
  lcd.init();
}

void loop(void)
{
  static int count = 0;
  ++count;
  lcd.setColorDepth(count & 1 ? 24 : 16);

  lcd.startWrite();

// pushImage関数は X, Y, Width, Height, data の5つの引数を指定して使います。
// dataの型に応じて色変換が行われます。実データと型が一致していない場合はキャストしてください。
// ※ void* が指定された場合は24bitカラー(3Byte)のデータと見なして扱います。

// データがリトルエンディアンの場合は事前にsetSwapBytes(true)を実行しておきます。
  lcd.setSwapBytes(true);
  lcd.pushImage(  0,  0, image_width, image_height, (void*)bgr888);  // bad
  lcd.pushImage( 32,  0, image_width, image_height,        swap565); // bad
  lcd.pushImage( 64,  0, image_width, image_height, (void*)rgb888);  // good
  lcd.pushImage( 96,  0, image_width, image_height,        rgb565);  // good
  lcd.pushImage(128,  0, image_width, image_height,        rgb332);  // good

// データがビッグエンディアンの場合は事前にsetSwapBytes(false)を実行しておきます。
  lcd.setSwapBytes(false);
  lcd.pushImage(  0, 40, image_width, image_height, (void*)bgr888);  // good
  lcd.pushImage( 32, 40, image_width, image_height,        swap565); // good
  lcd.pushImage( 64, 40, image_width, image_height, (void*)rgb888);  // bad
  lcd.pushImage( 96, 40, image_width, image_height,        rgb565);  // bad
  lcd.pushImage(128, 40, image_width, image_height,        rgb332);  // good

// LCDへ送信する際は基本的にビッグエンディアンで扱われます。
// 例えばLCDが16bitカラーの場合、swap565のデータは無変換で送信できるため高速な処理が期待できます。

// lgfx名前空間に定義されている各種画像データ型を利用する事もできます。
// これらの型にキャストする場合はsetSwapBytesの設定は無視されます。
  lcd.pushImage(  0, 80, image_width, image_height, (lgfx:: bgr888_t*) bgr888); // good
  lcd.pushImage( 32, 80, image_width, image_height, (lgfx::swap565_t*)swap565); // good
  lcd.pushImage( 64, 80, image_width, image_height, (lgfx:: rgb888_t*) rgb888); // good
  lcd.pushImage( 96, 80, image_width, image_height, (lgfx:: rgb565_t*) rgb565); // good
  lcd.pushImage(128, 80, image_width, image_height, (lgfx:: rgb332_t*) rgb332); // good

// pushImageDMAを使用し、かつ引数のデータがLCDへ無変換で送信できる場合は、
// 引数のポインタをそのままDMAコントローラに渡してDMA転送を行います。
// ※ DMAに対応していないメモリ空間のポインタを渡さないように注意してください。
  lcd.pushImageDMA(  0, 120, image_width, image_height, (lgfx:: bgr888_t*) bgr888); // use DMA (if colorDepth is 24)
  lcd.pushImageDMA( 32, 120, image_width, image_height, (lgfx::swap565_t*)swap565); // use DMA (if colorDepth is 16)
  lcd.pushImageDMA( 64, 120, image_width, image_height, (lgfx:: rgb888_t*) rgb888); // no DMA
  lcd.pushImageDMA( 96, 120, image_width, image_height, (lgfx:: rgb565_t*) rgb565); // no DMA
  lcd.pushImageDMA(128, 120, image_width, image_height, (lgfx:: rgb332_t*) rgb332); // no DMA

// DMA転送を使用した場合は、データ送信中にも処理が進みます。
// そのため、送信中のデータを意図せず書き換えてしまう可能性が生じます。
// 送信中のデータに変更を加えたい場合は、waitDMA関数を使用して送信完了を待機します。
  lcd.waitDMA();

  lcd.endWrite();

  delay(100);
}

