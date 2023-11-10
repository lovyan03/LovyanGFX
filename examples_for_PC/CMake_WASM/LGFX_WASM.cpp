#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>
#include <emscripten.h>

void setup(void);
void loop(void);

void loopThread(void)
{
  lgfx::Panel_sdl::loop();
  loop();
}

int main(int argc, char **argv)
{
  setup();
  lgfx::Panel_sdl::setup();
  emscripten_set_main_loop(loopThread, -1, true);
}
