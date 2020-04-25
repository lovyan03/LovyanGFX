// LovyanGFX SPIバスおよび使用パネルの設定を伴う使い方

// ヘッダをincludeします。
#include <LovyanGFX.hpp>


// SPI設定用の構造体を用意します。
// 構造体の名称に決まりはありませんが、
// 構造体の各メンバ変数の名前と型は例の通りにしてください。
struct LGFX_Config {

  // 使用するSPIを VSPI_HOST または HSPI_HOST で設定します。
  static constexpr spi_host_device_t spi_host = VSPI_HOST;

  // 使用するDMAチャンネルを 1か2で設定します。
  // 使用しない場合は省略するか0を設定します。
  static constexpr int dma_channel = 1;

  // SPIのSCLKのピン番号を設定します。
  static constexpr int spi_sclk = 18;

  // SPIのMOSIのピン番号を設定します。
  static constexpr int spi_mosi = 23;

  // SPIのMISOのピン番号を設定します。
  // SDカード等と共通のSPIバスを使う場合はMISOも必ず設定してください。
  // 使わない場合は省略するか-1を設定します。
  static constexpr int spi_miso = 19;
};

// 用意した設定用の構造体を、LGFX_SPIクラスにテンプレート引数として設定し、インスタンスを作成します。
static lgfx::LGFX_SPI<LGFX_Config> lcd;


// Panelクラスのインスタンスを作成します。使用するパネルにあったクラスを選択してください。
//static lgfx::Panel_DDUINO32_XS panel;
//static lgfx::Panel_LoLinD32 panel;
//static lgfx::Panel_M5Stack panel;
//static lgfx::Panel_M5StickC panel;
//static lgfx::Panel_ODROID_GO panel;
//static lgfx::Panel_TTGO_TS panel;
//static lgfx::Panel_TTGO_TWatch panel;

//static lgfx::Panel_HX8357B panel;
//static lgfx::Panel_HX8357D panel;
//static lgfx::Panel_ILI9163 panel;
//static lgfx::Panel_ILI9341 panel;
static lgfx::Panel_ILI9342 panel;
//static lgfx::Panel_ILI9486 panel;
//static lgfx::Panel_SSD1351 panel;
//static lgfx::Panel_ST7789 panel;
//static lgfx::Panel_ST7735S panel;



void setup(void)
{

// パネルクラスに各種設定値を代入していきます。
// （LCD一体型製品のパネルクラスを選択した場合は、
//   製品に合った初期値が設定されているので設定は不要です）

  // 通常動作時のSPIクロックを設定します。
  // ESP32のSPIは80MHzを整数で割った値のみ使用可能です。
  // 設定した値に一番近い設定可能な値が使用されます。
  panel.freq_write = 27000000;

  // 単色の塗り潰し処理時のSPIクロックを設定します。
  // 基本的にはfreq_writeと同じ値を設定しますが、
  // より高い値を設定しても動作する場合があります。
  panel.freq_fill  = 40000000;

  // LCDから画素データを読取る際のSPIクロックを設定します。
  panel.freq_read  = 16000000;

  // SPI通信モードを0~3から設定します。
  panel.spi_mode = 0;

  // データ読み取り時のSPI通信モードを0~3から設定します。
  panel.spi_mode_read = 0;

  // 画素読出し時のダミービット数を設定します。
  // 画素読出しでビットずれが起きる場合に調整してください。
  panel.len_dummy_read_pixel = 8;

  // データの読取りが可能なパネルの場合はtrueを設定します。
  // 省略時はtrueになります。
  panel.spi_read = true;

  // データの読取りMOSIピンで行うパネルの場合はtrueを設定します。
  // 省略時はfalseになります。
  panel.spi_3wire = false;

  // LCDのCSを接続したピン番号を設定します。
  // 使わない場合は省略するか-1を設定します。
  panel.spi_cs = 14;

  // LCDのDCを接続したピン番号を設定します。
  panel.spi_dc = 27;

  // LCDのRSTを接続したピン番号を設定します。
  // 使わない場合は省略するか-1を設定します。
  panel.gpio_rst = 33;

  // LCDのバックライトを接続したピン番号を設定します。
  // 使わない場合は省略するか-1を設定します。
  panel.gpio_bl  = 32;

  // バックライト使用時、輝度制御に使用するPWMチャンネル番号を設定します。
  // PWM輝度制御を使わない場合は省略するか-1を設定します。
  panel.pwm_ch_bl = 7;

  // バックライト点灯時の出力レベルがローかハイかを設定します。
  // 省略時は true。true=HIGHで点灯 / false=LOWで点灯になります。
  panel.backlight_level = true;

  // invertDisplayの初期値を設定します。trueを設定すると反転します。
  // 省略時は false。画面の色が反転している場合は設定を変更してください。
  panel.invert = false;

  // パネルの色順がを設定します。  RGB=true / BGR=false
  // 省略時はfalse。赤と青が入れ替わっている場合は設定を変更してください。
  panel.rgb_order = false;

  // パネルのメモリが持っているピクセル数（幅と高さ）を設定します。
  // 設定が合っていない場合、setRotationを使用した際の座標がずれます。
  // （例：ST7735は 132x162 / 128x160 / 132x132 の３通りが存在します）
  panel.memory_width  = 320;
  panel.memory_height = 240;

  // パネルの実際のピクセル数（幅と高さ）を設定します。
  // 省略時はパネルクラスのデフォルト値が使用されます。
  panel.panel_width  = 320;
  panel.panel_height = 240;

  // パネルのオフセット量を設定します。
  // 省略時はパネルクラスのデフォルト値が使用されます。
  panel.offset_x = 0;
  panel.offset_y = 0;

  // setRotationの初期化直後の値を設定します。
  panel.rotation = 0;

  // setRotationを使用した時の向きを変更したい場合、offset_rotationを設定します。
  // setRotation(0)での向きを 1の時の向きにしたい場合、 1を設定します。
  panel.offset_rotation = 0;


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
  lcd.startWrite();
  lcd.setRotation(++count & 7);

  lcd.setTextColor(random(65536));
  lcd.drawNumber(lcd.getRotation(), 16, 0);

  lcd.setTextColor(0xFF0000U);
  lcd.drawString("R", 30, 16);
  lcd.setTextColor(0x00FF00U);
  lcd.drawString("G", 40, 16);
  lcd.setTextColor(0x0000FFU);
  lcd.drawString("B", 50, 16);

  lcd.drawRect(30,30,lcd.width()-60,lcd.height()-60,random(65536));
  lcd.endWrite();
}

