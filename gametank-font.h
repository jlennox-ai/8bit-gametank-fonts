#ifndef GAMETANK_FONT_H
#define GAMETANK_FONT_H

#include <stdint.h>

// Define GT_NO_FONT8 or GT_NO_FONT6 project-wide to leave out that size's functions.
#if defined(GT_NO_FONT8) && defined(GT_NO_FONT6)
#error "GT_NO_FONT8 and GT_NO_FONT6 leave no fonts"
#endif

#define GT_FONT_GLYPH_SIZE 8
#define GT_FONT6_GLYPH_SIZE 6
#define GT_FONT_FIRST_CHARACTER 0x20
#define GT_FONT_LAST_CHARACTER 0x7F
#define GT_TEXT_MAX_WIDTH 127

#define GT_FONT_KIND_DEFAULT 0
#define GT_FONT_KIND_UPPER_ONLY 1
#define GT_FONT_KIND_NUMERIC_ONLY 2

#define GT_FONT_SHEET_WIDTH 128
#define GT_FONT_SHEET_HEIGHT 48
#define GT_FONT_UPPER_ONLY_SHEET_HEIGHT 32
#define GT_FONT_NUMERIC_ONLY_SHEET_HEIGHT 8

#define GT_FONT6_SHEET_WIDTH 96
#define GT_FONT6_SHEET_HEIGHT 36
#define GT_FONT6_UPPER_ONLY_SHEET_HEIGHT 24
#define GT_FONT6_NUMERIC_ONLY_SHEET_HEIGHT 6

// sheet must be an array.
#define GT_FONT_SHEET_HEIGHT_OF(sheet) (sizeof(sheet) / GT_FONT_SHEET_WIDTH)
#define GT_FONT6_SHEET_HEIGHT_OF(sheet) (sizeof(sheet) / GT_FONT6_SHEET_WIDTH)

typedef struct {
    uint8_t sheet_x;
    uint8_t glyph_row_origin;
    uint8_t glyph_size;
    uint8_t upper_only;
    uint8_t first_character;
    uint8_t last_character;
#if defined(__mos__)
    const uint8_t* sheet;
#elif defined(__CC65__)
    uint8_t sprite_slot;
#endif
} GtFont;

#define GT_FONT_NUMBER_MAX_WIDTH 21

#ifndef GT_NO_FONT8
void gt_font_draw(const GtFont* font, uint8_t x, uint8_t y, const char* text);
void gt_font_draw_char(const GtFont* font, uint8_t x, uint8_t y, char character);
// Right-aligned in width cells, padded with pad (' ' or '0').
void gt_font_draw_u8(const GtFont* font, uint8_t x, uint8_t y, uint8_t value, uint8_t width, char pad);
void gt_font_draw_u16(const GtFont* font, uint8_t x, uint8_t y, uint16_t value, uint8_t width, char pad);
// Packed BCD, most significant byte first.
void gt_font_draw_bcd(const GtFont* font, uint8_t x, uint8_t y, const uint8_t* bcd, uint8_t byte_count, char pad);
#endif

#ifndef GT_NO_FONT6
void gt_font6_draw(const GtFont* font, uint8_t x, uint8_t y, const char* text);
void gt_font6_draw_char(const GtFont* font, uint8_t x, uint8_t y, char character);
void gt_font6_draw_u8(const GtFont* font, uint8_t x, uint8_t y, uint8_t value, uint8_t width, char pad);
void gt_font6_draw_u16(const GtFont* font, uint8_t x, uint8_t y, uint16_t value, uint8_t width, char pad);
void gt_font6_draw_bcd(const GtFont* font, uint8_t x, uint8_t y, const uint8_t* bcd, uint8_t byte_count, char pad);
#endif

typedef struct {
    uint8_t gram_x;
    uint8_t gram_y;
    uint8_t width;
    uint8_t height;
#if defined(__CC65__)
    uint8_t sprite_slot;
#endif
} GtText;

typedef struct {
    uint8_t left;
    uint8_t top;
    uint16_t right;
    uint16_t bottom;
    uint16_t next_x;
    uint16_t next_y;
    uint8_t row_height;
#if defined(__CC65__)
    uint8_t sprite_slot;
#endif
} GtTextCache;

#if defined(__mos__)

#define GT_FONT_KIND_OF_SIZE(size, sheet_size, upper_only) \
    ((size) == (sheet_size) ? GT_FONT_KIND_DEFAULT : (size) == (upper_only) ? GT_FONT_KIND_UPPER_ONLY : GT_FONT_KIND_NUMERIC_ONLY)

#ifndef GT_NO_FONT8
#define GT_FONT_SHEET_SIZE (GT_FONT_SHEET_WIDTH * GT_FONT_SHEET_HEIGHT)
#define GT_FONT_UPPER_ONLY_SHEET_SIZE (GT_FONT_SHEET_WIDTH * GT_FONT_UPPER_ONLY_SHEET_HEIGHT)
#define GT_FONT_NUMERIC_ONLY_SHEET_SIZE (GT_FONT_SHEET_WIDTH * GT_FONT_NUMERIC_ONLY_SHEET_HEIGHT)

// sheet must be an array.
#define GT_FONT_LOAD(font, sheet, gram_x, gram_y) \
    do { \
        _Static_assert((sizeof(sheet) == GT_FONT_SHEET_SIZE) || (sizeof(sheet) == GT_FONT_UPPER_ONLY_SHEET_SIZE) \
            || (sizeof(sheet) == GT_FONT_NUMERIC_ONLY_SHEET_SIZE), "not a font sheet"); \
        gt_font_load((font), (sheet), \
            GT_FONT_KIND_OF_SIZE(sizeof(sheet), GT_FONT_SHEET_SIZE, GT_FONT_UPPER_ONLY_SHEET_SIZE), \
            (gram_x), (gram_y)); \
    } while (0)

void gt_font_load(GtFont* font, const uint8_t* sheet, uint8_t kind, uint8_t gram_x, uint8_t gram_y);
#endif

#ifndef GT_NO_FONT6
#define GT_FONT6_SHEET_SIZE (GT_FONT6_SHEET_WIDTH * GT_FONT6_SHEET_HEIGHT)
#define GT_FONT6_UPPER_ONLY_SHEET_SIZE (GT_FONT6_SHEET_WIDTH * GT_FONT6_UPPER_ONLY_SHEET_HEIGHT)
#define GT_FONT6_NUMERIC_ONLY_SHEET_SIZE (GT_FONT6_SHEET_WIDTH * GT_FONT6_NUMERIC_ONLY_SHEET_HEIGHT)

#define GT_FONT6_LOAD(font, sheet, gram_x, gram_y) \
    do { \
        _Static_assert((sizeof(sheet) == GT_FONT6_SHEET_SIZE) || (sizeof(sheet) == GT_FONT6_UPPER_ONLY_SHEET_SIZE) \
            || (sizeof(sheet) == GT_FONT6_NUMERIC_ONLY_SHEET_SIZE), "not a font6 sheet"); \
        gt_font6_load((font), (sheet), \
            GT_FONT_KIND_OF_SIZE(sizeof(sheet), GT_FONT6_SHEET_SIZE, GT_FONT6_UPPER_ONLY_SHEET_SIZE), \
            (gram_x), (gram_y)); \
    } while (0)

void gt_font6_load(GtFont* font, const uint8_t* sheet, uint8_t kind, uint8_t gram_x, uint8_t gram_y);
#endif

void gt_text_cache_init(GtTextCache* cache, uint8_t gram_x, uint8_t gram_y, uint16_t width, uint16_t height);

#elif defined(__CC65__)

#include "gfx/sprites.h"

#define gt_font_allocated(font) (!SPRITE_SLOT_INVALID((font)->sprite_slot))

#ifndef GT_NO_FONT8
#define GT_FONT_SDK_BMP_SIZE 9270
#define GT_FONT_SDK_UPPER_ONLY_BMP_SIZE 7222
#define GT_FONT_SDK_NUMERIC_ONLY_BMP_SIZE 5174

#define GT_FONT_ALLOCATE(font, asset) \
    do { \
        _Static_assert((asset##_size == GT_FONT_SDK_BMP_SIZE) || (asset##_size == GT_FONT_SDK_UPPER_ONLY_BMP_SIZE) \
            || (asset##_size == GT_FONT_SDK_NUMERIC_ONLY_BMP_SIZE), "not a gametank-sdk font .bmp"); \
        gt_font_allocate((font), &asset##_load_list, asset##_size); \
    } while (0)

void gt_font_allocate(GtFont* font, const SpritePage* load_list, uint16_t bmp_size);
#endif

#ifndef GT_NO_FONT6
#define GT_FONT6_SDK_BMP_SIZE 7222
#define GT_FONT6_SDK_UPPER_ONLY_BMP_SIZE 5686
#define GT_FONT6_SDK_NUMERIC_ONLY_BMP_SIZE 4150

#define GT_FONT6_ALLOCATE(font, asset) \
    do { \
        _Static_assert((asset##_size == GT_FONT6_SDK_BMP_SIZE) || (asset##_size == GT_FONT6_SDK_UPPER_ONLY_BMP_SIZE) \
            || (asset##_size == GT_FONT6_SDK_NUMERIC_ONLY_BMP_SIZE), "not a gametank-sdk font6 .bmp"); \
        gt_font6_allocate((font), &asset##_load_list, asset##_size); \
    } while (0)

void gt_font6_allocate(GtFont* font, const SpritePage* load_list, uint16_t bmp_size);
#endif

// blank_asset is gametank-fonts/gametank-sdk/text-cache.bmp.
#define GT_TEXT_CACHE_ALLOCATE(cache, blank_asset) gt_text_cache_allocate((cache), &blank_asset##_load_list)

#define gt_text_cache_allocated(cache) (!SPRITE_SLOT_INVALID((cache)->sprite_slot))

void gt_text_cache_allocate(GtTextCache* cache, const SpritePage* blank_load_list);

#else
#error "gametank-font.h supports gametank-llvm-mos and cc65 (with the GameTank SDK)"
#endif

// Invalidates its GtTexts.
void gt_text_cache_clear(GtTextCache* cache);

// Returns false when text doesn't fit.
uint8_t gt_text_cache_add(GtTextCache* cache, GtText* result, const GtFont* font, const char* text);

void gt_text_draw(const GtText* text, uint8_t x, uint8_t y);

#endif // #ifndef GAMETANK_FONT_H
