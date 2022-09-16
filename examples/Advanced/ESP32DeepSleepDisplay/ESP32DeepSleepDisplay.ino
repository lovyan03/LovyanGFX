#define LGFX_USE_V1
#define LGFX_AUTODETECT

#include <LovyanGFX.hpp>

#include <driver/rtc_io.h>

static LGFX lcd;

RTC_DATA_ATTR int bootCount = 0;  // 起動回数を保持（deepsleepしても値は消えない）

void setup(void)
{
  switch(esp_sleep_get_wakeup_cause())
  {
  case ESP_SLEEP_WAKEUP_EXT0 :
  case ESP_SLEEP_WAKEUP_EXT1 :
  case ESP_SLEEP_WAKEUP_TIMER :
  case ESP_SLEEP_WAKEUP_TOUCHPAD :
  case ESP_SLEEP_WAKEUP_ULP :
    lcd.init_without_reset(); // deep sleep からの復帰時はinit_without_resetを呼び出す。
    break;

  default :
    lcd.init();            // 通常起動時はinitを呼び出す。
    lcd.clear(TFT_WHITE);
    lcd.clear(TFT_BLACK);
    lcd.startWrite();      // 背景を描画しておく
    lcd.setColorDepth(24);
    {
      LGFX_Sprite sp(&lcd);
      sp.createSprite(128, 128);
      sp.createPalette();
      for (int y = 0; y < 128; y++)
        for (int x = 0; x < 128; x++)
          sp.writePixel(x, y, sp.color888(x << 1, x + y, y << 1));
      for (int y = 0; y < lcd.height(); y += 128)
        for (int x = 0; x < lcd.width(); x += 128)
          sp.pushSprite(x, y);
    }
    lcd.endWrite();
    break;
  }

  ++bootCount;
  lcd.setCursor(bootCount*6, bootCount*8);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);  // 一度白黒反転した状態を描画する
  lcd.print("DeepSleep test : " + String(bootCount));
  lcd.setCursor(bootCount*6, bootCount*8);
  lcd.setTextColor(TFT_BLACK, TFT_WHITE);
  lcd.print("DeepSleep test : " + String(bootCount));
  lcd.powerSaveOn(); // 省電力指定 M5Stack CoreInkで電源オフ時に色が薄くならないようにする
  lcd.waitDisplay(); // 待機

  auto pin_rst = (gpio_num_t)lcd.getPanel()->config().pin_rst;
  if ((uint32_t)pin_rst < GPIO_NUM_MAX)
  {
    // RSTピンをRTC_GPIOで管理しhigh状態を維持する
    rtc_gpio_set_level(pin_rst, 1);
    rtc_gpio_set_direction(pin_rst, RTC_GPIO_MODE_OUTPUT_ONLY);
    rtc_gpio_init(pin_rst);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  }

  auto light = lcd.getPanel()->getLight();
  if (light)
  {
    auto pin_bl = (gpio_num_t)((lgfx::Light_PWM*)light)->config().pin_bl;
    if ((uint32_t)pin_bl < GPIO_NUM_MAX)
    {
      // BackLightピンをRTC_GPIOで管理しhigh状態を維持する
      rtc_gpio_set_level(pin_bl, 1);
      rtc_gpio_set_direction(pin_bl, RTC_GPIO_MODE_OUTPUT_ONLY);
      rtc_gpio_init(pin_bl);
      esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    }
  }

  ESP_LOGW("sleep");
  esp_sleep_enable_timer_wakeup(1 * 1000 * 1000); // micro sec

  esp_deep_sleep_start();
}

void loop(void)
{
  delay(10000);
}
