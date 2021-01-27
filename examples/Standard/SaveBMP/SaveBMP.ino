#if defined (ARDUINO_WIO_TERMINAL)
  #include <Seeed_FS.h>
  #include <SD/Seeed_SD.h>
#else
  #include <SD.h>
  #include <SPIFFS.h>
#endif

#include <LovyanGFX.hpp>

static LGFX lcd;

static constexpr char filename[] = "/lovyangfx_test.bmp";

#ifndef SDCARD_SS_PIN
#define SDCARD_SS_PIN 4
#endif

#ifndef SDCARD_SPI
#define SDCARD_SPI SPI
#endif


bool saveToSD_16bit(void)
{
  bool result = false;
  File file = SD.open(filename, "w");
  if (file)
  {
    int width  = lcd.width();
    int height = lcd.height();

    int rowSize = (2 * width + 3) & ~ 3;

    lgfx::bitmap_header_t bmpheader;
    bmpheader.bfType = 0x4D42;
    bmpheader.bfSize = rowSize * height + sizeof(bmpheader);
    bmpheader.bfOffBits = sizeof(bmpheader);

    bmpheader.biSize = 40;
    bmpheader.biWidth = width;
    bmpheader.biHeight = height;
    bmpheader.biPlanes = 1;
    bmpheader.biBitCount = 16;
    bmpheader.biCompression = 3;

    file.write((std::uint8_t*)&bmpheader, sizeof(bmpheader));
    std::uint8_t buffer[rowSize];
    memset(&buffer[rowSize - 4], 0, 4);
    for (int y = lcd.height() - 1; y >= 0; y--)
    {
      lcd.readRect(0, y, lcd.width(), 1, (lgfx::rgb565_t*)buffer);
      file.write(buffer, rowSize);
    }
    file.close();
    result = true;
  }
  else
  {
    Serial.print("error:file open failure\n");
  }

  return result;
}

bool saveToSD_24bit(void)
{
  bool result = false;
  File file = SD.open(filename, "w");
  if (file)
  {
    int width  = lcd.width();
    int height = lcd.height();

    int rowSize = (3 * width + 3) & ~ 3;

    lgfx::bitmap_header_t bmpheader;
    bmpheader.bfType = 0x4D42;
    bmpheader.bfSize = rowSize * height + sizeof(bmpheader);
    bmpheader.bfOffBits = sizeof(bmpheader);

    bmpheader.biSize = 40;
    bmpheader.biWidth = width;
    bmpheader.biHeight = height;
    bmpheader.biPlanes = 1;
    bmpheader.biBitCount = 24;
    bmpheader.biCompression = 0;

    file.write((std::uint8_t*)&bmpheader, sizeof(bmpheader));
    std::uint8_t buffer[rowSize];
    memset(&buffer[rowSize - 4], 0, 4);
    for (int y = lcd.height() - 1; y >= 0; y--)
    {
      lcd.readRect(0, y, lcd.width(), 1, (lgfx::rgb888_t*)buffer);
      file.write(buffer, rowSize);
    }
    file.close();
    result = true;
  }
  else
  {
    Serial.print("error:file open failure\n");
  }

  return result;
}

void setup(void)
{
  lcd.init();

  Serial.begin(115200);

  lcd.setColorDepth(16);

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
  lcd.print("BMP save test\n");
  lcd.endWrite();

  do {
    SD.end();
    delay(1000);

    SD.begin(SDCARD_SS_PIN, SDCARD_SPI, 25000000);

  } while (!saveToSD_16bit());


  lcd.print("BMP save success.");
}

void loop(void)
{
  lcd.drawBmpFile(SD, filename, random(-20,20), random(-20, 20));
}
