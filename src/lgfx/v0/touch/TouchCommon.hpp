#ifndef LGFX_TOUCH_COMMON_HPP_
#define LGFX_TOUCH_COMMON_HPP_

#include <stdint.h>
#include <type_traits>

#include "../lgfx_common.hpp"

namespace lgfx
{
 inline namespace v0
 {
//----------------------------------------------------------------------------
  struct TouchCommon
  {
    union {
      struct {
        int_fast8_t spi_host; // ESP32:spi_host_device_t VSPI_HOST or HSPI_HOST
        int_fast16_t spi_sclk;
        int_fast16_t spi_mosi;
        int_fast16_t spi_miso;
        int_fast16_t spi_cs;
      };
      struct {
        int_fast8_t i2c_port; // ESP32:i2c_port_t I2C_NUM_0 or I2C_NUM_1
        int_fast16_t i2c_scl;
        int_fast16_t i2c_sda;
        int_fast16_t i2c_addr;
        //
      };
    };

    int_fast16_t gpio_int = -1;
    int32_t freq = 1000000;
    uint32_t x_min = 0;
    uint32_t x_max = 3600;
    uint32_t y_min = 0;
    uint32_t y_max = 3600;
    bool bus_shared = false;

    TouchCommon()
    : spi_host(0)
    , spi_sclk(-1)
    , spi_mosi(-1)
    , spi_miso(-1)
    , spi_cs  (-1)
    {}

    inline bool isSPI(void) const { return spi_cs >= 0; }

    virtual bool init(void) = 0;
    virtual void wakeup(void) {}
    virtual void sleep(void) {}
    virtual uint_fast8_t getTouch(touch_point_t* tp, int_fast8_t number) = 0;

  protected:
    bool _inited = false;
  };

//----------------------------------------------------------------------------
 }
}
#endif
