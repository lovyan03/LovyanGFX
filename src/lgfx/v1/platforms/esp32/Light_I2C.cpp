/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)

Contributors:
 [ciniml](https://github.com/ciniml)
 [mongonta0716](https://github.com/mongonta0716)
 [tobozo](https://github.com/tobozo)
/----------------------------------------------------------------------------*/
#if defined (ESP_PLATFORM)
#include <sdkconfig.h>
#if !defined (CONFIG_IDF_TARGET) || defined (CONFIG_IDF_TARGET_ESP32) || defined (CONFIG_IDF_TARGET_ESP32S2) || defined (CONFIG_IDF_TARGET_ESP32C3)

#include "Light_I2C.hpp"

#if defined ( ARDUINO )
 #include <esp32-hal-ledc.h>
#else
 #include <driver/ledc.h>
#endif

namespace lgfx
{
 inline namespace v1
 {

  //----------------------------------------------------------------------------

  bool Light_I2C::init ( uint8_t brightness )
  {
	
	  buf = 0x01 ;
    i2c_manager_write ( I2C_NUM_0, LCD_BL_ADDR, BL_GP_REG, &buf, 1 ) ;
    setBrightness (brightness) ;
    
    return true;
  }

  //----------------------------------------------------------------------------

  void Light_I2C::setBrightness ( uint8_t brightness )
  {
    if (_cfg.invert) brightness = ~brightness ;
	  
    buf = 0x01 ;
    i2c_manager_write ( I2C_NUM_0, LCD_BL_ADDR, BL_GP_REG, &buf, 1 ) ;
    
    buf = calculateBrightness (brightness) ;
    i2c_manager_write ( I2C_NUM_0, LCD_BL_ADDR, BANK_A_BL_CTRL, &buf, 1 ) ;
  }

  //----------------------------------------------------------------------------
  
  uint8_t Light_I2C::calculateBrightness ( uint8_t input )
  {
    // on invalid input or steps config set to ~50 % brightness:
    if ( input <= 0 || DRIVER_STEPS <= 0 || INPUT_STEPS <= 0 ) return 0x18 ;

	  return ( ( input * DRIVER_STEPS ) / INPUT_STEPS ) ;
  }

  //----------------------------------------------------------------------------

 }
}

#endif
#endif
