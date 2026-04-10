
#ifndef M5GFX_LVGL_TYPES_COMPAT_H
#define M5GFX_LVGL_TYPES_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__cplusplus) || __STDC_VERSION__ >= 199901L
/*If c99 or newer,  use the definition of uintptr_t directly from <stdint.h>*/
typedef uintptr_t lv_uintptr_t;
typedef intptr_t lv_intptr_t;

#else

/*Otherwise, use the arch size determination*/
#ifdef LV_ARCH_64
typedef uint64_t lv_uintptr_t;
typedef int64_t lv_intptr_t;
#else
typedef uint32_t lv_uintptr_t;
typedef int32_t lv_intptr_t;
#endif

#endif

typedef struct _lv_font_t lv_font_t;

typedef struct _lv_draw_buf_handlers_t lv_draw_buf_handlers_t;

typedef struct _lv_draw_buf_t lv_draw_buf_t;

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
