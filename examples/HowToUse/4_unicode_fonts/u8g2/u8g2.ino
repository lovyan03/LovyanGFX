
#include <LovyanGFX.hpp>

#include "profont.h"
#include "adobex11font.h"

static LGFX lcd;

static const lgfx::U8g2font profont10( u8g2_font_profont10_tr );
static const lgfx::U8g2font profont11( u8g2_font_profont11_tr );
static const lgfx::U8g2font profont12( u8g2_font_profont12_tr );
static const lgfx::U8g2font profont15( u8g2_font_profont15_tr );
static const lgfx::U8g2font profont17( u8g2_font_profont17_tr );
static const lgfx::U8g2font profont22( u8g2_font_profont22_tr );
static const lgfx::U8g2font profont29( u8g2_font_profont29_tr );

static const lgfx::U8g2font courB08 ( u8g2_font_courB08_tr );
static const lgfx::U8g2font courB10 ( u8g2_font_courB10_tr );
static const lgfx::U8g2font courB12 ( u8g2_font_courB12_tr );
static const lgfx::U8g2font courB14 ( u8g2_font_courB14_tr );
static const lgfx::U8g2font courB18 ( u8g2_font_courB18_tr );
static const lgfx::U8g2font courB24 ( u8g2_font_courB24_tr );

static const lgfx::U8g2font courR08 ( u8g2_font_courR08_tr );
static const lgfx::U8g2font courR10 ( u8g2_font_courR10_tr );
static const lgfx::U8g2font courR12 ( u8g2_font_courR12_tr );
static const lgfx::U8g2font courR14 ( u8g2_font_courR14_tr );
static const lgfx::U8g2font courR18 ( u8g2_font_courR18_tr );
static const lgfx::U8g2font courR24 ( u8g2_font_courR24_tr );

static const lgfx::U8g2font helvB08 ( u8g2_font_helvB08_tr );
static const lgfx::U8g2font helvB10 ( u8g2_font_helvB10_tr );
static const lgfx::U8g2font helvB12 ( u8g2_font_helvB12_tr );
static const lgfx::U8g2font helvB14 ( u8g2_font_helvB14_tr );
static const lgfx::U8g2font helvB18 ( u8g2_font_helvB18_tr );
static const lgfx::U8g2font helvB24 ( u8g2_font_helvB24_tr );

static const lgfx::U8g2font helvR08 ( u8g2_font_helvR08_tr );
static const lgfx::U8g2font helvR10 ( u8g2_font_helvR10_tr );
static const lgfx::U8g2font helvR12 ( u8g2_font_helvR12_tr );
static const lgfx::U8g2font helvR14 ( u8g2_font_helvR14_tr );
static const lgfx::U8g2font helvR18 ( u8g2_font_helvR18_tr );
static const lgfx::U8g2font helvR24 ( u8g2_font_helvR24_tr );

static const lgfx::U8g2font ncenB08 ( u8g2_font_ncenB08_tr );
static const lgfx::U8g2font ncenB10 ( u8g2_font_ncenB10_tr );
static const lgfx::U8g2font ncenB12 ( u8g2_font_ncenB12_tr );
static const lgfx::U8g2font ncenB14 ( u8g2_font_ncenB14_tr );
static const lgfx::U8g2font ncenB18 ( u8g2_font_ncenB18_tr );
static const lgfx::U8g2font ncenB24 ( u8g2_font_ncenB24_tr );

static const lgfx::U8g2font ncenR08 ( u8g2_font_ncenR08_tr );
static const lgfx::U8g2font ncenR10 ( u8g2_font_ncenR10_tr );
static const lgfx::U8g2font ncenR12 ( u8g2_font_ncenR12_tr );
static const lgfx::U8g2font ncenR14 ( u8g2_font_ncenR14_tr );
static const lgfx::U8g2font ncenR18 ( u8g2_font_ncenR18_tr );
static const lgfx::U8g2font ncenR24 ( u8g2_font_ncenR24_tr );

static const lgfx::U8g2font timB08  ( u8g2_font_timB08_tr  );
static const lgfx::U8g2font timB10  ( u8g2_font_timB10_tr  );
static const lgfx::U8g2font timB12  ( u8g2_font_timB12_tr  );
static const lgfx::U8g2font timB14  ( u8g2_font_timB14_tr  );
static const lgfx::U8g2font timB18  ( u8g2_font_timB18_tr  );
static const lgfx::U8g2font timB24  ( u8g2_font_timB24_tr  );

static const lgfx::U8g2font timR08  ( u8g2_font_timR08_tr  );
static const lgfx::U8g2font timR10  ( u8g2_font_timR10_tr  );
static const lgfx::U8g2font timR12  ( u8g2_font_timR12_tr  );
static const lgfx::U8g2font timR14  ( u8g2_font_timR14_tr  );
static const lgfx::U8g2font timR18  ( u8g2_font_timR18_tr  );
static const lgfx::U8g2font timR24  ( u8g2_font_timR24_tr  );


void setup()
{
  lcd.init();
}

void loop()
{
  lcd.setTextColor(random(0x10000)| 0x8410, random(0x10000)&0x7BEF);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.setFont(&profont10); lcd.print("profont10\n");
  lcd.setFont(&profont11); lcd.print("profont11\n");
  lcd.setFont(&profont12); lcd.print("profont12\n");
  lcd.setFont(&profont15); lcd.print("profont15\n");
  lcd.setFont(&profont17); lcd.print("profont17\n");
  lcd.setFont(&profont22); lcd.print("profont22\n");
  lcd.setFont(&profont29); lcd.print("profont29\n");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.setFont(&courB08); lcd.print("courB08\n");
  lcd.setFont(&courB10); lcd.print("courB10\n");
  lcd.setFont(&courB12); lcd.print("courB12\n");
  lcd.setFont(&courB14); lcd.print("courB14\n");
  lcd.setFont(&courB18); lcd.print("courB18\n");
  lcd.setFont(&courB24); lcd.print("courB24\n");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.setFont(&courR08); lcd.print("courR08\n");
  lcd.setFont(&courR10); lcd.print("courR10\n");
  lcd.setFont(&courR12); lcd.print("courR12\n");
  lcd.setFont(&courR14); lcd.print("courR14\n");
  lcd.setFont(&courR18); lcd.print("courR18\n");
  lcd.setFont(&courR24); lcd.print("courR24\n");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.setFont(&helvB08); lcd.print("helvB08\n");
  lcd.setFont(&helvB10); lcd.print("helvB10\n");
  lcd.setFont(&helvB12); lcd.print("helvB12\n");
  lcd.setFont(&helvB14); lcd.print("helvB14\n");
  lcd.setFont(&helvB18); lcd.print("helvB18\n");
  lcd.setFont(&helvB24); lcd.print("helvB24\n");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.setFont(&helvR08); lcd.print("helvR08\n");
  lcd.setFont(&helvR10); lcd.print("helvR10\n");
  lcd.setFont(&helvR12); lcd.print("helvR12\n");
  lcd.setFont(&helvR14); lcd.print("helvR14\n");
  lcd.setFont(&helvR18); lcd.print("helvR18\n");
  lcd.setFont(&helvR24); lcd.print("helvR24\n");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.setFont(&ncenB08); lcd.print("ncenB08\n");
  lcd.setFont(&ncenB10); lcd.print("ncenB10\n");
  lcd.setFont(&ncenB12); lcd.print("ncenB12\n");
  lcd.setFont(&ncenB14); lcd.print("ncenB14\n");
  lcd.setFont(&ncenB18); lcd.print("ncenB18\n");
  lcd.setFont(&ncenB24); lcd.print("ncenB24\n");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.setFont(&ncenR08); lcd.print("ncenR08\n");
  lcd.setFont(&ncenR10); lcd.print("ncenR10\n");
  lcd.setFont(&ncenR12); lcd.print("ncenR12\n");
  lcd.setFont(&ncenR14); lcd.print("ncenR14\n");
  lcd.setFont(&ncenR18); lcd.print("ncenR18\n");
  lcd.setFont(&ncenR24); lcd.print("ncenR24\n");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.setFont(&timB08 ); lcd.print("timB08\n");
  lcd.setFont(&timB10 ); lcd.print("timB10\n");
  lcd.setFont(&timB12 ); lcd.print("timB12\n");
  lcd.setFont(&timB14 ); lcd.print("timB14\n");
  lcd.setFont(&timB18 ); lcd.print("timB18\n");
  lcd.setFont(&timB24 ); lcd.print("timB24\n");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.setFont(&timR08 ); lcd.print("timR08\n");
  lcd.setFont(&timR10 ); lcd.print("timR10\n");
  lcd.setFont(&timR12 ); lcd.print("timR12\n");
  lcd.setFont(&timR14 ); lcd.print("timR14\n");
  lcd.setFont(&timR18 ); lcd.print("timR18\n");
  lcd.setFont(&timR24 ); lcd.print("timR24\n");
  delay(2000);
}
