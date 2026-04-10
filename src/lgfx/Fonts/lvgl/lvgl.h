#pragma once

#include "lgfx/v1/lvgl.h"

/*
 * Forward declarations for built-in LVGL font data.
 * These ensure external linkage when .c files are compiled as C++ (-xc++).
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if LVGL_VERSION_MAJOR >= 8
#define DEF_EXTERN_CONST_LV_FONT_T extern const lv_font_t
#else
#define DEF_EXTERN_CONST_LV_FONT_T extern lv_font_t
#endif

DEF_EXTERN_CONST_LV_FONT_T lv_font_montserrat_8;
DEF_EXTERN_CONST_LV_FONT_T lv_font_montserrat_10;
DEF_EXTERN_CONST_LV_FONT_T lv_font_montserrat_12;
DEF_EXTERN_CONST_LV_FONT_T lv_font_montserrat_14;
DEF_EXTERN_CONST_LV_FONT_T lv_font_montserrat_16;
DEF_EXTERN_CONST_LV_FONT_T lv_font_montserrat_18;
DEF_EXTERN_CONST_LV_FONT_T lv_font_montserrat_20;
DEF_EXTERN_CONST_LV_FONT_T lv_font_montserrat_22;
DEF_EXTERN_CONST_LV_FONT_T lv_font_montserrat_24;
DEF_EXTERN_CONST_LV_FONT_T lv_font_montserrat_26;
DEF_EXTERN_CONST_LV_FONT_T lv_font_montserrat_28;
DEF_EXTERN_CONST_LV_FONT_T lv_font_montserrat_28_compressed;
DEF_EXTERN_CONST_LV_FONT_T lv_font_montserrat_30;
DEF_EXTERN_CONST_LV_FONT_T lv_font_montserrat_32;
DEF_EXTERN_CONST_LV_FONT_T lv_font_montserrat_34;
DEF_EXTERN_CONST_LV_FONT_T lv_font_montserrat_36;
DEF_EXTERN_CONST_LV_FONT_T lv_font_montserrat_38;
DEF_EXTERN_CONST_LV_FONT_T lv_font_montserrat_40;
DEF_EXTERN_CONST_LV_FONT_T lv_font_montserrat_42;
DEF_EXTERN_CONST_LV_FONT_T lv_font_montserrat_44;
DEF_EXTERN_CONST_LV_FONT_T lv_font_montserrat_46;
DEF_EXTERN_CONST_LV_FONT_T lv_font_montserrat_48;
DEF_EXTERN_CONST_LV_FONT_T lv_font_simsun_14_cjk;
DEF_EXTERN_CONST_LV_FONT_T lv_font_simsun_16_cjk;
DEF_EXTERN_CONST_LV_FONT_T lv_font_unscii_8;
DEF_EXTERN_CONST_LV_FONT_T lv_font_unscii_16;

#undef DEF_EXTERN_CONST_LV_FONT_T

#ifdef __cplusplus
}
#endif /* __cplusplus */
