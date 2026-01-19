/*----------------------------------------------------------------------------/
 *  Lovyan GFX - Graphics library for embedded devices.
 *
 * Original Source:
 * https://github.com/lovyan03/LovyanGFX/
 *
 * Licence:
 * [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)
 *
 * Author:
 * [lovyan03](https://twitter.com/lovyan03)
 *
 * Contributors:
 * [ciniml](https://github.com/ciniml)
 * [mongonta0716](https://github.com/mongonta0716)
 * [tobozo](https://github.com/tobozo)
 * /----------------------------------------------------------------------------*/
#pragma once

#if defined (ESP_PLATFORM)

#include "Panel_LCD.hpp"


namespace lgfx
{
    inline namespace v1
    {
        //----------------------------------------------------------------------------

        struct Panel_NV3041A : public Panel_LCD
        {
        protected:

            static constexpr uint8_t CMD_RST_DELAY    = 120 ;   ///< delay ms wait for reset finish
            static constexpr uint8_t CMD_SLPIN_DELAY  = 120 ;   ///< delay ms wait for sleep in finish
            static constexpr uint8_t CMD_SLPOUT_DELAY = 120 ;   ///< delay ms wait for sleep out finish
            static constexpr uint8_t CMD_NOP          = 0x00;
            static constexpr uint8_t CMD_SWRESET      = 0x01;
            static constexpr uint8_t CMD_SLPIN        = 0x10;
            static constexpr uint8_t CMD_SLPOUT       = 0x11;
            static constexpr uint8_t CMD_INVOFF       = 0x20;
            static constexpr uint8_t CMD_INVON        = 0x21;
            static constexpr uint8_t CMD_DISPOFF      = 0x28;
            static constexpr uint8_t CMD_DISPON       = 0x29;
            static constexpr uint8_t CMD_CASET        = 0x2A;
            static constexpr uint8_t CMD_RASET        = 0x2B;
            static constexpr uint8_t CMD_RAMWR        = 0x2C;
            static constexpr uint8_t CMD_MADCTL       = 0x36;
            static constexpr uint8_t CMD_COLMOD       = 0x3A;
            static constexpr uint8_t CMD_GATEON       = 0x51;
            static constexpr uint8_t CMD_MADCTL_MY    = 0x80;
            static constexpr uint8_t CMD_MADCTL_MX    = 0x40;
            static constexpr uint8_t CMD_MADCTL_MV    = 0x20;
            static constexpr uint8_t CMD_MADCTL_ML    = 0x10;
            static constexpr uint8_t CMD_MADCTL_RGB   = 0x0 ;
            static constexpr uint8_t CMD_MADCTL_BGR   = 0x08;


            static constexpr uint8_t init_cmds[91*2] =
            {
                // init sequence grabbed from Arduino_GFX (kudos to @moononournation)
                // 91 pairs, delay after = 120
                0xff, 0xa5,
                0x36, 0xc0,
                0x3A, 0x01, // 01---565，00---666
                0x41, 0x03, // 01--8bit, 03-16bit
                0x44, 0x15, // VBP  ????? 21
                0x45, 0x15, // VFP  ????? 21
                0x7d, 0x03, // vdds_trim[2:0]
                0xc1, 0xbb, // avdd_clp_en avdd_clp[1:0] avcl_clp_en avcl_clp[1:0],  0xbb	 88		  a2
                0xc2, 0x05, // vgl_clp_en vgl_clp[2:0]
                0xc3, 0x10, // vgl_clp_en vgl_clp[2:0]
                0xc6, 0x3e, // avdd_ratio_sel avcl_ratio_sel vgh_ratio_sel[1:0] vgl_ratio_sel[1:0] = 35
                0xc7, 0x25, // mv_clk_sel[1:0] avdd_clk_sel[1:0] avcl_clk_sel[1:0] = 2e
                0xc8, 0x11, //	VGL_CLK_sel
                0x7a, 0x5f, //	user_vgsp .. 4f:0.8V		3f:1.04V	5f
                0x6f, 0x44, //	user_gvdd .. 1C:5.61	  5f	 53		   2a	    3a
                0x78, 0x70, //	user_gvcl .. 50:-3.22	  75			58	     	66
                0xc9, 0x00,
                0x67, 0x21,
                0x51, 0x0a, // gate_st_o[7:0]
                0x52, 0x76, // gate_ed_o[7:0] .. 76
                0x53, 0x0a, // gate_st_e[7:0] .. 76
                0x54, 0x76, // gate_ed_e[7:0]
                0x46, 0x0a, // fsm_hbp_o[5:0]
                0x47, 0x2a, // fsm_hfp_o[5:0]
                0x48, 0x0a, // fsm_hbp_e[5:0]
                0x49, 0x1a, // fsm_hfp_e[5:0]
                0x56, 0x43, // src_ld_wd[1:0] src_ld_st[5:0]
                0x57, 0x42, // pn_cs_en src_cs_st[5:0]
                0x58, 0x3c, // src_cs_p_wd[6:0]
                0x59, 0x64, // src_cs_n_wd[6:0]
                0x5a, 0x41, // src_pchg_st_o[6:0] .. 41
                0x5b, 0x3c, // src_pchg_wd_o[6:0]
                0x5c, 0x02, // src_pchg_st_e[6:0] .. 02
                0x5d, 0x3c, // src_pchg_wd_e[6:0] .. 3c
                0x5e, 0x1f, // src_pol_sw[7:0]
                0x60, 0x80, // src_op_st_o[7:0]
                0x61, 0x3f, // src_op_st_e[7:0]
                0x62, 0x21, // src_op_ed_o[9:8] src_op_ed_e[9:8]
                0x63, 0x07, // src_op_ed_o[7:0]
                0x64, 0xe0, // src_op_ed_e[7:0]
                0x65, 0x02, // chopper
                0xca, 0x20, // avdd_mux_st_o[7:0]
                0xcb, 0x52, // avdd_mux_ed_o[7:0] .. 52
                0xcc, 0x10, // avdd_mux_st_e[7:0]
                0xcD, 0x42, // avdd_mux_ed_e[7:0]
                0xD0, 0x20, // avcl_mux_st_o[7:0]
                0xD1, 0x52, // avcl_mux_ed_o[7:0]
                0xD2, 0x10, // avcl_mux_st_e[7:0]
                0xD3, 0x42, // avcl_mux_ed_e[7:0]
                0xD4, 0x0a, // vgh_mux_st[7:0]
                0xD5, 0x32, // vgh_mux_ed[7:0]
                ////gammma  weihuan pianguangpian 0913
                0x80, 0x00, // gam_vrp0	0					6bit
                0xA0, 0x00, // gam_VRN0		 0-
                0x81, 0x07, // gam_vrp1	1				   6bit
                0xA1, 0x06, // gam_VRN1		 1-
                0x82, 0x02, // gam_vrp2	 2					6bit
                0xA2, 0x01, // gam_VRN2		 2-
                0x86, 0x11, // gam_prp0	 7bit	8			7bit .. 33
                0xA6, 0x10, // gam_PRN0	 	8- .. 2a
                0x87, 0x27, // gam_prp1	 7bit	 40			 7bit .. 2d
                0xA7, 0x27, // gam_PRN1	 	40- .. 2d
                0x83, 0x37, // gam_vrp3	 61				 6bit
                0xA3, 0x37, // gam_VRN3		61-
                0x84, 0x35, // gam_vrp4	  62			 6bit
                0xA4, 0x35, // gam_VRN4		62-
                0x85, 0x3f, // gam_vrp5	  63			 6bit
                0xA5, 0x3f, // gam_VRN5		63-
                0x88, 0x0b, // gam_pkp0	  	 4			   5bit .. 0b
                0xA8, 0x0b, // gam_PKN0		4- .. 0b
                0x89, 0x14, // gam_pkp1	  5					5bit .. 14
                0xA9, 0x14, // gam_PKN1		5- .. 14
                0x8a, 0x1a, // gam_pkp2	  7					 5bit .. 1a
                0xAa, 0x1a, // gam_PKN2		7- .. 1a
                0x8b, 0x0a, // gam_PKP3	  10				 5bit
                0xAb, 0x0a, // gam_PKN3		10-
                0x8c, 0x14, // gam_PKP4	   16				 5bit
                0xAc, 0x08, // gam_PKN4		16-
                0x8d, 0x17, // gam_PKP5		22				 5bit
                0xAd, 0x07, // gam_PKN5		22-
                0x8e, 0x16, // gam_PKP6		28				 5bit .. 16 change
                0xAe, 0x06, // gam_PKN6		28- .. 13change
                0x8f, 0x1B, // gam_PKP7		34				  5bit
                0xAf, 0x07, // gam_PKN7		34-
                0x90, 0x04, // gam_PKP8		 46				   5bit
                0xB0, 0x04, // gam_PKN8		46-
                0x91, 0x0A, // gam_PKP9		 52					5bit
                0xB1, 0x0A, // gam_PKN9		52-
                0x92, 0x16, // gam_PKP10		58					5bit
                0xB2, 0x15, // gam_PKN10		58-
                0xff, 0x00,
                0x11, 0x00,

            };

        public:
            Panel_NV3041A(void)
            {
                _cfg.memory_width  = _cfg.panel_width  = 480;
                _cfg.memory_height = _cfg.panel_height = 272;
            }

            bool init(bool use_reset) override;
            void beginTransaction(void) override;
            void endTransaction(void) override;

            color_depth_t setColorDepth(color_depth_t depth) override;
            //void setRotation(uint_fast8_t r) override;
            void setInvert(bool invert) override;
            void setSleep(bool flg) override;
            void setPowerSave(bool flg) override;

            void waitDisplay(void) override;
            bool displayBusy(void) override;

            void writePixels(pixelcopy_t* param, uint32_t len, bool use_dma) override;
            void writeBlock(uint32_t rawcolor, uint32_t len) override;

            void setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye) override;
            void drawPixelPreclipped(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor) override;
            void writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor) override;
            void writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool use_dma) override;

            uint32_t readCommand(uint_fast16_t cmd, uint_fast8_t index, uint_fast8_t len) override;
            uint32_t readData(uint_fast8_t index, uint_fast8_t len) override;
            void readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param) override;

        protected:
            bool _in_transaction = false;

            void update_madctl(void) override;
            void write_cmd(uint8_t cmd);
            void start_qspi();
            void end_qspi();
            void write_bytes(const uint8_t* data, uint32_t len, bool use_dma);
        };

        //----------------------------------------------------------------------------
    }
}


#endif
