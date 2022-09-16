#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <thread>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>

void setup(void);
void loop(void);

void loopThread(void)
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
    lgfx::Panel_OpenCV::imshowall();
  }
}
