/*----------------------------------------------------------------------------/
  Lovyan GFX library - LCD graphics library .
  
  support platform:
    ESP32 (SPI/I2S) with Arduino/ESP-IDF
    ATSAMD51 (SPI) with Arduino
  
Original Source:  
 https://github.com/lovyan03/LovyanGFX/  

Licence:  
 [BSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)  

Author:  
 [lovyan03](https://twitter.com/lovyan03)  

Contributors:  
 [ciniml](https://github.com/ciniml)  
 [mongonta0716](https://github.com/mongonta0716)  
 [tobozo](https://github.com/tobozo)  
/----------------------------------------------------------------------------*/
#ifndef LOVYANGFX_HPP_
#define LOVYANGFX_HPP_

#ifdef setFont
#undef setFont
#endif

#if __has_include("lgfx/v1_init.hpp") && ( defined ( LGFX_USE_V1 ) || !__has_include("lgfx/v0_init.hpp") )

 #include "lgfx/v1_init.hpp"

 #if defined ( LGFX_AUTODETECT )

  #include "LGFX_AUTODETECT.hpp"

 #endif

#else  // if defined ( LGFX_USE_V0 )

 #if __has_include("lgfx/v0_init.hpp")
  #include "lgfx/v0_init.hpp"
 #endif

#endif

#endif
