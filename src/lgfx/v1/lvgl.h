#ifndef M5GFX_LVGL_FONT_COMPAT_H
#define M5GFX_LVGL_FONT_COMPAT_H

/*
 * M5GFX LVGL compatibility shim.
 *
 */

#if defined(__has_include)
    #if __has_include("lvgl/lvgl.h")
        #include "lvgl/lvgl.h"
        #define M5GFX_USING_REAL_LVGL 1
    #endif
#endif

#ifndef M5GFX_USING_REAL_LVGL

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif



/*Enable handling large font and/or fonts with a lot of characters.
 *The limit depends on the font size, font face and bpp.
 *Compiler error will be triggered if a font needs it.*/
#ifndef LV_FONT_FMT_TXT_LARGE
#define LV_FONT_FMT_TXT_LARGE 0
#endif

#ifndef LVGL_VERSION_MAJOR
#define LVGL_VERSION_MAJOR 9
#endif
#ifndef LVGL_VERSION_MINOR
#define LVGL_VERSION_MINOR 3
#endif
#ifndef LVGL_VERSION_PATCH
#define LVGL_VERSION_PATCH 0
#endif

#ifndef LV_VERSION_CHECK
#define LV_VERSION_CHECK(x, y, z) \
    ((LVGL_VERSION_MAJOR > (x)) || \
    ((LVGL_VERSION_MAJOR == (x)) && (LVGL_VERSION_MINOR > (y))) || \
    ((LVGL_VERSION_MAJOR == (x)) && (LVGL_VERSION_MINOR == (y)) && (LVGL_VERSION_PATCH >= (z))))
#endif

#ifndef LV_ATTRIBUTE_LARGE_CONST
#define LV_ATTRIBUTE_LARGE_CONST
#endif

#ifndef LV_FONT_SUBPX_NONE
#define LV_FONT_SUBPX_NONE 0
#endif

#include "lv_font/font.h"
#include "lv_font/font_fmt_txt.h"

#ifdef __cplusplus
}
#endif

#endif /* M5GFX_USING_REAL_LVGL */

#endif /* M5GFX_LVGL_COMPAT_H */
