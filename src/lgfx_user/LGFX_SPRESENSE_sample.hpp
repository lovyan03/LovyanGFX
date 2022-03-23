#pragma once

#define LGFX_USE_V1

#include <LovyanGFX.hpp>

// SPRESENSEでLovyanGFXを独自設定で利用する場合の設定例

/*
このファイルを複製し、新しい名前を付けて、環境に合わせて設定内容を変更してください。
作成したファイルをユーザープログラムからincludeすることで利用可能になります。

複製したファイルはライブラリのlgfx_userフォルダに置いて利用しても構いませんが、
その場合はライブラリのアップデート時に削除される可能性があるのでご注意ください。

安全に運用したい場合はバックアップを作成しておくか、ユーザープロジェクトのフォルダに置いてください。
//*/


/// 独自の設定を行うクラスを、LGFX_Deviceから派生して作成します。
class LGFX : public lgfx::LGFX_Device
{
/*
 クラス名は"LGFX"から別の名前に変更しても構いません。
 AUTODETECTと併用する場合は"LGFX"は使用されているため、LGFX以外の名前に変更してください。
 また、複数枚のパネルを同時使用する場合もそれぞれに異なる名前を付けてください。
 ※ クラス名を変更する場合はコンストラクタの名前も併せて同じ名前に変更が必要です。

 名前の付け方は自由に決めて構いませんが、設定が増えた場合を想定し、
 例えば SPRESENSE でSPI接続のILI9341の設定を行った場合、
  LGFX_SPRESENSE_SPI_ILI9341
 のような名前にし、ファイル名とクラス名を一致させておくことで、利用時に迷いにくくなります。
//*/


// 接続するパネルの型にあったインスタンスを用意します。
//lgfx::Panel_GC9A01      _panel_instance;
//lgfx::Panel_GDEW0154M09 _panel_instance;
//lgfx::Panel_HX8357B     _panel_instance;
//lgfx::Panel_HX8357D     _panel_instance;
//lgfx::Panel_ILI9163     _panel_instance;
  lgfx::Panel_ILI9341     _panel_instance;
//lgfx::Panel_ILI9342     _panel_instance;
//lgfx::Panel_ILI9481     _panel_instance;
//lgfx::Panel_ILI9486     _panel_instance;
//lgfx::Panel_ILI9488     _panel_instance;
//lgfx::Panel_IT8951      _panel_instance;
//lgfx::Panel_SH110x      _panel_instance; // SH1106, SH1107
//lgfx::Panel_SSD1306     _panel_instance;
//lgfx::Panel_SSD1327     _panel_instance;
//lgfx::Panel_SSD1331     _panel_instance;
//lgfx::Panel_SSD1351     _panel_instance; // SSD1351, SSD1357
//lgfx::Panel_SSD1963     _panel_instance;
//lgfx::Panel_ST7735      _panel_instance;
//lgfx::Panel_ST7735S     _panel_instance;
//lgfx::Panel_ST7789      _panel_instance;
//lgfx::Panel_ST7796      _panel_instance;


// SPIバスのインスタンスを用意します。
  lgfx::Bus_SPI       _bus_instance;   // SPIバスのインスタンス

public:

  // コンストラクタを作成し、ここで各種設定を行います。
  // クラス名を変更した場合はコンストラクタも同じ名前を指定してください。
  LGFX(void)
  {
    { // バス制御の設定を行います。
      auto cfg = _bus_instance.config();    // バス設定用の構造体を取得します。

      cfg.spi_mode = 0;             // SPI通信モードを設定 (0 ~ 3)
      cfg.freq_write = 40000000;    // 送信時のSPIクロック
      cfg.freq_read  = 16000000;    // 受信時のSPIクロック
      cfg.pin_dc   =  9;            // SPIのD/Cピン番号を設定  (-1 = disable)
      cfg.spi_port =  4;            // Arduino拡張ボードの場合は 4 

      _bus_instance.config(cfg);    // 設定値をバスに反映します。
      _panel_instance.setBus(&_bus_instance);      // バスをパネルにセットします。
    }

    { // 表示パネル制御の設定を行います。
      auto cfg = _panel_instance.config();    // 表示パネル設定用の構造体を取得します。

//    cfg.pin_cs           =    -1;  // CSが接続されているピン番号   (-1 = disable) HW CSピンの場合は-1を指定
      cfg.pin_rst          =     8;  // RSTが接続されているピン番号  (-1 = disable)
      cfg.pin_busy         =    -1;  // BUSYが接続されているピン番号 (-1 = disable)

      // ※ 以下の設定値はパネル毎に一般的な初期値が設定されていますので、不明な項目はコメントアウトして試してみてください。

      cfg.panel_width      =   240;  // 実際に表示可能な幅
      cfg.panel_height     =   320;  // 実際に表示可能な高さ
      cfg.offset_x         =     0;  // パネルのX方向オフセット量
      cfg.offset_y         =     0;  // パネルのY方向オフセット量
      cfg.offset_rotation  =     0;  // 回転方向の値のオフセット 0~7 (4~7は上下反転)
      cfg.dummy_read_pixel =     8;  // ピクセル読出し前のダミーリードのビット数
      cfg.dummy_read_bits  =     1;  // ピクセル以外のデータ読出し前のダミーリードのビット数
      cfg.readable         =  true;  // データ読出しが可能な場合 trueに設定
      cfg.invert           = false;  // パネルの明暗が反転してしまう場合 trueに設定
      cfg.rgb_order        = false;  // パネルの赤と青が入れ替わってしまう場合 trueに設定
      cfg.dlen_16bit       = false;  // データ長を16bit単位で送信するパネルの場合 trueに設定
      cfg.bus_shared       =  true;  // SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

// 以下はST7735やILI9163のようにピクセル数が変更できるドライバでのみ設定してください。
//    cfg.memory_width     =   240;  // ドライバICがサポートしている最大の幅
//    cfg.memory_height    =   320;  // ドライバICがサポートしている最大の高さ

      _panel_instance.config(cfg);
    }

    setPanel(&_panel_instance); // 使用するパネルをセットします。
  }
};

