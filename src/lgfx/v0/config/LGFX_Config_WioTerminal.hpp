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

  struct Panel_WioTerminal : public Panel_ILI9341
  {
    Panel_WioTerminal(void)
    {
      spi_cs   = 0x0115; // PORTB 21  (PORTB=0x0100 | 21=0x0015)
      spi_dc   = 0x0206; // PORTC  6  (PORTC=0x0200 |  6=0x0006)
      gpio_rst = 0x0207; // PORTC  7  (PORTC=0x0200 |  7=0x0007)
      gpio_bl  = 0x0205; // PORTC  5  (PORTC=0x0200 |  5=0x0005)
      freq_fill  =  75000000;
      freq_write =  60000000;
      freq_read  =  20000000;
      rotation = 1;
    }

    uint8_t maxBrightness = 255;

    void init(bool use_reset) override
    {
      /* Enable Peripheral Clocks */
      GCLK->PCHCTRL[9].reg = 0 | (1u<<6);         // TC0, TC1
      while(!GCLK->PCHCTRL[9].bit.CHEN);
      GCLK->PCHCTRL[11].reg = 0 | (1u<<6);    // EVSYS[0]
      while(!GCLK->PCHCTRL[11].bit.CHEN);
      GCLK->PCHCTRL[33].reg = 0 | (1u<<6);    // CCL
      while(!GCLK->PCHCTRL[33].bit.CHEN);
      /* Enable Peropheral APB Clocks */
      MCLK->APBAMASK.bit.TC0_ = 1;
      MCLK->APBBMASK.bit.EVSYS_ = 1;
      MCLK->APBCMASK.bit.CCL_ = 1;

      /* Configure PORT */
      PORT->Group[2].DIRSET.reg = (1<<5);
      PORT->Group[2].EVCTRL.reg = 0x85; // PC05, OUT
      /* Configure EVSYS */
      EVSYS->USER[1].reg = 0x01;  // Channel0 -> PORT_EV0
      EVSYS->Channel[0].CHANNEL.reg = 0x74 | (0x02<<8) | (0x00<<10);  // CCL_LUTOUT0, ASYNCHRONOUS, NO_EVT_OUTPUT
      /* Configure CCL */
      CCL->CTRL.reg = (1<<0); // SWRST
      CCL->SEQCTRL[0].reg = 0x0; // Disable SEQCTRL
      CCL->LUTCTRL[0].reg = (0xaau << 24) | (1u<<22) | (0x6<<8) | (1<<1); // TRUTH=0xaa, LUTEO, INSEL0=0x06(TC), ENABLE
      CCL->CTRL.reg = (1<<1); // ENABLE

      /* Configure TC0 */
      TC0->COUNT8.CTRLA.reg = (1u<<0);   // SWRST;
      while( TC0->COUNT8.SYNCBUSY.bit.SWRST );
      
      TC0->COUNT8.CTRLA.reg = (0x01 << 2) | (0x01 << 4) | (0x04 << 8);   // MODE=COUNT8, PRESCALER=DIV16, PRESCSYNC=PRESC
      TC0->COUNT8.WAVE.reg  = 0x02; // WAVEGEN=NPWM;
      TC0->COUNT8.CTRLBSET.reg = (1u<<1); // LUPD
      TC0->COUNT8.PER.reg = this->maxBrightness;
      TC0->COUNT8.CC[0].reg = this->brightness;
      TC0->COUNT8.CC[1].reg = 0u;
      TC0->COUNT8.DBGCTRL.bit.DBGRUN = 1;
      TC0->COUNT8.INTFLAG.reg = 0x33;    // Clear all flags
      while( TC0->COUNT8.SYNCBUSY.reg );
      
      TC0->COUNT8.CTRLA.bit.ENABLE = 1;   // ENABLE
      while( TC0->COUNT8.SYNCBUSY.bit.ENABLE );

      Panel_ILI9341::init(use_reset);
    }

    void setBrightness(uint8_t brightness) override
    {
      this->brightness = brightness < this->maxBrightness ? brightness : this->maxBrightness;
      TC0->COUNT8.CC[0].reg = this->brightness;
      while(TC0->COUNT8.SYNCBUSY.bit.CC0);
    }
  };
}

class LGFX : public lgfx::LGFX_SPI<lgfx::LGFX_Config>
{
public:
  LGFX(void) : lgfx::LGFX_SPI<lgfx::LGFX_Config>()
  {
    board = lgfx::board_t::board_WioTerminal;
    static lgfx::Panel_WioTerminal panel;
    setPanel(&panel);
  }
};

#endif
