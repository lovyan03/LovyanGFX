
#include "main.h"


void setup()
{
  gpioInit();
  initGPIOPin(25, "gpiochip4");
  initGPIOPin(24, "gpiochip4");

  Wire.begin(1);
  display = new LGFX();
  display->init();

  canvas.setColorDepth(8);
  canvas.setFont(&fonts::lgfxJapanMinchoP_32);
  canvas.setTextWrap(false);        // 右端到達時のカーソル折り返しを禁止
  canvas.createSprite(display->width(), 36);
  canvas.clear();
  canvas.setCursor(0, 0);
  canvas.printf("Touch to start\n");
  canvas.pushSprite(display, 0, 0);
}


void loop()
{
  if (display->touch()) {
    canvas.clear();
    canvas.setCursor(0, 0);
    lgfx::touch_point_t tp;
    auto blah = display->getTouchRaw(&tp, 1);
    if (blah > 0) {
      canvas.printf("Touch %i, %i\n", tp.x, tp.y);
      canvas.pushSprite(display, 0, 0);
    }
  }
}


int initGPIOPin(int pinNum, std::string gpioChipName)
{
  std::string gpio_name = "GPIO" + std::to_string(pinNum);
  try {
    GPIOPin *csPin;
    csPin = new LinuxGPIOPin(pinNum, gpioChipName.c_str(), pinNum, gpio_name.c_str());
    csPin->setSilent();
    gpioBind(csPin);
    return 1;
  } catch (...) {
    std::exception_ptr p = std::current_exception();
    std::cout << "Warning, cannot claim pin " << gpio_name << (p ? p.__cxa_exception_type()->name() : "null") << std::endl;
    return 0;
  }
}
