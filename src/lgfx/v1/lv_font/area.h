#ifndef M5GFX_LVGL_AREA_H
#define M5GFX_LVGL_AREA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

/** Represents an area of the screen.*/
typedef struct {
    int32_t x1;
    int32_t y1;
    int32_t x2;
    int32_t y2;
} lv_area_t;

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* M5GFX_LVGL_AREA_H */
