
#include <stdio.h>

#include <LovyanGFX.hpp>

static LGFX lcd;

static void tileRect(int x, int y, int w, int h, int tile, uint16_t color)
{
  if (tile == 0) return;
  int px = 0;
  int py = 0;
  for (int i = 0;i < h; i++) {
    py = y + i;
    for (int j = 0;j < w; j++) {
      px = x + j;
      switch (tile >> 1) {
      case 0: if ((px+py)&4)  { continue; }
      case 1: if (px&2)       { continue; }
      case 2: if ((px+py)&2)  { continue; }
      case 3: if (px&1)       { continue; }
      }
      if ( (px+py) & 1 ) { lcd.drawPixel(px, py, color); }
    }
  }
}

static void drawTile(int y, int h, uint16_t r, uint16_t g, uint16_t b)
{
  uint16_t color = lcd.color565(r,g,b);
  int step = 10;
  int x1 = lcd.width() / step;
  int x2 = (step-1) * lcd.width() / step;
  lcd.fillRect(0, y, x2, h, 0);
  lcd.fillRect(x2, y, lcd.width() - x2, h, color);
  for (uint32_t i = 1; i < step; i++) {
    x2 = i * lcd.width() / step;
    tileRect(x1, y, x2 - x1, h, i, color);
    x1 = x2;
  }
}

static void drawStep(int y, int h, int step, uint32_t r, uint32_t g, uint32_t b)
{
  int x1 = 0;
  int x2 = 0;
  for (uint32_t i = 0; i < step; i++) {
    x2 = (i+1) * lcd.width() / step;
    uint16_t color = i == 0 ? 0
                   : lcd.color565(r ? r*((i+1)>>1)/(step>>1) : 0
                                 ,g ? g*((i+1)>>1)/(step>>1) : 0
                                 ,b ? b*((i+1)>>1)/(step>>1) : 0);
    lcd.fillRect( x1, y, x2 - x1, h
                , color
                );
    x1 = x2;
  }
}

static void drawGrad(int y, int h, uint16_t r, uint16_t g, uint16_t b)
{
  int x1 = 0;
  int x2 = 0;
  for (int i = 0; i <= 256; i++) {
    x2 = i * lcd.width() / 256;
    if (x1 < x2) {
      lcd.fillRect(x1, y, x2 - x1, h, lcd.color565(r*i>>8, g*i>>8, b*i>>8));
      x1 = x2;
    }
  }
}

static void drawTestPattern(void)
{
  int w = lcd.width() >> 5;
  int h = (lcd.height() >> 5)+1;
  int minwh = std::min(w,h);

  drawGrad(h* 0, h,    255,   0,   0);
  drawStep(h* 1, h,32 ,255,   0,   0);
  drawStep(h* 2, h,16 ,255,   0,   0);
  drawStep(h* 3, h, 8 ,255,   0,   0);
  drawTile(h* 4, h,    255,   0,   0);
  drawGrad(h* 5, h,      0, 255,   0);
  drawStep(h* 6, h,32 ,  0, 255,   0);
  drawStep(h* 7, h,16 ,  0, 255,   0);
  drawStep(h* 8, h, 8 ,  0, 255,   0);
  drawTile(h* 9, h,      0, 255,   0);
  drawGrad(h*10, h,      0,   0, 255);
  drawStep(h*11, h,32 ,  0,   0, 255);
  drawStep(h*12, h,16 ,  0,   0, 255);
  drawStep(h*13, h, 8 ,  0,   0, 255);
  drawTile(h*14, h,      0,   0, 255);
  drawGrad(h*15, h,    255, 255, 255);
  drawStep(h*16, h,32 ,255, 255, 255);
  drawStep(h*17, h,16 ,255, 255, 255);
  drawStep(h*18, h, 8 ,255, 255, 255);
  drawTile(h*19, h,    255, 255, 255);

  for (int i = 0; i < minwh*6; i++) {
    lcd.drawRect(          i, h*20+i, w*8-(i<<1), h*12-(i<<1),             lcd.color565(255-(i<<4),0,0));
    lcd.drawRoundRect(w*8+i, h*20+i, w*8-(i<<1), h*12-(i<<1),(h*6-i)>>1, lcd.color565(0,255-(i<<4),0));
    lcd.drawCircle(   w*20 , h*26, i, lcd.color565(0,0,255-(i<<4)));
    lcd.drawLine  ( w*24, h*20 + (i<<1), lcd.width()-1, h*32-1-(i<<1), lcd.color565(i<<4,i<<4,255-(i<<4)));
    lcd.drawLine  ( lcd.width()-1-(i<<1), h*20, w*24+(i<<1), h*32-1, lcd.color565(255-(i<<4), i<<4,i<<4));
  }
}

void setup(void)
{
  lcd.init();

  lcd.fillScreen(0x00FF);
}

void loop()
{
  static int count = 0;

  lcd.startWrite();

  lcd.fillScreen(0);
  lcd.setColorDepth((count & 1) ? 24 : 16);
  lcd.setRotation((count>>1) & 7);
  ++count;

  drawTestPattern();
  lcd.setCursor(0, 10);
  lcd.printf("color %d\nrotation %d", lcd.getColorDepth(), lcd.getRotation());
  delay(500);

  int16_t ie = std::min(lcd.width() , lcd.height()) - 2;
  for (int i = 0; i < ie; ++i) {
    lcd.copyRect(i, i+1, lcd.width() - (i<<1), lcd.height() - i - 1, i, i);
  }
  delay(500);
  lcd.endWrite();

  return;
}

