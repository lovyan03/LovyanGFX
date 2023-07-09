
#include <lgfx/v1/platforms/sdl/Panel_sdl.hpp>
#if defined ( SDL_h_ )
#include <thread>

void setup(void);
void loop(void);

static void loopThread(void)
{
  setup();
  SDL_Delay(5);
  for (;;)
  {
    std::this_thread::yield();
    loop();
  }
}

#if __has_include(<windows.h>)
#include <windows.h>
#include <tchar.h>
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
#else
int main(int, char**)
#endif
{
  std::thread sub_thread(loopThread);
  for (;;)
  {
    std::this_thread::yield();
    lgfx::Panel_sdl::sdl_event_handler();
    SDL_Delay(5);
  }
  return 1;
}
#endif
