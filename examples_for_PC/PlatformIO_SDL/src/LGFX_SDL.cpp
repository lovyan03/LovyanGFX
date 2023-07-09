
#include <lgfx/v1/platforms/sdl/Panel_sdl.hpp>
#if defined ( SDL_h_ )

void setup(void);
void loop(void);

#if __has_include(<windows.h>)
#include <windows.h>
#include <tchar.h>
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
#else
int main(int, char**)
#endif
{
  setup();
  for (;;)
  {
    lgfx::Panel_sdl::sdl_event_handler();
    loop();
  }
  return 1;
}
#endif
