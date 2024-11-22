#include <pico/stdlib.h>

#include <LovyanGFX.hpp>
#include "LGFX_config.hpp"

#include "../../examples/Sprite/PartyParrot/partyparrot.h"

static LGFX lcd;
static LGFX_Sprite sprite[10];

void setup(void);
void loop(void);

static uint32_t count = 0;
static float zoom = 0;

extern const unsigned char parrot00[];
extern const unsigned char parrot01[];
extern const unsigned char parrot02[];
extern const unsigned char parrot03[];
extern const unsigned char parrot04[];
extern const unsigned char parrot05[];
extern const unsigned char parrot06[];
extern const unsigned char parrot07[];
extern const unsigned char parrot08[];
extern const unsigned char parrot09[];

int main()
{
	stdio_init_all();

	setup();

	while (1) {
		loop();
	}

	return 0;
}

void setup(void)
{
  lcd.init();
  lcd.setRotation(0);
  if (lcd.width() < lcd.height()) { lcd.setRotation(lcd.getRotation() ^ 1); }

  zoom = (float)lcd.width() / 128;
  float ztmp = (float)lcd.height() / 96;
  if (zoom > ztmp) { zoom = ztmp; }

  lcd.setPivot(lcd.width() >> 1, lcd.height() >> 1);
  lcd.fillScreen(0xFFFFFFU);

  sprite[0].createFromBmp(parrot00);
  sprite[1].createFromBmp(parrot01);
  sprite[2].createFromBmp(parrot02);
  sprite[3].createFromBmp(parrot03);
  sprite[4].createFromBmp(parrot04);
  sprite[5].createFromBmp(parrot05);
  sprite[6].createFromBmp(parrot06);
  sprite[7].createFromBmp(parrot07);
  sprite[8].createFromBmp(parrot08);
  sprite[9].createFromBmp(parrot09);
}

void loop(void)
{
  if (++count == 10) count = 0;
  sprite[count].pushRotateZoom(&lcd, lcd.width() >> 1, lcd.height() >> 1, 0, zoom, zoom);
}
