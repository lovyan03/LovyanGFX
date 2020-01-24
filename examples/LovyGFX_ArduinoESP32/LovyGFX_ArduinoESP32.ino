
#if defined(ARDUINO_M5Stick_C)
 #include <AXP192.h>
#endif

//#include <M5Stack.h>
//M5Display& tft_espi(M5.Lcd);
#include <LGFX_TFT_eSPI.hpp>


#if defined ( ARDUINO_ESP32_DEV ) // ESP-WROVER-KIT

  struct LGFX_Config2 {
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int spi_mosi = 23;
    static constexpr int spi_miso = 25;
    static constexpr int spi_sclk = 19;
    static constexpr int spi_dc   = 21;
    static constexpr int spi_cs   = 12;

    static constexpr int panel_rst = -1;
    static constexpr int panel_bl  = -1;
    static constexpr int freq_write = 27000000;
    static constexpr int freq_read  = 14000000;
    static constexpr int freq_fill  = 27000000;
    //static constexpr bool spi_3wire = true;
    static constexpr int panel_x      = 24;
    static constexpr int panel_y      = 0;
    static constexpr int panel_width  = 80;
    static constexpr int panel_height = 160;
    static constexpr int ram_width    = 128;
    static constexpr int ram_height   = 160;
  };

  struct LGFX_Config3 {
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int spi_mosi = 23;
    static constexpr int spi_miso = 25;
    static constexpr int spi_sclk = 19;
    static constexpr int spi_dc   = 21;
    static constexpr int spi_cs   = 13;

    static constexpr int panel_rst = -1;
    static constexpr int panel_bl  = -1;
    static constexpr int freq_write = 20000000;
    static constexpr int freq_read  = 10000000;
    static constexpr int freq_fill  = 20000000;
    static constexpr bool spi_3wire = true;
    static constexpr int ram_width    = 128;
    static constexpr int ram_height   = 160;
    static constexpr int panel_width  = 128;
    static constexpr int panel_height = 160;
  };

  struct LGFX_Config4 {
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int spi_mosi = 23;
    static constexpr int spi_miso = 25;
    static constexpr int spi_sclk = 19;
    static constexpr int spi_dc   = 21;
    static constexpr int spi_cs   = 27;

    static constexpr int panel_rst = -1;
    static constexpr int panel_bl  = -1;
    static constexpr int freq_write = 14000000;
    static constexpr int freq_read  = 10000000;
    static constexpr int freq_fill  = 14000000;
    //static constexpr bool spi_3wire = true;
    static constexpr int ram_width    = 128;
    static constexpr int ram_height   = 128;
    static constexpr int panel_width  = 128;
    static constexpr int panel_height = 96;
    static constexpr int panel_y      = 32;
  };

  struct LGFX_Config5 {
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int spi_mosi = 23;
    static constexpr int spi_miso = 25;
    static constexpr int spi_sclk = 19;
    static constexpr int spi_dc   = 21;
    static constexpr int spi_cs   = 14;

    static constexpr int panel_rst = -1;
    static constexpr int panel_bl  = -1;
    static constexpr int freq_write = 40000000;
    static constexpr int freq_read  = 10000000;
    static constexpr int freq_fill  = 40000000;
    static constexpr bool spi_3wire = true;
    static constexpr bool invert = true;
//    static constexpr int ram_width    = 128;
//    static constexpr int ram_height   = 128;
//    static constexpr int panel_width  = 128;
//    static constexpr int panel_height = 96;
//    static constexpr int panel_y      = 32;
  };

static lgfx::Panel_ST7735R<LGFX_Config2> tft2;
static lgfx::Panel_ILI9163<LGFX_Config3> tft3;
static lgfx::PanelSsdCommon<LGFX_Config4> tft4;
static lgfx::Panel_ST7789<LGFX_Config5> tft5;

#endif

static TFT_eSPI tft_lgfx;
static TFT_eSprite sprite(&tft_lgfx);


template<typename T>
void movingSprite(T& Lcd)
{
  int32_t w = sprite.width()>>1;
  int32_t h = sprite.height()>>1;

  for (int i = -w; i < Lcd.width() - w; i++) {
    if (i<0)delay(10);
    sprite.pushSprite(&Lcd, i, -h, 1);
  }
  for (int i = -h; i < Lcd.height() - h; i++) {
    if (i<0)delay(10);
    sprite.pushSprite(&Lcd, Lcd.width() - w, i, 1);
  }
  for (int i = Lcd.width() - w; i > -w; i--) {
    if (i<0)delay(10);
    sprite.pushSprite(&Lcd, i, Lcd.height() - h, 1);
  }
  for (int i = Lcd.height() - h; i > -h; i--) {
    if (i<0)delay(10);
    sprite.pushSprite(&Lcd, - w, i, 1);
  }
}

template<typename T>
void movingCopyRect(T& Lcd)
{
  int32_t w = Lcd.width()>>1;
  int32_t h = Lcd.height()>>1;

  Lcd.startWrite();
  for (int i = -(w>>1); i < Lcd.width() - (w>>1); i++) {
    Lcd.copyRect(w>>1, h>>1, w, h, i, - (h>>1));
  }
  for (int i = -(h>>1); i < Lcd.height() - (h>>1); i++) {
    Lcd.copyRect(w>>1, h>>1, w, h, Lcd.width() - (w>>1), i);
  }
  for (int i = Lcd.width() - (w>>1); i > -(w>>1); i--) {
    Lcd.copyRect(w>>1, h>>1, w, h, i, Lcd.height() - (h>>1));
  }
  for (int i = Lcd.height() - (h>>1); i > -(h>>1); i--) {
    Lcd.copyRect(w>>1, h>>1, w, h, - (w>>1), i);
  }
  Lcd.endWrite();
}

template<typename T>
void pixelCopy(T& Lcd)
{
  int32_t width  = Lcd.width();
  int32_t height = Lcd.height();
  Lcd.startWrite();
  uint32_t c;
  for (int count = 0; count < (width * height >> 1); count++) {
    c = Lcd.readPixel(count % (width>>1)             , count / (width>>1));
    Lcd.    drawPixel(count % (width>>1) + (width>>1), count / (width>>1), c);
  }
  Lcd.endWrite();
}

template<typename T>
void copy(T& Lcd, int x, int y, int w, int h, int dstx, int dsty)
{
  Lcd.startWrite();
  uint8_t buf[320*3];
  if (y < dsty) {
    for (int count = h - 1; count >= 0; count--) {
      Lcd.readRect(x, y + count, w, 1, buf);
      Lcd.pushRect(dstx, dsty + count, w, 1, buf);
    }
  } else {
    for (int count = 0; count < h; count++) {
      Lcd.readRect(x, y + count, w, 1, buf);
      Lcd.pushRect(dstx, dsty + count, w, 1, buf);
    }
  }
  Lcd.endWrite();
}
template<typename T>
void pushRectBuffer(T& Lcd, int x)
{
  int32_t width  = Lcd.width();
  int32_t height = Lcd.height();

  uint16_t buf[320*3];
  for (int i = 0; i < 320*3; i++ ) {
    buf[i] = Lcd.color565(0xFF-(i<<3), 0x80-(i<<6), i<<4);
  }

  Lcd.startWrite();
  Lcd.pushRect(x, x, 72,1, buf);
  Lcd.endWrite();
}

template<typename T>
void pushRectBuffer1(T& Lcd, int x)
{
  int32_t width  = Lcd.width();
  int32_t height = Lcd.height();

  uint16_t buf[320*3];
  for (int i = 0; i < 320*3; i++ ) {
    buf[i] = Lcd.color565(0xFF-(i<<3), 0x80-(i<<6), i<<4);
  }

  Lcd.startWrite();
  for (int count = 0; count < 72; count++) {
    Lcd.pushRect(x, count, 1+count,1, buf);
delay(30);
  }
  Lcd.endWrite();
}

template<typename T>
void pushRectBuffer2(T& Lcd, int x)
{
  int32_t width  = Lcd.width();
  int32_t height = Lcd.height();

  uint16_t buf[320*3];
  //lgfx::swap565_t buf[320*3];
  for (int i = 0; i < 320*3; i++ ) {
    buf[i] = Lcd.color565(i, i, 0xFF);
  }

  Lcd.startWrite();
  for (int count = 72; count; count--) {
    Lcd.pushImage(x, count, 1+count,1, &buf[count]);
delay(30);
  }
  Lcd.endWrite();
}

template<typename T>
void fillRectLoop(T& Lcd)
{
  int32_t width  = Lcd.width();
  int32_t height = Lcd.height();

  Lcd.startWrite();
//  Lcd.setWindow(count, count, 32+(count>>2),4);
  for (int count = 0; count < 240; count++) {
    Lcd.fillRect(count, count, 1+(count>>2),1, 255 - (count<<2));
    //Lcd.pushColor(255 - (count<<2), 32+(count));
delay(5);
  }
  for (int count = 240; count > 0; count--) {
    Lcd.fillRect(count, count, 1+(count>>2),1, 255 - (count<<2));
delay(5);
  }
  Lcd.endWrite();
}
/*

template<typename T>
void pixelReadWrite(T& Lcd)
{
  int32_t width  = Lcd.width();
  int32_t height = Lcd.height();
  Lcd.startWrite();
  Lcd.fillRect(0, 0, width / 2, height, 0);

  for (int count = 0; count < width<<3; count++) {
    int x = (count % (width>>1));
    Lcd.fillRect(x  , random(         0, height  /3),1,3, tft.getColorFromRGB(      (x<<1) , 0, 0));
    Lcd.fillRect(x+1, random(height  /3, height*2/3),2,2, tft.getColorFromRGB(0,    (x<<2) , 0));
    Lcd.fillRect(x+2, random(height*2/3, height    ),3,1, tft.getColorFromRGB(0, 0, (x<<3) ));
  }
  for (int count = 0; count < width<<5; count++) {
    int x = (count % (width>>1));
    Lcd.drawPixel(x  , random(         0, height  /6),    tft.getColorFromRGB(      (x<<2) , 0, 0));
    Lcd.drawPixel(x+1, random(height  /3, height*3/6),    tft.getColorFromRGB(0,    (x<<3) , 0));
    Lcd.drawPixel(x+2, random(height*2/3, height*5/6),    tft.getColorFromRGB(0, 0, (x<<4) ));
  }
  for (int count = 0; count < 10; count++) {
    Lcd.drawLine( random(0, width>>1), random(0, height)
                , random(0, width>>1), random(0, height), tft.getColorFromRGB(0xFF, 0, 0));
    Lcd.drawLine( random(0, width>>1), random(0, height)
                , random(0, width>>1), random(0, height), tft.getColorFromRGB(0, 0xFF, 0));
    Lcd.drawLine( random(0, width>>1), random(0, height)
                , random(0, width>>1), random(0, height), tft.getColorFromRGB(0, 0, 0xFF));
  }
  uint32_t c;
  for (int count = 0; count < (width * height >> 1); count++) {
    c = Lcd.readPixel(count % (width>>1)             , count / (width>>1));
    Lcd.    drawPixel(count % (width>>1) + (width>>1), count / (width>>1), c);
  }
  Lcd.endWrite();
}

template<typename T>
void blockReadPixelWrite(T& Lcd)
{
  int32_t width  = Lcd.width();
  int32_t height = Lcd.height();
  Lcd.fillRect(0, 0, width / 2, height, random(0, 0xFFFFFF));
  for (int count = 0; count < 80; count++) {
    int x = (count % (height>>3));
    Lcd.drawRoundRect(count,          0 + count, (width>>1) - (count<<1), height / 3 -(count<<1), (width-count) >>4, tft.getColorFromRGB(      (x<<5) + (sec << 1), 0, 0));
    Lcd.drawRoundRect(count, height  /3 + count, (width>>1) - (count<<1), height / 3 -(count<<1), (width-count) >>4, tft.getColorFromRGB(0,    (x<<4) + (sec << 3), 0   ));
    Lcd.drawRoundRect(count, height*2/3 + count, (width>>1) - (count<<1), height / 3 -(count<<1), (width-count) >>4, tft.getColorFromRGB(0, 0, (x<<3) + (sec << 5)      ));
  }
  uint8_t buf[320*3];
  for (int count = 0; count < height; count++) {
  Lcd.startWrite();
    Lcd.readRect(         0, count, width >> 1, 1, (uint16_t*)buf);
    if (Lcd.getColorDepth() == 16) {
      for (int x = 0; x < width>>1; x++) {
        Lcd.drawPixel((width>>1)+x, count, ((uint16_t*)buf)[x]);
      }
    } else {
      for (int x = 0; x < width>>1; x++) {
        Lcd.drawPixel((width>>1)+x, count, *(uint32_t*)(&buf[x*3]) );
      }
    }
  Lcd.endWrite();
  }
}
*/
template<typename T>
void drawRects(T& Lcd, int32_t offset = 0)
{
  int32_t width  = Lcd.width();
  int32_t height = Lcd.height();
  Lcd.startWrite();
//  Lcd.fillRect(offset, 0, width >> 1, height, random(0, 0xFFFFFF));
  int w = width>>1;
  int h = height/3;
  for (int count = 0; count < 80; count++) {
    int x = (count % (width>>1));
    Lcd.drawRect(x + offset, x             , w, h, Lcd.color565(      x<<4, 0, 0));
    Lcd.drawRect(x + offset, x + height  /3, w, h, Lcd.color565(0,    x<<4, 0   ));
    Lcd.drawRect(x + offset, x + height*2/3, w, h, Lcd.color565(0, 0, x<<4      ));
    if (1 > (w -= 2) || 1 > (h -= 2)) break;
  }
  Lcd.endWrite();
}

template<typename T>
void blockReadWrite(T& Lcd, int32_t offset = 0)
{
  int32_t width  = Lcd.width();
  int32_t height = Lcd.height();
  Lcd.startWrite();
  lgfx::swap888_t buf[width*2];
memset(buf, 0, width*2*sizeof(lgfx::swap888_t));
  for (int count = 0; count < height; count+=2) {
    if (1 > (width - (count>>1))) break;
    Lcd.readRect(offset           , count, (width - (count>>1)) >> 1, 2, buf);
    //Lcd.fillRect(offset+(width>>1), count, (width - (count>>1)) >> 1, 2, count);
    Lcd.pushImage(offset+(width>>1), count, (width - (count>>1)) >> 1, 2, buf);
    Lcd.drawPixel(offset + (count>>2), count, 0xFFFF);
  }
  Lcd.endWrite();
}

template<typename T>
void copyRect(T& Lcd, int32_t offset = 0)
{
  int32_t width  = Lcd.width();
  int32_t height = Lcd.height();
  Lcd.startWrite();
  uint16_t buf[320*2];
  for (int count = 0; count < height; count++) {
    if (1 > (width - (count>>1))) break;
    Lcd.copyRect(offset+(width>>1), count, (width - (count>>1)) >> 1, 1, offset, count);
  }
  Lcd.endWrite();
}

template<typename T>
void concentricFillCircle(T& Lcd)
{
  int32_t width  = Lcd.width();
  int32_t height = Lcd.height();
  Lcd.startWrite();
  Lcd.fillRect(0, 0, width, height, random(0, 0xFFFFFF));
  for (int count = 0; count < 200; count++) {
    Lcd.drawCircle(width>>1, height>>1, 200 - count, Lcd.color565((count&1 ? 0xFF:0), (count&2 ? 0xFF:0), (count&4 ? 0xFF:0)));
  }
  Lcd.endWrite();
}

template<typename T>
void dualFillCircle(T& Lcd)
{
  int32_t width  = Lcd.width();
  int32_t height = Lcd.height();
  Lcd.startWrite();
  Lcd.fillRect(0, 0, width, height, random(0, 0xFFFFFF));
  for (int count = 0; count < 100; count++) {
    Lcd.fillCircle (width  >>2, height>>1, 100 - count, Lcd.color565((count&1 ? 0xFF:0), (count&2 ? 0xFF:0), (count&4 ? 0xFF:0)));
    Lcd.fillCircle (width*3>>2, height>>1, 100 - count, Lcd.color565((count&1 ? 0xFF:0), (count&2 ? 0xFF:0), (count&4 ? 0xFF:0)));
  }
  Lcd.endWrite();
}


//*/


void setup()
{
  disableCore1WDT();
  disableCore0WDT();
  bool lcdver = false;
#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE)
  pinMode(33, INPUT);
  delay(100);
  lcdver = digitalRead(33);
  pinMode(33, OUTPUT);
  digitalWrite(33, HIGH);
#endif

#ifdef ARDUINO
//  M5.begin();
Serial.begin(115200);
Serial.print("Arduino\r\n");
//Serial.printf("LovyanGFX mosi:%d  miso:%d  sclk:%d  cs:%d  dc:%d  rst:%d \r\n"
//             , LGFX_Config::spi_mosi, LGFX_Config::spi_miso, LGFX_Config::spi_sclk, LGFX_Config::spi_cs, LGFX_Config::spi_dc, LGFX_Config::panel_rst);
#endif

  //lgfx::TPin<LGFX_Config2::spi_cs>::init();
  //lgfx::TPin<LGFX_Config3::spi_cs>::init();
  //lgfx::TPin<LGFX_Config4::spi_cs>::init();
  //lgfx::TPin<LGFX_Config5::spi_cs>::init();
  //lgfx::TPin<LGFX_Config2::spi_cs>::hi();
  //lgfx::TPin<LGFX_Config3::spi_cs>::hi();
  //lgfx::TPin<LGFX_Config4::spi_cs>::hi();
  //lgfx::TPin<LGFX_Config5::spi_cs>::hi();

  //tft_espi.init();
  tft_lgfx.init();
  //tft2.init();
  //tft3.init();
  //tft4.init();
  //tft5.init();

  sprite.setColorDepth(2);
  sprite.createSprite(159,159);


#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE)
  const int BLK_PWM_CHANNEL = 7;
  ledcSetup(BLK_PWM_CHANNEL, 12000, 8);
  ledcAttachPin(32, BLK_PWM_CHANNEL);
  ledcWrite(BLK_PWM_CHANNEL, 128);
#elif defined ( ARDUINO_ESP32_DEV ) || defined ( ARDUINO_T )
  const int BLK_PWM_CHANNEL = 7;
  ledcSetup(BLK_PWM_CHANNEL, 12000, 8);
  ledcAttachPin(LGFX_Config::panel_bl, BLK_PWM_CHANNEL);
  ledcWrite(BLK_PWM_CHANNEL, 128);
#elif defined(ARDUINO_M5Stick_C)
  AXP192 axp;
  axp.begin();
  lcdver = true;
#else
  lgfx::TPin<LGFX_Config::panel_bl>::init();
  lgfx::TPin<LGFX_Config::panel_bl>::hi();
#endif
#if defined(ARDUINO_M5Stick_C) || defined ( ARDUINO_T )
  lcdver = true;
#endif

  tft_lgfx.invertDisplay(lcdver);
  tft_lgfx.setRotation(2);
  tft_lgfx.fillScreen(0xFF00);
  //tft_espi.setRotation(2);
  tft_lgfx.setSwapBytes(true);
  //tft_espi.setSwapBytes(true);

  //tft_ext.setRotation(2);
  //tft_ext.setSwapBytes(true);

  //tft.setTextWrap(true, true);


/*
//  uint32_t id = tft.getLcdID();
//  Serial.printf("\r\n RDDID: 0x%06X \r\n", id);
Serial.printf("PanelID: %08x \r\n", tft.readPanelID());        //M5StickC=f0897c
Serial.printf("PanelID DA: %08x \r\n", tft.readPanelIDSub(0xDA)); //M5StickC=7c
Serial.printf("PanelID DB: %08x \r\n", tft.readPanelIDSub(0xDB)); //M5StickC=89
Serial.printf("PanelID DC: %08x \r\n", tft.readPanelIDSub(0xDC)); //M5StickC=f0
Serial.printf("PanelID D3: %08x \r\n", tft.readPanelIDSub(0xD3));
///*/

}

static uint32_t sec, psec, count = 0;

void loop()
{
Serial.printf("count:%d  ", count);
Serial.printf("sec: %d \n", (int)(sec));

delay(500);

  tft_lgfx.setColorDepth(count & 8 ? 24 : 16);
  tft_lgfx.setSwapBytes(count & 4);
  tft_lgfx.setRotation(count & 3);
  //tft2.setColorDepth(count & 8 ? 24 : 16);
  //tft2.setColorDepth(18);
  //tft2.setSwapBytes(count & 4);
  //tft2.setRotation(count & 3);
  //tft_ext.setSwapBytes(count & 4);
  //tft_ext.setRotation(count & 3);
  //tft_espi.setSwapBytes(count & 4);
  //tft_espi.setRotation(count & 3);
Serial.printf("colorDepth:%d  swapBytes:%d  rotation:%d \r\n"
             , tft_lgfx.getColorDepth(), tft_lgfx.getSwapBytes(), tft_lgfx.getRotation());
  drawRects(tft_lgfx, 0);
  copyRect(tft_lgfx, 0);
  //blockReadWrite(tft_lgfx, 0);

  sprite.fillScreen(1);
//  sprite.fillRect(80, 0,80,80,2);
//  sprite.fillRect( 0,80,80,80,3);
//  sprite.fillRect(80,80,80,80,4);
  sprite.fillCircle(80, 80, 70, 0);
  sprite.drawCircle(80, 80, 70, 6);
  tft_lgfx.fillRect(tft_lgfx.width()-sprite.width()>>1, tft_lgfx.height()-sprite.height()>>1, 100,100,1);
  sprite.pushSprite(&tft_lgfx, tft_lgfx.width()-sprite.width()>>1, tft_lgfx.height()-sprite.height()>>1, 1);

  movingSprite(tft_lgfx);

  drawRects(sprite, 0);
  tft_lgfx.fillRect(tft_lgfx.width()-sprite.width()>>1, tft_lgfx.height()-sprite.height()>>1, 100,100,1);
  sprite.pushSprite(&tft_lgfx, tft_lgfx.width()-sprite.width()>>1, tft_lgfx.height()-sprite.height()>>1, 1);

  movingSprite(tft_lgfx);

  copyRect(sprite, 0);
  //blockReadWrite(sprite, 0);
  tft_lgfx.fillRect(tft_lgfx.width()-sprite.width()>>1, tft_lgfx.height()-sprite.height()>>1, 100,100,1);
  sprite.pushSprite(&tft_lgfx, tft_lgfx.width()-sprite.width()>>1, tft_lgfx.height()-sprite.height()>>1, 1);

  movingSprite(tft_lgfx);

//lgfx::swap565_t buf[240];
taskDISABLE_INTERRUPTS();
sec = esp_timer_get_time();

//tft_lgfx.readRect(0,0,240,1,buf);

sec = esp_timer_get_time() - sec;
taskENABLE_INTERRUPTS();

//  movingCopyRect(tft_lgfx);

  //movingSprite(tft2);
  //movingSprite(tft3);
  //movingSprite(tft4);
  //movingSprite(tft5);

  //drawRects(tft2, 0);
  //drawRects(tft3, 0);
  //drawRects(tft4, 0);
  //drawRects(tft5, 0);

  //blockReadWrite(tft2, 0);
  //blockReadWrite(tft3, 0);
  //blockReadWrite(tft4, 0);
  //blockReadWrite(tft5, 0);


  //drawRects(tft_espi, tft_espi.width()>>1);
  //blockReadWrite(tft_espi, tft_espi.width()>>1);

  //movingSprite(tft_ext);
  //drawRects(tft_ext, 0);
  //blockReadWrite(tft_ext, 0);

  count++;
return;

/*
  blockReadPixelWrite(tft);
for ( int i = 0; i < 20; i++) { tft.copyRect(i, i, tft.width() - (i<<1), tft.height()-i, i, i+1);}
//for ( int i = 0; i < 20; i++) { copy(tft, i, i, tft.width() - (i<<1), tft.height()-i, i, i+1);}
  tft.setTextColor(0xFFFF, 0x0000);
  tft.setTextFont(1);
  tft.setCursor(20,20);
  tft.setTextSize(1);
  tft.print("SIZE 1-1\r\n");
  tft.setTextSize(2,1);
  tft.print("SIZE 2-1\r\n");
  tft.setTextSize(1,2);
  tft.print("SIZE 1-2\r\n");
  tft.setTextSize(2,2);
  tft.print("SIZE 2-2\r\n");
  tft.setTextSize(2,3);
  tft.print("SIZE 2-3\r\n");
  tft.setTextSize(1);
//  tft.setFreeFont(&FreeMono24pt7b);
//  tft.print("StringTest \n");
  tft.setFreeFont(&FreeMono18pt7b);
  tft.print("StringTest \n");
  tft.setFreeFont(&FreeMono12pt7b);
  tft.print("StringTest \n");
  tft.setFreeFont(&FreeMono9pt7b);
  tft.print("StringTest \n");

//  tft.setFreeFont(&FreeSans24pt7b);
//  tft.print("StringTest \n");
  tft.setFreeFont(&FreeSans18pt7b);
  tft.print("StringTest \n");
  tft.setFreeFont(&FreeSans12pt7b);
  tft.print("StringTest \n");
  tft.setFreeFont(&FreeSans9pt7b);
  tft.print("StringTest \n");
delay (1000);
*/

//  M5.Lcd.setSwapBytes(count&1);
/*
//  tft.setColorDepth(tft.getColorDepth() == 16 ? 24 : 16);
delay(1000);
return;
  fillRectLoop(tft);
  tft.setColorDepth(tft.getColorDepth() == 16 ? 24 : 16);
  fillRectLoop(tft);
  tft.setColorDepth(tft.getColorDepth() == 16 ? 24 : 16);
  pushRectBuffer1(tft,0);
  tft.setColorDepth(tft.getColorDepth() == 16 ? 24 : 16);
//*/
/*
  pushRectBuffer2(tft,0); do { M5.update(); } while (!M5.BtnA.wasPressed());
  blockReadWrite(tft);  do { M5.update(); } while (!M5.BtnA.wasPressed());
  pixelReadWrite(tft);  do { M5.update(); } while (!M5.BtnA.wasPressed());
  blockReadPixelWrite(tft);  do { M5.update(); } while (!M5.BtnA.wasPressed());
  concentricFillCircle(tft  );  do { M5.update(); } while (!M5.BtnA.wasPressed());
  dualFillCircle(tft  );  do { M5.update(); } while (!M5.BtnA.wasPressed());
//  pixelCopy(tft);  do { M5.update(); } while (!M5.BtnA.wasPressed());

//  M5.Lcd.setRotation(count & 3);

  return;

  int32_t width  = tft.width();
  int32_t height = tft.height();
  uint16_t color = random(0, 65535);
 //M5.update();
  if (psec & 1) {
    auto c = tft.readPixel(  count % (width>>1), count / (width>>1));
    tft.drawPixel((width>>1)+count % (width>>1), count / (width>>1), c);

/*
    uint16_t buf[512];
    tft.readRect(  0, count % height, width>>1, 1, buf);
    tft.pushRect(width>>1, count % height, width>>1, 1, buf);

    for (int i = 0; i < 128; i++) 
      tft.drawPixel(128 + i, count, buf[i]);

for(int i = 0; i < 3; i++) {
Lcd.startWrite();
for(int j = 0; j < 1; j++) {
    Lcd.fillRect(random(0,width-20), random(0,height-20), 5, 20, color);

    Lcd.drawLine( random(0,width)
                , random(0,height)
                , random(0,width)
                , random(0,height)
                , color);
}
Lcd.endWrite();
}
    tft.drawChar( width - (count>>2 & 31) - (sec % 10)*3 
                , height - (count>>2 & 31) - (sec % 10)*3
                , '0' + (sec % 10), color|0x8410, color&0x7BEF, 1 + (sec % 10));
    Lcd.drawFastVLine(random(0,100), random(0,100), 30, 0x00FF);
    Lcd.fillRect(random(0,100), random(0,200), 5, 20, color);
    Lcd.fillRect(0, 0, 320, 240, color);
    tft.drawChar(            - 10 + (count & 31) , - 20 + (count & 31) , '0' + (sec % 10), color|0x8410, color&0x7BEF, 1 + (sec % 10));
    tft.drawChar(0,0,'0' + (count % 10), color , color , (count % 10));
    tft.drawPixel(random(100,200), random(0,100), color);
    tft.drawChar( (count>>2 & 31) + (psec % 10)*3 
                ,  (count>>2 & 31) + (psec % 10)*3
                , '0' + (psec/2 % 10), color|0x8410, color&0x7BEF, 15);  //  + (psec/2 % 30)
    tft.fillRect(random(100,200), random(0,200), 5, 20, color);
    tft.drawChar( 0 //(count>>2 & 31) // + (psec % 10)*3 
                 , 0 //(count>>2 & 31) // + (psec % 10)*3
                 , '0' + (psec/2 % 10), color|0x8410, color&0x7BEF, 10+(psec/2 % 10) );
    tft.drawChar( 20 //(count>>2 & 31) // + (psec % 10)*3 
                 , 20 //(count>>2 & 31) // + (psec % 10)*3
                 , '0' + (psec/2 % 10), color, color, 1+(psec/2 % 5));
    tft.printf("count : %d", count);
    tft.drawCircle(100, 100, 1 + (count%130), count << 8);
    tft.drawChar( (count>>2 & 31) // + (psec % 10)*3 
                 , (count>>2 & 31) // + (psec % 10)*3
                 , '0' + (psec/2 % 10), color, color, 1+(psec/2 % 5));
//               , '0' + (psec/2 % 10), color|0x8410, color&0x7BEF, 1+(psec/2 % 10) );
//    tft.setCursor(-40 + (count >> 2), -20 + sec & 0x0F);
    //tft.setTextFont(1);
    //tft.setTextSize(4 - ((psec>>1) % 4) );
    //M5.Lcd.setTextColor(color|0x8410, color&0x7BEF);
    tft.setFreeFont(&FreeSansBoldOblique9pt7b);
    tft.setTextSize(2);// - ((psec>>1) % 4) );
    tft.setCursor(40, 40);
    tft.setTextColor(color|0x8410, color&0x7BEF);
    tft.printf("StringTest:%d\r\n", count);
  } else {
    int x = (count % width)>>1;
    tft.drawPixel(x, random(         0, height  /3),  (((x<<2) + (sec << 1)) <<16) & 0xFC0000);
    tft.drawPixel(x, random(height  /3, height*2/3),  (((x<<1) + (sec << 3)) << 8) & 0x00FC00);
    tft.drawPixel(x, random(height*2/3, height    ),  ( (x   ) + (sec << 5))       & 0x0000FC);
    tft.fillRect(  x, random(         0, height  /3), 3, 3, ((x + (sec << 3)) <<11) & 0xF800);
    M5.Lcd.drawPixel( x, random(height  /3, height*2/3)    , ((x + (sec << 3)) << 5) & 0x07E0);
    tft.drawCircle(x, random(height*2/3, height    ), 3   ,  (x + (sec << 3))       & 0x001F);
for(int i = 0; i < 3; i++) {
M5.Lcd.startWrite();
for(int j = 0; j < 1; j++) {
//M5.Lcd.drawCircle(random(10,M5.width - 10), random(10,M5.height - 10), 3, color);
    Lcd.fillRect(random(0,width-20), random(0,height-20), 20, 5, color);
//    M5.Lcd.drawPixel(random(0, M5.width), random(0, M5.height), color);
}
M5.Lcd.endWrite();
}
    LcdI.drawPixel(random(0,width), random(0,height), color);
    LcdI.fillRect(random(0,100), random(0,200), 20, 5, color);
    M5.Lcd.drawPixel(random(0,100), random(100,200), color);
    M5.Lcd.drawPixel(random(100,200), random(0,100), color);
    M5.Lcd.drawFastHLine(random(0,100), random(0,100), 30, 0x00FF);
    M5.Lcd.fillRect(0, 0, 320, 240, color);
    M5.Lcd.drawCircle(random(0,100), random(0,100), 10, color);
    M5.Lcd.drawChar(30, 30, '8', 0xFFFF, 0xF000, 2);
    M5.Lcd.drawChar(0,0,'0' + (count % 10), color , color , (count % 10));
    LcdI.drawPixel(random(100,200), random(0,100), color);
    M5.Lcd.fillRect(random(100,200), random(0,200), 20, 5, color);
    M5.Lcd.drawChar(0,0,'0' + (count % 10), color , color , (count % 10));
    M5.Lcd.drawChar( (count>>2 & 31) + (psec % 10)*3 
                   , (count>>2 & 31) + (psec % 10)*3
                   , '0' + (psec/2 % 10), color|0x8410, color&0x7BEF, 15);  // 1 + (psec/2 % 30)
    M5.Lcd.fillRect(random(0,100), random(0,200), 20, 5, color);
    M5.Lcd.drawCircle(100, 100, 1 + (count%130), count << 8);
    M5.Lcd.drawChar( (count>>2 & 31) // + (psec % 10)*3 
                   , (count>>2 & 31) // + (psec % 10)*3
                   , '0' + (psec/2 % 10), color, color, 1+(psec/2 % 5));
//                 , '0' + (psec/2 % 10), color|0x8410, color&0x7BEF, 1+(psec/2 % 10) );
    //M5.Lcd.setTextFont(1);
    M5.Lcd.setFreeFont(&FreeSansBoldOblique9pt7b);
    M5.Lcd.setTextSize(2);// - ((psec>>1) % 4) );
    M5.Lcd.setTextColor(0xFF00, 0);
    //M5.Lcd.setTextColor(color|0x8410, color&0x7BEF);
    M5.Lcd.drawString("StringTest",40,80);
    M5.Lcd.setCursor(40, 80);
    M5.Lcd.printf("StringTest:%d\r\n", count);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("StringTest",40,80);

    M5.Lcd.drawString("StringTest",40,80);
    tft.drawPixel(128 + (count & 0x7F), 1+(count >> 7), c);
    uint16_t buf[512];
    M5.Lcd.readRect(  0, count << 2, 128, 4, buf);
    M5.Lcd.pushRect(140, count << 2, 128, 4, buf);
  }
  count++;
  sec = millis() / 1000;
  if (psec != sec) {
    Serial.printf("fps:%d , %d \r\n", count, psec & 1);
    psec = sec;
    count = 0;
    tft.setRotation((sec >> 1) & 3);
    M5.Lcd.setRotation((sec >> 1) & 3);
  }
*/
}
