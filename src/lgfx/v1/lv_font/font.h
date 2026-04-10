#ifndef M5GFX_LVGL_FONT_H
#define M5GFX_LVGL_FONT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include "draw_buf.h"
#include "area.h"

/** The font format.*/
typedef enum {
    LV_FONT_GLYPH_FORMAT_NONE   = 0, /**< Maybe not visible*/

    /**< Legacy simple formats with no byte padding at end of the lines*/
    LV_FONT_GLYPH_FORMAT_A1     = 0x01, /**< 1 bit per pixel*/
    LV_FONT_GLYPH_FORMAT_A2     = 0x02, /**< 2 bit per pixel*/
    LV_FONT_GLYPH_FORMAT_A3     = 0x03, /**< 3 bit per pixel*/
    LV_FONT_GLYPH_FORMAT_A4     = 0x04, /**< 4 bit per pixel*/
    LV_FONT_GLYPH_FORMAT_A8     = 0x08, /**< 8 bit per pixel*/

    /**< Legacy simple formats with byte padding at end of the lines*/
    LV_FONT_GLYPH_FORMAT_A1_ALIGNED = 0x011, /**< 1 bit per pixel*/
    LV_FONT_GLYPH_FORMAT_A2_ALIGNED = 0x012, /**< 2 bit per pixel*/
    LV_FONT_GLYPH_FORMAT_A4_ALIGNED = 0x014, /**< 4 bit per pixel*/
    LV_FONT_GLYPH_FORMAT_A8_ALIGNED = 0x018, /**< 8 bit per pixel*/

    LV_FONT_GLYPH_FORMAT_IMAGE  = 0x19, /**< Image format*/

    /**< Advanced formats*/
    LV_FONT_GLYPH_FORMAT_VECTOR = 0x1A, /**< Vectorial format*/
    LV_FONT_GLYPH_FORMAT_SVG    = 0x1B, /**< SVG format*/
    LV_FONT_GLYPH_FORMAT_CUSTOM = 0xFF, /**< Custom format*/
} lv_font_glyph_format_t;

/** Describes the properties of a glyph.*/
typedef struct {
    const lv_font_t *
    resolved_font;  /**< Pointer to a font where the glyph was actually found after handling fallbacks*/
    uint16_t adv_w; /**< The glyph needs this space. Draw the next glyph after this width.*/
    uint16_t box_w; /**< Width of the glyph's bounding box*/
    uint16_t box_h; /**< Height of the glyph's bounding box*/
    int16_t ofs_x;  /**< x offset of the bounding box*/
    int16_t ofs_y;  /**< y offset of the bounding box*/
    lv_font_glyph_format_t format;  /**< Font format of the glyph see lv_font_glyph_format_t */
    uint8_t is_placeholder: 1;      /**< Glyph is missing. But placeholder will still be displayed*/

    /** 0: Get bitmap should return an A8 or ARGB8888 image.
     * 1: return the bitmap as it is (Maybe A1/2/4 or any proprietary formats). */
    uint8_t req_raw_bitmap: 1;

    union {
        uint32_t index;       /**< Unicode code point*/
        const void * src;     /**< Pointer to the source data used by image fonts*/
    } gid;                    /**< The index of the glyph in the font file. Used by the font cache*/
    void * entry; //lv_cache_entry_t * entry; /**< The cache entry of the glyph draw data. Used by the font cache*/
    uint8_t bpp;
} lv_font_glyph_dsc_t;

/** Describe the properties of a font*/
struct _lv_font_t {
    /** Get a glyph's descriptor from a font*/
    bool (*get_glyph_dsc)(const lv_font_t * font, lv_font_glyph_dsc_t * dsc_out, uint32_t unicode_letter, uint32_t unicode_letter_next);

    /** Get a glyph's bitmap from a font*/
    const void * (*get_glyph_bitmap)(lv_font_glyph_dsc_t * g_dsc, lv_draw_buf_t * draw_buf);

    /** Release a glyph*/
    void (*release_glyph)(const lv_font_t *, lv_font_glyph_dsc_t *);

    /*Pointer to the font in a font pack (must have the same line height)*/
    int32_t line_height;         /**< The real line height where any text fits*/
    int32_t base_line;           /**< Base line measured from the bottom of the line_height*/
    uint8_t subpx   : 2;            /**< An element of `lv_font_subpx_t`*/
    uint8_t kerning : 1;            /**< An element of `lv_font_kerning_t`*/

    int8_t underline_position;      /**< Distance between the top of the underline and base line (< 0 means below the base line)*/
    int8_t underline_thickness;     /**< Thickness of the underline*/

    const void * dsc;               /**< Store implementation specific or run_time data or caching here*/
    const lv_font_t * fallback;   /**< Fallback font for missing glyph. Resolved recursively */
    void * user_data;               /**< Custom user data for font.*/
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
