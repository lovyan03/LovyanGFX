// LovyanGFX SPIバスおよび使用パネルの設定を伴う使い方

// ヘッダをincludeします。
#include <LovyanGFX.hpp>


// SPI設定用の構造体を用意します。
// 構造体の名称に決まりはありませんが、
// 構造体の各メンバ変数の名前と型は例の通りにしてください。
struct LGFX_Config {

  // 使用するSPIを VSPI_HOST または HSPI_HOST で指定します。
  static constexpr spi_host_device_t spi_host = VSPI_HOST;

  // 使用するDMAチャンネルを 1か2で指定します。
  // 使用しない場合は省略するか0を指定します。
  static constexpr int dma_channel = 1;

  // SPIのSCLKのピン番号を指定します。
  static constexpr int spi_sclk = 19;

  // SPIのMOSIのピン番号を指定します。
  static constexpr int spi_mosi = 23;

  // SPIのMISOのピン番号を指定します。
  // SDカード等と共通のSPIバスを使う場合はMISOも必ず設定してください。
  // MISOが必要ない場合は省略するか-1を指定します。
  static constexpr int spi_miso = 25;
};

// 用意した設定用の構造体を、LGFX_SPIクラスにテンプレート引数として指定し、インスタンスを作成します。
static lgfx::LGFX_SPI<LGFX_Config> lcd;


// Panelクラスのインスタンスを作成します。使用するパネルにあったクラスを選択してください。
//static lgfx::Panel_M5Stack panel;
//static lgfx::Panel_M5StickC panel;
//static lgfx::Panel_ODROID_GO panel;
//static lgfx::Panel_TTGO_TWatch panel;

//static lgfx::Panel_ILI9163 panel;
static lgfx::Panel_ILI9341 panel;
//static lgfx::Panel_ILI9342 panel;
//static lgfx::Panel_SSD1351 panel;
//static lgfx::Panel_ST7789 panel;
//static lgfx::Panel_ST7735S panel;



void setup(void)
{

// パネルクラスに各種設定値を代入していきます。
// （LCD一体型製品のパネルクラスを選択した場合は、
//   製品に合った初期値が設定されているので設定は不要です）

  // 通常動作時のSPI送信周波数を指定します。
  panel.freq_write = 40000000;
  // ESP32のSPIは80MHzを整数で割った値のみ使用可能です。
  // 指定した値に一番近い設定可能な値が使用されます。

  // 単色の塗り潰し処理時のSPI送信周波数を指定します。
  panel.freq_fill  = 80000000;
  // freq_writeより高い値を設定できる場合があります。

  // LCDから画素データを読取る際のSPI受信周波数を指定します。
  panel.freq_read  = 20000000;

  // データの読取りが可能なパネルの場合はtrueを指定します。
  // 省略時はtrueになります。
  panel.spi_read = true;

  // データの読取りMOSIピンで行うパネルの場合はtrueを指定します。
  // 省略時はfalseになります。
  panel.spi_3wire = false;

  // LCDのCSを接続したピン番号を指定します。
  // 使わない場合は省略するか-1を指定します。
  panel.spi_cs = 22;

  // LCDのDCを接続したピン番号を指定します。
  panel.spi_dc = 21;

  // LCDのRSTを接続したピン番号を指定します。
  // 使わない場合は省略するか-1を指定します。
  panel.gpio_rst = 18;

  // LCDのバックライトを接続したピン番号を指定します。
  // 使わない場合は省略するか-1を指定します。
  panel.gpio_bl  = 5;

  // バックライト制御に使用するPWMチャンネル番号を指定します。
  // 使わない場合は省略するか-1を指定します。
  panel.pwm_ch_bl = 7;

  // 輝度反転を使用するか否かを指定します。trueを指定すると反転します。
  // 省略時は false 
  panel.invert = false;

  // パネルのピクセル数（幅と高さ）を指定します。
  // 省略時はパネルクラスのデフォルト値が使用されます。
  panel.panel_width  = 240;
  panel.panel_height = 240;

  // パネルのオフセット量を指定します。
  // 省略時はパネルクラスのデフォルト値が使用されます。
  panel.offset_x = 0;
  panel.offset_y = 0;



  // 設定を終えたら、LGFXのsetPanel関数でパネルのポインタを渡します。
  lcd.setPanel(&panel);

  // SPIバスの初期化とパネルの初期化を実行すると使用可能になります。
  lcd.init();



  lcd.drawRect(0,0,lcd.width(),lcd.height(),0xFFFF);

  lcd.setTextSize(2);
}

uint32_t count = ~0;
void loop(void)
{
  lcd.setRotation(++count & 7);

  lcd.setTextColor(random(65536));
  lcd.drawNumber(lcd.getRotation(), 16, 0);

  lcd.setTextColor(0xFF0000U);
  lcd.drawString("R", 30, 16);
  lcd.setTextColor(0x00FF00U);
  lcd.drawString("G", 40, 16);
  lcd.setTextColor(0x0000FFU);
  lcd.drawString("B", 50, 16);

  lcd.fillRect(30,30,lcd.width()-60,lcd.height()-60,random(65536));
}

