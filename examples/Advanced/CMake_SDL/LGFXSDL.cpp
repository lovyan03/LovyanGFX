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

static int taskThread(void*)
{
  for (;;)
  {
    lgfx::Panel_sdl::sdl_event_handler();
    std::this_thread::yield();
  }
}

int main(int, char**)
{
//*
//    setup();
//    SDL_CreateThread(loopThread, "loopThread", nullptr);
  std::thread sub_thread(loopThread, nullptr);
  for (;;)
  {
    std::this_thread::yield();
    lgfx::Panel_sdl::sdl_event_handler();
//    SDL_Delay(1);
  }
/*/
  setup();
//  std::thread sub_thread(taskThread);
  SDL_CreateThread(taskThread, "taskThread", nullptr);
  for (;;)
  {
    loop();
    SDL_Delay(1);
    std::this_thread::yield();
  }
//*/
}
