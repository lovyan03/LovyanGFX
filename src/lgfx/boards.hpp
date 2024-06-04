#ifndef __LGFX_BOARDS_HPP__
#define __LGFX_BOARDS_HPP__

namespace lgfx
{
  namespace boards
  {
    enum board_t
    { board_unknown
    , board_Non_Panel
    , board_M5Stack
    , board_M5StackCore2
    , board_M5StickC
    , board_M5StickCPlus
    , board_M5StickCPlus2
    , board_TTGO_TS
    , board_TTGO_TWatch
    , board_TTGO_TWristband
    , board_TTGO_TDisplay
    , board_TTGO_T4_Display
    , board_ODROID_GO
    , board_DDUINO32_XS
    , board_ESP_WROVER_KIT
    , board_LoLinD32
    , board_WioTerminal
    , board_WiFiBoy_Pro
    , board_WiFiBoy_Mini
    , board_Makerfabs_TouchCamera
    , board_Makerfabs_MakePython
    , board_M5StackCoreInk
    , board_M5Stack_CoreInk = board_M5StackCoreInk
    , board_M5Paper
    , board_ESP32_S2_Kaluga_1
    , board_WT32_SC01
    , board_PyBadge
    , board_M5Tough
    , board_OpenCV
    , board_FrameBuffer
    , board_M5Station
    , board_ESPboy
    , board_M5UnitLCD
    , board_M5UnitOLED
    , board_M5UnitGLASS
    , board_M5AtomDisplay
    , board_FeatherM4_HX8357
    , board_ESP32_S3_BOX
    , board_SDL
    , board_M5StackCoreS3
    , board_M5AtomS3
    , board_HalloWingM0
    , board_HalloWingM4
    , board_Feather_ESP32_S2_TFT
    , board_Feather_ESP32_S3_TFT
    , board_FunHouse
    , board_PyGamer
    , board_Sunton_ESP32_2432S028
    , board_ESP32_2432S028 = board_Sunton_ESP32_2432S028
    , board_Makerfabs_ESP32_C3_TFT_Touch_SPI
    , board_Makerfabs_ESP32_S3_TFT_Touch_SPI
    , board_Makerfabs_ESP32_S2_TFT_Touch_Parallel16
    , board_Makerfabs_ESP32_S3_TFT_Touch_Parallel16
    , board_wywy_ESP32S3_HMI_DevKit
    , board_M5ModuleDisplay
    , board_ESP32_S3_BOX_Lite
    , board_LoLinS3Pro
    , board_ESP32_S3_BOX_V3
    };
  }
  using namespace boards;
}

#endif
