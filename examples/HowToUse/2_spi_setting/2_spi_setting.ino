// LovyanGFX SPIバスおよび使用パネルの設定を伴う使い方

// ヘッダをincludeします。
#include <LovyanGFX.hpp>

// パネル設定用の構造体を用意します。
// 構造体の名称に決まりはありません。
// 構造体を構成する変数名と型は例の通りにしてください。
struct Panel_Config {

  // 通常動作時のSPI送信周波数を指定します。
  static constexpr int freq_write = 40000000;
  // ESP32のSPIは80MHzを整数で割った値のみ使用可能です。
  // 指定した値に一番近い設定可能な値が使用されます。


  // 単色の塗り潰し処理時のSPI送信周波数を指定します。
  static constexpr int freq_fill  = 80000000;
  // freq_writeより高い値を設定できる場合があります。


  // LCDから画素データを読取る際のSPI受信周波数を指定します。
  static constexpr int freq_read  = 20000000;

  // データの読取りが可能なパネルの場合はtrueを指定します。
  // コメントアウト時はtrueになります。
  static constexpr bool spi_read = true;

  // データの読取りMOSIピンで行うパネルの場合はtrueを指定します。
  // コメントアウト時はfalseになります。
  static constexpr bool spi_3wire = true;

  // LCDのCSを接続したピン番号を指定します。
  // 使わない場合はコメントアウトするか-1を指定します。
  static constexpr int spi_cs = 14;

  // LCDのDCを接続したピン番号を指定します。
  static constexpr int spi_dc = 27;

  // LCDのRSTを接続したピン番号を指定します。
  // 使わない場合はコメントアウトするか-1を指定します。
  static constexpr int gpio_rst = 33;

  // LCDのバックライトを接続したピン番号を指定します。
  // 使わない場合はコメントアウトするか-1を指定します。
  static constexpr int gpio_bl  = 32;

  // バックライト制御に使用するPWMチャンネル番号を指定します。
  // 使わない場合はコメントアウトするか-1を指定します。
  static constexpr int pwm_ch_bl = 7;
};


// SPI設定用の構造体を用意します。
// 構造体の名称に決まりはありません。
// 構造体を構成する変数名と型は例の通りにしてください。
struct LGFX_Config {

  // 使用するSPIを VSPI_HOST または HSPI_HOST で指定します。
  static constexpr spi_host_device_t spi_host = VSPI_HOST;

  // 使用するDMAチャンネルを 1か2で指定します。
  // 使用しない場合はコメントアウトするか0を指定します。
  static constexpr int dma_channel = 1;

  // SPIのMOSI,MISO,SCLKの各ピン番号を指定します。
  // SDカード等と共通のSPIバスを使う場合はMISOも必ず設定してください。
  // MISOが必要ない場合はコメントアウトするか-1を指定します。
  static constexpr int spi_mosi = 23;
  static constexpr int spi_miso = 19;
  static constexpr int spi_sclk = 18;
};

// 用意した構造体を、LGFX_SPIクラスにテンプレート引数として指定してインスタンスを作成します。
static lgfx::LGFX_SPI<LGFX_Config> lcd;

// 用意した構造体を、使用パネルに対応したPanelクラスにテンプレート引数として指定してインスタンスを作成します。
static const lgfx::Panel_ILI9342<Panel_Config> panel;

void setup(void)
{
  // LGFX_SPIクラスのsetPanel関数で作成したパネルのインスタンスを渡すことで使用可能になります。
  lcd.setPanel(panel);

  lcd.init();


}


void loop(void)
{
  lcd.fillRect(0, 0, random(lcd.width()), random(lcd.height()), lcd.color888(random(256),random(128),0));
}

