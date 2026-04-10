#ifndef M5GFX_LVGL_DRAW_BUF_H
#define M5GFX_LVGL_DRAW_BUF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "types.h"
#include "color.h"
#include "area.h"

typedef void * (*lv_draw_buf_malloc_cb)(size_t size, lv_color_format_t color_format);

typedef void (*lv_draw_buf_free_cb)(void * draw_buf);

typedef void * (*lv_draw_buf_align_cb)(void * buf, lv_color_format_t color_format);

typedef void (*lv_draw_buf_cache_operation_cb)(const lv_draw_buf_t * draw_buf, const lv_area_t * area);

typedef uint32_t (*lv_draw_buf_width_to_stride_cb)(uint32_t w, lv_color_format_t color_format);

#if LV_BIG_ENDIAN_SYSTEM
typedef struct {
    uint32_t reserved_2: 16;    /**< Reserved to be used later*/
    uint32_t stride: 16;        /**< Number of bytes in a row*/
    uint32_t h: 16;
    uint32_t w: 16;
    uint32_t flags: 16;         /**< Image flags, see `lv_image_flags_t`*/
    uint32_t cf : 8;            /**< Color format: See `lv_color_format_t`*/
    uint32_t magic: 8;          /**< Magic number. Must be LV_IMAGE_HEADER_MAGIC*/
} lv_image_header_t;
#else
typedef struct {
    uint32_t magic: 8;          /**< Magic number. Must be LV_IMAGE_HEADER_MAGIC*/
    uint32_t cf : 8;            /**< Color format: See `lv_color_format_t`*/
    uint32_t flags: 16;         /**< Image flags, see `lv_image_flags_t`*/

    uint32_t w: 16;
    uint32_t h: 16;
    uint32_t stride: 16;        /**< Number of bytes in a row*/
    uint32_t reserved_2: 16;    /**< Reserved to be used later*/
} lv_image_header_t;
#endif

struct _lv_draw_buf_t {
    lv_image_header_t header;
    uint32_t data_size;       /**< Total buf size in bytes */
    uint8_t * data;
    void * unaligned_data;    /**< Unaligned address of `data`, used internally by lvgl */
    const lv_draw_buf_handlers_t * handlers; /**< draw buffer alloc/free ops. */
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*M5GFX_LVGL_DRAW_BUF_H*/
