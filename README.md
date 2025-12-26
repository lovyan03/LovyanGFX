# LovyanGFX

[![arduino-library-badge](https://www.ardu-badge.com/badge/LovyanGFX.svg?)](https://www.ardu-badge.com/LovyanGFX)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/lovyan03/library/LovyanGFX.svg)](https://registry.platformio.org/packages/libraries/lovyan03/LovyanGFX)

[![Arduino](https://github.com/lovyan03/LovyanGFX/actions/workflows/ArduinoBuild.yml/badge.svg?branch=master)](https://github.com/lovyan03/LovyanGFX/actions/workflows/ArduinoBuild.yml)
[![Platformio](https://github.com/lovyan03/LovyanGFX/actions/workflows/PlatformioBuild.yml/badge.svg?branch=master)](https://github.com/lovyan03/LovyanGFX/actions/workflows/PlatformioBuild.yml)
[![esp-idf](https://github.com/lovyan03/LovyanGFX/actions/workflows/IDFBuild.yml/badge.svg?branch=master)](https://github.com/lovyan03/LovyanGFX/actions/workflows/IDFBuild.yml)

Display (LCD / OLED / EPD) graphics library (for ESP32 SPI, I2C, 8bitParallel / ESP8266 SPI, I2C / ATSAMD51 SPI).
M5Stack / M5StickC / TTGO T-Watch / ODROID-GO / ESP-WROVER-KIT / WioTerminal / and more...
[![examples](http://img.youtube.com/vi/SMOHRPqUZcQ/0.jpg)](http://www.youtube.com/watch?v=SMOHRPqUZcQ "examples")
[![examples](http://img.youtube.com/vi/F5gsp41Elac/0.jpg)](http://www.youtube.com/watch?v=F5gsp41Elac "MultiPanel")

## Overview
This is a graphics library that works with a combination of ESP32 with SPI, I2C, 8-bit parallel / ESP8266 with SPI / ATSAMD51 with SPI to the Display. (see compatibility list below).

This library mimics [AdafruitGFX](https://github.com/adafruit/Adafruit-GFX-Library) and [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) APIs while aiming for higher functional coverage and performances.

This library has the following advantages.
- ArduinoESP32 and ESP-IDF are supported.
- Both 16bit and 24bit color modes are supported. (actual number of colors depends on display specifications)
- Execute another process during communication operation using DMA transfer.
- Fast rotation/expansion of the off-screen buffer (sprite).
- Simultaneous use of multiple displays.
- Automatic processing of color reduction drawing for monochrome displays.
- OpenCV,SDL2 can be used as a drawing destination and can run on a PC.
- Composite video signal (NTSC, PAL) output (only ESP32)

| | SPI | I2C | 8bit Para |16bit Para | RGB | CVBS |
|:------:|:---:|:---:|:---------:|:---------:|:---------:|:--------:|
|ESP32 | HW | HW | HW (I2S) | --- | --- |HW(I2SDAC)|
|ESP32-S2| HW | HW | HW (I2S) | HW (I2S) | --- | --- |
|ESP32-S3| HW | HW |HW(LCD/CAM)|HW(LCD/CAM)|HW(LCD/CAM)| --- |
|ESP32-C3| HW | HW | SW | --- | --- | --- |
|ESP8266 | HW | SW | --- | --- | --- | --- |
|SAMD51 | HW | HW | --- | --- | --- | --- |
|SAMD21 | HW | HW | --- | --- | --- | --- |
|RP2040 | HW | --- | --- | --- | --- | --- |

※ HW = HardWare Peripheral / SW = SoftWare implementation

| |TouchScreens|
|:------:|:----------:|
|ESP32 | supported |
|ESP32-S2| supported |
|ESP32-S3| supported |
|ESP32-C3| supported |
|ESP8266 | supported |
|SAMD51 | supported |
|SAMD21 | supported |
|RP2040 | --- |

## Supported environments
- Platform
  - ESP-IDF
  - Arduino ESP32
  - Arduino ATSAMD51 (Seeed)
  - Arduino RP2040

- Displays
  - GC9107 (M5AtomS3)
  - GC9A01
  - GDEW0154M09 (M5Stack CoreInk)
  - HX8357
  - ILI9163
  - ILI9225
  - ILI9341 (WioTerminal, ESP-WROVER-KIT, ODROID-GO, LoLin D32 Pro, WiFiBoy Pro)
  - ILI9342 (M5Stack, M5Stack Core2, ESP32-S3-BOX)
  - ILI9481
  - ILI9486
  - ILI9488 (Makerfabs Touch with Camera)
  - IT8951 (M5Paper)
  - NT35510/OTM8009A
  - R61529
  - RA8875
  - RM68120
  - SH110x (SH1106, SH1107, M5Stack Unit OLED)
  - S6D04K1
  - SSD1306 (SSD1309)
  - SSD1327
  - SSD1331
  - SSD1351 (SSD1357)
  - SSD1963
  - ST7735 (M5StickC, TTGO T-Wristband, TTGO TS, LoLin D32 Pro, WiFiBoy mini, ESPboy, PyBadge)
  - ST7789 (M5StickCPlus, TTGO T-Watch, ESP-WROVER-KIT, Makerfabs MakePython, DSTIKE D-duino-32 XS)
  - ST7796 (WT32-SC01)
  - M5Stack Unit LCD
  - M5Stack AtomDisplay

- TouchScreens
  - I2C CST816S
  - I2C FT5x06 (FT5206, FT5306, FT5406, FT6206, FT6236, FT6336, FT6436)
  - I2C GSLx680 (GSL1680)
  - I2C GT911
  - I2C NS2009
  - I2C TT21xxx (TT21100)
  - SPI XPT2046
  - SPI STMPE610

Setting examples are in [src/lgfx_user](src/lgfx_user)
This library is also compatible with the above models and display panels with a similar command system,
but only those that have been obtained and confirmed to work are officially supported.

## How to use
There are specific samples in [examples](examples/).
### Basic usage
```c
// ※ If you are using a compatible model in an environment other than the Arduino IDE,
// or if the compatible model is not in the board manager (such as TTGO T-Wristband or ESP-WROVER-KIT),
// please write the definition of define LGFX_～ before including LovyanGFX.hpp.

// #define LGFX_M5STACK // M5Stack M5Stack Basic / Gray / Go / Fire
// #define LGFX_M5STACK_CORE2 // M5Stack M5Stack Core2
// #define LGFX_M5STACK_COREINK // M5Stack M5Stack CoreInk
// #define LGFX_M5STICK_C // M5Stack M5Stick C / CPlus
// #define LGFX_M5PAPER // M5Stack M5Paper
// #define LGFX_M5TOUGH // M5Stack M5Tough
// #define LGFX_M5ATOMS3 // M5Stack M5ATOMS3
// #define LGFX_ODROID_GO // ODROID-GO
// #define LGFX_TTGO_TS // TTGO TS
// #define LGFX_TTGO_TWATCH // TTGO T-Watch
// #define LGFX_TTGO_TWRISTBAND // TTGO T-Wristband
// #define LGFX_TTGO_TDISPLAY // TTGO T-Display
// #define LGFX_DDUINO32_XS // DSTIKE D-duino-32 XS
// #define LGFX_LOLIN_D32_PRO // LoLin D32 Pro
// #define LGFX_LOLIN_S3_PRO // LoLin S3 Pro
// #define LGFX_ESP_WROVER_KIT // Espressif ESP-WROVER-KIT
// #define LGFX_ESP32_S3_BOX // Espressif ESP32-S3-BOX
// #define LGFX_ESP32_S3_BOX_LITE // Espressif ESP32-S3-BOX Lite
// #define LGFX_WIFIBOY_PRO // WiFiBoy Pro
// #define LGFX_WIFIBOY_MINI // WiFiBoy mini
// #define LGFX_MAKERFABS_TOUCHCAMERA // Makerfabs Touch with Camera
// #define LGFX_MAKERFABS_MAKEPYTHON // Makerfabs MakePython
// #define LGFX_MAKERFABS_TFT_TOUCH_SPI // Makerfabs TFT Touch SPI
// #define LGFX_MAKERFABS_TFT_TOUCH_PARALLEL16// Makerfabs TFT Touch Parallel 16
// #define LGFX_WT32_SC01 // Seeed WT32-SC01
// #define LGFX_WIO_TERMINAL // Seeed Wio Terminal
// #define LGFX_PYBADGE // Adafruit PyBadge
// #define LGFX_FUNHOUSE // Adafruit FunHouse
// #define LGFX_FEATHER_ESP32_S2_TFT // Adafruit Feather ESP32 S2 TFT
// #define LGFX_FEATHER_ESP32_S3_TFT // Adafruit Feather ESP32 S3 TFT
// #define LGFX_ESPBOY // ESPboy
// #define LGFX_WYWY_ESP32S3_HMI_DEVKIT // wywy ESP32S3 HMI DevKit
// #define LGFX_SUNTON_ESP32_2432S028 // Sunton ESP32 2432S028

#define LGFX_AUTODETECT // Automatic recognition (D-duino-32 XS, WT32-SC01, PyBadge are excluded from automatic recognition because the panel ID cannot be read)

// By defining multiple models or defining LGFX_AUTODETECT, the board will be automatically recognized at runtime.

// Include the header.
#include <LovyanGFX.hpp>

#include <LGFX_AUTODETECT.hpp> // Prepare the class "LGFX"
// #include <lgfx_user/LGFX_ESP32_sample.hpp> // Or prepare your own LGFX class

static LGFX lcd; // Create an instance of LGFX.
static LGFX_Sprite sprite(&lcd); // Create an instance of LGFX_Sprite if you want to use sprites.

// If you are currently using TFT_eSPI and want to avoid changing the source as much as possible, you can use this header.
// #include <LGFX_TFT_eSPI.hpp>
// static TFT_eSPI lcd; // TFT_eSPI is defined as an alias for LGFX.
// static TFT_eSprite sprite(&lcd); // TFT_eSprite is defined as an alias for LGFX_Sprite.

// If you want to use a configuration that is not in the compatible models, refer to examples/HowToUse/2_user_setting.ino.
// Also, there are setting examples in the src/lgfx_user folder.

void setup(void)
{
  // First, call the initialization function.
  lcd.init();

  // Set the rotation direction from 4 directions, 0-3. (Using 4-7 will result in an upside-down display.)
  lcd.setRotation(1);

  // Set the backlight brightness in the range of 0-255.
  lcd.setBrightness(128);

  // Set the color mode as needed. (The initial value is 16)
  // 16 is faster because the amount of SPI communication is smaller, but the gradation of red and blue is 5 bits.
  // 24 has a larger amount of SPI communication, but the gradation is expressed beautifully.
  //lcd.setColorDepth(16); // Set to 16-bit RGB565
  lcd.setColorDepth(24); // Set to 24-bit RGB888 (The number of colors displayed will be 18-bit RGB666 depending on the panel performance)

  // The basic drawing functions for shapes are as follows.
  /*
  fillScreen ( color); // Fill the entire screen
  drawPixel ( x, y , color); // Point
  drawFastVLine ( x, y , h , color); // Vertical line
  drawFastHLine ( x, y, w , color); // Horizontal line
  drawRect ( x, y, w, h , color); // Rectangle outline
  fillRect ( x, y, w, h , color); // Filled rectangle
  drawRoundRect ( x, y, w, h, r, color); // Rounded rectangle outline
  fillRoundRect ( x, y, w, h, r, color); // Filled rounded rectangle
  drawCircle ( x, y , r, color); // Circle outline
  fillCircle ( x, y , r, color); // Filled circle
  drawEllipse ( x, y, rx, ry , color); // Ellipse outline
  fillEllipse ( x, y, rx, ry , color); // Filled ellipse
  drawLine ( x0, y0, x1, y1 , color); // Line between two points
  drawTriangle ( x0, y0, x1, y1, x2, y2, color); // Triangle outline between three points
  fillTriangle ( x0, y0, x1, y1, x2, y2, color); // Filled triangle between three points
  drawBezier ( x0, y0, x1, y1, x2, y2, color); // Bezier curve between three points
  drawBezier ( x0, y0, x1, y1, x2, y2, x3, y3, color); // Bezier curve between four points
  drawArc ( x, y, r0, r1, angle0, angle1, color); // Arc outline
  fillArc ( x, y, r0, r1, angle0, angle1, color); // Filled arc
  */

  // For example, to draw a point with drawPixel, the arguments are X coordinate, Y coordinate, and color.
  lcd.drawPixel(0, 0, 0xFFFF); // Draw a white point at coordinate 0:0

  // A function to generate a color code is available and can be used to specify colors.
  // The arguments are red, green, and blue, each specified from 0 to 255.
  // It is recommended to use color888 to prevent the loss of color information.
  lcd.drawFastVLine(2, 0, 100, lcd.color888(255, 0, 0)); // Draw a vertical line in red
  lcd.drawFastVLine(4, 0, 100, lcd.color565( 0, 255, 0)); // Draw a vertical line in green
  lcd.drawFastVLine(6, 0, 100, lcd.color332( 0, 0, 255)); // Draw a vertical line in blue

  // If you do not use the color code generation function, it will be as follows.
  // RGB888 24-bit specified uint32_t type
  // RGB565 16-bit specified uint16_t type, int32_t type
  // RGB332 8-bit specified uint8_t type

  // Using the uint32_t type is treated as 24-bit RGB888.
  // You can write red, green, and blue in order with two hexadecimal digits.
  // Use a uint32_t type variable, add a U at the end, or cast it to the uint32_t type.
  uint32_t red = 0xFF0000;
  lcd.drawFastHLine(0, 2, 100, red); // Draw a horizontal line in red
  lcd.drawFastHLine(0, 4, 100, 0x00FF00U); // Draw a horizontal line in green
  lcd.drawFastHLine(0, 6, 100, (uint32_t)0xFF); // Draw a horizontal line in blue

  // Using the uint16_t and int32_t types are treated as 16-bit RGB565.
  // If you do not write it in a special way, it will be treated as the int32_t type, so this method will be used.
  // (This is for compatibility with AdafruitGFX and TFT_eSPI.)
  uint16_t green = 0x07E0;
  lcd.drawRect(10, 10, 50, 50, 0xF800); // Draw a rectangle outline in red
  lcd.drawRect(12, 12, 50, 50, green); // Draw a rectangle outline in green
  lcd.drawRect(14, 14, 50, 50, (uint16_t)0x1F); // Draw a rectangle outline in blue

  // Using the int8_t and uint8_t types are treated as 8-bit RGB332.
  uint8_t blue = 0x03;
  lcd.fillRect(20, 20, 20, 20, (uint8_t)0xE0); // Fill a rectangle in red
  lcd.fillRect(30, 30, 20, 20, (uint8_t)0x1C); // Fill a rectangle in green
  lcd.fillRect(40, 40, 20, 20, blue); // Fill a rectangle in blue

  // The color of the arguments of the drawing function can be omitted.
  // If omitted, the color set with the setColor function or the last used color will be used as the drawing color.
  // If you draw repeatedly with the same color, it will work slightly faster if you omit it.
  lcd.setColor(0xFF0000U); // Specify red as the drawing color
  lcd.fillCircle ( 40, 80, 20 ); // Fill a circle in red
  lcd.fillEllipse( 80, 40, 10, 20); // Fill an ellipse in red
  lcd.fillArc ( 80, 80, 20, 10, 0, 90); // Fill an arc in red
  lcd.fillTriangle(80, 80, 60, 80, 80, 60); // Fill a triangle in red
  lcd.setColor(0x0000FFU); // Specify blue as the drawing color
  lcd.drawCircle ( 40, 80, 20 ); // Draw a circle outline in blue
  lcd.drawEllipse( 80, 40, 10, 20); // Draw an ellipse outline in blue
  lcd.drawArc ( 80, 80, 20, 10, 0, 90); // Draw an arc outline in blue
  lcd.drawTriangle(60, 80, 80, 80, 80, 60); // Draw a triangle outline in blue
  lcd.setColor(0x00FF00U); // Specify green as the drawing color
  lcd.drawBezier( 60, 80, 80, 80, 80, 60); // Draw a quadratic Bezier curve in green
  lcd.drawBezier( 60, 80, 80, 20, 20, 80, 80, 60);// Draw a cubic Bezier curve in green

  // The drawGradientLine that draws a gradient line cannot omit the color specification.
  lcd.drawGradientLine( 0, 80, 80, 0, 0xFF0000U, 0x0000FFU);// A gradient line from red to blue

  delay(1000);

  // You can fill the entire screen with clear or fillScreen.
  // fillScreen is the same as specifying the entire screen for fillRect, and the color specification is treated as the drawing color.
  lcd.fillScreen(0xFFFFFFu); // Fill with white
  lcd.setColor(0x00FF00u); // Specify green as the drawing color
  lcd.fillScreen(); // Fill with green

  // clear holds the color as a background color, separate from the drawing functions.
  // The background color is not used often, but it is also used as the color to fill the gap when using the scroll function.
  lcd.clear(0xFFFFFFu); // Specify white as the background color and fill
  lcd.setBaseColor(0x000000u);// Specify black as the background color
  lcd.clear(); // Fill with black

  // The SPI bus is automatically secured and released when the drawing function is called,
  // but if you want to emphasize the drawing speed, use startWrite and endWrite before and after the drawing process.
  // The securing and releasing of the SPI bus is suppressed, and the speed is improved.
  // In the case of electronic paper (EPD), the drawing after startWrite() is reflected on the screen by calling endWrite().
  lcd.drawLine(0, 1, 39, 40, red); // Secure SPI bus, draw line, release SPI bus
  lcd.drawLine(1, 0, 40, 39, blue); // Secure SPI bus, draw line, release SPI bus
  lcd.startWrite(); // Secure SPI bus
  lcd.drawLine(38, 0, 0, 38, 0xFFFF00U); // Draw line
  lcd.drawLine(39, 1, 1, 39, 0xFF00FFU); // Draw line
  lcd.drawLine(40, 2, 2, 40, 0x00FFFFU); // Draw line
  lcd.endWrite(); // Release SPI bus

  // startWrite and endWrite count the number of calls internally,
  // and if called repeatedly, only the first and last will work.
  // Be sure to use startWrite and endWrite in pairs.
  // (If you don't mind occupying the SPI bus, you can also call startWrite once at the beginning and not endWrite.)
  lcd.startWrite(); // Count +1, secure SPI bus
  lcd.startWrite(); // Count +1
  lcd.startWrite(); // Count +1
  lcd.endWrite(); // Count -1
  lcd.endWrite(); // Count -1
  lcd.endWrite(); // Count -1, release SPI bus
  lcd.endWrite(); // Do nothing
  // In addition, if you call endWrite excessively, it will do nothing and the count will not be negative.

  // If you want to forcibly release and secure the SPI bus regardless of the startWrite count status,
  // use endTransaction and beginTransaction.
  // The count is not cleared, so be careful not to get it out of sync.
  lcd.startWrite(); // Count +1, secure SPI bus
  lcd.startWrite(); // Count +1
  lcd.drawPixel(0, 0); // Draw
  lcd.endTransaction(); // Release SPI bus
  // Other SPI devices can be used here
  // When using another device on the same SPI bus (such as an SD card),
  // be sure to do so with the SPI bus released.
  lcd.beginTransaction(); // Secure SPI bus
  lcd.drawPixel(0, 0); // Draw
  lcd.endWrite(); // Count -1
  lcd.endWrite(); // Count -1, release SPI bus

  // In addition to drawPixel, there is a function called writePixel that draws a point.
  // drawPixel secures the SPI bus as needed,
  // while writePixel does not check the status of the SPI bus.
  lcd.startWrite(); // Secure SPI bus
  for (uint32_t x = 0; x < 128; ++x) {
    for (uint32_t y = 0; y < 128; ++y) {
      lcd.writePixel(x, y, lcd.color888( x*2, x + y, y*2));
    }
  }
  lcd.endWrite(); // Release SPI bus
  // All functions starting with write~ must be explicitly called with startWrite beforehand.
  // This applies to writePixel, writeFastVLine, writeFastHLine, and writeFillRect.

  delay(1000);

  // The same drawing functions can be used for drawing on a sprite (off-screen).
  // First, specify the color depth of the sprite with setColorDepth. (If omitted, it is treated as 16.)
  //sprite.setColorDepth(1); // Set to 1-bit (2 colors) palette mode
  //sprite.setColorDepth(2); // Set to 2-bit (4 colors) palette mode
  //sprite.setColorDepth(4); // Set to 4-bit (16 colors) palette mode
  //sprite.setColorDepth(8); // Set to 8-bit RGB332
  //sprite.setColorDepth(16); // Set to 16-bit RGB565
  sprite.setColorDepth(24); // Set to 24-bit RGB888

  // ※ After setting setColorDepth(8);, calling createPalette() will enter 256-color palette mode.
  // sprite.createPalette();

  // Use createSprite to specify the width and height and secure the memory.
  // The memory consumed is proportional to the color depth and area. Be careful as it will fail to secure memory if it is too large.
  sprite.createSprite(65, 65); // Create a sprite with a width of 65 and a height of 65.

  for (uint32_t x = 0; x < 64; ++x) {
    for (uint32_t y = 0; y < 64; ++y) {
      sprite.drawPixel(x, y, lcd.color888(3 + x*4, (x + y)*2, 3 + y*4)); // Draw on the sprite
    }
  }
  sprite.drawRect(0, 0, 65, 65, 0xFFFF);

  // The created sprite can be output to any coordinate with pushSprite.
  // The output destination will be the LGFX that was passed as an argument when the instance was created.
  sprite.pushSprite(64, 0); // Draw the sprite at coordinate 64,0 of the lcd

  // If you did not pass a pointer to the drawing destination when creating the sprite instance,
  // or if there are multiple LGFXs, you can also specify the output destination as the first argument and pushSprite.
  sprite.pushSprite(&lcd, 0, 64); // Draw the sprite at coordinate 0,64 of the lcd

  delay(1000);

  // You can rotate, enlarge, and reduce the sprite with pushRotateZoom and draw it.
  // The coordinates set with setPivot are treated as the center of rotation, and the drawing is performed so that the center of rotation is located at the coordinates of the drawing destination.
  sprite.setPivot(32, 32); // Treat coordinate 32,32 as the center
  int32_t center_x = lcd.width()/2;
  int32_t center_y = lcd.height()/2;
  lcd.startWrite();
  for (int angle = 0; angle <= 360; ++angle) {
    sprite.pushRotateZoom(center_x, center_y, angle, 2.5, 3); // Draw at the center of the screen with an angle, width 2.5 times, height 3 times

    if ((angle % 36) == 0) lcd.display(); // Update the display once every 36 times in the case of electronic paper
  }
  lcd.endWrite();

  delay(1000);

  // To release the memory of a sprite that is no longer in use, use deleteSprite.
  sprite.deleteSprite();

  // After deleteSprite, you can still reuse the same instance.
  sprite.setColorDepth(4); // Set to 4-bit (16 colors) palette mode
  sprite.createSprite(65, 65);

  // In a palette mode sprite, the color of the arguments of the drawing function is treated as a palette number.
  // When drawing with pushSprite, etc., the actual drawing color is determined by referring to the palette.

  // In 4-bit (16 colors) palette mode, palette numbers 0-15 can be used.
  // The initial color of the palette is black at 0 and white at the end, with a gradation from 0 to the end.
  // To set the color of the palette, use setPaletteColor.
  sprite.setPaletteColor(1, 0x0000FFU); // Set palette number 1 to blue
  sprite.setPaletteColor(2, 0x00FF00U); // Set palette number 2 to green
  sprite.setPaletteColor(3, 0xFF0000U); // Set palette number 3 to red

  sprite.fillRect(10, 10, 45, 45, 1); // Fill a rectangle with palette number 1
  sprite.fillCircle(32, 32, 22, 2); // Fill a circle with palette number 2
  sprite.fillTriangle(32, 12, 15, 43, 49, 43, 3); // Fill a triangle with palette number 3

  // You can specify a color that is not drawn with the last argument of pushSprite.
  sprite.pushSprite( 0, 0, 0); // Draw the sprite with palette 0 as transparent
  sprite.pushSprite(65, 0, 1); // Draw the sprite with palette 1 as transparent
  sprite.pushSprite( 0, 65, 2); // Draw the sprite with palette 2 as transparent
  sprite.pushSprite(65, 65, 3); // Draw the sprite with palette 3 as transparent

  delay(5000);

  lcd.startWrite(); // By calling startWrite() here, the SPI bus is kept occupied.
}

void loop(void)
{
  static int count = 0;
  static int a = 0;
  static int x = 0;
  static int y = 0;
  static float zoom = 3;
  ++count;
  if ((a += 1) >= 360) a -= 360;
  if ((x += 2) >= lcd.width()) x -= lcd.width();
  if ((y += 1) >= lcd.height()) y -= lcd.height();
  sprite.setPaletteColor(1, lcd.color888( 0, 0, count & 0xFF));
  sprite.setPaletteColor(2, lcd.color888( 0,~count & 0xFF, 0));
  sprite.setPaletteColor(3, lcd.color888( count & 0xFF, 0, 0));

  sprite.pushRotateZoom(x, y, a, zoom, zoom, 0);

  if ((count % 100) == 0) lcd.display(); // Update the display once every 100 times in the case of electronic paper
}
```

# Notes and restrictions
## How to coexist with M5Stack.h (M5StickC.h)
### Method 1
Please write include <LovyanGFX.hpp> after include <M5Stack.h>.
Do not use M5.Lcd, but prepare and use a separate LGFX instance.
### Method 2
If you use [ESP32-Chimera-Core](https://github.com/tobozo/ESP32-Chimera-Core), M5.Lcd will be LovyanGFX.

## Motivation behind this library
TFT_eSPI is a great library. However, it is structurally complex because it targets multiple architectures, making it very difficult to add required functions such as ESP-IDF support and 18-bit color support.
LovyanGFX has been created to add these features and optimize performance.

## Acknowledgements
Thanks to [Bodmer](https://github.com/Bodmer/), author of the [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) library, for the inspiration to create this library.
Thanks to [Adafruit Industries](https://github.com/adafruit/) for publishing [AdafruitGFX](https://github.com/adafruit/Adafruit-GFX-Library), which is the basis for TFT_eSPI.
Thanks to [ChaN](http://elm-chan.org/), author of [TJpgDec](http://elm-chan.org/fsw/tjpgd/00index.html) (Tiny JPEG Decompressor).
Thanks to [kikuchan](https://github.com/kikuchan/), author of [Pngle](https://github.com/kikuchan/pngle) (PNG Loader for Embedding).
Thanks to [Richard Moore](https://github.com/ricmoo/), author of [QRCode](https://github.com/ricmoo/QRCode/) (QR code generation library).
Thanks to [ciniml](https://github.com/ciniml), for many technical tips and help with validation in the ESP-IDF environment.
Thanks to [mongonta0716](https://github.com/mongonta0716), for verifying the work from the beginning of the development with many bugs and for his advice.
Thanks to [tobozo](https://github.com/tobozo), for testing it on various boards, translating it into English and giving me a lot of advice.
Thanks to [TANAKA Masayuki](https://github.com/tanakamasayuki), for creating the font data.
Thanks to [YAMANEKO](https://github.com/yamamaya), for creating the [lgfxFontSubsetGenerator](https://github.com/yamamaya/lgfxFontSubsetGenerator).
Thanks to [yasuhirok](https://github.com/yasuhirok-git), for add Raspberry pi pico (RP2040) support.
Thanks to [IAMLIUBO](https://github.com/imliubo), for add Linux FrameBuffer support.
Thanks to [rossum](https://github.com/rossumur) and [Roger Cheng](https://github.com/Roger-random), published the project to output a composite video signal from ESP32.

## Included library
[TJpgDec](http://elm-chan.org/fsw/tjpgd/00index.html) [ChaN](http://elm-chan.org/)
[Pngle](https://github.com/kikuchan/pngle) [kikuchan](https://github.com/kikuchan/)
[QRCode](https://github.com/ricmoo/QRCode/) [Richard Moore](https://github.com/ricmoo/) and [Nayuki](https://www.nayuki.io/page/qr-code-generator-library)

## Credits
- Inspiration: [Bodmer](https://github.com/Bodmer)
- Author: [lovyan03](https://github.com/lovyan03)
- Contributors:
  - [ciniml](https://github.com/ciniml)
  - [mongonta0716](https://github.com/mongonta0716)
  - [tobozo](https://github.com/tobozo)

## License
main : [FreeBSD](license.txt)
TJpgDec : [original](src/lgfx/utility/lgfx_tjpgd.c) ChaN
Pngle : [MIT](https://github.com/kikuchan/pngle/blob/master/LICENSE) kikuchan
QRCode : [MIT](https://github.com/ricmoo/QRCode/blob/master/LICENSE.txt) Richard Moore and Nayuki
result : [MIT](https://github.com/bitwizeshift/result/blob/master/LICENSE) Matthew Rodusek
GFX font and GLCD font : [2-clause BSD](https://github.com/adafruit/Adafruit-GFX-Library/blob/master/license.txt) Adafruit Industries
Font 2,4,6,7,8 : [FreeBSD](https://github.com/Bodmer/TFT_eSPI/blob/master/license.txt) Bodmer
converted IPA font : [IPA Font License](src/lgfx/Fonts/IPA/IPA_Font_License_Agreement_v1.0.txt) IPA
efont : [3-clause BSD](src/lgfx/Fonts/efont/COPYRIGHT.txt) The Electronic Font Open Laboratory
TomThumb font : [3-clause BSD](src/lgfx/Fonts/GFXFF/TomThumb.h) Brian J. Swetland / Vassilii Khachaturov / Dan Marks

## Unimplemented request
- Displays
  - SEPS525
  - LT7680A / LT7685
  - RA8873 / RA8876
