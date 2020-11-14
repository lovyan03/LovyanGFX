#if defined (ARDUINO_WIO_TERMINAL)
  #include <Seeed_FS.h>
  #include <SD/Seeed_SD.h>
#else
  #include <SD.h>
  #include <SPIFFS.h>
#endif

#include <LovyanGFX.hpp>

static LGFX lcd;

static constexpr char filename[] = "/lovyangfx_test.png";

#ifndef SDCARD_SS_PIN
#define SDCARD_SS_PIN 4
#endif

#ifndef SDCARD_SPI
#define SDCARD_SPI SPI
#endif


bool saveToSD(void)
{
  // createPng関数で指定範囲の画像からPNG形式のデータを生成します。
  // SAMD51の場合 172x172程度が上限です。
  // ESP32の場合 192x192程度が上限です。
  // メモリ使用状況によってさらに縮みます。
  // ESP32でPSRAMが有効な場合は大きなサイズでも保存できる可能性があります。
  std::size_t dlen;
  std::uint8_t* png = (std::uint8_t*)lcd.createPng(&dlen, 0, 0, 128, 128);
  if (!png)
  {
    Serial.print("error:createPng\n");
    return false;
  }

  Serial.print("success:createPng\n");

  bool result = false;
  File file = SD.open(filename, "w");
  if (file)
  {
    file.write((std::uint8_t*)png, dlen);
    file.close();
    result = true;
  }
  else
  {
    Serial.print("error:file open failure\n");
  }

  free(png);
  return result;
}

void setup(void)
{
  lcd.init();

  Serial.begin(115200);

  lcd.setColorDepth(24);

  lcd.setColor(TFT_WHITE);
  lcd.startWrite();
  lcd.setAddrWindow(0, 0, lcd.width(), lcd.height());
  for (int y = 0; y < lcd.height(); ++y)
  {
    for (int x = 0; x < lcd.width(); ++x)
    {
      lcd.writeColor( lcd.color888(x << 1, x + y, y << 1), 1);
    }
  }
  lcd.print("PNG save test\n");
  lcd.endWrite();

  do {
    SD.end();
    delay(1000);

    SD.begin(SDCARD_SS_PIN, SDCARD_SPI, 25000000);

  } while (!saveToSD());


  lcd.print("PNG save success.");
}

void loop(void)
{
  lcd.drawPngFile(SD, filename, random(-20,20), random(-20, 20));
}
