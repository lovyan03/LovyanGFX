
#if defined(ARDUINO_M5Stick_C)
 #include <AXP192.h>
#endif

#include <LovyanGFX.hpp>

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE) // M5Stack
  struct LGFX_Config {
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int spi_mosi = 23;
    static constexpr int spi_miso = 19;
    static constexpr int spi_sclk = 18;
    static constexpr int spi_cs   = 14;
    static constexpr int spi_dc   = 27;
    static constexpr int panel_rst = 33;
    static constexpr int panel_bl  = 32;
    static constexpr int freq_write = 40000000;
    static constexpr int freq_read  = 16000000;
    static constexpr int freq_fill  = 40000000;
    static constexpr bool spi_half_duplex = true;
    const lgfx::Panel_M5Stack panel;
  };
  static LGFX<LGFX_Config> tft;

#elif defined(ARDUINO_M5Stick_C) // M5Stick C
  struct LGFX_Config {
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int spi_mosi = 15;
    static constexpr int spi_miso = 14;
    static constexpr int spi_sclk = 13;
    static constexpr int spi_cs   =  5;
    static constexpr int spi_dc   = 23;
    static constexpr int panel_rst = 18;
    static constexpr int freq_write = 27000000;
    static constexpr int freq_read  = 16000000;
    static constexpr int freq_fill  = 27000000;
    static constexpr bool spi_half_duplex = true;
    const lgfx::Panel_ST7735_GREENTAB160x80 panel;
  };
  static LGFX<LGFX_Config> tft;

#elif defined ( ARDUINO_ESP32_DEV ) // ESP-WROVER-KIT
  struct LGFX_Config {
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int spi_mosi = 23;
    static constexpr int spi_miso = 25;
    static constexpr int spi_sclk = 19;
    static constexpr int spi_cs   = 22;
    static constexpr int spi_dc   = 21;
    static constexpr int panel_rst = 18;
    static constexpr int panel_bl  = 5;
    static constexpr int freq_write = 40000000;
    static constexpr int freq_read  = 20000000;
    static constexpr int freq_fill  = 40000000;
    static constexpr bool spi_half_duplex = false;
    const lgfx::Panel_ILI9341_240x320 panel;
  };
  static LGFX<LGFX_Config> tft;

#elif defined(ARDUINO_ODROID_ESP32) // ODROID-GO
  struct LGFX_Config {
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int spi_mosi = 23;
    static constexpr int spi_miso = 19;
    static constexpr int spi_sclk = 18;
    static constexpr int spi_cs   =  5;
    static constexpr int spi_dc   = 21;
    static constexpr int panel_rst = -1;
    static constexpr int panel_bl  = 14;
    static constexpr int freq_write = 40000000;
    static constexpr int freq_read  = 20000000;
    static constexpr int freq_fill  = 80000000;
    static constexpr bool spi_half_duplex = true;
    const lgfx::Panel_ILI9341_240x320 panel;
  };
  static LGFX<LGFX_Config> tft;

#elif defined(ARDUINO_T) // TTGO T-Watch
  struct LGFX_Config {
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int spi_mosi = 19;
    static constexpr int spi_miso = -1;
    static constexpr int spi_sclk = 18;
    static constexpr int spi_cs   =  5;
    static constexpr int spi_dc   = 27;
    static constexpr int panel_rst = -1;
    static constexpr int panel_bl  = 12;
    static constexpr int freq_write = 80000000;
    static constexpr int freq_read  = 16000000;
    static constexpr int freq_fill  = 80000000;
    static constexpr bool spi_half_duplex = true;
    const lgfx::Panel_ST7789_240x240 panel;
  };
  static LGFX<LGFX_Config> tft;

#elif !defined(ESP32) & defined(ARDUINO) // Arduino UNO
  struct LGFX_Config
  {
/*
    static constexpr int spi_mosi = 23;
    static constexpr int spi_miso = 19;
    static constexpr int spi_sclk = 18;
    static constexpr int spi_cs   = 16;
    static constexpr int spi_dc   = 17;
    static constexpr int freq_write = 40000000;
    static constexpr int freq_read  = 16000000;
    static constexpr int freq_fill  = 40000000;
    static constexpr bool spi_half_duplex = true;
    const lgfx::Panel_ST7789_240x320 panel;
*/
  };
  static LovyanGFX<lgfx::AvrSpi<LGFX_Config> > tft;

#endif

static LGFXSprite sprite;

void setup()
{
Serial.begin(115200);

#ifdef ARDUINO
Serial.print("Arduino\r\n");
#endif

sprite.createSprite(100,100);


/*
  auto _pspi_t = SPI.bus();
  Serial.printf("dev :%x \r\n", *(uint32_t*)_pspi_t);
  Serial.printf("spi0:%x \r\n", DR_REG_SPI0_BASE);
  Serial.printf("spi1:%x \r\n", DR_REG_SPI1_BASE);
  Serial.printf("spi2:%x \r\n", DR_REG_SPI2_BASE);
  Serial.printf("spi3:%x \r\n", DR_REG_SPI3_BASE);
*/
Serial.printf("LovyanGFX mosi:%d  miso:%d  sclk:%d  cs:%d  dc:%d  rst:%d \r\n", LGFX_Config::spi_mosi, LGFX_Config::spi_miso, LGFX_Config::spi_sclk, LGFX_Config::spi_cs, LGFX_Config::spi_dc, LGFX_Config::panel_rst);
#if defined(ARDUINO_M5Stick_C)
  AXP192 axp;
  axp.begin();
#endif
#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE) || defined ( ARDUINO_ESP32_DEV ) || defined ( ARDUINO_T )
  const int BLK_PWM_CHANNEL = 7;
  ledcSetup(BLK_PWM_CHANNEL, 12000, 8);
  ledcAttachPin(LGFX_Config::panel_bl, BLK_PWM_CHANNEL);
  ledcWrite(BLK_PWM_CHANNEL, 128);
  pinMode(LGFX_Config::panel_rst, INPUT);
  bool lcdver = digitalRead(LGFX_Config::panel_rst);
#else
  lgfx::TPin<LGFX_Config::panel_bl>::init();
  lgfx::TPin<LGFX_Config::panel_bl>::hi();
#endif
#if defined(ARDUINO_M5Stick_C) || defined ( ARDUINO_T )
  tft.invertDisplay(true);
#endif
  tft.init();
  tft.invertDisplay(lcdver);
  tft.setRotation(1);
  //tft.setTextWrap(true, true);
  tft.fillScreen(0xFF00);
/*
auto* dev = tft.getDevice();
tft.spi_begin();
dev->setSPIFrequency(80000000); Serial.printf("clock:%d \r\n", dev->getSPIFrequency());
dev->setSPIFrequency(60000000); Serial.printf("clock:%d \r\n", dev->getSPIFrequency());
dev->setSPIFrequency(40000000); Serial.printf("clock:%d \r\n", dev->getSPIFrequency());
dev->setSPIFrequency(30000000); Serial.printf("clock:%d \r\n", dev->getSPIFrequency());
dev->setSPIFrequency(20000000); Serial.printf("clock:%d \r\n", dev->getSPIFrequency());
dev->setSPIFrequency(10000000); Serial.printf("clock:%d \r\n", dev->getSPIFrequency());
dev->setSPIFrequency( 9000000); Serial.printf("clock:%d \r\n", dev->getSPIFrequency());
dev->setSPIFrequency( 8000000); Serial.printf("clock:%d \r\n", dev->getSPIFrequency());
dev->setSPIFrequency( 7000000); Serial.printf("clock:%d \r\n", dev->getSPIFrequency());
dev->setSPIFrequency( 6000000); Serial.printf("clock:%d \r\n", dev->getSPIFrequency());
dev->setSPIFrequency( 5000000); Serial.printf("clock:%d \r\n", dev->getSPIFrequency());
dev->setSPIFrequency( 4000000); Serial.printf("clock:%d \r\n", dev->getSPIFrequency());
dev->setSPIFrequency( 3000000); Serial.printf("clock:%d \r\n", dev->getSPIFrequency());
dev->setSPIFrequency( 2000000); Serial.printf("clock:%d \r\n", dev->getSPIFrequency());
dev->setSPIFrequency( 1000000); Serial.printf("clock:%d \r\n", dev->getSPIFrequency());
dev->setSPIFrequency(  900000); Serial.printf("clock:%d \r\n", dev->getSPIFrequency());
dev->setSPIFrequency(  800000); Serial.printf("clock:%d \r\n", dev->getSPIFrequency());
dev->setSPIFrequency(  700000); Serial.printf("clock:%d \r\n", dev->getSPIFrequency());
dev->setSPIFrequency(  600000); Serial.printf("clock:%d \r\n", dev->getSPIFrequency());
dev->setSPIFrequency(  500000); Serial.printf("clock:%d \r\n", dev->getSPIFrequency());
tft.spi_end();
///*/
//  uint32_t id = tft.getLcdID();
//  Serial.printf("\r\n RDDID: 0x%06X \r\n", id);

Serial.printf("PanelID: %08x \r\n", tft.readPanelID());
//M5StickC=f0897c

Serial.printf("PanelID DA: %08x \r\n", tft.readPanelIDSub(0xDA));
//M5StickC=7c

Serial.printf("PanelID DB: %08x \r\n", tft.readPanelIDSub(0xDB));
//M5StickC=89

Serial.printf("PanelID DC: %08x \r\n", tft.readPanelIDSub(0xDC));
//M5StickC=f0

Serial.printf("PanelID D3: %08x \r\n", tft.readPanelIDSub(0xD3));

}

volatile char ctmp;
uint32_t sec, psec, count;
void loop()
{
Serial.printf("colorDepth:%d  swapBytes:%d  rotation:%d \r\n"
             , tft.getColorDepth(), tft.getSwapBytes(), tft.getRotation());

sprite.fillRect( 0, 0,50,50,sprite.color(0,0,255));
sprite.fillRect(50, 0,50,50,sprite.color(0,255,0));
sprite.fillRect( 0,50,50,50,sprite.color(255,0,0));
sprite.fillRect(50,50,50,50,sprite.color(255,255,255));
sprite.fillCircle(50, 50, 40, 0);

uint8_t* spbuf = sprite.getDevice()->buffer();
for (int i = 0; i < 1000; i++) {
  tft.pushImage(random(0,220), random(0,140), 100, 100, spbuf);
}
  tft.setColorDepth(count & 8 ? 24 : 16);
  tft.setSwapBytes(count & 4);
  tft.setRotation(count & 3);
  pushRectBuffer2(tft,0);
  pushRectBuffer1(tft,0);
  blockReadWrite(tft);
  blockReadPixelWrite(tft);
//  tft.init();

//for ( int i = 0; i < 20; i++) { copy(tft, 0, 0, tft.width()>>1, tft.height()-8, 0, 8);}
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
  count++;
return;
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
//*/
  }
}

template<typename T>
void blockReadWrite(T& Lcd)
{
  int32_t width  = Lcd.width();
  int32_t height = Lcd.height();
  Lcd.startWrite();
  Lcd.fillRect(0, 0, width / 2, height, random(0, 0xFFFFFF));
  for (int count = 0; count < 80; count++) {
    int x = (count % (width>>1));
    Lcd.fillRect(x,          0, (width >> 1)-(x<<1), height/3-count, tft.getColorFromRGB(      (x<<4) + (sec << 1), 0, 0));
    Lcd.drawRect(x, height  /3, (width >> 1)-(x<<1), height/3-count, tft.getColorFromRGB(0,    (x<<4) + (sec << 3), 0   ));
    Lcd.fillRect(x, height*2/3, (width >> 1)-(x<<1), height/3-count, tft.getColorFromRGB(0, 0, (x<<4) + (sec << 5)      ));
  }
  uint8_t buf[320*3];
  for (int count = 0; count < height>>1; count++) {
    Lcd.readRect(         0, count<<1, (width-count) >> 1, 2, (uint16_t*)buf);
    Lcd.pushRect((width>>1), count<<1, (width-count) >> 1, 2, (uint16_t*)buf);
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
void pushRectBuffer1(T& Lcd, int x)
{
  int32_t width  = Lcd.width();
  int32_t height = Lcd.height();

  uint16_t buf[320*3];
  for (int i = 0; i < 320*3; i++ ) {
    buf[i] = Lcd.color565(0xFF-(i<<3), 0x80-(i<<6), i<<4);
  }

  Lcd.startWrite();
  for (int count = 0; count < 240; count++) {
    Lcd.pushRect(x, count, 1+(count>>2),1, buf);
  }
  Lcd.endWrite();
}

template<typename T>
void pushRectBuffer2(T& Lcd, int x)
{
  int32_t width  = Lcd.width();
  int32_t height = Lcd.height();

  uint16_t buf[320*3];
  for (int i = 0; i < 320*3; i++ ) {
    buf[i] = Lcd.color565(i, i, 0xFF);
  }

  Lcd.startWrite();
  for (int count = 239; count; count--) {
    Lcd.pushRect(x, count, 1+(count>>2),1, &buf[count]);
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
template<typename T>
void concentricFillCircle(T& Lcd)
{
  int32_t width  = Lcd.width();
  int32_t height = Lcd.height();
  Lcd.startWrite();
  Lcd.fillRect(0, 0, width, height, random(0, 0xFFFFFF));
  for (int count = 0; count < 200; count++) {
    Lcd.drawCircle(width>>1, height>>1, 200 - count, tft.getColorFromRGB((count&1 ? 0xFF:0), (count&2 ? 0xFF:0), (count&4 ? 0xFF:0)));
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
    Lcd.fillCircle (width  >>2, height>>1, 100 - count, tft.getColorFromRGB((count&1 ? 0xFF:0), (count&2 ? 0xFF:0), (count&4 ? 0xFF:0)));
    Lcd.fillCircle (width*3>>2, height>>1, 100 - count, tft.getColorFromRGB((count&1 ? 0xFF:0), (count&2 ? 0xFF:0), (count&4 ? 0xFF:0)));
  }
  Lcd.endWrite();
}
