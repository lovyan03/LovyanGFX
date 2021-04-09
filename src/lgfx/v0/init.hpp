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
#pragma once

#include "gitTagVersion.h"

#include "lgfx_common.hpp"         // common include (always include)

#include "lgfx_filesystem_support.hpp" // filesystem extention (optional)

#include "LGFXBase.hpp"           // base class (always include)

#include "LGFX_Sprite.hpp"         // sprite class (optional)

#include "panel/PanelCommon.hpp"
#include "touch/TouchCommon.hpp"

#include "panel/Panel_HX8357.hpp"
#include "panel/Panel_ILI9163.hpp"
#include "panel/Panel_ILI9341.hpp"   // and ILI9342 / M5Stack / ODROID-GO / ESP-WROVER-KIT4.1 / WioTerminal
#include "panel/Panel_ILI9481.hpp"
#include "panel/Panel_ILI9486.hpp"
#include "panel/Panel_ILI9488.hpp"
#include "panel/Panel_SSD1351.hpp"
#include "panel/Panel_ST7735.hpp"    // M5StickC / LilyGO TTGO T-Wristband
#include "panel/Panel_ST7789.hpp"    // M5StickCPlus / LilyGO TTGO T-Watch / ESP-WROVER-KIT4.1
#include "panel/Panel_ST7796.hpp"

#include "touch/Touch_XPT2046.hpp"
#include "touch/Touch_STMPE610.hpp"
#include "touch/Touch_FT5x06.hpp"
#include "touch/Touch_GT911.hpp"

#if defined (ESP32) || defined (CONFIG_IDF_TARGET_ESP32) || defined (ESP_PLATFORM)
  #include "platforms/LGFX_SPI_ESP32.hpp"
  #include "platforms/LGFX_PARALLEL_ESP32.hpp"

#elif defined (__SAMD51__)
  #include "platforms/LGFX_SPI_SAMD51.hpp"

#endif

// ArduinoIDEで利用する場合、ボードマネージャで選択したボードに合うConfigが読み込まれます。
// ESP-IDFやHarmonyで利用する場合は、#include<LovyanGFX.hpp> より前に #define LGFX_ボード名 の記述をしてください。
// お使いのボードが対応機種にない場合、"config/LGFX_Config_Custom" をコピーしてプロジェクトフォルダに配置し、
// 動作環境に応じて内容に修正を加えて include して使用してください。

// When using the Arduino IDE, the configuration for the board selected in the Board Manager will be loaded.
// When using ESP-IDF or Harmony, please specify a #define LGFX_YOUR_BOARD before the #include<LovyanGFX.hpp> .
// If the board you are using is not supported,  Put a copy of "config/LGFX_Config_Custom" in your project folder,
// and include it with modifications to the content to suit your environment.

#if defined (ESP32) || defined (CONFIG_IDF_TARGET_ESP32) || defined (ESP_PLATFORM)

 #if defined( ARDUINO_M5Stack_Core_ESP32 ) || defined( ARDUINO_M5STACK_FIRE )
  #define LGFX_M5STACK
  #define LGFX_M5STACK_CORE2
 #elif defined( ARDUINO_M5STACK_Core2 ) // M5Stack Core2
  #define LGFX_M5STACK_CORE2
 #elif defined( ARDUINO_M5Stick_C ) || defined( ARDUINO_M5Stick_C_Plus ) // M5Stick C / CPlus
  #define LGFX_M5STICK_C
 #elif defined( ARDUINO_M5Stack_CoreInk ) // M5Stack CoreInk
  #define LGFX_M5STACK_COREINK
 #elif defined( ARDUINO_M5STACK_Paper ) // M5Paper
  #define LGFX_M5PAPER
 #elif defined( ARDUINO_ODROID_ESP32 ) // ODROID-GO
  #define LGFX_ODROID_GO
 #elif defined( ARDUINO_TTGO_T1 ) // TTGO TS
  #define LGFX_TTGO_TS
 #elif defined( ARDUINO_TWatch ) || defined( ARDUINO_T ) // TTGO T-Watch
  #define LGFX_TTGO_TWATCH
 #elif defined( ARDUINO_D ) || defined( ARDUINO_DDUINO32_XS ) // DSTIKE D-duino-32 XS
  #define LGFX_DDUINO32_XS
 #elif defined( ARDUINO_LOLIN_D32_PRO )
  #define LGFX_LOLIN_D32_PRO
 #elif defined( ARDUINO_ESP32_WROVER_KIT )
  #define LGFX_ESP_WROVER_KIT
 #endif

 #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5STACK_COREINK )
  #include "panel/Panel_GDEW0154M09.hpp"
 #endif
 #if defined ( LGFX_AUTODETECT ) || defined ( LGFX_M5PAPER )
  #include "panel/Panel_IT8951.hpp"
 #endif

 #include "config/LGFX_Config_AutoDetectESP32.hpp"

#elif defined (__SAMD51__)

 #if defined( LGFX_WIO_TERMINAL ) || defined (ARDUINO_WIO_TERMINAL) || defined(WIO_TERMINAL)
  #include "config/LGFX_Config_WioTerminal.hpp"

 #endif

#endif


