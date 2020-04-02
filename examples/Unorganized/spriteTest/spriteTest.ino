
#if defined(ARDUINO_M5Stick_C)
 #include <AXP192.h>
#endif

//#include <TFT_eSPI.h>
//#include <M5Stack.h>
#include <LovyanGFX.hpp>

static LGFX tft;
static LGFX_Sprite buffer_p1(&tft);
static LGFX_Sprite buffer_p2(&tft);
static LGFX_Sprite buffer_p4(&tft);
static LGFX_Sprite buffer_p8(&tft);
static LGFX_Sprite buffer_256(&tft);
static LGFX_Sprite buffer_16b(&tft);
static LGFX_Sprite buffer_18b(&tft);
static LGFX_Sprite buffer_24b(&tft);
static LGFX_Sprite item_p1;
static LGFX_Sprite item_p2;
static LGFX_Sprite item_p4;
static LGFX_Sprite item_p8;
static LGFX_Sprite item_256;
static LGFX_Sprite item_16b;
static LGFX_Sprite item_18b;
static LGFX_Sprite item_24b;
static LGFX_Sprite* buffers[] = { &buffer_p1, &buffer_p2, &buffer_p4, &buffer_p8, &buffer_256, &buffer_16b, &buffer_18b, &buffer_24b };
static LGFX_Sprite* items[] = { &item_p1, &item_p2, &item_p4, &item_p8, &item_256, &item_16b, &item_18b, &item_24b };

/*
auto transp = lgfx::color565(255,255,0);
struct panel_config {
  static constexpr int freq_write = 40000000;
  static constexpr int freq_read  = 20000000;
  static constexpr int freq_fill  = 40000000;
  static constexpr int spi_mode   = 0;
  static constexpr int spi_dc     = 5;
  static constexpr int spi_cs     = -1;
  static constexpr int gpio_rst   = 33;
  static constexpr int gpio_bl    = -1;
  static constexpr bool spi_3wire = true;
  static constexpr bool invert    = true;
  static constexpr int panel_height = 240;
};
static lgfx::Panel_ST7789<panel_config> panel_st7789;
*/
void setup(void)
{
  Serial.begin(115200);

//  tft.setPanel(panel_st7789);

  tft.init();

  //tft.setPanel<lgfx::Panel_ST7789<panel_config> >();


#if defined(ARDUINO_M5Stick_C)
  AXP192 axp;
  axp.begin();
#elif defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE) // || defined ( ARDUINO_ESP32_DEV ) || defined ( ARDUINO_T )
 #ifdef _SD_H_
  SD.begin(4, SPI, 20000000);
 #endif
#endif


  //needle.setColorDepth(1);  needle.createSprite(240,320);
  //needle.setColorDepth(lgfx::palette_1bit);  needle.createSprite(240,320);
  //needle.setColorDepth(lgfx::palette_2bit);  needle.createSprite(240,320);
  //needle.setColorDepth(lgfx::palette_4bit);  needle.createSprite(240,320);
  //needle.setColorDepth(lgfx::palette_8bit);  needle.createSprite(240,320);
  //needle.setColorDepth( 8);  needle.createSprite(240,320);

  tft.setRotation(1);
  tft.setPivot(tft.width()>>1, tft.height()>>1);
  tft.setColorDepth(16);
  tft.fillScreen(0x8710);

  buffer_p1 .setColorDepth(lgfx::palette_1bit);
  buffer_p2 .setColorDepth(lgfx::palette_2bit);
  buffer_p4 .setColorDepth(lgfx::palette_4bit);
  buffer_p8 .setColorDepth(lgfx::rgb332_1Byte);
  buffer_256.setColorDepth(lgfx::rgb332_1Byte);
  buffer_16b.setColorDepth(lgfx::rgb565_2Byte);
  buffer_18b.setColorDepth(lgfx::rgb666_3Byte);
  buffer_24b.setColorDepth(lgfx::rgb888_3Byte);
  item_p1 .setColorDepth(lgfx::palette_1bit);
  item_p2 .setColorDepth(lgfx::palette_2bit);
  item_p4 .setColorDepth(lgfx::palette_4bit);
  item_p8 .setColorDepth(lgfx::rgb332_1Byte);
  item_256.setColorDepth(lgfx::rgb332_1Byte);
  item_16b.setColorDepth(lgfx::rgb565_2Byte);
  item_18b.setColorDepth(lgfx::rgb666_3Byte);
  item_24b.setColorDepth(lgfx::rgb888_3Byte);

  for (int i = 0 ; i < 8; i++) {
    buffers[i]->createSprite(tft.width() >> 3, tft.height() >> 3);
    items[i]  ->createSprite(tft.width() >> 4, tft.height() >> 4);
    //items[i]  ->createSprite(25, 25);
  }
  for (int i = 0 ; i < 4; i++) {
    buffers[i]->createPalette();
    items[i]  ->createPalette();
  }
  for (int i = 0 ; i < 8; i++) {
    items[i]->drawLine( 0,  0, items[i]->width()-1, items[i]->height()-1,  0xCC33);
    items[i]->drawLine( 0, items[i]->height()-1, items[i]->width()-1,  0,  0x33CC);
    items[i]->drawFastVLine( items[i]->width()>>1, 0, items[i]->height(), 0x55AA);
    items[i]->drawFastHLine( 0, items[i]->height()>>1, items[i]->width(), 0xAA55);
    items[i]->drawRect( 0,  0, items[i]->width()  , items[i]->height()  ,      0);
    items[i]->drawRect( 1,  1, items[i]->width()-2, items[i]->height()-2, 0xFFFF);
    items[i]->fillRect((items[i]->width()>>1)-1, (items[i]->height()>>1)-1,  3,  3, 0xFFFF);
    items[i]->drawPixel(items[i]->width()>>1, items[i]->height()>>1, 0);
  }

//  tft.startWrite();
}

static uint32_t count = 28800;
void loop(void)
{
  count++;
  if ( count == 288000 ) count = 0;

  for (int j = 0; j < 4; j++) {
    for (int i = 0; i < (1<<(1<<(j))); i++) {
      buffers[j]->setPaletteColor(i, ~count, count>>(i >> j), count>>i);
      items[j]->setPaletteColor(i, count>>i, count>>(i >> j), ~count);
    }
  }

  uint64_t timer = micros();
  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {
      auto buf = buffers[y];
      auto bw = buf->width();
      auto bh = buf->height();
      if (y < 4) { // palette mode
        for (int bx = 0; bx < bw; bx++) {
          buf->drawFastVLine( bx
                            , 0
                            , bh
                            , bx * (1 << (1 << y)) / bw);
        }
      } else {
        for (int j = 0; j < 7; j++) {
          for (int bx = 0; bx < bw; bx++) {
            buf->drawFastVLine( bx
                              , 4 * j  +1
                              , 4
                              , lgfx::color888((j&1)?0:(0xFF * bx / bw)
                                              ,(j&2)?0:(0xFF * bx / bw)
                                              ,(j&4)?0:(0xFF * bx / bw)));
//                              , lgfx::color888((x*32 + 0x1F * bx / bw)
//                                              ,(x*32 + 0x1F * bx / bw)
//                                              ,(x*32 + 0x1F * bx / bw)));
          }
        }
      }
//      buf->fillSprite(0);
      auto bpx = buf->getPivotX();
      auto bpy = buf->getPivotY();

      buf->drawRect(0, 0, bw, bh, 0x5555);
      buf->drawFastVLine(bpx, 0, bh, 0xFFFF);
      buf->drawFastHLine(0, bpy, bw, 0xFFFF);
/*
items[x]->pushRotateZoom(buf, bpx, bpy, 45/16, 1.0, 1.0   );
/*/
//items[x]->setPivot(count & 31, count & 31);
//if ((count/20)&1) items[x]->pushRotateZoom(buf, -12 + (count&63), -12 + (count&63), (float)count*5, 1.0, 1.0);
//else              items[x]->pushRotateZoom(buf, -12 + (count&63), -12 + (count&63), (float)count*5, 1.0, 1.0, 0);
      switch ((count>>6)&3) {
      case 0:  items[x]->pushRotateZoom(buf, bpx, bpy, (float)count*45/16, 1.0, 1.0   ); break;
      case 2:  items[x]->pushRotateZoom(buf, bpx, bpy, (float)count*45/16, 1.0, 1.0, 0); break;
      case 1:  items[x]->pushSprite(buf, bpx-(items[x]->getPivotX())+((count+32) & 63)-31, bpy-(items[x]->getPivotY())+((count+32) & 63)-31   ); break;
      case 3:  items[x]->pushSprite(buf, bpx-(items[x]->getPivotX())+((count+32) & 63)-31, bpy-(items[x]->getPivotY())+((count+32) & 63)-31, 0); break;
      }
/*
      buf->scroll( 0, 1);
      buf->scroll( 1, 0);
      buf->scroll( 0,-2);
      buf->scroll(-2, 0);
      buf->scroll( 0, 1);
      buf->scroll( 1, 0);
//*/
      buf->pushSprite(x * tft.width() / 8, y * tft.height() / 8);
    }
  }
  timer = micros() - timer;
  Serial.printf("time : %d\r\n", (uint32_t)timer);
/*
  timer = micros();
  tft.scroll( 0, 1);
  tft.scroll( 1, 0);
  tft.scroll( 0,-2);
  tft.scroll(-2, 0);
  tft.scroll( 1, 1);
  timer = micros() - timer;
  Serial.printf("scroll: %d\r\n", (uint32_t)timer);
//*/
}

