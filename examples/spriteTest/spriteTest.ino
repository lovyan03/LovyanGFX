
#if defined(ARDUINO_M5Stick_C)
 #include <AXP192.h>
#endif

#include <LGFX_TFT_eSPI.hpp>
//#include <TFT_eSPI.h>
//#include <M5Stack.h>
#include <driver/ledc.h>

static TFT_eSPI tft;
static TFT_eSprite buffer_p1(&tft);
static TFT_eSprite buffer_p2(&tft);
static TFT_eSprite buffer_p4(&tft);
static TFT_eSprite buffer_p8(&tft);
static TFT_eSprite buffer_256(&tft);
static TFT_eSprite buffer_16b(&tft);
static TFT_eSprite buffer_18b(&tft);
static TFT_eSprite buffer_24b(&tft);
static TFT_eSprite item_p1;
static TFT_eSprite item_p2;
static TFT_eSprite item_p4;
static TFT_eSprite item_p8;
static TFT_eSprite item_256;
static TFT_eSprite item_16b;
static TFT_eSprite item_18b;
static TFT_eSprite item_24b;
static TFT_eSprite* buffers[] = { &buffer_p1, &buffer_p2, &buffer_p4, &buffer_p8, &buffer_256, &buffer_16b, &buffer_18b, &buffer_24b };
static TFT_eSprite* items[] = { &item_p1, &item_p2, &item_p4, &item_p8, &item_256, &item_16b, &item_18b, &item_24b };

auto transp = lgfx::color565(255,255,0);

void setup() {
  Serial.begin(115200);

#if defined(ARDUINO_M5Stick_C)
  AXP192 axp;
  axp.begin();
#elif defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE) // || defined ( ARDUINO_ESP32_DEV ) || defined ( ARDUINO_T )
 #ifdef _SD_H_
  SD.begin(4, SPI, 20000000);
 #endif

 #define GPIO_BL 32
#elif defined ( ARDUINO_T ) // T-Watch
 #define GPIO_BL 12
#elif defined ( ARDUINO_ESP32_DEV )
 #define GPIO_BL 5
#endif

#ifdef GPIO_BL
  lgfx::TPin<GPIO_BL>::init();
  lgfx::TPin<GPIO_BL>::hi();

  ledc_timer_config_t ledc_timer;
  {
    ledc_timer.duty_resolution = LEDC_TIMER_13_BIT; // resolution of PWM duty
    ledc_timer.freq_hz = 5000;                      // frequency of PWM signal
    ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;   // timer mode
    ledc_timer.timer_num = LEDC_TIMER_0;            // timer index
  };
  ledc_timer_config(&ledc_timer);

  ledc_channel_config_t ledc_channel;
  {
   ledc_channel.channel    = LEDC_CHANNEL_7;
   ledc_channel.intr_type  = LEDC_INTR_DISABLE;
   ledc_channel.duty       = 5000;
   ledc_channel.gpio_num   = GPIO_BL;
   ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
   ledc_channel.hpoint     = 0;
   ledc_channel.timer_sel  = LEDC_TIMER_0;
  };
  ledc_channel_config(&ledc_channel);
#endif

  //needle.setColorDepth(1);  needle.createSprite(240,320);
  //needle.setColorDepth(lgfx::palette_1bit);  needle.createSprite(240,320);
  //needle.setColorDepth(lgfx::palette_2bit);  needle.createSprite(240,320);
  //needle.setColorDepth(lgfx::palette_4bit);  needle.createSprite(240,320);
  //needle.setColorDepth(lgfx::palette_8bit);  needle.createSprite(240,320);
  //needle.setColorDepth( 8);  needle.createSprite(240,320);
  tft.init();
  tft.setRotation(1);
  tft.setPivot(tft.width()>>1, tft.height()>>1);
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
    items[i]->drawRect( 1,  1, items[i]->width()-2, items[i]->height()-2, 0xFFFF);
    items[i]->fillRect((items[i]->width()>>1)-1, (items[i]->height()>>1)-1,  3,  3, 0xFFFF);
    items[i]->drawPixel(items[i]->width()>>1, items[i]->height()>>1, 0);
  }

  tft.startWrite();
}

static uint32_t count = 0;
void loop(void)
{
  uint64_t timer = micros();
  count++;
  if ( count == 36000 ) count = 0;
  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {
      auto buf = buffers[y];
      auto bw = buf->width();
      auto bh = buf->height();
      if (y < 4) {
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
          }
        }
      }
//      buf->fillSprite(0);
      buf->drawRect(0, 0, bw, bh, 0x5555);
      buf->drawFastVLine(buf->getPivotX(), 0, bh, 0xFFFF);
      buf->drawFastHLine(0, buf->getPivotY(), bw, 0xFFFF);

//items[x]->setPivot(count & 31, count & 31);
//if ((count/20)&1) items[x]->pushRotateZoom(buf, -12 + (count&63), -12 + (count&63), (float)count*5, 1.0, 1.0);
//else              items[x]->pushRotateZoom(buf, -12 + (count&63), -12 + (count&63), (float)count*5, 1.0, 1.0, 0);

      switch ((count>>6)&3) {
      case 0:  items[x]->pushRotateZoom(buf, buf->getPivotX(), buf->getPivotY(), (float)count*5, 1.0, 1.0   ); break;
      case 1:  items[x]->pushRotateZoom(buf, buf->getPivotX(), buf->getPivotY(), (float)count*5, 1.0, 1.0, 0); break;
      case 2:  items[x]->pushSprite(buf, (count & 63) -29, (count & 63) -29   ); break;
      case 3:  items[x]->pushSprite(buf, (count & 63) -29, (count & 63) -29, 0); break;
      }
      //buf->scroll( 0, 1);
      //buf->scroll( 1, 0);
      //buf->scroll( 0,-2);
      //buf->scroll(-2, 0);
      //buf->scroll( 0, 1);
      //buf->scroll( 1, 0);
      buf->pushSprite(x * tft.width() / 8, y * tft.height() / 8);
    }
  }

  timer = micros() - timer;
  Serial.print(F("time:"));
  Serial.println((uint32_t)timer);
}

