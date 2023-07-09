#define LGFX_AUTODETECT
#include <LovyanGFX.h>
#include <LGFX_AUTODETECT.hpp>

#if defined ( SDL_h_ )
static LGFX lcd ( 320, 240, 2 );
#else
static LGFX lcd;
#endif

void setup(void)
{
  lcd.init();
}

void loop(void)
{
  lcd.fillCircle(rand()%lcd.width(), rand()%lcd.height(), 16, rand());
}



#if defined ( ESP_PLATFORM ) && !defined ( ARDUINO )
extern "C" {
int app_main(int, char**)
{
    setup();
    for (;;) {
      loop();
    }
    return 0;
}
}
#endif
