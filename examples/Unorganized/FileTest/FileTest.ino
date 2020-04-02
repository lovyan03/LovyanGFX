
#include <SD.h>
#include <esp_heap_caps.h>

/*
#include <M5Stack.h>
//#include <ESP32-Chimera-Core.h>
static M5Display &tft (M5.Lcd);
static TFT_eSprite sprite(&tft);
/*/
#include <LovyanGFX.hpp>
static LGFX tft;
static LGFX_Sprite sprite(&tft);
//*/

void dj(const char* filename) {
//  if (!tft.drawJpgFile(SD, filename, 0, 0))
//Serial.printf("jpg:%s\r\n", filename);

  File file = SD.open(filename);
  uint8_t* tmp = (uint8_t*)heap_caps_malloc(file.size(), MALLOC_CAP_DEFAULT);
  if (tmp) {
    file.read(tmp, file.size());
    if (!tft.drawJpg(tmp, file.size(), 0, 0))
//    if (!tft.drawJpg(tmp, file.size(), 0, 0, 0, 0, 0, 0, lgfx::jpeg_div_t::JPEG_DIV_2))
    Serial.printf("jpg:%s\r\n", filename);
    heap_caps_free(tmp);
  }
  file.close();
}

void drawPngMem(const char* filename, int32_t x = 0, int32_t y = 0) {
//  if (!tft.drawJpgFile(SD, filename, 0, 0))
//Serial.printf("jpg:%s\r\n", filename);

  File file = SD.open(filename);
  uint8_t* tmp = (uint8_t*)heap_caps_malloc(file.size(), MALLOC_CAP_DEFAULT);
  if (tmp) {
    file.read(tmp, file.size());
    if (!tft.drawPng(tmp, file.size(), x, y))
//    if (!tft.drawJpg(tmp, file.size(), 0, 0, 0, 0, 0, 0, lgfx::jpeg_div_t::JPEG_DIV_2))
    Serial.printf("png:%s\r\n", filename);
    heap_caps_free(tmp);
  }
  file.close();
}

void drawPng(const char* filename, int32_t x = 0, int32_t y = 0) {
  if (!tft.drawPngFile(SD, filename, x, y, 0, 0, 0, 0, 1)) {
    Serial.printf("png:%s\r\n", filename);
  }
}

void drawPngScore(const char* filename) {
//  if (!tft.drawJpgFile(SD, filename, 0, 0))
//Serial.printf("jpg:%s\r\n", filename);

  File file = SD.open(filename);
  uint8_t* tmp = (uint8_t*)heap_caps_malloc(file.size(), MALLOC_CAP_DEFAULT);
  if (tmp) {
    file.read(tmp, file.size());

//taskDISABLE_INTERRUPTS();
    uint64_t ms = micros();
    for (int32_t i= 0; i < 4; ++i) { tft.drawPng(tmp, file.size(), i, i); }
    ms = micros() - ms;
//taskENABLE_INTERRUPTS();
    Serial.printf("%d ms \r\n", ms);

    heap_caps_free(tmp);
  }
  file.close();
}

void drawJpgScore(const char* filename) {
  File file = SD.open(filename);
  uint8_t* tmp = (uint8_t*)heap_caps_malloc(file.size(), MALLOC_CAP_DEFAULT);
  if (tmp) {
    file.read(tmp, file.size());

taskDISABLE_INTERRUPTS();
    uint64_t ms = micros();
    for (int32_t i= 0; i < 16; ++i) { tft.drawJpg(tmp, file.size(), i, i); }
    ms = micros() - ms;
taskENABLE_INTERRUPTS();
    Serial.printf("%d ms \r\n", ms);

    heap_caps_free(tmp);
  }
  file.close();
}

void setup()
{
disableCore0WDT();
disableCore1WDT();
  Serial.begin(115200);
  SD.begin(4, SPI, 20000000);  // 4=TFCARD_CS_PIN

  //sprite.setPsram(true); // PSRAM使う場合
  sprite.setColorDepth(8); // 8 , 16 , 24
//  sprite.createSprite(320,240);
//  sprite.drawBmpFile(SD, "/m5stack_mono.bmp",      0,  10);
//  sprite.drawBmpFile(SD, "/m5stack_16color.bmp", 110,  10);
//  sprite.drawBmpFile(SD, "/m5stack_256color.bmp",220,  10);
//  sprite.drawBmpFile(SD, "/m5stack_16bpp.bmp",     0, 130);
//  sprite.drawBmpFile(SD, "/m5stack_24bpp.bmp",   110, 130);
//  sprite.drawBmpFile(SD, "/m5stack_32bpp.bmp",   220, 130);

  tft.init();
  tft.setRotation(1);

  tft.setColorDepth(24);

  tft.fillRect(  0, 10,50,50,0x8410);
  tft.fillRect(110, 10,50,50,0x8410);
  tft.fillRect(220, 10,50,50,0x8410);
  tft.fillRect(  0,130,50,50,0x8410);
  tft.fillRect(110,130,50,50,0x8410);
  tft.fillRect(220,130,50,50,0x8410);

//  sprite.createFromBmpFile(SD, "/m5stack_mono.bmp"    );  sprite.pushSprite(  0,  10);
//  sprite.createFromBmpFile(SD, "/m5stack_16color.bmp" );  sprite.pushSprite(110,  10);
//  sprite.createFromBmpFile(SD, "/m5stack_256color.bmp");  sprite.pushSprite(220,  10);
//  sprite.createFromBmpFile(SD, "/m5stack_16bpp.bmp"   );  sprite.pushSprite(  0, 130);
//  sprite.createFromBmpFile(SD, "/m5stack_24bpp.bmp"   );  sprite.pushSprite(110, 130);
//  sprite.createFromBmpFile(SD, "/m5stack_32bpp.bmp"   );  sprite.pushSprite(220, 130);

sprite.createFromBmpFile(SD, "/bmp/argb8888.bmp");  sprite.pushSprite(  0,  10);
sprite.createFromBmpFile(SD, "/bmp/rgb888.bmp"  );  sprite.pushSprite(110,  10);
sprite.createFromBmpFile(SD, "/bmp/rgb565.bmp"  );  sprite.pushSprite(220,  10);
sprite.createFromBmpFile(SD, "/bmp/index256.bmp");  sprite.pushSprite(  0, 130);
sprite.createFromBmpFile(SD, "/bmp/index16.bmp" );  sprite.pushSprite(110, 130);
//sprite.createFromBmpFile(SD, "/bmp/index4.bmp"  );  sprite.pushSprite(220, 130);
//sprite.createFromBmpFile(SD, "/bmp/index1.bmp"  );
sprite.createFromBmpFile(SD, "/bmp/rle256.bmp"  );  sprite.pushSprite(220, 130);
//sprite.createFromBmpFile(SD, "/bmp/rle16.bmp"   );


delay(1000);
}

void loop() {
  static uint32_t count, sec, psec=-1;
  count++;
  sec = millis()/1000;
  if (psec != sec) {
    psec = sec;
//    Serial.printf("%d \r\n", (float)count);
    count = 0;
//    tft.setRotation((sec&1) ? 1:3);
    tft.setColorDepth( ( tft.getColorDepth()==24 ) ? 16 : 24);
    Serial.printf("colorDepth = %d \r\n", tft.getColorDepth());
//    tft.setAddrWindow(0, 0, 320, 240);
  }

  drawJpgScore("/m5stack.jpg");
delay(10);

//*
  drawPngScore("/m5stack.png");

  tft.fillRect(9,9,52,52,0xFF00FFU);
  tft.drawJpgFile(SD, "/m5stack.jpg", 10,10,50,50,10,10);

  tft.fillRect(9,79,52,52,0xFF00FFU);
  tft.drawJpgFile(SD, "/m5stack.jpg", 10,80,50,50,-10,-10);

  tft.fillRect(9,159,52,52,0xFF00FFU);
  tft.drawJpgFile(SD, "/m5stack.jpg", 10,160,50,50,30,30, (lgfx::jpeg_div_t)1);

  tft.fillRect(129,9,52,52,0xFF00FFU);
  tft.drawPngFile(SD, "/m5stack.png", 130,10,50,50,10,10);

  tft.fillRect(129,79,52,52,0xFF00FFU);
  tft.drawPngFile(SD, "/m5stack.png", 130,80,50,50,-10,-10);

  tft.fillRect(129,159,52,52,0xFF00FFU);
  tft.drawPngFile(SD, "/m5stack.png", 130,160,50,50,30,30,0.5);

  tft.drawBmpFile(SD, "/bmp/rle256.bmp",   100, 100);
delay(1000);

//*/



/*
//tft.fillScreen(++count<<4);
//  sprite.pushSprite(random(-20, 20), random(-20, 20), 0xFFFFFFFF);
  //sprite.pushSprite(random(-20, 20), random(-20, 20));
  //sprite.pushSprite(0,0);
  tft.drawBmpFile(SD, "/m5stack_mono.bmp",      0,  10);
  tft.drawBmpFile(SD, "/m5stack_16color.bmp", 110,  10);
  tft.drawBmpFile(SD, "/m5stack_256color.bmp",220,  10);
  tft.drawBmpFile(SD, "/m5stack_16bpp.bmp",     0, 130);
  tft.drawBmpFile(SD, "/m5stack_24bpp.bmp",   110, 130);
  tft.drawBmpFile(SD, "/m5stack_32bpp.bmp",   220, 130);
//*/

  uint64_t ms = micros();
/*
drawPng("/pngsuite/PngSuite.png");
drawPng("/pngsuite/basi0g01.png",   0,   0);
drawPng("/pngsuite/basi0g02.png",  20,   0);
drawPng("/pngsuite/basi0g04.png",  40,   0);
drawPng("/pngsuite/basi0g08.png",  60,   0);
drawPng("/pngsuite/basi0g16.png",  80,   0);
drawPng("/pngsuite/basi2c08.png", 100,   0);
drawPng("/pngsuite/basi2c16.png", 120,   0);
drawPng("/pngsuite/basi3p01.png", 140,   0);
drawPng("/pngsuite/basi3p02.png", 160,   0);
drawPng("/pngsuite/basi3p04.png", 180,   0);
drawPng("/pngsuite/basi3p08.png", 200,   0);
drawPng("/pngsuite/basi4a08.png", 220,   0);
drawPng("/pngsuite/basi4a16.png", 240,   0);
drawPng("/pngsuite/basi6a08.png", 260,   0);
drawPng("/pngsuite/basi6a16.png", 280,   0);
drawPng("/pngsuite/basn0g01.png",   0,  20);
drawPng("/pngsuite/basn0g02.png",  20,  20);
drawPng("/pngsuite/basn0g04.png",  40,  20);
drawPng("/pngsuite/basn0g08.png",  60,  20);
drawPng("/pngsuite/basn0g16.png",  80,  20);
drawPng("/pngsuite/basn2c08.png", 100,  20);
drawPng("/pngsuite/basn2c16.png", 120,  20);
drawPng("/pngsuite/basn3p01.png", 140,  20);
drawPng("/pngsuite/basn3p02.png", 160,  20);
drawPng("/pngsuite/basn3p04.png", 180,  20);
drawPng("/pngsuite/basn3p08.png", 200,  20);
drawPng("/pngsuite/basn4a08.png", 220,  20);
drawPng("/pngsuite/basn4a16.png", 240,  20);
drawPng("/pngsuite/basn6a08.png", 260,  20);
drawPng("/pngsuite/basn6a16.png", 280,  20);
drawPng("/pngsuite/bgai4a08.png",   0,  40);
drawPng("/pngsuite/bgai4a16.png",  20,  40);
drawPng("/pngsuite/bgan6a08.png",  40,  40);
drawPng("/pngsuite/bgan6a16.png",  60,  40);
drawPng("/pngsuite/bgbn4a08.png",  80,  40);
drawPng("/pngsuite/bggn4a16.png", 100,  40);
drawPng("/pngsuite/bgwn6a08.png", 120,  40);
drawPng("/pngsuite/bgyn6a16.png", 140,  40);
drawPng("/pngsuite/ccwn2c08.png", 160,  40);
drawPng("/pngsuite/ccwn3p08.png", 180,  40);
drawPng("/pngsuite/cdfn2c08.png", 200,  40);
drawPng("/pngsuite/cdhn2c08.png", 220,  40);
drawPng("/pngsuite/cdsn2c08.png", 240,  40);
drawPng("/pngsuite/cdun2c08.png", 260,  40);
drawPng("/pngsuite/ch1n3p04.png", 280,  40);
drawPng("/pngsuite/ch2n3p08.png",   0,  60);
drawPng("/pngsuite/cm0n0g04.png",  20,  60);
drawPng("/pngsuite/cm7n0g04.png",  40,  60);
drawPng("/pngsuite/cm9n0g04.png",  60,  60);
drawPng("/pngsuite/cs3n2c16.png",  80,  60);
drawPng("/pngsuite/cs3n3p08.png", 100,  60);
drawPng("/pngsuite/cs5n2c08.png", 120,  60);
drawPng("/pngsuite/cs5n3p08.png", 140,  60);
drawPng("/pngsuite/cs8n2c08.png", 160,  60);
drawPng("/pngsuite/cs8n3p08.png", 180,  60);
drawPng("/pngsuite/ct0n0g04.png", 200,  60);
drawPng("/pngsuite/ct1n0g04.png", 220,  60);
drawPng("/pngsuite/cten0g04.png", 240,  60);
drawPng("/pngsuite/ctfn0g04.png", 260,  60);
drawPng("/pngsuite/ctgn0g04.png", 280,  60);
drawPng("/pngsuite/cthn0g04.png",   0,  80);
drawPng("/pngsuite/ctjn0g04.png",  20,  80);
drawPng("/pngsuite/ctzn0g04.png",  40,  80);
drawPng("/pngsuite/exif2c08.png",  60,  80);
drawPng("/pngsuite/f00n0g08.png",  80,  80);
drawPng("/pngsuite/f00n2c08.png", 100,  80);
drawPng("/pngsuite/f01n0g08.png", 120,  80);
drawPng("/pngsuite/f01n2c08.png", 140,  80);
drawPng("/pngsuite/f02n0g08.png", 160,  80);
drawPng("/pngsuite/f02n2c08.png", 180,  80);
drawPng("/pngsuite/f03n0g08.png", 200,  80);
drawPng("/pngsuite/f03n2c08.png", 220,  80);
drawPng("/pngsuite/f04n0g08.png", 240,  80);
drawPng("/pngsuite/f04n2c08.png", 260,  80);
drawPng("/pngsuite/f99n0g04.png", 280,  80);
drawPng("/pngsuite/g03n0g16.png",   0, 100);
drawPng("/pngsuite/g03n2c08.png",  20, 100);
drawPng("/pngsuite/g03n3p04.png",  40, 100);
drawPng("/pngsuite/g04n0g16.png",  60, 100);
drawPng("/pngsuite/g04n2c08.png",  80, 100);
drawPng("/pngsuite/g04n3p04.png", 100, 100);
drawPng("/pngsuite/g05n0g16.png", 120, 100);
drawPng("/pngsuite/g05n2c08.png", 140, 100);
drawPng("/pngsuite/g05n3p04.png", 160, 100);
drawPng("/pngsuite/g07n0g16.png", 180, 100);
drawPng("/pngsuite/g07n2c08.png", 200, 100);
drawPng("/pngsuite/g07n3p04.png", 220, 100);
drawPng("/pngsuite/g10n0g16.png", 240, 100);
drawPng("/pngsuite/g10n2c08.png", 260, 100);
drawPng("/pngsuite/g10n3p04.png", 280, 100);
drawPng("/pngsuite/g25n0g16.png",   0, 120);
drawPng("/pngsuite/g25n2c08.png",  20, 120);
drawPng("/pngsuite/g25n3p04.png",  40, 120);
drawPng("/pngsuite/oi1n0g16.png",  60, 120);
drawPng("/pngsuite/oi1n2c16.png",  80, 120);
drawPng("/pngsuite/oi2n0g16.png", 100, 120);
drawPng("/pngsuite/oi2n2c16.png", 120, 120);
drawPng("/pngsuite/oi4n0g16.png", 140, 120);
drawPng("/pngsuite/oi4n2c16.png", 160, 120);
drawPng("/pngsuite/oi9n0g16.png", 180, 120);
drawPng("/pngsuite/oi9n2c16.png", 200, 120);
drawPng("/pngsuite/pp0n2c16.png", 220, 120);
drawPng("/pngsuite/pp0n6a08.png", 240, 120);
drawPng("/pngsuite/ps1n0g08.png", 260, 120);
drawPng("/pngsuite/ps1n2c16.png", 280, 120);
drawPng("/pngsuite/ps2n0g08.png",   0, 140);
drawPng("/pngsuite/ps2n2c16.png",  20, 140);
drawPng("/pngsuite/s01i3p01.png",  40, 140);
drawPng("/pngsuite/s01n3p01.png",  60, 140);
drawPng("/pngsuite/s02i3p01.png",  80, 140);
drawPng("/pngsuite/s02n3p01.png", 100, 140);
drawPng("/pngsuite/s03i3p01.png", 120, 140);
drawPng("/pngsuite/s03n3p01.png", 140, 140);
drawPng("/pngsuite/s04i3p01.png", 160, 140);
drawPng("/pngsuite/s04n3p01.png", 180, 140);
drawPng("/pngsuite/s05i3p02.png", 200, 140);
drawPng("/pngsuite/s05n3p02.png", 220, 140);
drawPng("/pngsuite/s06i3p02.png", 240, 140);
drawPng("/pngsuite/s06n3p02.png", 260, 140);
drawPng("/pngsuite/s07i3p02.png", 280, 140);
drawPng("/pngsuite/s07n3p02.png",   0, 160);
drawPng("/pngsuite/s08i3p02.png",  20, 160);
drawPng("/pngsuite/s08n3p02.png",  40, 160);
drawPng("/pngsuite/s09i3p02.png",  60, 160);
drawPng("/pngsuite/s09n3p02.png",  80, 160);
drawPng("/pngsuite/s32i3p04.png", 100, 160);
drawPng("/pngsuite/s32n3p04.png", 120, 160);
drawPng("/pngsuite/s33i3p04.png", 140, 160);
drawPng("/pngsuite/s33n3p04.png", 160, 160);
drawPng("/pngsuite/s34i3p04.png", 180, 160);
drawPng("/pngsuite/s34n3p04.png", 200, 160);
drawPng("/pngsuite/s35i3p04.png", 220, 160);
drawPng("/pngsuite/s35n3p04.png", 240, 160);
drawPng("/pngsuite/s36i3p04.png", 260, 160);
drawPng("/pngsuite/s36n3p04.png", 280, 160);
drawPng("/pngsuite/s37i3p04.png",   0, 180);
drawPng("/pngsuite/s37n3p04.png",  20, 180);
drawPng("/pngsuite/s38i3p04.png",  40, 180);
drawPng("/pngsuite/s38n3p04.png",  60, 180);
drawPng("/pngsuite/s39i3p04.png",  80, 180);
drawPng("/pngsuite/s39n3p04.png", 100, 180);
drawPng("/pngsuite/s40i3p04.png", 120, 180);
drawPng("/pngsuite/s40n3p04.png", 140, 180);
drawPng("/pngsuite/tbbn0g04.png", 160, 180);
drawPng("/pngsuite/tbbn2c16.png", 180, 180);
drawPng("/pngsuite/tbbn3p08.png", 200, 180);
drawPng("/pngsuite/tbgn2c16.png", 220, 180);
drawPng("/pngsuite/tbgn3p08.png", 240, 180);
drawPng("/pngsuite/tbrn2c08.png", 260, 180);
drawPng("/pngsuite/tbwn0g16.png", 280, 180);
drawPng("/pngsuite/tbwn3p08.png",   0, 200);
drawPng("/pngsuite/tbyn3p08.png",  20, 200);
drawPng("/pngsuite/tm3n3p02.png",  40, 200);
drawPng("/pngsuite/tp0n0g08.png",  60, 200);
drawPng("/pngsuite/tp0n2c08.png",  80, 200);
drawPng("/pngsuite/tp0n3p08.png", 100, 200);
drawPng("/pngsuite/tp1n3p08.png", 120, 200);
drawPng("/pngsuite/z00n2c08.png", 140, 200);
drawPng("/pngsuite/z03n2c08.png", 160, 200);
drawPng("/pngsuite/z06n2c08.png", 180, 200);
drawPng("/pngsuite/z09n2c08.png", 200, 200);
//drawPng("/pngsuite/xc1n0g08.png");
//drawPng("/pngsuite/xc9n2c08.png");
//drawPng("/pngsuite/xcrn0g04.png");
//drawPng("/pngsuite/xcsn0g01.png");
//drawPng("/pngsuite/xd0n2c08.png");
//drawPng("/pngsuite/xd3n2c08.png");
//drawPng("/pngsuite/xd9n2c08.png");
//drawPng("/pngsuite/xdtn0g01.png");
//drawPng("/pngsuite/xhdn0g08.png");
//drawPng("/pngsuite/xlfn0g04.png");
//drawPng("/pngsuite/xs1n0g01.png");
//drawPng("/pngsuite/xs2n0g01.png");
//drawPng("/pngsuite/xs4n0g01.png");
//drawPng("/pngsuite/xs7n0g01.png");
*/
  dj("/umauma/image_001.jpg"); // , -10,  -5);
  dj("/umauma/image_002.jpg"); // ,  -5, -10);
  dj("/umauma/image_003.jpg"); // ,   0, -10);
  dj("/umauma/image_004.jpg"); // ,   5, -10);
  dj("/umauma/image_005.jpg"); // ,  10,  -5);
  dj("/umauma/image_006.jpg"); // ,  10,   0);
  dj("/umauma/image_007.jpg"); // ,  10,   5);
  dj("/umauma/image_008.jpg"); // ,   5,  10);
  dj("/umauma/image_009.jpg"); // ,   0,  10);
  dj("/umauma/image_010.jpg"); // ,  -5,  10);
  dj("/umauma/image_011.jpg"); // , -10,   5);
  dj("/umauma/image_012.jpg"); // , -10,   0);
/*

  dj("/jpeg_gray.jpg");
delay(500);
  dj("/m5stack.jpg");
  dj("/jpeg_color.jpg");
  dj("/m5stack.jpg");
  dj("/jpeg_nosub.jpg");
//  tft.drawJpgFile(SD, "/m5stack_p.jpg", random(-50, 50), random(50, 150));
//delay(100);
//  tft.drawJpgFile(SD, "/jpeg_progre.jpg" , 0, 0);
//delay(100);

dj("/jpg/Avatar_fugu.jpg");
dj("/jpg/BLEChorder.jpg");
dj("/jpg/BLEChorder_gh.jpg");
dj("/jpg/DrawNumber.jpg");
dj("/jpg/SWRasterizer.jpg");
dj("/jpg/SetWiFi_Mic_gh.jpg");
dj("/jpg/WebRadio_Avatar4.jpg");
dj("/jpg/WebRadio_Avatar5.jpg");
dj("/jpg/tetris_bg.jpg");
dj("/jpg/tetrisbg.jpg");
dj("/jpg/_gh.jpg");
dj("/jpg/2048.jpg");
dj("/jpg/2048_gh.jpg");
dj("/jpg/AERO_TRACKERS_M5.jpg");
dj("/jpg/AERO_TRACKERS_M5_gh.jpg");
dj("/jpg/Arduinomegachess.jpg");
dj("/jpg/Arduinomegachess_gh.jpg");
dj("/jpg/Avatar_fugu_bg.jpg");
dj("/jpg/Avatar_fugu_gh.jpg");
dj("/jpg/Avatar_fugu_water1.jpg");
dj("/jpg/Avatar_fugu_water2.jpg");
dj("/jpg/Avatar_fugu_water3.jpg");
dj("/jpg/Avatar_m5Face.jpg");
dj("/jpg/Avatar_m5Face_gh.jpg");
dj("/jpg/Avatar_Mic3_LightMusic.jpg");
dj("/jpg/Avatar_Mic3_LightMusic_gh.jpg");
dj("/jpg/Colours_Demo.jpg");
dj("/jpg/Colours_Demo_gh.jpg");
dj("/jpg/crack.jpg");
dj("/jpg/CrackScreen.jpg");
dj("/jpg/CrackScreen_gh.jpg");
dj("/jpg/CrazyAsteroids.jpg");
dj("/jpg/CrazyAsteroids_gh.jpg");
dj("/jpg/D_invader.jpg");
dj("/jpg/D_invader_gh.jpg");
dj("/jpg/Downloader.jpg");
dj("/jpg/Downloader_gh.jpg");
dj("/jpg/DrawNumber_gh.jpg");
dj("/jpg/ESP32_radio_gh.jpg");
dj("/jpg/ETERNAL_STRIKER_M5_S_wPAD.jpg");
dj("/jpg/ETERNAL_STRIKER_M5_S_wPAD_gh.jpg");
dj("/jpg/FlappyBird.jpg");
dj("/jpg/FlappyBird_gh.jpg");
dj("/jpg/I_COLLECTOR_SOUND_M5.jpg");
dj("/jpg/I_COLLECTOR_SOUND_M5_gh.jpg");
dj("/jpg/Lora_Frequency_Hopping.jpg");
dj("/jpg/Lora_Frequency_Hopping_gh.jpg");
dj("/jpg/LovyanLauncher.jpg");
dj("/jpg/LovyanLauncher_gh.jpg");
dj("/jpg/LovyanToyBox.jpg");
dj("/jpg/LovyanToyBox_gh.jpg");
dj("/jpg/M5Apple2.jpg");
dj("/jpg/M5Apple2_gh.jpg");
dj("/jpg/M5Bala_Avatar.jpg");
dj("/jpg/M5Bala_Avatar_gh.jpg");
dj("/jpg/menu.jpg");
dj("/jpg/menu_gh.jpg");
dj("/jpg/MiniOthello.jpg");
dj("/jpg/MiniOthello_gh.jpg");
dj("/jpg/Mp3-player.jpg");
dj("/jpg/Mp3-player_gh.jpg");
dj("/jpg/MultiApps-Adv.jpg");
dj("/jpg/MultiApps-Adv_gh.jpg");
dj("/jpg/Nixietube.jpg");
dj("/jpg/Nixietube_gh.jpg");
dj("/jpg/NyanCat.jpg");
dj("/jpg/NyanCat_Ext.jpg");
dj("/jpg/NyanCat_Ext_gh.jpg");
dj("/jpg/NyanCat_gh.jpg");
dj("/jpg/Oscilloscope.jpg");
dj("/jpg/Oscilloscope_gh.jpg");
dj("/jpg/PacketMonitor.jpg");
dj("/jpg/PacketMonitor_gh.jpg");
dj("/jpg/Pacman-JoyPSP.jpg");
dj("/jpg/Pacman-JoyPSP_gh.jpg");
dj("/jpg/Particle_demo.jpg");
dj("/jpg/Particle_demo_gh.jpg");
dj("/jpg/Pixel-Fun.jpg");
dj("/jpg/Pixel-Fun_gh.jpg");
dj("/jpg/Raytracer.jpg");
dj("/jpg/Raytracer_gh.jpg");
dj("/jpg/Rickroll.jpg");
dj("/jpg/Rickroll_gh.jpg");
dj("/jpg/RotateyCube.jpg");
dj("/jpg/SD-Updater.jpg");
dj("/jpg/SetWiFi_Mic.jpg");
dj("/jpg/Sokoban.jpg");
dj("/jpg/Sokoban_gh.jpg");
dj("/jpg/SpaceDefense.jpg");
dj("/jpg/SpaceDefense_gh.jpg");
dj("/jpg/SpaceShooter.jpg");
dj("/jpg/SpaceShooter_gh.jpg");
dj("/jpg/SWRasterizer_gh.jpg");
dj("/jpg/Tetris.jpg");
dj("/jpg/Tetris_gh.jpg");
dj("/jpg/Tetris_T_K_KAI2_TRH.jpg");
dj("/jpg/Tetris_T_K_KAI2_TRH_gh.jpg");
dj("/jpg/TFT_Clock_Digital_NTP.jpg");
dj("/jpg/TFT_Clock_Digital_NTP_gh.jpg");
dj("/jpg/Thermal-Camera.jpg");
dj("/jpg/Thermal-Camera_gh.jpg");
dj("/jpg/TinyBasicPlus_CardKB.jpg");
dj("/jpg/TinyBasicPlus_CardKB_gh.jpg");
dj("/jpg/TinyBasicPlus_FaceKB.jpg");
dj("/jpg/TinyBasicPlus_FaceKB_gh.jpg");
dj("/jpg/TobozoLauncher.jpg");
dj("/jpg/TobozoLauncher_gh.jpg");
dj("/jpg/Tube.jpg");
dj("/jpg/Tube_gh.jpg");
dj("/jpg/WebRadio_Avatar4_gh.jpg");
dj("/jpg/WebRadio_Avatar5_gh.jpg");
dj("/jpg/WebRadio_Avator.jpg");
dj("/jpg/WebRadio_Avator_DotFace.jpg");
dj("/jpg/WebRadio_Avator_DotFace_gh.jpg");
dj("/jpg/WebRadio_Avator_gh.jpg");
dj("/jpg/WebRadio_Avator_Saizou.jpg");
dj("/jpg/WebRadio_Avator_Saizou_gh.jpg");
dj("/jpg/WiFiScanner.jpg");
dj("/jpg/WiFiScanner_gh.jpg");

//*/

  ms = micros() - ms;
  Serial.printf("%d ms \r\n", ms);
}
