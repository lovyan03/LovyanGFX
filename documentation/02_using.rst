***************
Using LovyanGFX
***************

The Basics of Using LovyanGFX
=============================

While development continues at a high pace, the current best way to see what LovyanGFX can do is to look at the example below. It should show you the basics of how to include the library, initialise the display and use the various commands to draw things on the display.

There are lots of useful and working `examples <https://github.com/lovyan03/LovyanGFX/tree/master/examples>`_. Below is just one that should give you some idea of how to use LovyanGFX.

.. code-block:: cpp
   :name: 1_simple_use.ino
   :linenos:

	// Basic Usage of LovyanGFX

	// If you use a compatible display in an environment other than Arduino IDE, or
	// If the compatible model is not in the board manager (TTGO T-Wristband,
	// ESP-WROVER-KIT, etc.), please put a '#define LGFX_[..]' statement before
	// #include LovyanGFX.hpp

	// #define LGFX_M5STACK               // M5Stack (Basic / Gray / Go / Fire)
	// #define LGFX_M5STACK_CORE2         // M5Stack Core2
	// #define LGFX_M5STACK_COREINK       // M5Stack CoreInk
	// #define LGFX_M5STICK_C             // M5Stick C / CPlus
	// #define LGFX_M5PAPER               // M5Paper
	// #define LGFX_ODROID_GO             // ODROID-GO
	// #define LGFX_TTGO_TS               // TTGO TS
	// #define LGFX_TTGO_TWATCH           // TTGO T-Watch
	// #define LGFX_TTGO_TWRISTBAND       // TTGO T-Wristband
	// #define LGFX_DDUINO32_XS           // DSTIKE D-duino-32 XS
	// #define LGFX_LOLIN_D32_PRO         // LoLin D32 Pro
	// #define LGFX_ESP_WROVER_KIT        // ESP-WROVER-KIT
	// #define LGFX_WIFIBOY_PRO           // WiFiBoy Pro
	// #define LGFX_WIFIBOY_MINI          // WiFiBoy mini
	// #define LGFX_MAKERFABS_TOUCHCAMERA // Makerfabs Touch with Camera
	// #define LGFX_MAKERFABS_MAKEPYTHON  // Makerfabs MakePython
	// #define LGFX_WIO_TERMINAL          // Wio Terminal


	// Automatic detection (Currently supports M5Stack, M5StickC/CPlus, ODROID-GO,
	// TTGO T-Watch, TTGO T-Wristband, LoLin D32 Pro, ESP-WROVER-KIT)
	#define LGFX_AUTODETECT

	// If multiple models are defined, or if you define LGFX_AUTODETECT, the board
	// will be automatically recognized at runtime.


	// include the header file
	#include <LovyanGFX.hpp>

	static LGFX lcd;                 // Instance of LGFX
	static LGFX_Sprite sprite(&lcd); // Instance of LGFX_Sprite when using sprites

	// If you are currently using TFT_eSPI and want to minimize changes to your
	// code, you can use this header.
	// #include <LGFX_TFT_eSPI.hpp>
	// static TFT_eSPI lcd;               // TFT_eSPI is an alias for LGFX.
	// static TFT_eSprite sprite(&lcd);   // TFT_eSprite is alias for LGFX_Sprite


	// See examples/HowToUse/2_spi_setting.ino if you want to use it in a
	// configuration that is not available on compatible models. Copy
	// LGFX_Config_Custom.hpp in the config folder and edit to suit your environment.
	// Include here or paste the contents of the file as it is and use it.


	void setup(void)
	{
	  // First call the initialization function.
	  lcd.init();


	  // Set the rotation direction from 4 directions from 0 to 3.
	  // (If you use rotations 4 to 7, the image will be mirrored.)
	  lcd.setRotation(1);


	  // Set the backlight brightness in the range 0-255
	  lcd.setBrightness(128);


	  // Set the color mode as needed. (Initial value is 16)
	  // 16 - Faster, but the red and blue tones are 5 bits.
	  // 24 - Slower, but the gradation expression is cleaner.
	  //lcd.setColorDepth(16);  // Set to 16 bits of RGB565
	  lcd.setColorDepth(24);    // Set to 24 bits for RGB888 - Note that the actual
								// number of colors displayed may be 18 bits (RGB666)
								// depending on the display hardware.


	  // The basic figure drawing function are as follows.
	  /*
		fillScreen    (                color);
		drawPixel     ( x, y         , color);
		drawFastVLine ( x, y   , h   , color);
		drawFastHLine ( x, y, w      , color);
		drawRect      ( x, y, w, h   , color);
		fillRect      ( x, y, w, h   , color);
		drawRoundRect ( x, y, w, h, r, color);
		fillRoundRect ( x, y, w, h, r, color);
		drawCircle    ( x, y      , r, color);
		fillCircle    ( x, y      , r, color);
		drawEllipse   ( x, y, rx, ry , color);
		fillEllipse   ( x, y, rx, ry , color);
		drawLine      ( x0, y0, x1, y1        , color);
		drawTriangle  ( x0, y0, x1, y1, x2, y2, color);
		fillTriangle  ( x0, y0, x1, y1, x2, y2, color);
		drawBezier    ( x0, y0, x1, y1, x2, y2, color);         // 3-point Bezier
		drawBezier    ( x0, y0, x1, y1, x2, y2, x3, y3, color); // 4-point Bezier
		drawArc       ( x, y, r0, r1, angle0, angle1, color);
		fillArc       ( x, y, r0, r1, angle0, angle1, color);
	  */


	  // For example, when drawing a point with drawPixel, there are three
	  // arguments: X coordinate, Y coordinate, and color.
	  lcd.drawPixel(0, 0, 0xFFFF);    // White dot at coordinates 0, 0


	  // A function to generate a color code is provided and can be used to specify
	  // a color. As arguments specify red, green, and blue from 0 to 255, resp.
	  // It is recommended to use color888 to prevent missing color information.
	  lcd.drawFastVLine(2, 0, 100, lcd.color888(255,   0,   0));  // red
	  lcd.drawFastVLine(4, 0, 100, lcd.color565(  0, 255,   0));  // green
	  lcd.drawFastVLine(6, 0, 100, lcd.color332(  0,   0, 255));  // blue


	  // If the color code generation function is not used, it will be as follows.
	  // RGB888 24-bit specified uint32_t type
	  // RGB565 16-bit specification uint16_t type, int32_t type
	  // RGB332 Specify with 8 bits uint8_t type

	  // If you use uint32_t type, it will be treated as 24-bit RGB888.
	  // You can write in the order of red, green, and blue with 2 hexadecimal digits.
	  // Use a uint32_t type variable, add a U at the end, or cast it to a uint32_t.
	  uint32_t red = 0xFF0000;
	  lcd.drawFastHLine(0, 2, 100, red);            // horiz. line in red
	  lcd.drawFastHLine(0, 4, 100, 0x00FF00U);      // horiz. line in green
	  lcd.drawFastHLine(0, 6, 100, (uint32_t)0xFF); // horiz. line in blue

	  // If you use uint16_t type and int32_t type, it will be treated as 16 bits
	  // of RGB565. This method is used because it is treated as int32_t type unless
	  // it is written in a special way. (This is done for compatibility with
	  // AdafruitGFX and TFT_eSPI.)
	  uint16_t green = 0x07E0;
	  lcd.drawRect(10, 10, 50, 50, 0xF800);         // red
	  lcd.drawRect(12, 12, 50, 50, green);          // green
	  lcd.drawRect(14, 14, 50, 50, (uint16_t)0x1F); // blue


	  // If you use int8_t or uint8_t type, it will be treated as 8 bits, RGB332.
	  uint8_t blue = 0x03;
	  lcd.fillRect(20, 20, 20, 20, (uint8_t)0xE0);  // red
	  lcd.fillRect(30, 30, 20, 20, (uint8_t)0x1C);  // green
	  lcd.fillRect(40, 40, 20, 20, blue);           // blue


	  // The color of the drawing function argument can be omitted.
	  // If omitted, the color set by the setColor function or the last used color
	  // will be used as the foreground color. If you draw repeatedly in the same
	  // color, omitting it will render slightly faster.
	  lcd.setColor(0xFF0000U);                        // red as drawing color
	  lcd.fillCircle ( 40, 80, 20    );               // fill circle in red
	  lcd.fillEllipse( 80, 40, 10, 20);               // fill arc in red
	  lcd.fillArc    ( 80, 80, 20, 10, 0, 90);        // fill ellipse in red
	  lcd.fillTriangle(80, 80, 60, 80, 80, 60);       // fill triangle red
	  lcd.setColor(0x0000FFU);                        // blue as drawing color
	  lcd.drawCircle ( 40, 80, 20    );               // circle outline in blue
	  lcd.drawEllipse( 80, 40, 10, 20);               // ellipse outline in blue
	  lcd.drawArc    ( 80, 80, 20, 10, 0, 90);        // arc outline in blue
	  lcd.drawTriangle(60, 80, 80, 80, 80, 60);       // triable outline in blue
	  lcd.setColor(0x00FF00U);                        // green as drawing color
	  lcd.drawBezier( 60, 80, 80, 80, 80, 60);        // green 3-point Bezier curve
	  lcd.drawBezier( 60, 80, 80, 20, 20, 80, 80, 60);// green 4-point Bezier curve

	  // Using DrawGradientLine you cannot omit the color specification
	  lcd.drawGradientLine( 0, 80, 80, 0, 0xFF0000U, 0x0000FFU);// Red to blue

	  delay(1000);

	  // You can fill the entire screen with clear or fillScreen.
	  // fillScreen is the same as specifying the entire screen of fillRect, and
	  // the color specification is treated as the drawing color.
	  lcd.fillScreen(0xFFFFFFu);   // Fill with white
	  lcd.setColor(0x00FF00u);     // Green as the drawing color
	  lcd.fillScreen();            // Fill with green

	  // clear is different from the drawing function and holds the color as a
	  // background color. The background color is rarely used, but it is used as
	  // the color to fill the gap when using the scroll function.
	  lcd.clear(0xFFFFFFu);        // Fill with white as background color
	  lcd.setBaseColor(0x000000u); // Specify black as the background color
	  lcd.clear();                 // Fill with black


	  // The SPI bus is allocated and released automatically when drawing functions
	  // are called. If drawing speed is important, use startWrite and endWrite
	  // before and after the drawing process. This suppresses securing and releasing
	  // the SPI bus, improving speed. In the case of electronic paper (EPD), any
	  // drawing after startWrite() is held until calling endWrite().
	  lcd.drawLine(0, 1, 39, 40, red);       // Secure SPI bus, draw line, release
	  lcd.drawLine(1, 0, 40, 39, blue);      // Secure SPI bus, draw line, release
	  lcd.startWrite();                      // Secure SPI bus
	  lcd.drawLine(38, 0, 0, 38, 0xFFFF00U); // Draw a line
	  lcd.drawLine(39, 1, 1, 39, 0xFF00FFU); // Draw a line
	  lcd.drawLine(40, 2, 2, 40, 0x00FFFFU); // Draw a line
	  lcd.endWrite();                        // Release SPI bus


	  // startWrite and endWrite internally count the number of calls and if you call
	  // it repeatedly, it will only work at the beginning and end. Be sure to use
	  // startWrite and endWrite so that they are paired. (If you don't mind occupying
	  // the SPI bus, you can call startWrite once first and not endWrite.)
	  lcd.startWrite();     // Count +1, secure SPI bus
	  lcd.startWrite();     // Count +1
	  lcd.startWrite();     // Count +1
	  lcd.endWrite();       // Count -1
	  lcd.endWrite();       // Count -1
	  lcd.endWrite();       // Count -1, SPI bus release
	  lcd.endWrite();       // do nothing
	  // If you call endWrite excessively, nothing will be done and the count will
	  // not go negative.

	  // If you want to forcibly release / secure the SPI bus regardless of the count
	  // status of startWrite, use endTransaction / beginTransaction.
	  // The count will not be cleared, so still be careful to make counts match.
	  lcd.startWrite();       // Count +1, secure SPI bus
	  lcd.startWrite();       // Count +1
	  lcd.drawPixel(0, 0);    // Draw
	  lcd.endTransaction();   // Release SPI bus
	  // Other SPI devices can be used here.
	  // When using another device (SD card, etc.) on the same SPI bus, make sure
	  // that the SPI bus is free before continuing.
	  lcd.beginTransaction(); // Secure SPI bus
	  lcd.drawPixel(0, 0);    // Draw
	  lcd.endWrite();         // Count -1
	  lcd.endWrite();         // Count -1, SPI bus release



	  // Apart from drawPixel, there is a function that draws a point called
	  // writePixel. drawPixel reserves the SPI bus as needed, while writePixel does
	  // not check the status of the SPI bus.
	  lcd.startWrite();   // Secure SPI bus
	  for (uint32_t x = 0; x < 128; ++x) {
		for (uint32_t y = 0; y < 128; ++y) {
		  lcd.writePixel(x, y, lcd.color888( x*2, x + y, y*2));
		}
	  }
	  lcd.endWrite();    // SPI bus release
	  // All functions whose names start with write (writePixel, writeFastVLine,
	  // writeFastHLine, writeFillRect) must explicitly call startWrite.

	  delay(1000);

	  // Similar drawing functions can be used for drawing on sprites (offscreen).
	  // First, specify the color depth of the sprite with setColorDepth. (If
	  // omitted, it will be treated as 16.)
	  //sprite.setColorDepth(1);   // Set to 1-bit (2 colors) palette mode
	  //sprite.setColorDepth(2);   // Set to 2-bit (4 colors) palette mode
	  //sprite.setColorDepth(4);   // Set to 4-bit (16 colors) palette mode
	  //sprite.setColorDepth(8);   // Set to 8-bit RGB332
	  //sprite.setColorDepth(16);  // Set to 16 bits in RGB565
	  sprite.setColorDepth(24);    // Set to 24-bit RGB888


	  // If you calling createPalette() after setting setColorDepth(8), it will be
	  // in 256 color palette mode.
	  // sprite.createPalette();


	  // Use createSprite to specify the width and height to allocate memory.
	  // Memory consumption is proportional to color depth and area. Please note
	  // that if it is too large, memory allocation will fail.
	  sprite.createSprite(65, 65); // Create sprite with width 65 and height 65.

	  for (uint32_t x = 0; x < 64; ++x) {
		for (uint32_t y = 0; y < 64; ++y) {
		  // Draw on sprite
		  sprite.drawPixel(x, y, lcd.color888(3 + x*4, (x + y)*2, 3 + y*4));
		}
	  }
	  sprite.drawRect(0, 0, 65, 65, 0xFFFF);

	  // The created sprite can be output to any coordinates with pushSprite.
	  // The output destination will be the LGFX passed as an argument when creating
	  // the sprite instance.
	  sprite.pushSprite(64, 0);        // Draw sprite at coordinates 64, 0 on lcd

	  // If you did not pass the pointer of the drawing destination when creating
	  // the instance of sprite, or if you have multiple LGFX, you can also pushSprite
	  // by specifying the output destination as the first argument.
	  sprite.pushSprite(&lcd, 0, 64);  // SPI bus release

	  delay(1000);

	  // You can rotate, scale, and draw sprites with pushRotateZoom.
	  // The coordinates set by setPivot are treated as the center of rotation, and
	  // the center of rotation is drawn so that it is located at the coordinates
	  // of the drawing destination.
	  sprite.setPivot(32, 32);    // Rotate around 32, 32
	  int32_t center_x = lcd.width()/2;
	  int32_t center_y = lcd.height()/2;
	  lcd.startWrite();
	  for (int angle = 0; angle <= 360; ++angle) {
		// Draw at all angles, width 2.5 times, height 3 times,
		// in the center of the screen
		sprite.pushRotateZoom(center_x, center_y, angle, 2.5, 3);

		// Update the display for electronic paper once every 36 times
		if ((angle % 36) == 0) lcd.display();
	  }
	  lcd.endWrite();

	  delay(1000);

	  // Use deleteSprite to free memory for sprites that are no longer in use.
	  sprite.deleteSprite();

	  // The same instance can be reused after deleteSprite.
	  sprite.setColorDepth(4);     // Set to 4-bit (16 colors) palette mode
	  sprite.createSprite(65, 65);

	  // In palette mode sprites, the color of the drawing function argument is
	  // treated as the palette number. When drawing with pushSprite etc., the
	  // actual drawing color is determined by referring to the palette.


	  // In 4-bit (16 colors) palette mode, palette numbers 0 to 15 can be used.
	  // The initial color of the palette is black at 0, white at the end of the
	  // palette, and a gradation from 0 to the end. Use setPaletteColor() to set
	  // the palette color.
	  sprite.setPaletteColor(1, 0x0000FFU);    // Set palette 1 to blue
	  sprite.setPaletteColor(2, 0x00FF00U);    // Set palette 2 to green
	  sprite.setPaletteColor(3, 0xFF0000U);    // Set palette 3 to red

	  sprite.fillRect(10, 10, 45, 45, 1);   // Fill rectangle with palette color 1
	  sprite.fillCircle(32, 32, 22, 2);               // same for color 2
	  sprite.fillTriangle(32, 12, 15, 43, 49, 43, 3); // same for color 3

	  // The last argument of pushSprite allows you to specify a color that is not
	  // drawn, meaning that where it appears in a sprite, the previous pixels that
	  // were on the screen will still be visible after pushSprite.
	  sprite.pushSprite( 0,  0, 0); // Draw sprite with palette color 0 transparent
	  sprite.pushSprite(65,  0, 1); // Draw sprite with palette color 1 transparent
	  sprite.pushSprite( 0, 65, 2); // Draw sprite with palette color 2 transparent
	  sprite.pushSprite(65, 65, 3); // Draw sprite with palette color 3 transparent

	  delay(5000);

	  lcd.startWrite(); // StartWrite() here to keep the SPI bus occupied.
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

	  // Update the display for electronic paper once every 100 times.
	  if ((count % 100) == 0) lcd.display();
	}