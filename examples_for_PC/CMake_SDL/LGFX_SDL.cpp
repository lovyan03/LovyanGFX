#include <lgfx/v1/platforms/sdl/Panel_sdl.hpp>
#if defined ( SDL_h_ )

void setup(void);
void loop(void);

int main(int, char**)
{
  setup();
  for (;;)
  {
    lgfx::Panel_sdl::sdl_event_handler();
    loop();
  }
  return 0;
}
#endif
