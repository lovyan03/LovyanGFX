#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>
#include <emscripten.h>

void setup(void);
void loop(void);

void loopThread(void *arg)
{
  loop();
  lgfx::Panel_sdl::sdl_event_handler();
}

int monitor_hor_res, monitor_ver_res;

int main(int argc, char **argv)
{
  setup();
  emscripten_set_main_loop_arg(loopThread, NULL, -1, true);
}
