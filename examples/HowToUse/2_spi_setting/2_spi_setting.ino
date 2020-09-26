// How to set up the SPI with LovyanGFX
// LovyanGFX SPIバスおよび使用パネルの設定を伴う使い方


#include <LovyanGFX.hpp>

// Create a structure for SPI configuration
// SPI設定用の構造体を作成します。

// for ESP32 SPI
struct LGFX_Config
{
// You can change the name of the structure from "LGFX_Config"
// But do not change the name and type of the members.
// 構造体の名称 "LGFX_Config" は変更しても構いませんが、
// 構造体の各メンバ変数の名前と型は例の通りにしてください。

  // Select the type of SPI  (VSPI_HOST or HSPI_HOST)
  // 使用するSPIを VSPI_HOST または HSPI_HOST で設定します。
  static constexpr spi_host_device_t spi_host = VSPI_HOST;

  // Set the DMA channel number (1 or 2)
  // If you don't want to use DMA, omit it or set it to 0.
  // 使用するDMAチャンネルを 1か2で設定します。
  // 使用しない場合は省略するか0を設定します。
  static constexpr int dma_channel = 1;

  // Set the SPI SCLK pin number
  // SPIのSCLKのピン番号を設定します。
  static constexpr int spi_sclk = 18;

  // Set the SPI MOSI pin number
  // SPIのMOSIのピン番号を設定します。
  static constexpr int spi_mosi = 23;

  // Set the SPI MISO pin number
  // If you share the SPI bus with an SD card, be sure to set up MISO as well.
  // If you don't want to use MISO, omit it or set it to -1.
  // SPIのMISOのピン番号を設定します。
  // SDカード等と共通のSPIバスを使う場合はMISOも必ず設定してください。
  // 使わない場合は省略するか-1を設定します。
  static constexpr int spi_miso = 19;

  // Set the data length of SPI communication.
  // If you want to use the LCD for RPi, set it to 16.
  // SPI通信のデータ長を指定します。
  // RaspberryPi用のLCD等を使用する場合に16を指定します。
  // 省略時は 8 です。大抵のパネルは8ですので、基本的には省略してください。
  static constexpr int spi_dlen = 8;
};

/* for ESP32 Parallel
struct LGFX_Config
{
  // Select the type of I2S  (I2S_NUM_0 or I2S_NUM_1)
  // 使用するI2Sを I2S_NUM_0 または I2S_NUM_1 で設定します。
  // 省略時は I2S_NUM_0です。
  static constexpr i2s_port_t i2s_port = I2S_NUM_0;

  // Set the parallel pin number
  // パラレル接続の各種ピン番号を設定します。
  static constexpr int gpio_wr  =  4;
  static constexpr int gpio_rd  =  2;
  static constexpr int gpio_rs  = 15;
  static constexpr int gpio_d0  = 12;
  static constexpr int gpio_d1  = 13;
  static constexpr int gpio_d2  = 26;
  static constexpr int gpio_d3  = 25;
  static constexpr int gpio_d4  = 17;
  static constexpr int gpio_d5  = 16;
  static constexpr int gpio_d6  = 27;
  static constexpr int gpio_d7  = 14;
};

//*/
/* 
// for SAMD51 SPI
struct LGFX_Config
{
// 使用するSPIのSERCOM番号を設定します。
  static constexpr int sercom_index = 5;

// SERCOMのクロックソースを設定します。
// -1を指定した場合、クロックソースを設定せずに動作しますので別途設定を行ってください。
  static constexpr int sercom_clksrc = 0;   // -1=notchange / 0=select GCLK0

// 上記で設定したクロックソースの動作周波数を設定します。
// Harmony等で行った設定値をそのまま設定してください。
//static constexpr int sercom_clkfreq = 120000000;
  static constexpr int sercom_clkfreq = F_CPU; // Seeeduino環境ではF_CPU定数でCPUクロック値が利用できます。

// SAMD51でのピン番号の設定はArduino向けの番号ではなく、SAMD51のポート+ピン番号で設定します。

// SPIのSCLKのピン番号を設定します。 PORTA=0x000 / PORTB=0x100 / PORTC=0x200 / PORTD=0x300…
  static constexpr int spi_sclk = 0x0100 | 3; // PORTB 3 (PORTB=0x0100)

// SPIのMOSIのピン番号を設定します。
  static constexpr int spi_mosi = 0x0100 | 2; // PORTB 2 (PORTB=0x0100)

// SPIのMISOのピン番号を設定します。
  static constexpr int spi_miso = 0x0100 | 0; // PORTB 0 (PORTB=0x0100)

// SPIで使用するTX Padを設定します。
  static constexpr SercomSpiTXPad pad_mosi = SPI_PAD_0_SCK_1;  // PAD_SPI0_TX;

// SPIで使用するRX Padを設定します。
  static constexpr SercomRXPad    pad_miso = SERCOM_RX_PAD_2;  // PAD_SPI0_RX;

  // SPI通信のデータ長を指定します。
  // RaspberryPi用のLCD等を使用する場合に16を指定します。
  // 省略時は 8 です。大抵のパネルは8ですので、基本的には省略してください。
  static constexpr int spi_dlen = 8;
};
//*/

// Create an LGFX_SPI instance with the configuration structure you just created as a template argument.
// 用意した設定用の構造体を、LGFX_SPIクラスにテンプレート引数として設定し、インスタンスを作成します。
// パラレル接続の場合は LGFX_PARALLELクラスを使用します。
static lgfx::LGFX_SPI<LGFX_Config> lcd;
//static lgfx::LGFX_PARALLEL<LGFX_Config> lcd;


// Create an instance of the Panel class. Comment out the description of the panel you want to use.
// Panelクラスのインスタンスを作成します。使用するパネルにあった記述をコメントアウトしてください。
//static lgfx::Panel_HX8357B panel;
//static lgfx::Panel_HX8357D panel;
//static lgfx::Panel_ILI9163 panel;
//static lgfx::Panel_ILI9341 panel;
static lgfx::Panel_ILI9342 panel;
//static lgfx::Panel_ILI9486 panel;
//static lgfx::Panel_ILI9486L panel;
//static lgfx::Panel_ILI9488 panel;
//static lgfx::Panel_SSD1351 panel;
//static lgfx::Panel_ST7735  panel;
//static lgfx::Panel_ST7735S panel;
//static lgfx::Panel_ST7789 panel;
//static lgfx::Panel_ST7796 panel;

// If you want to use a touch panel, create an instance of the Touch class.
// タッチパネルを使用する場合、Touchクラスのインスタンスを作成します。
//static lgfx::Touch_SPI_STMPE610 touch;
//static lgfx::Touch_FT5x06 touch;
//static lgfx::Touch_XPT2046 touch;


void setup(void)
{
// Assign various setting values to the panel class.
// パネルクラスに各種設定値を代入していきます。

  // Set the SPI clock for normal write operation.
  // 通常動作時のSPIクロックを設定します。
  // ESP32のSPIは80MHzを整数で割った値のみ使用可能です。
  // 設定した値に一番近い設定可能な値が使用されます。
  panel.freq_write = 20000000;

  // Set the SPI clock for fill write operation.
  // It may work even if you set the clock higher than freq_write.
  // 単色の塗り潰し処理時のSPIクロックを設定します。
  // 基本的にはfreq_writeと同じ値を設定しますが、
  // より高い値を設定しても動作する場合があります。
  panel.freq_fill  = 27000000;

  // Set the SPI clock for read operation.
  // LCDから画素データを読取る際のSPIクロックを設定します。
  panel.freq_read  = 16000000;

  // Set the SPI mode. (0~3)
  // SPI通信モードを0~3から設定します。
  panel.spi_mode = 0;

  // Set the SPI mode when read operation. (0~3)
  // データ読み取り時のSPI通信モードを0~3から設定します。
  panel.spi_mode_read = 0;

  // Sets the number of dummy bits for pixel readout.
  // 画素読出し時のダミービット数を設定します。
  // 画素読出しでビットずれが起きる場合に調整してください。
  panel.len_dummy_read_pixel = 8;

  // Set the readability of the data. If reading of the data is not possible, set false.
  // データの読取りの可否を設定します。読取り不可の場合はfalseを設定します。
  // ※ CSピンのないST7789等はfalseにしてください。
  // 省略時はtrueになります。
  panel.spi_read = true;

  // Set to "true" for a panel that uses MOSI pins to read data.
  // データの読取りMOSIピンで行うパネルの場合はtrueを設定します。
  // 省略時はfalseになります。
  panel.spi_3wire = false;

  // Set the pin number for connecting the CS pins of the LCD.
  // LCDのCSを接続したピン番号を設定します。
  // 使わない場合は省略するか-1を設定します。
  panel.spi_cs = 14;

  // Set the pin number for connecting the D/C pins of the LCD.
  // LCDのD/Cを接続したピン番号を設定します。
  panel.spi_dc = 27;

  // Set the pin number for connecting the RST pins of the LCD.
  // LCDのRSTを接続したピン番号を設定します。
  // 使わない場合は省略するか-1を設定します。
  panel.gpio_rst = 33;

  // Set the backlight pin number.
  // LCDのバックライトを接続したピン番号を設定します。
  // 使わない場合は省略するか-1を設定します。
  panel.gpio_bl  = 32;

  // Set the backlight control PWM channel number.
  // バックライト使用時、輝度制御に使用するPWMチャンネル番号を設定します。
  // PWM輝度制御を使わない場合は省略するか-1を設定します。
  panel.pwm_ch_bl = 7;

  // Set the backlight level (rue=turns on HIGH / false=turns on LOW)
  // バックライト点灯時の出力レベルがローかハイかを設定します。
  // 省略時は true。true=HIGHで点灯 / false=LOWで点灯になります。
  panel.backlight_level = true;

  // Set the panel color inversion.
  // パネルの色反転設定です。trueを設定すると色が反転します。(例:黒が白に、青が黄色に)
  // 省略時は false。画面の色が反転している場合は設定を変更してください。
  panel.reverse_invert = false;

  // Set the RGB/BGR color order.
  // パネルの色順がを設定します。  RGB=true / BGR=false
  // 省略時はfalse。赤と青が入れ替わっている場合は設定を変更してください。
  panel.rgb_order = false;

  // Set the internal memory size of the LCD driver.
  // LCDドライバチップ内のメモリサイズ（幅と高さ）を設定します。
  // 設定が合っていない場合、setRotationを使用した際の座標がずれます。
  // （例：ST7735は 132x162 / 128x160 / 132x132 の３通りが存在します）
  panel.memory_width  = 320;
  panel.memory_height = 240;

  // Set the size of the pixels that can be displayed on the LCD panel.
  // パネルが実際に表示可能なピクセル数（幅と高さ）を設定します。
  // 省略時はパネルクラスのデフォルト値が使用されます。
  panel.panel_width  = 320;
  panel.panel_height = 240;

  // Set the number of offset pixels.
  // パネルのオフセット量を設定します。
  // 省略時はパネルクラスのデフォルト値が使用されます。
  panel.offset_x = 0;
  panel.offset_y = 0;

  // Set the default rotation number.
  // setRotationの初期化直後の値を設定します。
  panel.rotation = 0;

  // Set the number of rotation offset number.
  // setRotationを使用した時の向きを変更したい場合、offset_rotationを設定します。
  // setRotation(0)での向きを 1の時の向きにしたい場合、 1を設定します。
  panel.offset_rotation = 0;

  // After setting up, you can pass the panel pointer to the lcd.setPanel function.
  // 設定を終えたら、lcdのsetPanel関数でパネルクラスのポインタを渡します。
  lcd.setPanel(&panel);


/*
  // If you use a touch panel, you will assign various setting values to the touch class.
  // タッチパネルを使用する場合、タッチクラスに各種設定値を代入していきます。

  // for SPI setting.  (XPT2046 / STMPE610)
  // SPI接続のタッチパネルの場合
  touch.spi_host = VSPI_HOST;  // VSPI_HOST or HSPI_HOST
  touch.spi_sclk = 18;
  touch.spi_mosi = 23;
  touch.spi_miso = 19;
  touch.spi_cs   =  5;
  touch.freq = 1000000;
  touch.bus_shared = true;  // If the LCD and touch share SPI, set to true.

  // for I2C setting. (FT5x06)
  // I2C接続のタッチパネルの場合
  touch.i2c_port = I2C_NUM_1;
  touch.i2c_sda  = 21;
  touch.i2c_scl  = 22;
  touch.i2c_addr = 0x38;
  touch.freq = 400000;

  // タッチパネルから得られるレンジを設定
  // (キャリブレーションを実施する場合は省略可)
  // (This can be omitted if calibration is performed.)
  touch.x_min = 0;
  touch.x_max = 319;
  touch.y_min = 0;
  touch.y_max = 319;

  // After setting up, you can pass the touch pointer to the lcd.setTouch function.
  // 設定を終えたら、lcdのsetTouch関数でタッチクラスのポインタを渡します。
  lcd.setTouch(&touch);
*/


  // Initializing the SPI bus and panel will make it available.
  // SPIバスとパネルの初期化を実行すると使用可能になります。
  lcd.init();

  if (lcd.width() > 240 || lcd.height() > 240) lcd.setTextSize(2);

  if (lcd.touch())
  {
    if (lcd.width() < lcd.height()) lcd.setRotation(3 & (lcd.getRotation() + 1));

    // Draw the information text.
    // 画面に案内文章を描画します。
    lcd.drawString("touch the arrow marker.", 0, lcd.height()>>1);

    // If the touch is enabled, perform calibration. Touch the arrow tips that appear in the four corners of the screen in order.
    // タッチを使用する場合、キャリブレーションを行います。画面の四隅に表示される矢印の先端を順にタッチしてください。
    lcd.calibrateTouch(nullptr, 0xFFFFFFU, 0x000000U, 30);

    lcd.clear();
  }
}

uint32_t count = ~0;
void loop(void)
{
  delay(10);
  lcd.startWrite();
  lcd.setRotation(++count & 7);
  lcd.setColorDepth((count & 8) ? 16 : 24);

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

  int32_t x, y;
  if (lcd.getTouch(&x, &y)) {
    lcd.fillRect(x-2, y-2, 5, 5, random(65536));
  }
}

