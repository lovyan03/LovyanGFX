/*
#include <soc/rtc.h>
void setup(void) {
  rtc_cpu_freq_config_t c;
  rtc_clk_cpu_freq_get_config(&c);
  Serial.printf("source:%d   source_freq_mhz:%d  div:%d  freq_mhz:%d \r\n",
               c.source  , c.source_freq_mhz , c.div , c.freq_mhz);
}
void loop(void){delay(1000);}
///*/

#include <LovyanGFX.hpp>

struct LGFX_Config {
  static constexpr spi_host_device_t spi_host = VSPI_HOST;
  static constexpr int spi_mosi = 23;
  static constexpr int spi_miso = 19;
  static constexpr int spi_sclk = 18;
  static constexpr int spi_cs   = 14;
  static constexpr int spi_dc   = 27;
  static constexpr int panel_rst = 33;
  static constexpr int panel_bl  = 32;
  static constexpr int freq_write = 40000000;
  static constexpr int freq_read  = 16000000;
  static constexpr int freq_fill  = 40000000;
  static constexpr bool spi_3wire = true;
};
static LovyanGFX<lgfx::Esp32Spi<lgfx::Panel_M5Stack, LGFX_Config> > tft;


void setup(void)
{
  Serial.begin(115200);
  tft.testFunc();


}

void loop(void)
{
delay(100);
}

//*/

