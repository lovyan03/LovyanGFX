
#include <LovyanGFX.hpp>

static LGFX lcd;

#include "emojis_packed.h"

struct EmojiFlashEntry
{
  uint32_t code = 0;
  uint8_t* data = nullptr;
  uint32_t len = 0;
  int16_t png_w = 0;
  int16_t png_h = 0; // 0 = file missing / invalid
};

const EmojiFlashEntry* emoji_flash_lookup(uint32_t code)
{
  static EmojiFlashEntry entry;

  entry = EmojiFlashEntry();
  entry.code = code;

  size_t emojis_count = sizeof(emoji_path_by_code)/sizeof(emoji_path_by_code_t);

  for(int i=0;i<emojis_count;i++)
  {
    auto emoji = emoji_path_by_code[i];
    if( emoji.code ==code )
    {
      if( !emoji.data || emoji.data_len == 0 )
        break;

      entry.data = (uint8_t*)emoji.data;
      entry.len  = emoji.data_len;
      entry.png_w = (int16_t)((entry.data[18] << 8) | entry.data[19]);
      entry.png_h = (int16_t)((entry.data[22] << 8) | entry.data[23]);
    }
  }
  return &entry;
}


int32_t emoji_draw_callback(lgfx::LGFXBase* gfx, int32_t x, int32_t y, uint32_t code, int32_t font_height)
{
  auto* e = emoji_flash_lookup(code);
  if (!e->data || e->png_h <= 0)
    return 0;

  float scale = (float)font_height / (float)e->png_h;

  auto style = gfx->getTextStyle();
  if (style.fore_rgb888 != style.back_rgb888)
  {
    gfx->fillRect(x, y, font_height, font_height, style.back_rgb888);
  }

  if (gfx->drawPng(e->data, e->len, x, y, font_height, font_height, 0, 0, scale))
  {
    return font_height;
  }

  return 0;
}



void setup()
{
  Serial.begin();
  Serial.println("Hello Emojis example");

  lcd.init();

  lcd.setEmojiCallback( emoji_draw_callback );

  lcd.setTextColor(TFT_BLUE, TFT_BLACK);
  lcd.setTextSize(4.5);
  lcd.println();

  lcd.println("abc😁def");

  lcd.printf("%s\n", "😋");

  lcd.drawString("🖖", 100, 100);

  delay(5000);

  lcd.setTextSize(2);
  lcd.setCursor(0,0);
}


void loop()
{
  lcd.setTextColor(random(0x10000)| 0x8410, random(0x10000)&0x7BEF);

  size_t emojis_count = sizeof(emoji_path_by_code)/sizeof(emoji_path_by_code_t);
  for(int i=0;i<emojis_count;i++)
  {
    if(lcd.getCursorY()+lcd.fontHeight()/2>lcd.height()) {
      lcd.setCursor(0,0);
    }
    lcd.print(emoji_path_by_code[i].emoji);
  }
}
