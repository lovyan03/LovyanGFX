# lgfx::Panel_CVBS for ESP32

 - ESP32の GPIO25, または GPIO26 からビデオ信号を出力できます。(ESP32-S2,S3,C3には対応していません)
 - I2S の channel0 と DAC を利用してビデオ信号を生成しています。
 - ESP32のメモリ上に映像フレームバッファを確保するためメモリ消費は多めです。(PSRAMの利用が可能です)


出力可能な信号の種類 (Types of signals that can be output)
----------------

 - NTSC
 - NTSC-J
 - PAL
 - PAL-M
 - PAL-N

NTSCとNTSC-Jは黒の信号レベルが異なっており、NTSCは7.5IRE、NTSC-Jは0IREに設定されています。<br>
NTSCを使用した際に黒が僅かに白浮きしていると感じる場合はNTSC-Jに変更してみてください。



出力解像度 (Output resolution)
----------------

 - 出力できる最大解像度は信号タイプによって差があります。
   - 720 x 480  (NTSC,NTSC-J)
   - 864 x 576  (PAL,PAL-M)
   - 720 x 576  (PAL-N)
 - 最大解像度以下であれば、任意の解像度を設定可能です。
 - 最大解像度を整数で約分した解像度の指定を推奨します。
 - それ以外の解像度を指定した場合はピクセルの比率が場所によって差が生じ、表示が少し歪になります。


<TABLE>
 <TR>
  <TH></TH>
  <TH> NTSC <BR> NTSC-J </TH>
  <TH> PAL-N </TH>
  <TH> PAL <BR> PAL-M </TH>
 </TR>
 <TR align="center">
  <TH> max width </TH>
  <TD colspan="2"> 720 </TD>
  <TD> 864 </TD>
 </TR>
 <TR align="center">
  <TH> max height </TH>
  <TD> 480 </TD>
  <TD colspan="2"> 576 </TD>
 </TR>
 <TR align="center">
  <TH> recommended<BR>width<BR>推奨 幅</TH>
  <TD colspan="2"> 
    720/1 = 720<br>
    720/1.5=480<br>
    720/2 = 360<br>
    720/3 = 240<br>
    720/4 = 180<br>
    720/5 = 144<br>
    720/6 = 120
  </TD>
  <TD>
    864/1 = 864<br>
    864/1.5=576<br>
    864/2 = 432<br>
    864/3 = 288<br>
    864/4 = 216<br>
    864/5 = 173<br>
    864/6 = 144
  </TD>
 </TR>
 <TR align="center">
  <TH> recommended<BR>height<BR>推奨 高さ</TH>
  <TD>
    480/1 = 480<br>
    480/2 = 240<br>
    480/3 = 160<br>
    480/4 = 120<br>
    480/5 =  96<br>
    480/6 =  80<br>
    480/8 =  60
  </TD>
  <TD colspan="2"> 
    576/1 = 576<br>
    576/2 = 288<br>
    576/3 = 192<br>
    576/4 = 144<br>
    576/5 = 113<br>
    576/6 =  96<br>
    576/8 =  72
  </TD>
 </TR>
</TABLE>




使い方 How to use
----------------
```c

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device
{
public:

  lgfx::Panel_CVBS _panel_instance;

  LGFX(void)
  {
    { // 表示パネル制御の設定を行います。
      auto cfg = _panel_instance.config();    // 表示パネル設定用の構造体を取得します。

      // 出力解像度を設定;
      cfg.memory_width  = 240; // 出力解像度 幅
      cfg.memory_height = 160; // 出力解像度 高さ

      // 実際に利用する解像度を設定;
      cfg.panel_width  = 208;  // 実際に使用する幅   (memory_width と同値か小さい値を設定する)
      cfg.panel_height = 128;  // 実際に使用する高さ (memory_heightと同値か小さい値を設定する)

      // 表示位置オフセット量を設定;
      cfg.offset_x = 16;       // 表示位置を右にずらす量 (初期値 0)
      cfg.offset_y = 16;       // 表示位置を下にずらす量 (初期値 0)

      _panel_instance.config(cfg);


// 通常は memory_width と panel_width に同じ値を指定し、 offset_x = 0 で使用します。;
// 画面端の表示が画面外に隠れるのを防止したい場合は、 panel_width の値をmemory_widthより小さくし、offset_x で左右の位置調整をします。;
// 例えば memory_width より panel_width を 32 小さい値に設定した場合、offset_x に 16 を設定することで左右位置が中央寄せになります。;
// 上下方向 (memory_height , panel_height , offset_y ) についても同様に、必要に応じて調整してください。;

    }

    {
      auto cfg = _panel_instance.config_detail();

      // 出力信号の種類を設定;
      // cfg.signal_type = cfg.signal_type_t::NTSC;
      cfg.signal_type = cfg.signal_type_t::NTSC_J;
      // cfg.signal_type = cfg.signal_type_t::PAL;
      // cfg.signal_type = cfg.signal_type_t::PAL_M;
      // cfg.signal_type = cfg.signal_type_t::PAL_N;

      // 出力先のGPIO番号を設定;
      cfg.pin_dac = 26;       // DACを使用するため、 25 または 26 のみが選択できます;

      // PSRAMメモリ割当の設定;
      cfg.use_psram = 1;      // 0=PSRAM不使用 / 1=PSRAMとSRAMを半々使用 / 2=全部PSRAM使用;

      // 出力信号の振幅の強さを設定;
      cfg.output_level = 128; // 初期値128
      // ※ GPIOに保護抵抗が付いている等の理由で信号が減衰する場合は数値を上げる。;
      // ※ M5StackCore2 はGPIOに保護抵抗が付いているため 200 を推奨。;

      // 彩度信号の振幅の強さを設定;
      cfg.chroma_level = 128; // 初期値128
      // 数値を下げると彩度が下がり、0で白黒になります。数値を上げると彩度が上がります。;

      _panel_instance.config_detail(cfg);
    }

    setPanel(&_panel_instance);
  }
};


LGFX gfx;

void setup(void)
{
  gfx.init();
}

void loop(void)
{
  gfx.fillRect(rand() % gfx.width() - 8, rand() % gfx.height() - 8, 16, 16, rand());
}
```
