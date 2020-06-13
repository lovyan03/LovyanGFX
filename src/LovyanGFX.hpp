/*----------------------------------------------------------------------------/
  Lovyan GFX library - ESP32 hardware SPI graphics library .  
  
    for Arduino and ESP-IDF  
  
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

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#ifdef setFont
#undef setFont
#endif

#include "lgfx/lgfx_common.hpp"         // common include (always include)

#include "lgfx/utility/lgfx_tjpgd.h"    // JPEG decode support (optional)
#include "lgfx/utility/lgfx_pngle.h"    // PNG decode support (optional)
#include "lgfx/utility/lgfx_qrcode.h"   // QR code support (optional)
#include "lgfx/lgfx_img_support.hpp"    // image format extention (optional)
#include "lgfx/lgfx_font_support.hpp"   // font extention (optional)

#include "lgfx/LGFXBase.hpp"           // base class (always include)

#include "lgfx/LGFX_Sprite.hpp"         // sprite class (optional)

#include "lgfx/panel/Panel_HX8357.hpp"
#include "lgfx/panel/Panel_ILI9163.hpp"
#include "lgfx/panel/Panel_ILI9341.hpp"   // and ILI9342 / M5Stack / ODROID-GO / ESP-WROVER-KIT4.1
#include "lgfx/panel/Panel_ILI9486.hpp"
#include "lgfx/panel/Panel_ILI9488.hpp"
#include "lgfx/panel/Panel_SSD1351.hpp"
#include "lgfx/panel/Panel_ST7789.hpp"    // LilyGO TTGO T-Watch
#include "lgfx/panel/Panel_ST7735.hpp"    // M5StickC


#if defined (ESP32) || (CONFIG_IDF_TARGET_ESP32)

  #include "lgfx/platforms/LGFX_SPI_ESP32.hpp"

#elif defined (__SAMD51__)
  #include "lgfx/platforms/LGFX_SPI_SAMD51.hpp"

#endif


// When using the Arduino IDE, the configuration for the board selected in the Board Manager will be loaded.
// When using ESP-IDF or Harmony, please specify a #define LGFX_M5STACK or other #define before the #include<LovyanGFX.hpp> .
// ArduinoIDEで利用する場合、ボードマネージャで選択したボードに合うConfigが読み込まれます。
// ESP-IDFやHarmonyで利用する場合は、#include<LovyanGFX.hpp> より前に #define LGFX_M5STACK 等の #define を記述してください。

#if defined( LGFX_M5STACK ) || defined( ARDUINO_M5Stack_Core_ESP32 ) || defined( ARDUINO_M5STACK_FIRE ) // M5Stack
  #include "config/LGFX_Config_M5Stack.hpp"

#elif defined( LGFX_M5STICKC ) || defined( ARDUINO_M5Stick_C ) // M5Stick C
  #include "config/LGFX_Config_M5StickC.hpp"

#elif defined( LGFX_ODROID_GO ) || defined( ARDUINO_ODROID_ESP32 ) // ODROID-GO
  #include "config/LGFX_Config_ODROID_GO.hpp"

#elif defined( LGFX_TTGO_TS ) || defined( ARDUINO_TTGO_T1 ) // TTGO TS
  #include "config/LGFX_Config_TTGO_TS.hpp"

#elif defined( LGFX_TTGO_TWATCH ) || defined( ARDUINO_T ) // TTGO T-Watch
  #include "config/LGFX_Config_TTGO_TWatch.hpp"

#elif defined( LGFX_DDUINO32_XS ) || defined( ARDUINO_D ) || defined( ARDUINO_DDUINO32_XS )
  #include "config/LGFX_Config_DDUINO32_XS.hpp"

#elif defined( LGFX_LOLIN_D32 ) || defined( ARDUINO_LOLIN_D32_PRO ) // LoLin D32 Pro
  #include "config/LGFX_Config_LoLinD32.hpp"

#elif defined( LGFX_ESP_WROVER_KIT ) || defined( ARDUINO_ESP32_DEV ) // ESP-WROVER-KIT
  #include "config/LGFX_Config_ESP_WROVER_KIT.hpp"

#elif defined( LGFX_WIO_TERMINAL ) || defined (ARDUINO_WIO_TERMINAL) || defined(WIO_TERMINAL)
  #include "config/LGFX_Config_WioTerminal.hpp"

#else

  // If none of the above apply, Put a copy of "config/LGFX_Config_Custom" in libraries folder,
  // and modify the content according to the environment.
  // 上記のいずれにも該当しない場合、"config/LGFX_Config_Custom" のコピーをライブラリフォルダに配置し、
  // 動作環境に応じて内容を修正してください。
  #include "../LGFX_Config_Custom.hpp"

#endif



#endif
