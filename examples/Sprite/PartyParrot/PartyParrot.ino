
// The parrot image data is from this site.
// https://cultofthepartyparrot.com/

#include "parrot00.h"
#include "parrot01.h"
#include "parrot02.h"
#include "parrot03.h"
#include "parrot04.h"
#include "parrot05.h"
#include "parrot06.h"
#include "parrot07.h"
#include "parrot08.h"
#include "parrot09.h"

#include <LovyanGFX.hpp>

static LGFX lcd;
static LGFX_Sprite sprite[10];

static std::uint32_t count = 0;
static float zoom = 0;

void setup()
{
  lcd.init();
  lcd.setRotation(0);
  if (lcd.width() < lcd.height()) { lcd.setRotation(1); }

  zoom = std::min((float)lcd.width() / 128, (float)lcd.height() / 96);

  lcd.setPivot(lcd.width() >> 1, lcd.height() >> 1);
  lcd.fillScreen(0xFFFFFFU);

  sprite[0].createFromBmp(parrot00, ~0u);
  sprite[1].createFromBmp(parrot01, ~0u);
  sprite[2].createFromBmp(parrot02, ~0u);
  sprite[3].createFromBmp(parrot03, ~0u);
  sprite[4].createFromBmp(parrot04, ~0u);
  sprite[5].createFromBmp(parrot05, ~0u);
  sprite[6].createFromBmp(parrot06, ~0u);
  sprite[7].createFromBmp(parrot07, ~0u);
  sprite[8].createFromBmp(parrot08, ~0u);
  sprite[9].createFromBmp(parrot09, ~0u);
}

void loop() {
  if (++count == 10) count = 0;
  sprite[count].pushRotateZoom(&lcd, lcd.width() >> 1, lcd.height() >> 1, 0, zoom, zoom);
}
