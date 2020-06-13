#ifndef LOVYANGFX_CONFIG_HPP_
#define LOVYANGFX_CONFIG_HPP_

namespace lgfx
{
  struct LGFX_Config
  {
    static constexpr int sercom_index = 7;
    static constexpr int sercom_clksrc = 0;   // -1=notchange / 0=select GCLK0
    static constexpr int sercom_clkfreq = F_CPU;
    static constexpr int spi_miso = 0x0112; // PORTB 18 (PORTB=0x0100 | 18=0x0012)
    static constexpr int spi_mosi = 0x0113; // PORTB 19 (PORTB=0x0100 | 19=0x0013)
    static constexpr int spi_sclk = 0x0114; // PORTB 20 (PORTB=0x0100 | 20=0x0014)
    static constexpr SercomSpiTXPad pad_mosi = SPI_PAD_3_SCK_1;  // PAD_SPI3_TX;
    static constexpr SercomRXPad    pad_miso = SERCOM_RX_PAD_2;  // PAD_SPI3_RX;
  };
}

class LGFX : public lgfx::LGFX_SPI<lgfx::LGFX_Config>
{
public:
  LGFX(void) : lgfx::LGFX_SPI<lgfx::LGFX_Config>()
  {
    static lgfx::Panel_WioTerminal panel;

    setPanel(&panel);
  }
};

#endif
