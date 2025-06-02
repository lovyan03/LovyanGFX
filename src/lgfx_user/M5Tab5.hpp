

#include <LovyanGFX.hpp>

#if !defined (CONFIG_IDF_TARGET_ESP32P4)
  #error "CONFIG_IDF_TARGET_ESP32P4 should be set"
#endif

#define M5GFX_SPI_HOST SPI2_HOST


// for Tab5Add commentMore actions


int i2c_port = 1;
int i2c_sda  = GPIO_NUM_31;
int i2c_scl  = GPIO_NUM_32;
int spi_cs   = GPIO_NUM_48;
int spi_mosi = GPIO_NUM_18;
int spi_miso = GPIO_NUM_19;
int spi_sclk = GPIO_NUM_5;
