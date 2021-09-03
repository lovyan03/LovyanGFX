#ifndef SAMD51_ARDUINO_COMPAT_HPP__
#define SAMD51_ARDUINO_COMPAT_HPP__

#include <stdio.h>
#include <device.h>

#ifndef __I
#define __I const
#define __O volatile
#define __IO volatile
#endif

typedef uint8_t RoReg8;

typedef union {
  struct {
    uint8_t  SWRST:1;          /*!< bit:      0  Software Reset                     */
    uint8_t  :7;               /*!< bit:  1.. 7  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} GCLK_CTRLA_Type;

typedef union {
  struct {
    uint32_t SWRST:1;          /*!< bit:      0  Software Reset Synchroniation Busy bit */
    uint32_t :1;               /*!< bit:      1  Reserved                           */
    uint32_t GENCTRL0:1;       /*!< bit:      2  Generic Clock Generator Control 0 Synchronization Busy bits */
    uint32_t GENCTRL1:1;       /*!< bit:      3  Generic Clock Generator Control 1 Synchronization Busy bits */
    uint32_t GENCTRL2:1;       /*!< bit:      4  Generic Clock Generator Control 2 Synchronization Busy bits */
    uint32_t GENCTRL3:1;       /*!< bit:      5  Generic Clock Generator Control 3 Synchronization Busy bits */
    uint32_t GENCTRL4:1;       /*!< bit:      6  Generic Clock Generator Control 4 Synchronization Busy bits */
    uint32_t GENCTRL5:1;       /*!< bit:      7  Generic Clock Generator Control 5 Synchronization Busy bits */
    uint32_t GENCTRL6:1;       /*!< bit:      8  Generic Clock Generator Control 6 Synchronization Busy bits */
    uint32_t GENCTRL7:1;       /*!< bit:      9  Generic Clock Generator Control 7 Synchronization Busy bits */
    uint32_t GENCTRL8:1;       /*!< bit:     10  Generic Clock Generator Control 8 Synchronization Busy bits */
    uint32_t GENCTRL9:1;       /*!< bit:     11  Generic Clock Generator Control 9 Synchronization Busy bits */
    uint32_t GENCTRL10:1;      /*!< bit:     12  Generic Clock Generator Control 10 Synchronization Busy bits */
    uint32_t GENCTRL11:1;      /*!< bit:     13  Generic Clock Generator Control 11 Synchronization Busy bits */
    uint32_t :18;              /*!< bit: 14..31  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  struct {
    uint32_t :2;               /*!< bit:  0.. 1  Reserved                           */
    uint32_t GENCTRL:12;       /*!< bit:  2..13  Generic Clock Generator Control x Synchronization Busy bits */
    uint32_t :18;              /*!< bit: 14..31  Reserved                           */
  } vec;                       /*!< Structure used for vec  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} GCLK_SYNCBUSY_Type;

typedef union {
  struct {
    uint32_t SRC:4;            /*!< bit:  0.. 3  Source Select                      */
    uint32_t :4;               /*!< bit:  4.. 7  Reserved                           */
    uint32_t GENEN:1;          /*!< bit:      8  Generic Clock Generator Enable     */
    uint32_t IDC:1;            /*!< bit:      9  Improve Duty Cycle                 */
    uint32_t OOV:1;            /*!< bit:     10  Output Off Value                   */
    uint32_t OE:1;             /*!< bit:     11  Output Enable                      */
    uint32_t DIVSEL:1;         /*!< bit:     12  Divide Selection                   */
    uint32_t RUNSTDBY:1;       /*!< bit:     13  Run in Standby                     */
    uint32_t :2;               /*!< bit: 14..15  Reserved                           */
    uint32_t DIV:16;           /*!< bit: 16..31  Division Factor                    */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} GCLK_GENCTRL_Type;

typedef union {
  struct {
    uint32_t GEN:4;            /*!< bit:  0.. 3  Generic Clock Generator            */
    uint32_t :2;               /*!< bit:  4.. 5  Reserved                           */
    uint32_t CHEN:1;           /*!< bit:      6  Channel Enable                     */
    uint32_t WRTLOCK:1;        /*!< bit:      7  Write Lock                         */
    uint32_t :24;              /*!< bit:  8..31  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} GCLK_PCHCTRL_Type;

typedef struct {
  __IO GCLK_CTRLA_Type           CTRLA;       /**< \brief Offset: 0x00 (R/W  8) Control */
       RoReg8                    Reserved1[0x3];
  __I  GCLK_SYNCBUSY_Type        SYNCBUSY;    /**< \brief Offset: 0x04 (R/  32) Synchronization Busy */
       RoReg8                    Reserved2[0x18];
  __IO GCLK_GENCTRL_Type         GENCTRL[12]; /**< \brief Offset: 0x20 (R/W 32) Generic Clock Generator Control */
       RoReg8                    Reserved3[0x30];
  __IO GCLK_PCHCTRL_Type         PCHCTRL[48]; /**< \brief Offset: 0x80 (R/W 32) Peripheral Clock Control */
} Gclk;

typedef union {
  struct {
    uint8_t  CKRDY:1;          /*!< bit:      0  Clock Ready Interrupt Enable       */
    uint8_t  :7;               /*!< bit:  1.. 7  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} MCLK_INTENCLR_Type;

typedef union {
  struct {
    uint8_t  CKRDY:1;          /*!< bit:      0  Clock Ready Interrupt Enable       */
    uint8_t  :7;               /*!< bit:  1.. 7  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} MCLK_INTENSET_Type;

typedef union { // __I to avoid read-modify-write on write-to-clear register
  struct {
    __I uint8_t  CKRDY:1;          /*!< bit:      0  Clock Ready                        */
    __I uint8_t  :7;               /*!< bit:  1.. 7  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} MCLK_INTFLAG_Type;

typedef union {
  struct {
    uint8_t  DIV:8;            /*!< bit:  0.. 7  CPU Clock Division Factor          */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} MCLK_HSDIV_Type;

typedef union {
  struct {
    uint8_t  DIV:8;            /*!< bit:  0.. 7  Low-Power Clock Division Factor    */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} MCLK_CPUDIV_Type;

typedef union {
  struct {
    uint32_t HPB0_:1;          /*!< bit:      0  HPB0 AHB Clock Mask                */
    uint32_t HPB1_:1;          /*!< bit:      1  HPB1 AHB Clock Mask                */
    uint32_t HPB2_:1;          /*!< bit:      2  HPB2 AHB Clock Mask                */
    uint32_t HPB3_:1;          /*!< bit:      3  HPB3 AHB Clock Mask                */
    uint32_t DSU_:1;           /*!< bit:      4  DSU AHB Clock Mask                 */
    uint32_t HMATRIX_:1;       /*!< bit:      5  HMATRIX AHB Clock Mask             */
    uint32_t NVMCTRL_:1;       /*!< bit:      6  NVMCTRL AHB Clock Mask             */
    uint32_t HSRAM_:1;         /*!< bit:      7  HSRAM AHB Clock Mask               */
    uint32_t CMCC_:1;          /*!< bit:      8  CMCC AHB Clock Mask                */
    uint32_t DMAC_:1;          /*!< bit:      9  DMAC AHB Clock Mask                */
    uint32_t USB_:1;           /*!< bit:     10  USB AHB Clock Mask                 */
    uint32_t BKUPRAM_:1;       /*!< bit:     11  BKUPRAM AHB Clock Mask             */
    uint32_t PAC_:1;           /*!< bit:     12  PAC AHB Clock Mask                 */
    uint32_t QSPI_:1;          /*!< bit:     13  QSPI AHB Clock Mask                */
    uint32_t :1;               /*!< bit:     14  Reserved                           */
    uint32_t SDHC0_:1;         /*!< bit:     15  SDHC0 AHB Clock Mask               */
    uint32_t SDHC1_:1;         /*!< bit:     16  SDHC1 AHB Clock Mask               */
    uint32_t :2;               /*!< bit: 17..18  Reserved                           */
    uint32_t ICM_:1;           /*!< bit:     19  ICM AHB Clock Mask                 */
    uint32_t PUKCC_:1;         /*!< bit:     20  PUKCC AHB Clock Mask               */
    uint32_t QSPI_2X_:1;       /*!< bit:     21  QSPI_2X AHB Clock Mask             */
    uint32_t NVMCTRL_SMEEPROM_:1; /*!< bit:     22  NVMCTRL_SMEEPROM AHB Clock Mask    */
    uint32_t NVMCTRL_CACHE_:1; /*!< bit:     23  NVMCTRL_CACHE AHB Clock Mask       */
    uint32_t :8;               /*!< bit: 24..31  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} MCLK_AHBMASK_Type;

typedef union {
  struct {
    uint32_t PAC_:1;           /*!< bit:      0  PAC APB Clock Enable               */
    uint32_t PM_:1;            /*!< bit:      1  PM APB Clock Enable                */
    uint32_t MCLK_:1;          /*!< bit:      2  MCLK APB Clock Enable              */
    uint32_t RSTC_:1;          /*!< bit:      3  RSTC APB Clock Enable              */
    uint32_t OSCCTRL_:1;       /*!< bit:      4  OSCCTRL APB Clock Enable           */
    uint32_t OSC32KCTRL_:1;    /*!< bit:      5  OSC32KCTRL APB Clock Enable        */
    uint32_t SUPC_:1;          /*!< bit:      6  SUPC APB Clock Enable              */
    uint32_t GCLK_:1;          /*!< bit:      7  GCLK APB Clock Enable              */
    uint32_t WDT_:1;           /*!< bit:      8  WDT APB Clock Enable               */
    uint32_t RTC_:1;           /*!< bit:      9  RTC APB Clock Enable               */
    uint32_t EIC_:1;           /*!< bit:     10  EIC APB Clock Enable               */
    uint32_t FREQM_:1;         /*!< bit:     11  FREQM APB Clock Enable             */
    uint32_t SERCOM0_:1;       /*!< bit:     12  SERCOM0 APB Clock Enable           */
    uint32_t SERCOM1_:1;       /*!< bit:     13  SERCOM1 APB Clock Enable           */
    uint32_t TC0_:1;           /*!< bit:     14  TC0 APB Clock Enable               */
    uint32_t TC1_:1;           /*!< bit:     15  TC1 APB Clock Enable               */
    uint32_t :16;              /*!< bit: 16..31  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} MCLK_APBAMASK_Type;

typedef union {
  struct {
    uint32_t USB_:1;           /*!< bit:      0  USB APB Clock Enable               */
    uint32_t DSU_:1;           /*!< bit:      1  DSU APB Clock Enable               */
    uint32_t NVMCTRL_:1;       /*!< bit:      2  NVMCTRL APB Clock Enable           */
    uint32_t :1;               /*!< bit:      3  Reserved                           */
    uint32_t PORT_:1;          /*!< bit:      4  PORT APB Clock Enable              */
    uint32_t :1;               /*!< bit:      5  Reserved                           */
    uint32_t HMATRIX_:1;       /*!< bit:      6  HMATRIX APB Clock Enable           */
    uint32_t EVSYS_:1;         /*!< bit:      7  EVSYS APB Clock Enable             */
    uint32_t :1;               /*!< bit:      8  Reserved                           */
    uint32_t SERCOM2_:1;       /*!< bit:      9  SERCOM2 APB Clock Enable           */
    uint32_t SERCOM3_:1;       /*!< bit:     10  SERCOM3 APB Clock Enable           */
    uint32_t TCC0_:1;          /*!< bit:     11  TCC0 APB Clock Enable              */
    uint32_t TCC1_:1;          /*!< bit:     12  TCC1 APB Clock Enable              */
    uint32_t TC2_:1;           /*!< bit:     13  TC2 APB Clock Enable               */
    uint32_t TC3_:1;           /*!< bit:     14  TC3 APB Clock Enable               */
    uint32_t TAL_:1;           /*!< bit:     15  TAL APB Clock Enable               */
    uint32_t RAMECC_:1;        /*!< bit:     16  RAMECC APB Clock Enable            */
    uint32_t :15;              /*!< bit: 17..31  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} MCLK_APBBMASK_Type;

typedef union {
  struct {
    uint32_t :3;               /*!< bit:  0.. 2  Reserved                           */
    uint32_t TCC2_:1;          /*!< bit:      3  TCC2 APB Clock Enable              */
    uint32_t TCC3_:1;          /*!< bit:      4  TCC3 APB Clock Enable              */
    uint32_t TC4_:1;           /*!< bit:      5  TC4 APB Clock Enable               */
    uint32_t TC5_:1;           /*!< bit:      6  TC5 APB Clock Enable               */
    uint32_t PDEC_:1;          /*!< bit:      7  PDEC APB Clock Enable              */
    uint32_t AC_:1;            /*!< bit:      8  AC APB Clock Enable                */
    uint32_t AES_:1;           /*!< bit:      9  AES APB Clock Enable               */
    uint32_t TRNG_:1;          /*!< bit:     10  TRNG APB Clock Enable              */
    uint32_t ICM_:1;           /*!< bit:     11  ICM APB Clock Enable               */
    uint32_t :1;               /*!< bit:     12  Reserved                           */
    uint32_t QSPI_:1;          /*!< bit:     13  QSPI APB Clock Enable              */
    uint32_t CCL_:1;           /*!< bit:     14  CCL APB Clock Enable               */
    uint32_t :17;              /*!< bit: 15..31  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} MCLK_APBCMASK_Type;

typedef union {
  struct {
    uint32_t SERCOM4_:1;       /*!< bit:      0  SERCOM4 APB Clock Enable           */
    uint32_t SERCOM5_:1;       /*!< bit:      1  SERCOM5 APB Clock Enable           */
    uint32_t SERCOM6_:1;       /*!< bit:      2  SERCOM6 APB Clock Enable           */
    uint32_t SERCOM7_:1;       /*!< bit:      3  SERCOM7 APB Clock Enable           */
    uint32_t TCC4_:1;          /*!< bit:      4  TCC4 APB Clock Enable              */
    uint32_t TC6_:1;           /*!< bit:      5  TC6 APB Clock Enable               */
    uint32_t TC7_:1;           /*!< bit:      6  TC7 APB Clock Enable               */
    uint32_t ADC0_:1;          /*!< bit:      7  ADC0 APB Clock Enable              */
    uint32_t ADC1_:1;          /*!< bit:      8  ADC1 APB Clock Enable              */
    uint32_t DAC_:1;           /*!< bit:      9  DAC APB Clock Enable               */
    uint32_t I2S_:1;           /*!< bit:     10  I2S APB Clock Enable               */
    uint32_t PCC_:1;           /*!< bit:     11  PCC APB Clock Enable               */
    uint32_t :20;              /*!< bit: 12..31  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} MCLK_APBDMASK_Type;

typedef struct {
       RoReg8                    Reserved1[0x1];
  __IO MCLK_INTENCLR_Type        INTENCLR;    /**< \brief Offset: 0x01 (R/W  8) Interrupt Enable Clear */
  __IO MCLK_INTENSET_Type        INTENSET;    /**< \brief Offset: 0x02 (R/W  8) Interrupt Enable Set */
  __IO MCLK_INTFLAG_Type         INTFLAG;     /**< \brief Offset: 0x03 (R/W  8) Interrupt Flag Status and Clear */
  __I  MCLK_HSDIV_Type           HSDIV;       /**< \brief Offset: 0x04 (R/   8) HS Clock Division */
  __IO MCLK_CPUDIV_Type          CPUDIV;      /**< \brief Offset: 0x05 (R/W  8) CPU Clock Division */
       RoReg8                    Reserved2[0xA];
  __IO MCLK_AHBMASK_Type         AHBMASK;     /**< \brief Offset: 0x10 (R/W 32) AHB Mask */
  __IO MCLK_APBAMASK_Type        APBAMASK;    /**< \brief Offset: 0x14 (R/W 32) APBA Mask */
  __IO MCLK_APBBMASK_Type        APBBMASK;    /**< \brief Offset: 0x18 (R/W 32) APBB Mask */
  __IO MCLK_APBCMASK_Type        APBCMASK;    /**< \brief Offset: 0x1C (R/W 32) APBC Mask */
  __IO MCLK_APBDMASK_Type        APBDMASK;    /**< \brief Offset: 0x20 (R/W 32) APBD Mask */
} Mclk;

typedef union {
  struct {
    uint8_t  SWRST:1;          /*!< bit:      0  Software Reset                     */
    uint8_t  ENABLE:1;         /*!< bit:      1  Enable                             */
    uint8_t  :4;               /*!< bit:  2.. 5  Reserved                           */
    uint8_t  RUNSTDBY:1;       /*!< bit:      6  Run in Standby                     */
    uint8_t  :1;               /*!< bit:      7  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} CCL_CTRL_Type;

typedef union {
  struct {
    uint8_t  SEQSEL:4;         /*!< bit:  0.. 3  Sequential Selection               */
    uint8_t  :4;               /*!< bit:  4.. 7  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} CCL_SEQCTRL_Type;

typedef union {
  struct {
    uint32_t :1;               /*!< bit:      0  Reserved                           */
    uint32_t ENABLE:1;         /*!< bit:      1  LUT Enable                         */
    uint32_t :2;               /*!< bit:  2.. 3  Reserved                           */
    uint32_t FILTSEL:2;        /*!< bit:  4.. 5  Filter Selection                   */
    uint32_t :1;               /*!< bit:      6  Reserved                           */
    uint32_t EDGESEL:1;        /*!< bit:      7  Edge Selection                     */
    uint32_t INSEL0:4;         /*!< bit:  8..11  Input Selection 0                  */
    uint32_t INSEL1:4;         /*!< bit: 12..15  Input Selection 1                  */
    uint32_t INSEL2:4;         /*!< bit: 16..19  Input Selection 2                  */
    uint32_t INVEI:1;          /*!< bit:     20  Inverted Event Input Enable        */
    uint32_t LUTEI:1;          /*!< bit:     21  LUT Event Input Enable             */
    uint32_t LUTEO:1;          /*!< bit:     22  LUT Event Output Enable            */
    uint32_t :1;               /*!< bit:     23  Reserved                           */
    uint32_t TRUTH:8;          /*!< bit: 24..31  Truth Value                        */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} CCL_LUTCTRL_Type;

typedef struct {
  __IO CCL_CTRL_Type             CTRL;        /**< \brief Offset: 0x0 (R/W  8) Control */
       RoReg8                    Reserved1[0x3];
  __IO CCL_SEQCTRL_Type          SEQCTRL[2];  /**< \brief Offset: 0x4 (R/W  8) SEQ Control x */
       RoReg8                    Reserved2[0x2];
  __IO CCL_LUTCTRL_Type          LUTCTRL[4];  /**< \brief Offset: 0x8 (R/W 32) LUT Control x */
} Ccl;

typedef union {
  struct {
    uint8_t  SWRST:1;          /*!< bit:      0  Software Reset                     */
    uint8_t  :7;               /*!< bit:  1.. 7  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} EVSYS_CTRLA_Type;

typedef union {
  struct {
    uint32_t CHANNEL0:1;       /*!< bit:      0  Channel 0 Software Selection       */
    uint32_t CHANNEL1:1;       /*!< bit:      1  Channel 1 Software Selection       */
    uint32_t CHANNEL2:1;       /*!< bit:      2  Channel 2 Software Selection       */
    uint32_t CHANNEL3:1;       /*!< bit:      3  Channel 3 Software Selection       */
    uint32_t CHANNEL4:1;       /*!< bit:      4  Channel 4 Software Selection       */
    uint32_t CHANNEL5:1;       /*!< bit:      5  Channel 5 Software Selection       */
    uint32_t CHANNEL6:1;       /*!< bit:      6  Channel 6 Software Selection       */
    uint32_t CHANNEL7:1;       /*!< bit:      7  Channel 7 Software Selection       */
    uint32_t CHANNEL8:1;       /*!< bit:      8  Channel 8 Software Selection       */
    uint32_t CHANNEL9:1;       /*!< bit:      9  Channel 9 Software Selection       */
    uint32_t CHANNEL10:1;      /*!< bit:     10  Channel 10 Software Selection      */
    uint32_t CHANNEL11:1;      /*!< bit:     11  Channel 11 Software Selection      */
    uint32_t CHANNEL12:1;      /*!< bit:     12  Channel 12 Software Selection      */
    uint32_t CHANNEL13:1;      /*!< bit:     13  Channel 13 Software Selection      */
    uint32_t CHANNEL14:1;      /*!< bit:     14  Channel 14 Software Selection      */
    uint32_t CHANNEL15:1;      /*!< bit:     15  Channel 15 Software Selection      */
    uint32_t CHANNEL16:1;      /*!< bit:     16  Channel 16 Software Selection      */
    uint32_t CHANNEL17:1;      /*!< bit:     17  Channel 17 Software Selection      */
    uint32_t CHANNEL18:1;      /*!< bit:     18  Channel 18 Software Selection      */
    uint32_t CHANNEL19:1;      /*!< bit:     19  Channel 19 Software Selection      */
    uint32_t CHANNEL20:1;      /*!< bit:     20  Channel 20 Software Selection      */
    uint32_t CHANNEL21:1;      /*!< bit:     21  Channel 21 Software Selection      */
    uint32_t CHANNEL22:1;      /*!< bit:     22  Channel 22 Software Selection      */
    uint32_t CHANNEL23:1;      /*!< bit:     23  Channel 23 Software Selection      */
    uint32_t CHANNEL24:1;      /*!< bit:     24  Channel 24 Software Selection      */
    uint32_t CHANNEL25:1;      /*!< bit:     25  Channel 25 Software Selection      */
    uint32_t CHANNEL26:1;      /*!< bit:     26  Channel 26 Software Selection      */
    uint32_t CHANNEL27:1;      /*!< bit:     27  Channel 27 Software Selection      */
    uint32_t CHANNEL28:1;      /*!< bit:     28  Channel 28 Software Selection      */
    uint32_t CHANNEL29:1;      /*!< bit:     29  Channel 29 Software Selection      */
    uint32_t CHANNEL30:1;      /*!< bit:     30  Channel 30 Software Selection      */
    uint32_t CHANNEL31:1;      /*!< bit:     31  Channel 31 Software Selection      */
  } bit;                       /*!< Structure used for bit  access                  */
  struct {
    uint32_t CHANNEL:32;       /*!< bit:  0..31  Channel x Software Selection       */
  } vec;                       /*!< Structure used for vec  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} EVSYS_SWEVT_Type;

typedef union {
  struct {
    uint8_t  PRI:4;            /*!< bit:  0.. 3  Channel Priority Number            */
    uint8_t  :3;               /*!< bit:  4.. 6  Reserved                           */
    uint8_t  RREN:1;           /*!< bit:      7  Round-Robin Scheduling Enable      */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} EVSYS_PRICTRL_Type;

typedef union {
  struct {
    uint16_t ID:4;             /*!< bit:  0.. 3  Channel ID                         */
    uint16_t :4;               /*!< bit:  4.. 7  Reserved                           */
    uint16_t OVR:1;            /*!< bit:      8  Channel Overrun                    */
    uint16_t EVD:1;            /*!< bit:      9  Channel Event Detected             */
    uint16_t :4;               /*!< bit: 10..13  Reserved                           */
    uint16_t READY:1;          /*!< bit:     14  Ready                              */
    uint16_t BUSY:1;           /*!< bit:     15  Busy                               */
  } bit;                       /*!< Structure used for bit  access                  */
  uint16_t reg;                /*!< Type      used for register access              */
} EVSYS_INTPEND_Type;

typedef union {
  struct {
    uint32_t CHINT0:1;         /*!< bit:      0  Channel 0 Pending Interrupt        */
    uint32_t CHINT1:1;         /*!< bit:      1  Channel 1 Pending Interrupt        */
    uint32_t CHINT2:1;         /*!< bit:      2  Channel 2 Pending Interrupt        */
    uint32_t CHINT3:1;         /*!< bit:      3  Channel 3 Pending Interrupt        */
    uint32_t CHINT4:1;         /*!< bit:      4  Channel 4 Pending Interrupt        */
    uint32_t CHINT5:1;         /*!< bit:      5  Channel 5 Pending Interrupt        */
    uint32_t CHINT6:1;         /*!< bit:      6  Channel 6 Pending Interrupt        */
    uint32_t CHINT7:1;         /*!< bit:      7  Channel 7 Pending Interrupt        */
    uint32_t CHINT8:1;         /*!< bit:      8  Channel 8 Pending Interrupt        */
    uint32_t CHINT9:1;         /*!< bit:      9  Channel 9 Pending Interrupt        */
    uint32_t CHINT10:1;        /*!< bit:     10  Channel 10 Pending Interrupt       */
    uint32_t CHINT11:1;        /*!< bit:     11  Channel 11 Pending Interrupt       */
    uint32_t :20;              /*!< bit: 12..31  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  struct {
    uint32_t CHINT:12;         /*!< bit:  0..11  Channel x Pending Interrupt        */
    uint32_t :20;              /*!< bit: 12..31  Reserved                           */
  } vec;                       /*!< Structure used for vec  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} EVSYS_INTSTATUS_Type;

typedef union {
  struct {
    uint32_t BUSYCH0:1;        /*!< bit:      0  Busy Channel 0                     */
    uint32_t BUSYCH1:1;        /*!< bit:      1  Busy Channel 1                     */
    uint32_t BUSYCH2:1;        /*!< bit:      2  Busy Channel 2                     */
    uint32_t BUSYCH3:1;        /*!< bit:      3  Busy Channel 3                     */
    uint32_t BUSYCH4:1;        /*!< bit:      4  Busy Channel 4                     */
    uint32_t BUSYCH5:1;        /*!< bit:      5  Busy Channel 5                     */
    uint32_t BUSYCH6:1;        /*!< bit:      6  Busy Channel 6                     */
    uint32_t BUSYCH7:1;        /*!< bit:      7  Busy Channel 7                     */
    uint32_t BUSYCH8:1;        /*!< bit:      8  Busy Channel 8                     */
    uint32_t BUSYCH9:1;        /*!< bit:      9  Busy Channel 9                     */
    uint32_t BUSYCH10:1;       /*!< bit:     10  Busy Channel 10                    */
    uint32_t BUSYCH11:1;       /*!< bit:     11  Busy Channel 11                    */
    uint32_t :20;              /*!< bit: 12..31  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  struct {
    uint32_t BUSYCH:12;        /*!< bit:  0..11  Busy Channel x                     */
    uint32_t :20;              /*!< bit: 12..31  Reserved                           */
  } vec;                       /*!< Structure used for vec  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} EVSYS_BUSYCH_Type;

typedef union {
  struct {
    uint32_t READYUSR0:1;      /*!< bit:      0  Ready User for Channel 0           */
    uint32_t READYUSR1:1;      /*!< bit:      1  Ready User for Channel 1           */
    uint32_t READYUSR2:1;      /*!< bit:      2  Ready User for Channel 2           */
    uint32_t READYUSR3:1;      /*!< bit:      3  Ready User for Channel 3           */
    uint32_t READYUSR4:1;      /*!< bit:      4  Ready User for Channel 4           */
    uint32_t READYUSR5:1;      /*!< bit:      5  Ready User for Channel 5           */
    uint32_t READYUSR6:1;      /*!< bit:      6  Ready User for Channel 6           */
    uint32_t READYUSR7:1;      /*!< bit:      7  Ready User for Channel 7           */
    uint32_t READYUSR8:1;      /*!< bit:      8  Ready User for Channel 8           */
    uint32_t READYUSR9:1;      /*!< bit:      9  Ready User for Channel 9           */
    uint32_t READYUSR10:1;     /*!< bit:     10  Ready User for Channel 10          */
    uint32_t READYUSR11:1;     /*!< bit:     11  Ready User for Channel 11          */
    uint32_t :20;              /*!< bit: 12..31  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  struct {
    uint32_t READYUSR:12;      /*!< bit:  0..11  Ready User for Channel x           */
    uint32_t :20;              /*!< bit: 12..31  Reserved                           */
  } vec;                       /*!< Structure used for vec  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} EVSYS_READYUSR_Type;

typedef union {
  struct {
    uint32_t EVGEN:7;          /*!< bit:  0.. 6  Event Generator Selection          */
    uint32_t :1;               /*!< bit:      7  Reserved                           */
    uint32_t PATH:2;           /*!< bit:  8.. 9  Path Selection                     */
    uint32_t EDGSEL:2;         /*!< bit: 10..11  Edge Detection Selection           */
    uint32_t :2;               /*!< bit: 12..13  Reserved                           */
    uint32_t RUNSTDBY:1;       /*!< bit:     14  Run in standby                     */
    uint32_t ONDEMAND:1;       /*!< bit:     15  Generic Clock On Demand            */
    uint32_t :16;              /*!< bit: 16..31  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} EVSYS_CHANNEL_Type;

typedef union {
  struct {
    uint8_t  OVR:1;            /*!< bit:      0  Channel Overrun Interrupt Disable  */
    uint8_t  EVD:1;            /*!< bit:      1  Channel Event Detected Interrupt Disable */
    uint8_t  :6;               /*!< bit:  2.. 7  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} EVSYS_CHINTENCLR_Type;

typedef union {
  struct {
    uint8_t  OVR:1;            /*!< bit:      0  Channel Overrun Interrupt Enable   */
    uint8_t  EVD:1;            /*!< bit:      1  Channel Event Detected Interrupt Enable */
    uint8_t  :6;               /*!< bit:  2.. 7  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} EVSYS_CHINTENSET_Type;

typedef union { // __I to avoid read-modify-write on write-to-clear register
  struct {
    __I uint8_t  OVR:1;            /*!< bit:      0  Channel Overrun                    */
    __I uint8_t  EVD:1;            /*!< bit:      1  Channel Event Detected             */
    __I uint8_t  :6;               /*!< bit:  2.. 7  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} EVSYS_CHINTFLAG_Type;

typedef union {
  struct {
    uint8_t  RDYUSR:1;         /*!< bit:      0  Ready User                         */
    uint8_t  BUSYCH:1;         /*!< bit:      1  Busy Channel                       */
    uint8_t  :6;               /*!< bit:  2.. 7  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} EVSYS_CHSTATUS_Type;

typedef union {
  struct {
    uint32_t CHANNEL:6;        /*!< bit:  0.. 5  Channel Event Selection            */
    uint32_t :26;              /*!< bit:  6..31  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} EVSYS_USER_Type;

typedef struct {
  __IO EVSYS_CHANNEL_Type        CHANNEL;     /**< \brief Offset: 0x000 (R/W 32) Channel n Control */
  __IO EVSYS_CHINTENCLR_Type     CHINTENCLR;  /**< \brief Offset: 0x004 (R/W  8) Channel n Interrupt Enable Clear */
  __IO EVSYS_CHINTENSET_Type     CHINTENSET;  /**< \brief Offset: 0x005 (R/W  8) Channel n Interrupt Enable Set */
  __IO EVSYS_CHINTFLAG_Type      CHINTFLAG;   /**< \brief Offset: 0x006 (R/W  8) Channel n Interrupt Flag Status and Clear */
  __I  EVSYS_CHSTATUS_Type       CHSTATUS;    /**< \brief Offset: 0x007 (R/   8) Channel n Status */
} EvsysChannel;

typedef struct {
  __IO EVSYS_CTRLA_Type          CTRLA;       /**< \brief Offset: 0x000 (R/W  8) Control */
       RoReg8                    Reserved1[0x3];
  __O  EVSYS_SWEVT_Type          SWEVT;       /**< \brief Offset: 0x004 ( /W 32) Software Event */
  __IO EVSYS_PRICTRL_Type        PRICTRL;     /**< \brief Offset: 0x008 (R/W  8) Priority Control */
       RoReg8                    Reserved2[0x7];
  __IO EVSYS_INTPEND_Type        INTPEND;     /**< \brief Offset: 0x010 (R/W 16) Channel Pending Interrupt */
       RoReg8                    Reserved3[0x2];
  __I  EVSYS_INTSTATUS_Type      INTSTATUS;   /**< \brief Offset: 0x014 (R/  32) Interrupt Status */
  __I  EVSYS_BUSYCH_Type         BUSYCH;      /**< \brief Offset: 0x018 (R/  32) Busy Channels */
  __I  EVSYS_READYUSR_Type       READYUSR;    /**< \brief Offset: 0x01C (R/  32) Ready Users */
       EvsysChannel              Channel[32]; /**< \brief Offset: 0x020 EvsysChannel groups [CHANNELS] */
  __IO EVSYS_USER_Type           USER[67];    /**< \brief Offset: 0x120 (R/W 32) User Multiplexer n */
} Evsys;

typedef union {
  struct {
    uint32_t SWRST:1;          /*!< bit:      0  Software Reset                     */
    uint32_t ENABLE:1;         /*!< bit:      1  Enable                             */
    uint32_t MODE:2;           /*!< bit:  2.. 3  Timer Counter Mode                 */
    uint32_t PRESCSYNC:2;      /*!< bit:  4.. 5  Prescaler and Counter Synchronization */
    uint32_t RUNSTDBY:1;       /*!< bit:      6  Run during Standby                 */
    uint32_t ONDEMAND:1;       /*!< bit:      7  Clock On Demand                    */
    uint32_t PRESCALER:3;      /*!< bit:  8..10  Prescaler                          */
    uint32_t ALOCK:1;          /*!< bit:     11  Auto Lock                          */
    uint32_t :4;               /*!< bit: 12..15  Reserved                           */
    uint32_t CAPTEN0:1;        /*!< bit:     16  Capture Channel 0 Enable           */
    uint32_t CAPTEN1:1;        /*!< bit:     17  Capture Channel 1 Enable           */
    uint32_t :2;               /*!< bit: 18..19  Reserved                           */
    uint32_t COPEN0:1;         /*!< bit:     20  Capture On Pin 0 Enable            */
    uint32_t COPEN1:1;         /*!< bit:     21  Capture On Pin 1 Enable            */
    uint32_t :2;               /*!< bit: 22..23  Reserved                           */
    uint32_t CAPTMODE0:2;      /*!< bit: 24..25  Capture Mode Channel 0             */
    uint32_t :1;               /*!< bit:     26  Reserved                           */
    uint32_t CAPTMODE1:2;      /*!< bit: 27..28  Capture mode Channel 1             */
    uint32_t :3;               /*!< bit: 29..31  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  struct {
    uint32_t :16;              /*!< bit:  0..15  Reserved                           */
    uint32_t CAPTEN:2;         /*!< bit: 16..17  Capture Channel x Enable           */
    uint32_t :2;               /*!< bit: 18..19  Reserved                           */
    uint32_t COPEN:2;          /*!< bit: 20..21  Capture On Pin x Enable            */
    uint32_t :10;              /*!< bit: 22..31  Reserved                           */
  } vec;                       /*!< Structure used for vec  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} TC_CTRLA_Type;

typedef union {
  struct {
    uint8_t  DIR:1;            /*!< bit:      0  Counter Direction                  */
    uint8_t  LUPD:1;           /*!< bit:      1  Lock Update                        */
    uint8_t  ONESHOT:1;        /*!< bit:      2  One-Shot on Counter                */
    uint8_t  :2;               /*!< bit:  3.. 4  Reserved                           */
    uint8_t  CMD:3;            /*!< bit:  5.. 7  Command                            */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} TC_CTRLBCLR_Type;

typedef union {
  struct {
    uint8_t  DIR:1;            /*!< bit:      0  Counter Direction                  */
    uint8_t  LUPD:1;           /*!< bit:      1  Lock Update                        */
    uint8_t  ONESHOT:1;        /*!< bit:      2  One-Shot on Counter                */
    uint8_t  :2;               /*!< bit:  3.. 4  Reserved                           */
    uint8_t  CMD:3;            /*!< bit:  5.. 7  Command                            */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} TC_CTRLBSET_Type;

typedef union {
  struct {
    uint16_t EVACT:3;          /*!< bit:  0.. 2  Event Action                       */
    uint16_t :1;               /*!< bit:      3  Reserved                           */
    uint16_t TCINV:1;          /*!< bit:      4  TC Event Input Polarity            */
    uint16_t TCEI:1;           /*!< bit:      5  TC Event Enable                    */
    uint16_t :2;               /*!< bit:  6.. 7  Reserved                           */
    uint16_t OVFEO:1;          /*!< bit:      8  Event Output Enable                */
    uint16_t :3;               /*!< bit:  9..11  Reserved                           */
    uint16_t MCEO0:1;          /*!< bit:     12  MC Event Output Enable 0           */
    uint16_t MCEO1:1;          /*!< bit:     13  MC Event Output Enable 1           */
    uint16_t :2;               /*!< bit: 14..15  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  struct {
    uint16_t :12;              /*!< bit:  0..11  Reserved                           */
    uint16_t MCEO:2;           /*!< bit: 12..13  MC Event Output Enable x           */
    uint16_t :2;               /*!< bit: 14..15  Reserved                           */
  } vec;                       /*!< Structure used for vec  access                  */
  uint16_t reg;                /*!< Type      used for register access              */
} TC_EVCTRL_Type;

typedef union {
  struct {
    uint8_t  OVF:1;            /*!< bit:      0  OVF Interrupt Disable              */
    uint8_t  ERR:1;            /*!< bit:      1  ERR Interrupt Disable              */
    uint8_t  :2;               /*!< bit:  2.. 3  Reserved                           */
    uint8_t  MC0:1;            /*!< bit:      4  MC Interrupt Disable 0             */
    uint8_t  MC1:1;            /*!< bit:      5  MC Interrupt Disable 1             */
    uint8_t  :2;               /*!< bit:  6.. 7  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  struct {
    uint8_t  :4;               /*!< bit:  0.. 3  Reserved                           */
    uint8_t  MC:2;             /*!< bit:  4.. 5  MC Interrupt Disable x             */
    uint8_t  :2;               /*!< bit:  6.. 7  Reserved                           */
  } vec;                       /*!< Structure used for vec  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} TC_INTENCLR_Type;

typedef union {
  struct {
    uint8_t  OVF:1;            /*!< bit:      0  OVF Interrupt Enable               */
    uint8_t  ERR:1;            /*!< bit:      1  ERR Interrupt Enable               */
    uint8_t  :2;               /*!< bit:  2.. 3  Reserved                           */
    uint8_t  MC0:1;            /*!< bit:      4  MC Interrupt Enable 0              */
    uint8_t  MC1:1;            /*!< bit:      5  MC Interrupt Enable 1              */
    uint8_t  :2;               /*!< bit:  6.. 7  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  struct {
    uint8_t  :4;               /*!< bit:  0.. 3  Reserved                           */
    uint8_t  MC:2;             /*!< bit:  4.. 5  MC Interrupt Enable x              */
    uint8_t  :2;               /*!< bit:  6.. 7  Reserved                           */
  } vec;                       /*!< Structure used for vec  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} TC_INTENSET_Type;

typedef union { // __I to avoid read-modify-write on write-to-clear register
  struct {
    __I uint8_t  OVF:1;            /*!< bit:      0  OVF Interrupt Flag                 */
    __I uint8_t  ERR:1;            /*!< bit:      1  ERR Interrupt Flag                 */
    __I uint8_t  :2;               /*!< bit:  2.. 3  Reserved                           */
    __I uint8_t  MC0:1;            /*!< bit:      4  MC Interrupt Flag 0                */
    __I uint8_t  MC1:1;            /*!< bit:      5  MC Interrupt Flag 1                */
    __I uint8_t  :2;               /*!< bit:  6.. 7  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  struct {
    __I uint8_t  :4;               /*!< bit:  0.. 3  Reserved                           */
    __I uint8_t  MC:2;             /*!< bit:  4.. 5  MC Interrupt Flag x                */
    __I uint8_t  :2;               /*!< bit:  6.. 7  Reserved                           */
  } vec;                       /*!< Structure used for vec  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} TC_INTFLAG_Type;

typedef union {
  struct {
    uint8_t  STOP:1;           /*!< bit:      0  Stop Status Flag                   */
    uint8_t  SLAVE:1;          /*!< bit:      1  Slave Status Flag                  */
    uint8_t  :1;               /*!< bit:      2  Reserved                           */
    uint8_t  PERBUFV:1;        /*!< bit:      3  Synchronization Busy Status        */
    uint8_t  CCBUFV0:1;        /*!< bit:      4  Compare channel buffer 0 valid     */
    uint8_t  CCBUFV1:1;        /*!< bit:      5  Compare channel buffer 1 valid     */
    uint8_t  :2;               /*!< bit:  6.. 7  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  struct {
    uint8_t  :4;               /*!< bit:  0.. 3  Reserved                           */
    uint8_t  CCBUFV:2;         /*!< bit:  4.. 5  Compare channel buffer x valid     */
    uint8_t  :2;               /*!< bit:  6.. 7  Reserved                           */
  } vec;                       /*!< Structure used for vec  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} TC_STATUS_Type;

typedef union {
  struct {
    uint8_t  WAVEGEN:2;        /*!< bit:  0.. 1  Waveform Generation Mode           */
    uint8_t  :6;               /*!< bit:  2.. 7  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} TC_WAVE_Type;

typedef union {
  struct {
    uint8_t  INVEN0:1;         /*!< bit:      0  Output Waveform Invert Enable 0    */
    uint8_t  INVEN1:1;         /*!< bit:      1  Output Waveform Invert Enable 1    */
    uint8_t  :6;               /*!< bit:  2.. 7  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  struct {
    uint8_t  INVEN:2;          /*!< bit:  0.. 1  Output Waveform Invert Enable x    */
    uint8_t  :6;               /*!< bit:  2.. 7  Reserved                           */
  } vec;                       /*!< Structure used for vec  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} TC_DRVCTRL_Type;

typedef union {
  struct {
    uint8_t  DBGRUN:1;         /*!< bit:      0  Run During Debug                   */
    uint8_t  :7;               /*!< bit:  1.. 7  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} TC_DBGCTRL_Type;

typedef union {
  struct {
    uint32_t SWRST:1;          /*!< bit:      0  swrst                              */
    uint32_t ENABLE:1;         /*!< bit:      1  enable                             */
    uint32_t CTRLB:1;          /*!< bit:      2  CTRLB                              */
    uint32_t STATUS:1;         /*!< bit:      3  STATUS                             */
    uint32_t COUNT:1;          /*!< bit:      4  Counter                            */
    uint32_t PER:1;            /*!< bit:      5  Period                             */
    uint32_t CC0:1;            /*!< bit:      6  Compare Channel 0                  */
    uint32_t CC1:1;            /*!< bit:      7  Compare Channel 1                  */
    uint32_t :24;              /*!< bit:  8..31  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  struct {
    uint32_t :6;               /*!< bit:  0.. 5  Reserved                           */
    uint32_t CC:2;             /*!< bit:  6.. 7  Compare Channel x                  */
    uint32_t :24;              /*!< bit:  8..31  Reserved                           */
  } vec;                       /*!< Structure used for vec  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} TC_SYNCBUSY_Type;

typedef union {
  struct {
    uint16_t COUNT:16;         /*!< bit:  0..15  Counter Value                      */
  } bit;                       /*!< Structure used for bit  access                  */
  uint16_t reg;                /*!< Type      used for register access              */
} TC_COUNT16_COUNT_Type;

typedef union {
  struct {
    uint32_t COUNT:32;         /*!< bit:  0..31  Counter Value                      */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} TC_COUNT32_COUNT_Type;

typedef union {
  struct {
    uint8_t  COUNT:8;          /*!< bit:  0.. 7  Counter Value                      */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} TC_COUNT8_COUNT_Type;

typedef union {
  struct {
    uint8_t  PER:8;            /*!< bit:  0.. 7  Period Value                       */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} TC_COUNT8_PER_Type;

typedef union {
  struct {
    uint16_t CC:16;            /*!< bit:  0..15  Counter/Compare Value              */
  } bit;                       /*!< Structure used for bit  access                  */
  uint16_t reg;                /*!< Type      used for register access              */
} TC_COUNT16_CC_Type;

typedef union {
  struct {
    uint32_t CC:32;            /*!< bit:  0..31  Counter/Compare Value              */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} TC_COUNT32_CC_Type;

typedef union {
  struct {
    uint8_t  CC:8;             /*!< bit:  0.. 7  Counter/Compare Value              */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} TC_COUNT8_CC_Type;

typedef union {
  struct {
    uint8_t  PERBUF:8;         /*!< bit:  0.. 7  Period Buffer Value                */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} TC_COUNT8_PERBUF_Type;

typedef union {
  struct {
    uint8_t  CCBUF:8;          /*!< bit:  0.. 7  Counter/Compare Buffer Value       */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} TC_COUNT8_CCBUF_Type;

typedef struct { /* 8-bit Counter Mode */
  __IO TC_CTRLA_Type             CTRLA;       /**< \brief Offset: 0x00 (R/W 32) Control A */
  __IO TC_CTRLBCLR_Type          CTRLBCLR;    /**< \brief Offset: 0x04 (R/W  8) Control B Clear */
  __IO TC_CTRLBSET_Type          CTRLBSET;    /**< \brief Offset: 0x05 (R/W  8) Control B Set */
  __IO TC_EVCTRL_Type            EVCTRL;      /**< \brief Offset: 0x06 (R/W 16) Event Control */
  __IO TC_INTENCLR_Type          INTENCLR;    /**< \brief Offset: 0x08 (R/W  8) Interrupt Enable Clear */
  __IO TC_INTENSET_Type          INTENSET;    /**< \brief Offset: 0x09 (R/W  8) Interrupt Enable Set */
  __IO TC_INTFLAG_Type           INTFLAG;     /**< \brief Offset: 0x0A (R/W  8) Interrupt Flag Status and Clear */
  __IO TC_STATUS_Type            STATUS;      /**< \brief Offset: 0x0B (R/W  8) Status */
  __IO TC_WAVE_Type              WAVE;        /**< \brief Offset: 0x0C (R/W  8) Waveform Generation Control */
  __IO TC_DRVCTRL_Type           DRVCTRL;     /**< \brief Offset: 0x0D (R/W  8) Control C */
       RoReg8                    Reserved1[0x1];
  __IO TC_DBGCTRL_Type           DBGCTRL;     /**< \brief Offset: 0x0F (R/W  8) Debug Control */
  __I  TC_SYNCBUSY_Type          SYNCBUSY;    /**< \brief Offset: 0x10 (R/  32) Synchronization Status */
  __IO TC_COUNT8_COUNT_Type      COUNT;       /**< \brief Offset: 0x14 (R/W  8) COUNT8 Count */
       RoReg8                    Reserved2[0x6];
  __IO TC_COUNT8_PER_Type        PER;         /**< \brief Offset: 0x1B (R/W  8) COUNT8 Period */
  __IO TC_COUNT8_CC_Type         CC[2];       /**< \brief Offset: 0x1C (R/W  8) COUNT8 Compare and Capture */
       RoReg8                    Reserved3[0x11];
  __IO TC_COUNT8_PERBUF_Type     PERBUF;      /**< \brief Offset: 0x2F (R/W  8) COUNT8 Period Buffer */
  __IO TC_COUNT8_CCBUF_Type      CCBUF[2];    /**< \brief Offset: 0x30 (R/W  8) COUNT8 Compare and Capture Buffer */
} TcCount8;

typedef union {
       TcCount8                  COUNT8;      /**< \brief Offset: 0x00 8-bit Counter Mode */
//       TcCount16                 COUNT16;     /**< \brief Offset: 0x00 16-bit Counter Mode */
//       TcCount32                 COUNT32;     /**< \brief Offset: 0x00 32-bit Counter Mode */
} Tc;

typedef union {
  struct {
    uint32_t DIR:32;           /*!< bit:  0..31  Port Data Direction                */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} PORT_DIR_Type;

typedef union {
  struct {
    uint32_t DIRCLR:32;        /*!< bit:  0..31  Port Data Direction Clear          */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} PORT_DIRCLR_Type;

typedef union {
  struct {
    uint32_t DIRSET:32;        /*!< bit:  0..31  Port Data Direction Set            */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} PORT_DIRSET_Type;

typedef union {
  struct {
    uint32_t DIRTGL:32;        /*!< bit:  0..31  Port Data Direction Toggle         */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} PORT_DIRTGL_Type;

typedef union {
  struct {
    uint32_t OUT:32;           /*!< bit:  0..31  PORT Data Output Value             */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} PORT_OUT_Type;

typedef union {
  struct {
    uint32_t OUTCLR:32;        /*!< bit:  0..31  PORT Data Output Value Clear       */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} PORT_OUTCLR_Type;

typedef union {
  struct {
    uint32_t OUTSET:32;        /*!< bit:  0..31  PORT Data Output Value Set         */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} PORT_OUTSET_Type;

typedef union {
  struct {
    uint32_t OUTTGL:32;        /*!< bit:  0..31  PORT Data Output Value Toggle      */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} PORT_OUTTGL_Type;

typedef union {
  struct {
    uint32_t IN:32;            /*!< bit:  0..31  PORT Data Input Value              */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} PORT_IN_Type;

typedef union {
  struct {
    uint32_t SAMPLING:32;      /*!< bit:  0..31  Input Sampling Mode                */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} PORT_CTRL_Type;

typedef union {
  struct {
    uint32_t PINMASK:16;       /*!< bit:  0..15  Pin Mask for Multiple Pin Configuration */
    uint32_t PMUXEN:1;         /*!< bit:     16  Peripheral Multiplexer Enable      */
    uint32_t INEN:1;           /*!< bit:     17  Input Enable                       */
    uint32_t PULLEN:1;         /*!< bit:     18  Pull Enable                        */
    uint32_t :3;               /*!< bit: 19..21  Reserved                           */
    uint32_t DRVSTR:1;         /*!< bit:     22  Output Driver Strength Selection   */
    uint32_t :1;               /*!< bit:     23  Reserved                           */
    uint32_t PMUX:4;           /*!< bit: 24..27  Peripheral Multiplexing            */
    uint32_t WRPMUX:1;         /*!< bit:     28  Write PMUX                         */
    uint32_t :1;               /*!< bit:     29  Reserved                           */
    uint32_t WRPINCFG:1;       /*!< bit:     30  Write PINCFG                       */
    uint32_t HWSEL:1;          /*!< bit:     31  Half-Word Select                   */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} PORT_WRCONFIG_Type;

typedef union {
  struct {
    uint32_t PID0:5;           /*!< bit:  0.. 4  PORT Event Pin Identifier 0        */
    uint32_t EVACT0:2;         /*!< bit:  5.. 6  PORT Event Action 0                */
    uint32_t PORTEI0:1;        /*!< bit:      7  PORT Event Input Enable 0          */
    uint32_t PID1:5;           /*!< bit:  8..12  PORT Event Pin Identifier 1        */
    uint32_t EVACT1:2;         /*!< bit: 13..14  PORT Event Action 1                */
    uint32_t PORTEI1:1;        /*!< bit:     15  PORT Event Input Enable 1          */
    uint32_t PID2:5;           /*!< bit: 16..20  PORT Event Pin Identifier 2        */
    uint32_t EVACT2:2;         /*!< bit: 21..22  PORT Event Action 2                */
    uint32_t PORTEI2:1;        /*!< bit:     23  PORT Event Input Enable 2          */
    uint32_t PID3:5;           /*!< bit: 24..28  PORT Event Pin Identifier 3        */
    uint32_t EVACT3:2;         /*!< bit: 29..30  PORT Event Action 3                */
    uint32_t PORTEI3:1;        /*!< bit:     31  PORT Event Input Enable 3          */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} PORT_EVCTRL_Type;

typedef union {
  struct {
    uint8_t  PMUXE:4;          /*!< bit:  0.. 3  Peripheral Multiplexing for Even-Numbered Pin */
    uint8_t  PMUXO:4;          /*!< bit:  4.. 7  Peripheral Multiplexing for Odd-Numbered Pin */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} PORT_PMUX_Type;

typedef union {
  struct {
    uint8_t  PMUXEN:1;         /*!< bit:      0  Peripheral Multiplexer Enable      */
    uint8_t  INEN:1;           /*!< bit:      1  Input Enable                       */
    uint8_t  PULLEN:1;         /*!< bit:      2  Pull Enable                        */
    uint8_t  :3;               /*!< bit:  3.. 5  Reserved                           */
    uint8_t  DRVSTR:1;         /*!< bit:      6  Output Driver Strength Selection   */
    uint8_t  :1;               /*!< bit:      7  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} PORT_PINCFG_Type;

typedef struct {
  __IO PORT_DIR_Type             DIR;         /**< \brief Offset: 0x00 (R/W 32) Data Direction */
  __IO PORT_DIRCLR_Type          DIRCLR;      /**< \brief Offset: 0x04 (R/W 32) Data Direction Clear */
  __IO PORT_DIRSET_Type          DIRSET;      /**< \brief Offset: 0x08 (R/W 32) Data Direction Set */
  __IO PORT_DIRTGL_Type          DIRTGL;      /**< \brief Offset: 0x0C (R/W 32) Data Direction Toggle */
  __IO PORT_OUT_Type             OUT;         /**< \brief Offset: 0x10 (R/W 32) Data Output Value */
  __IO PORT_OUTCLR_Type          OUTCLR;      /**< \brief Offset: 0x14 (R/W 32) Data Output Value Clear */
  __IO PORT_OUTSET_Type          OUTSET;      /**< \brief Offset: 0x18 (R/W 32) Data Output Value Set */
  __IO PORT_OUTTGL_Type          OUTTGL;      /**< \brief Offset: 0x1C (R/W 32) Data Output Value Toggle */
  __I  PORT_IN_Type              IN;          /**< \brief Offset: 0x20 (R/  32) Data Input Value */
  __IO PORT_CTRL_Type            CTRL;        /**< \brief Offset: 0x24 (R/W 32) Control */
  __O  PORT_WRCONFIG_Type        WRCONFIG;    /**< \brief Offset: 0x28 ( /W 32) Write Configuration */
  __IO PORT_EVCTRL_Type          EVCTRL;      /**< \brief Offset: 0x2C (R/W 32) Event Input Control */
  __IO PORT_PMUX_Type            PMUX[16];    /**< \brief Offset: 0x30 (R/W  8) Peripheral Multiplexing */
  __IO PORT_PINCFG_Type          PINCFG[32];  /**< \brief Offset: 0x40 (R/W  8) Pin Configuration */
       RoReg8                    Reserved1[0x20];
} PortGroup;

typedef struct {
       PortGroup                 Group[4];    /**< \brief Offset: 0x00 PortGroup groups [GROUPS] */
} Port;

typedef union {
  struct {
    uint32_t SWRST:1;          /*!< bit:      0  Software Reset                     */
    uint32_t ENABLE:1;         /*!< bit:      1  Enable                             */
    uint32_t MODE:3;           /*!< bit:  2.. 4  Operating Mode                     */
    uint32_t :2;               /*!< bit:  5.. 6  Reserved                           */
    uint32_t RUNSTDBY:1;       /*!< bit:      7  Run during Standby                 */
    uint32_t IBON:1;           /*!< bit:      8  Immediate Buffer Overflow Notification */
    uint32_t :7;               /*!< bit:  9..15  Reserved                           */
    uint32_t DOPO:2;           /*!< bit: 16..17  Data Out Pinout                    */
    uint32_t :2;               /*!< bit: 18..19  Reserved                           */
    uint32_t DIPO:2;           /*!< bit: 20..21  Data In Pinout                     */
    uint32_t :2;               /*!< bit: 22..23  Reserved                           */
    uint32_t FORM:4;           /*!< bit: 24..27  Frame Format                       */
    uint32_t CPHA:1;           /*!< bit:     28  Clock Phase                        */
    uint32_t CPOL:1;           /*!< bit:     29  Clock Polarity                     */
    uint32_t DORD:1;           /*!< bit:     30  Data Order                         */
    uint32_t :1;               /*!< bit:     31  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} SERCOM_SPI_CTRLA_Type;

typedef union {
  struct {
    uint32_t CHSIZE:3;         /*!< bit:  0.. 2  Character Size                     */
    uint32_t :3;               /*!< bit:  3.. 5  Reserved                           */
    uint32_t PLOADEN:1;        /*!< bit:      6  Data Preload Enable                */
    uint32_t :2;               /*!< bit:  7.. 8  Reserved                           */
    uint32_t SSDE:1;           /*!< bit:      9  Slave Select Low Detect Enable     */
    uint32_t :3;               /*!< bit: 10..12  Reserved                           */
    uint32_t MSSEN:1;          /*!< bit:     13  Master Slave Select Enable         */
    uint32_t AMODE:2;          /*!< bit: 14..15  Address Mode                       */
    uint32_t :1;               /*!< bit:     16  Reserved                           */
    uint32_t RXEN:1;           /*!< bit:     17  Receiver Enable                    */
    uint32_t :14;              /*!< bit: 18..31  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} SERCOM_SPI_CTRLB_Type;

typedef union {
  struct {
    uint32_t ICSPACE:6;        /*!< bit:  0.. 5  Inter-Character Spacing            */
    uint32_t :18;              /*!< bit:  6..23  Reserved                           */
    uint32_t DATA32B:1;        /*!< bit:     24  Data 32 Bit                        */
    uint32_t :7;               /*!< bit: 25..31  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} SERCOM_SPI_CTRLC_Type;

typedef union {
  struct {
    uint8_t  BAUD:8;           /*!< bit:  0.. 7  Baud Rate Value                    */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} SERCOM_SPI_BAUD_Type;

typedef union {
  struct {
    uint8_t  DRE:1;            /*!< bit:      0  Data Register Empty Interrupt Disable */
    uint8_t  TXC:1;            /*!< bit:      1  Transmit Complete Interrupt Disable */
    uint8_t  RXC:1;            /*!< bit:      2  Receive Complete Interrupt Disable */
    uint8_t  SSL:1;            /*!< bit:      3  Slave Select Low Interrupt Disable */
    uint8_t  :3;               /*!< bit:  4.. 6  Reserved                           */
    uint8_t  ERROR:1;          /*!< bit:      7  Combined Error Interrupt Disable   */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} SERCOM_SPI_INTENCLR_Type;

typedef union {
  struct {
    uint8_t  DRE:1;            /*!< bit:      0  Data Register Empty Interrupt Enable */
    uint8_t  TXC:1;            /*!< bit:      1  Transmit Complete Interrupt Enable */
    uint8_t  RXC:1;            /*!< bit:      2  Receive Complete Interrupt Enable  */
    uint8_t  SSL:1;            /*!< bit:      3  Slave Select Low Interrupt Enable  */
    uint8_t  :3;               /*!< bit:  4.. 6  Reserved                           */
    uint8_t  ERROR:1;          /*!< bit:      7  Combined Error Interrupt Enable    */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} SERCOM_SPI_INTENSET_Type;

typedef union { // __I to avoid read-modify-write on write-to-clear register
  struct {
    __I uint8_t  DRE:1;            /*!< bit:      0  Data Register Empty Interrupt      */
    __I uint8_t  TXC:1;            /*!< bit:      1  Transmit Complete Interrupt        */
    __I uint8_t  RXC:1;            /*!< bit:      2  Receive Complete Interrupt         */
    __I uint8_t  SSL:1;            /*!< bit:      3  Slave Select Low Interrupt Flag    */
    __I uint8_t  :3;               /*!< bit:  4.. 6  Reserved                           */
    __I uint8_t  ERROR:1;          /*!< bit:      7  Combined Error Interrupt           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} SERCOM_SPI_INTFLAG_Type;

typedef union {
  struct {
    uint16_t :2;               /*!< bit:  0.. 1  Reserved                           */
    uint16_t BUFOVF:1;         /*!< bit:      2  Buffer Overflow                    */
    uint16_t :8;               /*!< bit:  3..10  Reserved                           */
    uint16_t LENERR:1;         /*!< bit:     11  Transaction Length Error           */
    uint16_t :4;               /*!< bit: 12..15  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint16_t reg;                /*!< Type      used for register access              */
} SERCOM_SPI_STATUS_Type;

typedef union {
  struct {
    uint32_t SWRST:1;          /*!< bit:      0  Software Reset Synchronization Busy */
    uint32_t ENABLE:1;         /*!< bit:      1  SERCOM Enable Synchronization Busy */
    uint32_t CTRLB:1;          /*!< bit:      2  CTRLB Synchronization Busy         */
    uint32_t :1;               /*!< bit:      3  Reserved                           */
    uint32_t LENGTH:1;         /*!< bit:      4  LENGTH Synchronization Busy        */
    uint32_t :27;              /*!< bit:  5..31  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} SERCOM_SPI_SYNCBUSY_Type;

typedef union {
  struct {
    uint16_t LEN:8;            /*!< bit:  0.. 7  Data Length                        */
    uint16_t LENEN:1;          /*!< bit:      8  Data Length Enable                 */
    uint16_t :7;               /*!< bit:  9..15  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint16_t reg;                /*!< Type      used for register access              */
} SERCOM_SPI_LENGTH_Type;

typedef union {
  struct {
    uint32_t ADDR:8;           /*!< bit:  0.. 7  Address Value                      */
    uint32_t :8;               /*!< bit:  8..15  Reserved                           */
    uint32_t ADDRMASK:8;       /*!< bit: 16..23  Address Mask                       */
    uint32_t :8;               /*!< bit: 24..31  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} SERCOM_SPI_ADDR_Type;

typedef union {
  struct {
    uint32_t DATA:32;          /*!< bit:  0..31  Data Value                         */
  } bit;                       /*!< Structure used for bit  access                  */
  uint32_t reg;                /*!< Type      used for register access              */
} SERCOM_SPI_DATA_Type;

typedef union {
  struct {
    uint8_t  DBGSTOP:1;        /*!< bit:      0  Debug Mode                         */
    uint8_t  :7;               /*!< bit:  1.. 7  Reserved                           */
  } bit;                       /*!< Structure used for bit  access                  */
  uint8_t reg;                 /*!< Type      used for register access              */
} SERCOM_SPI_DBGCTRL_Type;

typedef struct { /* SPI Mode */
  __IO SERCOM_SPI_CTRLA_Type     CTRLA;       /**< \brief Offset: 0x00 (R/W 32) SPI Control A */
  __IO SERCOM_SPI_CTRLB_Type     CTRLB;       /**< \brief Offset: 0x04 (R/W 32) SPI Control B */
  __IO SERCOM_SPI_CTRLC_Type     CTRLC;       /**< \brief Offset: 0x08 (R/W 32) SPI Control C */
  __IO SERCOM_SPI_BAUD_Type      BAUD;        /**< \brief Offset: 0x0C (R/W  8) SPI Baud Rate */
       RoReg8                    Reserved1[0x7];
  __IO SERCOM_SPI_INTENCLR_Type  INTENCLR;    /**< \brief Offset: 0x14 (R/W  8) SPI Interrupt Enable Clear */
       RoReg8                    Reserved2[0x1];
  __IO SERCOM_SPI_INTENSET_Type  INTENSET;    /**< \brief Offset: 0x16 (R/W  8) SPI Interrupt Enable Set */
       RoReg8                    Reserved3[0x1];
  __IO SERCOM_SPI_INTFLAG_Type   INTFLAG;     /**< \brief Offset: 0x18 (R/W  8) SPI Interrupt Flag Status and Clear */
       RoReg8                    Reserved4[0x1];
  __IO SERCOM_SPI_STATUS_Type    STATUS;      /**< \brief Offset: 0x1A (R/W 16) SPI Status */
  __I  SERCOM_SPI_SYNCBUSY_Type  SYNCBUSY;    /**< \brief Offset: 0x1C (R/  32) SPI Synchronization Busy */
       RoReg8                    Reserved5[0x2];
  __IO SERCOM_SPI_LENGTH_Type    LENGTH;      /**< \brief Offset: 0x22 (R/W 16) SPI Length */
  __IO SERCOM_SPI_ADDR_Type      ADDR;        /**< \brief Offset: 0x24 (R/W 32) SPI Address */
  __IO SERCOM_SPI_DATA_Type      DATA;        /**< \brief Offset: 0x28 (R/W 32) SPI Data */
       RoReg8                    Reserved6[0x4];
  __IO SERCOM_SPI_DBGCTRL_Type   DBGCTRL;     /**< \brief Offset: 0x30 (R/W  8) SPI Debug Control */
} SercomSpi;

typedef union {
       SercomSpi                 SPI;         /**< \brief Offset: 0x00 SPI Mode */
} Sercom;

#define CCL               ((Ccl      *)0x42003800UL) /**< \brief (CCL) APB Base Address */
#define CCL_INST_NUM      1                          /**< \brief (CCL) Number of instances */
#define CCL_INSTS         { CCL }                    /**< \brief (CCL) Instances List */

#define EVSYS             ((Evsys    *)0x4100E000UL) /**< \brief (EVSYS) APB Base Address */
#define EVSYS_INST_NUM    1                          /**< \brief (EVSYS) Number of instances */
#define EVSYS_INSTS       { EVSYS }                  /**< \brief (EVSYS) Instances List */

#define GCLK              ((Gclk     *)0x40001C00UL) /**< \brief (GCLK) APB Base Address */
#define GCLK_INST_NUM     1                          /**< \brief (GCLK) Number of instances */
#define GCLK_INSTS        { GCLK }                   /**< \brief (GCLK) Instances List */

#define MCLK              ((Mclk     *)0x40000800UL) /**< \brief (MCLK) APB Base Address */
#define MCLK_INST_NUM     1                          /**< \brief (MCLK) Number of instances */
#define MCLK_INSTS        { MCLK }                   /**< \brief (MCLK) Instances List */

#define PORT              ((Port     *)0x41008000UL) /**< \brief (PORT) APB Base Address */
#define PORT_INST_NUM     1                          /**< \brief (PORT) Number of instances */
#define PORT_INSTS        { PORT }                   /**< \brief (PORT) Instances List */

#define SERCOM0           (0x40003000UL) /**< \brief (SERCOM0) APB Base Address */
#define SERCOM1           (0x40003400UL) /**< \brief (SERCOM1) APB Base Address */
#define SERCOM2           (0x41012000UL) /**< \brief (SERCOM2) APB Base Address */
#define SERCOM3           (0x41014000UL) /**< \brief (SERCOM3) APB Base Address */
#define SERCOM4           (0x43000000UL) /**< \brief (SERCOM4) APB Base Address */
#define SERCOM5           (0x43000400UL) /**< \brief (SERCOM5) APB Base Address */
#define SERCOM6           (0x43000800UL) /**< \brief (SERCOM6) APB Base Address */
#define SERCOM7           (0x43000C00UL) /**< \brief (SERCOM7) APB Base Address */
#define SERCOM_INST_NUM   8                          /**< \brief (SERCOM) Number of instances */
#define SERCOM_INSTS      { SERCOM0, SERCOM1, SERCOM2, SERCOM3, SERCOM4, SERCOM5, SERCOM6, SERCOM7 } /**< \brief (SERCOM) Instances List */

#define TC0               ((Tc       *)0x40003800UL) /**< \brief (TC0) APB Base Address */
#define TC1               ((Tc       *)0x40003C00UL) /**< \brief (TC1) APB Base Address */
#define TC2               ((Tc       *)0x4101A000UL) /**< \brief (TC2) APB Base Address */
#define TC3               ((Tc       *)0x4101C000UL) /**< \brief (TC3) APB Base Address */
#define TC4               ((Tc       *)0x42001400UL) /**< \brief (TC4) APB Base Address */
#define TC5               ((Tc       *)0x42001800UL) /**< \brief (TC5) APB Base Address */
#define TC6               ((Tc       *)0x43001400UL) /**< \brief (TC6) APB Base Address */
#define TC7               ((Tc       *)0x43001800UL) /**< \brief (TC7) APB Base Address */
#define TC_INST_NUM       8                          /**< \brief (TC) Number of instances */
#define TC_INSTS          { TC0, TC1, TC2, TC3, TC4, TC5, TC6, TC7 } /**< \brief (TC) Instances List */

typedef uint32_t SercomSpiTXPad;
typedef uint32_t SercomRXPad;
static constexpr SercomSpiTXPad SPI_PAD_3_SCK_1 = 2;
static constexpr SercomSpiTXPad SERCOM_RX_PAD_2 = 2;


#define SERCOM0_GCLK_ID_CORE        7       
#define SERCOM0_GCLK_ID_SLOW        3       
#define SERCOM1_GCLK_ID_CORE        8       
#define SERCOM1_GCLK_ID_SLOW        3       
#define SERCOM2_GCLK_ID_CORE        23      
#define SERCOM2_GCLK_ID_SLOW        3       
#define SERCOM3_GCLK_ID_CORE        24      
#define SERCOM3_GCLK_ID_SLOW        3       
#define SERCOM4_GCLK_ID_CORE        34      
#define SERCOM4_GCLK_ID_SLOW        3    
#define SERCOM5_GCLK_ID_CORE        35      
#define SERCOM5_GCLK_ID_SLOW        3       
#define SERCOM6_GCLK_ID_CORE        36      
#define SERCOM6_GCLK_ID_SLOW        3       
#define SERCOM7_GCLK_ID_CORE        37      
#define SERCOM7_GCLK_ID_SLOW        3       


#define SERCOM0_DMAC_ID_RX          4        // Index of DMA RX trigger
#define SERCOM0_DMAC_ID_TX          5        // Index of DMA TX trigger
#define SERCOM1_DMAC_ID_RX          6        // Index of DMA RX trigger
#define SERCOM1_DMAC_ID_TX          7        // Index of DMA TX trigger
#define SERCOM2_DMAC_ID_RX          8        // Index of DMA RX trigger
#define SERCOM2_DMAC_ID_TX          9        // Index of DMA TX trigger
#define SERCOM3_DMAC_ID_RX          10       // Index of DMA RX trigger
#define SERCOM3_DMAC_ID_TX          11       // Index of DMA TX trigger
#define SERCOM4_DMAC_ID_RX          12       // Index of DMA RX trigger
#define SERCOM4_DMAC_ID_TX          13       // Index of DMA TX trigger
#define SERCOM5_DMAC_ID_RX          14       // Index of DMA RX trigger
#define SERCOM5_DMAC_ID_TX          15       // Index of DMA TX trigger
#define SERCOM6_DMAC_ID_RX          16       // Index of DMA RX trigger
#define SERCOM6_DMAC_ID_TX          17       // Index of DMA TX trigger
#define SERCOM7_DMAC_ID_RX          18       // Index of DMA RX trigger
#define SERCOM7_DMAC_ID_TX          19       // Index of DMA TX trigger

#define SERCOM0_3_IRQn SERCOM0_OTHER_IRQn
#define SERCOM1_3_IRQn SERCOM1_OTHER_IRQn
#define SERCOM2_3_IRQn SERCOM2_OTHER_IRQn
#define SERCOM3_3_IRQn SERCOM3_OTHER_IRQn
#define SERCOM4_3_IRQn SERCOM4_OTHER_IRQn
#define SERCOM5_3_IRQn SERCOM5_OTHER_IRQn
#define SERCOM6_3_IRQn SERCOM6_OTHER_IRQn
#define SERCOM7_3_IRQn SERCOM7_OTHER_IRQn

#endif //SAMD51_ARDUINO_COMPAT_HPP__