#include <thread>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>

void setup(void);
void loop(void);

static void loopThread(void)
{
  setup();
  for (;;)
  {
    std::this_thread::yield();
    loop();
  }
}

int main(int, char**)
{
  std::thread sub_thread(loopThread);
  for (;;)
  {
    std::this_thread::yield();
    lgfx::Panel_sdl::sdl_event_handler();
    SDL_Delay(5);
  }
}
