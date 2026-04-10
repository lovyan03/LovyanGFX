#ifndef M5GFX_LVGL_DRAW_BUF_PRIVATE_H
#define M5GFX_LVGL_DRAW_BUF_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "draw_buf.h"

struct _lv_draw_buf_handlers_t {
    lv_draw_buf_malloc_cb buf_malloc_cb;
    lv_draw_buf_free_cb buf_free_cb;
    lv_draw_buf_align_cb align_pointer_cb;
    lv_draw_buf_cache_operation_cb invalidate_cache_cb;
    lv_draw_buf_cache_operation_cb flush_cache_cb;
    lv_draw_buf_width_to_stride_cb width_to_stride_cb;
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*M5GFX_LVGL_DRAW_BUF_PRIVATE_H*/
