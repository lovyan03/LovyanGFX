#include <thread>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>

void setup(void);
void loop(void);

static int loopThread(void*)
{
  setup();
  for (;;)
  {
    loop();
    std::this_thread::yield();
  }
}

int main(int, char**)
{
  std::thread sub_thread(loopThread, nullptr);
  for (;;)
  {
    std::this_thread::yield();
    lgfx::Panel_sdl::sdl_event_handler();
  }
}
