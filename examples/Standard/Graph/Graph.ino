#include <LovyanGFX.hpp>

#include <vector>

#define LINE_COUNT 6

static LGFX lcd;

static std::vector<int> points[LINE_COUNT];
static int colors[] = { TFT_RED, TFT_GREEN, TFT_BLUE, TFT_CYAN, TFT_MAGENTA, TFT_YELLOW };
static int xoffset, yoffset, point_count;

int getBaseColor(int x, int y)
{
  return ((x^y)&3 || ((x-xoffset)&31 && y&31) ? TFT_BLACK : ((!y || x == xoffset) ? TFT_WHITE : TFT_DARKGREEN));
}

void setup(void)
{
  lcd.init();

  if (lcd.width() < lcd.height()) lcd.setRotation(lcd.getRotation() ^ 1);

  yoffset = lcd.height() >> 1;
  xoffset = lcd.width()  >> 1;
  point_count = lcd.width() + 1;

  for (int i = 0; i < LINE_COUNT; i++)
  {
    points[i].resize(point_count);
  }

  lcd.startWrite();
  lcd.setAddrWindow(0, 0, lcd.width(), lcd.height());
  for (int y = 0; y < lcd.height(); y++)
  {
    for (int x = 0; x < lcd.width(); x++)
    {
      lcd.writeColor(getBaseColor(x, y - yoffset), 1);
    }
  }
  lcd.endWrite();
}

void loop(void)
{
  static int prev_sec;
  static int fps;
  ++fps;
  int sec = millis() / 1000;
  if (prev_sec != sec)
  {
    prev_sec = sec;
    lcd.setCursor(0,0);
    lcd.printf("fps:%03d", fps);
    fps = 0;
  }

  static int count;

  for (int i = 0; i < LINE_COUNT; i++)
  {
    points[i][count % point_count] = (sinf((float)count / (10 + 30 * i))+sinf((float)count / (13 + 37 * i))) * (lcd.height() >> 2);
  }

  ++count;

  lcd.startWrite();
  int index1 = count % point_count;
  for (int x = 0; x < point_count-1; x++)
  {
    int index0 = index1;
    index1 = (index0 +1) % point_count;
    for (int i = 0; i < LINE_COUNT; i++)
    {
      int y = points[i][index0];
      if (y != points[i][index1])
      {
        lcd.writePixel(x, y + yoffset, getBaseColor(x, y));
      }
    }

    for (int i = 0; i < LINE_COUNT; i++)
    {
      int y = points[i][index1];
      lcd.writePixel(x, y + yoffset, colors[i]);
    }
  }
  lcd.endWrite();
}
