#include "gametank-font.h"

#define FONT_COLUMNS 16
#define QUADRANT_SIZE 128
#define QUADRANT_MASK 0x7F

static void set_font(GtFont* font, uint8_t kind, uint8_t sheet_x, uint8_t glyph_row_origin, uint8_t glyph_size) {
    font->sheet_x = sheet_x;
    font->glyph_row_origin = glyph_row_origin;
    font->glyph_size = glyph_size;
    font->upper_only = kind == GT_FONT_KIND_UPPER_ONLY;
    if (kind == GT_FONT_KIND_NUMERIC_ONLY) {
        font->first_character = '0';
        font->last_character = '?';
    } else {
        font->first_character = GT_FONT_FIRST_CHARACTER;
        font->last_character = GT_FONT_LAST_CHARACTER;
    }
}

#if defined(__mos__)

#include "gametank.h"

#define FONT_ASSERT GT_ASSERT

#define PREPARE_FONT(font)

#define BLIT(x, y, gram_x, gram_y, width, height) \
    gt_sprite_draw((x), (y), (gram_x), (gram_y), (width), (height))

static void load_font(GtFont* font, const uint8_t* sheet, uint8_t kind, uint8_t gram_x, uint8_t gram_y,
    uint8_t glyph_size) {
    uint8_t rows = kind == GT_FONT_KIND_DEFAULT ? 6 : kind == GT_FONT_KIND_UPPER_ONLY ? 4 : 1;
    uint8_t first_row = kind == GT_FONT_KIND_NUMERIC_ONLY ? 3 : 2;
    gt_sprite_sheet_load(sheet, glyph_size * FONT_COLUMNS, rows * glyph_size, gram_x, gram_y);
    set_font(font, kind, gram_x, (uint8_t)(gram_y - first_row * glyph_size), glyph_size);
    font->sheet = sheet;
}

#ifndef GT_NO_FONT8
void gt_font_load(GtFont* font, const uint8_t* sheet, uint8_t kind, uint8_t gram_x, uint8_t gram_y) {
    load_font(font, sheet, kind, gram_x, gram_y, GT_FONT_GLYPH_SIZE);
}
#endif

#ifndef GT_NO_FONT6
void gt_font6_load(GtFont* font, const uint8_t* sheet, uint8_t kind, uint8_t gram_x, uint8_t gram_y) {
    load_font(font, sheet, kind, gram_x, gram_y, GT_FONT6_GLYPH_SIZE);
}
#endif

#elif defined(__CC65__)

#include "gametank.h"
#include "gfx/draw_direct.h"

#define FONT_ASSERT(condition)

#define PREPARE_FONT(font) direct_prepare_sprite_mode((font)->sprite_slot)

#define BLIT(x, y, gram_x, gram_y, width, height) \
    do { \
        await_drawing(); \
        vram[VX] = (x); \
        vram[VY] = (y); \
        vram[GX] = (gram_x); \
        vram[GY] = (gram_y); \
        vram[WIDTH] = (width); \
        vram[HEIGHT] = (height); \
        DIRECT_DRAW_START(); \
    } while (0)

static void allocate_font(GtFont* font, const SpritePage* load_list, uint8_t kind, uint8_t glyph_size) {
    font->sprite_slot = allocate_sprite(load_list);
    set_font(font, kind, SPRITE_OFFSET_X(font->sprite_slot), SPRITE_OFFSET_Y(font->sprite_slot), glyph_size);
}

static uint8_t kind_of_bmp_size(uint16_t bmp_size, uint16_t upper_only_size, uint16_t numeric_only_size) {
    if (bmp_size == upper_only_size) return GT_FONT_KIND_UPPER_ONLY;
    if (bmp_size == numeric_only_size) return GT_FONT_KIND_NUMERIC_ONLY;
    return GT_FONT_KIND_DEFAULT;
}

#ifndef GT_NO_FONT8
void gt_font_allocate(GtFont* font, const SpritePage* load_list, uint16_t bmp_size) {
    allocate_font(font, load_list,
        kind_of_bmp_size(bmp_size, GT_FONT_SDK_UPPER_ONLY_BMP_SIZE, GT_FONT_SDK_NUMERIC_ONLY_BMP_SIZE),
        GT_FONT_GLYPH_SIZE);
}
#endif

#ifndef GT_NO_FONT6
void gt_font6_allocate(GtFont* font, const SpritePage* load_list, uint16_t bmp_size) {
    allocate_font(font, load_list,
        kind_of_bmp_size(bmp_size, GT_FONT6_SDK_UPPER_ONLY_BMP_SIZE, GT_FONT6_SDK_NUMERIC_ONLY_BMP_SIZE),
        GT_FONT6_GLYPH_SIZE);
}
#endif

#else
#error "gametank-font.c supports gametank-llvm-mos and cc65 (with the GameTank SDK)"
#endif

#define CHECK_CHARACTER(character, font) \
    FONT_ASSERT(((character) == ' ') \
        || (((character) >= (font)->first_character) && ((character) <= (font)->last_character)))

#define FOLD_TO_UPPER(character) \
    do { if ((character) >= 0x60) { (character) &= ~0x20; } } while (0)

#define GLYPH_COLUMN8(column, character, sheet_x) \
    do { (column) = ((character) & 15) << 3; (column) += (sheet_x); } while (0)
#define GLYPH_ROW8(row, character, glyph_row_origin) \
    do { (row) = ((character) >> 1) & 0x78; (row) += (glyph_row_origin); } while (0)

#define TIMES_6(value, temporary) \
    do { (temporary) = (value) << 1; (value) <<= 2; (value) += (temporary); } while (0)
#define GLYPH_COLUMN6(column, character, sheet_x, temporary) \
    do { (column) = (character) & 15; TIMES_6(column, temporary); (column) += (sheet_x); } while (0)
#define GLYPH_ROW6(row, character, glyph_row_origin, temporary) \
    do { (row) = (character) >> 4; TIMES_6(row, temporary); (row) += (glyph_row_origin); } while (0)

#ifndef GT_NO_FONT8
void gt_font_draw(const GtFont* font, uint8_t x, uint8_t y, const char* text) {
    uint8_t sheet_x = font->sheet_x;
    uint8_t glyph_row_origin = font->glyph_row_origin;
    uint8_t upper_only = font->upper_only;
    uint8_t character;
    uint8_t column;
    uint8_t row;
    FONT_ASSERT(font->glyph_size == GT_FONT_GLYPH_SIZE);
    PREPARE_FONT(font);
    for (; *text != '\0'; ++text) {
        character = (uint8_t)*text;
        CHECK_CHARACTER(character, font);
        if (character != ' ') {
            if (upper_only) {
                FOLD_TO_UPPER(character);
            }
            GLYPH_COLUMN8(column, character, sheet_x);
            GLYPH_ROW8(row, character, glyph_row_origin);
            BLIT(x, y, column, row, GT_FONT_GLYPH_SIZE, GT_FONT_GLYPH_SIZE);
        }
        x += GT_FONT_GLYPH_SIZE;
    }
}

void gt_font_draw_char(const GtFont* font, uint8_t x, uint8_t y, char text_character) {
    uint8_t character = (uint8_t)text_character;
    uint8_t column;
    uint8_t row;
    FONT_ASSERT(font->glyph_size == GT_FONT_GLYPH_SIZE);
    CHECK_CHARACTER(character, font);
    if (character == ' ') return;
    if (font->upper_only) {
        FOLD_TO_UPPER(character);
    }
    PREPARE_FONT(font);
    GLYPH_COLUMN8(column, character, font->sheet_x);
    GLYPH_ROW8(row, character, font->glyph_row_origin);
    BLIT(x, y, column, row, GT_FONT_GLYPH_SIZE, GT_FONT_GLYPH_SIZE);
}
#endif

#ifndef GT_NO_FONT6
void gt_font6_draw(const GtFont* font, uint8_t x, uint8_t y, const char* text) {
    uint8_t sheet_x = font->sheet_x;
    uint8_t glyph_row_origin = font->glyph_row_origin;
    uint8_t upper_only = font->upper_only;
    uint8_t character;
    uint8_t column;
    uint8_t row;
    uint8_t temporary;
    FONT_ASSERT(font->glyph_size == GT_FONT6_GLYPH_SIZE);
    PREPARE_FONT(font);
    for (; *text != '\0'; ++text) {
        character = (uint8_t)*text;
        CHECK_CHARACTER(character, font);
        if (character != ' ') {
            if (upper_only) {
                FOLD_TO_UPPER(character);
            }
            GLYPH_COLUMN6(column, character, sheet_x, temporary);
            GLYPH_ROW6(row, character, glyph_row_origin, temporary);
            BLIT(x, y, column, row, GT_FONT6_GLYPH_SIZE, GT_FONT6_GLYPH_SIZE);
        }
        x += GT_FONT6_GLYPH_SIZE;
    }
}

void gt_font6_draw_char(const GtFont* font, uint8_t x, uint8_t y, char text_character) {
    uint8_t character = (uint8_t)text_character;
    uint8_t column;
    uint8_t row;
    uint8_t temporary;
    FONT_ASSERT(font->glyph_size == GT_FONT6_GLYPH_SIZE);
    CHECK_CHARACTER(character, font);
    if (character == ' ') return;
    if (font->upper_only) {
        FOLD_TO_UPPER(character);
    }
    PREPARE_FONT(font);
    GLYPH_COLUMN6(column, character, font->sheet_x, temporary);
    GLYPH_ROW6(row, character, font->glyph_row_origin, temporary);
    BLIT(x, y, column, row, GT_FONT6_GLYPH_SIZE, GT_FONT6_GLYPH_SIZE);
}
#endif

typedef void (*DrawText)(const GtFont* font, uint8_t x, uint8_t y, const char* text);

static char _numberText[GT_FONT_NUMBER_MAX_WIDTH + 1];

static void draw_digits(const GtFont* font, uint8_t x, uint8_t y, const char* digits, uint8_t digit_count,
    uint8_t width, char pad, DrawText draw) {
    uint8_t leading;
    uint8_t i;
    FONT_ASSERT(digit_count <= width);
    FONT_ASSERT(width <= GT_FONT_NUMBER_MAX_WIDTH);
    if (width < digit_count) {
        width = digit_count;
    }
    if (width > GT_FONT_NUMBER_MAX_WIDTH) return;
    leading = width - digit_count;
    for (i = 0; i < leading; ++i) {
        _numberText[i] = pad;
    }
    for (i = 0; i < digit_count; ++i) {
        _numberText[leading + i] = digits[i];
    }
    _numberText[width] = '\0';
    draw(font, x, y, _numberText);
}

static const uint16_t _powersOfTen[] = { 10000, 1000, 100, 10 };

static void draw_u16(const GtFont* font, uint8_t x, uint8_t y, uint16_t value, uint8_t width, char pad,
    DrawText draw) {
    char digits[5];
    uint8_t count = 0;
    uint8_t i;
    char digit;
    for (i = 0; i < 4; ++i) {
        digit = '0';
        while (value >= _powersOfTen[i]) {
            value -= _powersOfTen[i];
            ++digit;
        }
        if (digit != '0' || count != 0) {
            digits[count] = digit;
            ++count;
        }
    }
    digits[count] = (char)('0' + value);
    ++count;
    draw_digits(font, x, y, digits, count, width, pad, draw);
}

static void draw_u8(const GtFont* font, uint8_t x, uint8_t y, uint8_t value, uint8_t width, char pad,
    DrawText draw) {
    char digits[3];
    uint8_t count = 0;
    char digit = '0';
    while (value >= 100) {
        value -= 100;
        ++digit;
    }
    if (digit != '0') {
        digits[count] = digit;
        ++count;
    }
    digit = '0';
    while (value >= 10) {
        value -= 10;
        ++digit;
    }
    if (digit != '0' || count != 0) {
        digits[count] = digit;
        ++count;
    }
    digits[count] = (char)('0' + value);
    ++count;
    draw_digits(font, x, y, digits, count, width, pad, draw);
}

static void draw_bcd(const GtFont* font, uint8_t x, uint8_t y, const uint8_t* bcd, uint8_t byte_count, char pad,
    DrawText draw) {
    char digits[GT_FONT_NUMBER_MAX_WIDTH];
    uint8_t count = 0;
    uint8_t width = byte_count * 2;
    uint8_t i;
    uint8_t nibble;
    FONT_ASSERT(width <= GT_FONT_NUMBER_MAX_WIDTH);
    if (width > GT_FONT_NUMBER_MAX_WIDTH) return;
    for (i = 0; i < width; ++i) {
        nibble = (i & 1) ? (bcd[i >> 1] & 0x0F) : (bcd[i >> 1] >> 4);
        FONT_ASSERT(nibble <= 9);
        if (nibble != 0 || count != 0 || i == width - 1) {
            digits[count] = (char)('0' + nibble);
            ++count;
        }
    }
    draw_digits(font, x, y, digits, count, width, pad, draw);
}

#ifndef GT_NO_FONT8
void gt_font_draw_u8(const GtFont* font, uint8_t x, uint8_t y, uint8_t value, uint8_t width, char pad) {
    draw_u8(font, x, y, value, width, pad, gt_font_draw);
}

void gt_font_draw_u16(const GtFont* font, uint8_t x, uint8_t y, uint16_t value, uint8_t width, char pad) {
    draw_u16(font, x, y, value, width, pad, gt_font_draw);
}

void gt_font_draw_bcd(const GtFont* font, uint8_t x, uint8_t y, const uint8_t* bcd, uint8_t byte_count, char pad) {
    draw_bcd(font, x, y, bcd, byte_count, pad, gt_font_draw);
}
#endif

#ifndef GT_NO_FONT6
void gt_font6_draw_u8(const GtFont* font, uint8_t x, uint8_t y, uint8_t value, uint8_t width, char pad) {
    draw_u8(font, x, y, value, width, pad, gt_font6_draw);
}

void gt_font6_draw_u16(const GtFont* font, uint8_t x, uint8_t y, uint16_t value, uint8_t width, char pad) {
    draw_u16(font, x, y, value, width, pad, gt_font6_draw);
}

void gt_font6_draw_bcd(const GtFont* font, uint8_t x, uint8_t y, const uint8_t* bcd, uint8_t byte_count, char pad) {
    draw_bcd(font, x, y, bcd, byte_count, pad, gt_font6_draw);
}
#endif

static uint8_t _textRow[GT_TEXT_MAX_WIDTH];

static void clear_glyph_row(uint8_t* destination, uint8_t glyph_size) {
    uint8_t column;
    for (column = 0; column < glyph_size; ++column) {
        destination[column] = 0;
    }
}

#if defined(__mos__)

#define SELECT_GLYPH_SOURCE(font)

static void copy_glyph_row(const GtFont* font, uint8_t character, uint8_t pixel_row, uint8_t* destination) {
    uint8_t glyph_size = font->glyph_size;
    uint8_t sheet_row = (character >> 4) - (font->first_character >> 4);
    const uint8_t* source = font->sheet
        + (uint16_t)(sheet_row * glyph_size + pixel_row) * (uint16_t)(glyph_size * FONT_COLUMNS)
        + (character & 15) * glyph_size;
    for (uint8_t column = 0; column < glyph_size; ++column) {
        destination[column] = source[column];
    }
}

static void store_text_row(const GtTextCache* cache, uint8_t width, uint8_t pixel_row) {
    gt_sprite_sheet_load(_textRow, width, 1, (uint8_t)cache->next_x, (uint8_t)(cache->next_y + pixel_row));
}

#define END_TEXT_ROWS(cache)

void gt_text_cache_init(GtTextCache* cache, uint8_t gram_x, uint8_t gram_y, uint16_t width, uint16_t height) {
    GT_ASSERT((width > 0) && (height > 0));
    GT_ASSERT((gram_x + width <= 256) && (gram_y + height <= 256));
    cache->left = gram_x;
    cache->top = gram_y;
    cache->right = gram_x + width;
    cache->bottom = gram_y + height;
    gt_text_cache_clear(cache);
}

void gt_text_draw(const GtText* text, uint8_t x, uint8_t y) {
    BLIT(x, y, text->gram_x, text->gram_y, text->width, text->height);
}

#elif defined(__CC65__)

#define SELECT_GLYPH_SOURCE(font) direct_prepare_sprite_ram_array_mode((font)->sprite_slot)

static void copy_glyph_row(const GtFont* font, uint8_t character, uint8_t pixel_row, uint8_t* destination) {
    uint8_t glyph_size = font->glyph_size;
    uint8_t column;
    const uint8_t* source = vram
        + (uint16_t)((character >> 4) * glyph_size + pixel_row) * QUADRANT_SIZE
        + (character & 15) * glyph_size;
    for (column = 0; column < glyph_size; ++column) {
        destination[column] = source[column];
    }
}

static void store_text_row(const GtTextCache* cache, uint8_t width, uint8_t pixel_row) {
    uint8_t column;
    uint8_t* destination;
    direct_prepare_sprite_ram_array_mode(cache->sprite_slot);
    destination = vram
        + (uint16_t)(((uint8_t)cache->next_y & QUADRANT_MASK) + pixel_row) * QUADRANT_SIZE
        + ((uint8_t)cache->next_x & QUADRANT_MASK);
    for (column = 0; column < width; ++column) {
        destination[column] = _textRow[column];
    }
}

#define END_TEXT_ROWS(cache) direct_prepare_sprite_mode((cache)->sprite_slot)

void gt_text_cache_allocate(GtTextCache* cache, const SpritePage* blank_load_list) {
    cache->sprite_slot = allocate_sprite(blank_load_list);
    cache->left = SPRITE_OFFSET_X(cache->sprite_slot);
    cache->top = SPRITE_OFFSET_Y(cache->sprite_slot);
    cache->right = cache->left + QUADRANT_SIZE;
    cache->bottom = cache->top + QUADRANT_SIZE;
    gt_text_cache_clear(cache);
}

void gt_text_draw(const GtText* text, uint8_t x, uint8_t y) {
    direct_prepare_sprite_mode(text->sprite_slot);
    BLIT(x, y, text->gram_x, text->gram_y, text->width, text->height);
}

#endif

void gt_text_cache_clear(GtTextCache* cache) {
    cache->next_x = cache->left;
    cache->next_y = cache->top;
    cache->row_height = 0;
}

uint8_t gt_text_cache_add(GtTextCache* cache, GtText* result, const GtFont* font, const char* text) {
    uint8_t glyph_size = font->glyph_size;
    uint8_t length = 0;
    uint16_t width = 0;
    uint8_t pixel_row;
    uint8_t i;
    uint8_t character;
    while (text[length] != '\0') {
        ++length;
        width += glyph_size;
        FONT_ASSERT(width <= GT_TEXT_MAX_WIDTH);
        if (width > GT_TEXT_MAX_WIDTH) return 0;
    }
    FONT_ASSERT(length > 0);
    if (length == 0) return 0;
    FONT_ASSERT(cache->left + width <= cache->right);

    if (cache->next_x + width > cache->right) {
        cache->next_x = cache->left;
        cache->next_y += cache->row_height;
        cache->row_height = 0;
    }
    if (cache->next_y + glyph_size > cache->bottom) return 0;

    for (pixel_row = 0; pixel_row < glyph_size; ++pixel_row) {
        SELECT_GLYPH_SOURCE(font);
        for (i = 0; i < length; ++i) {
            character = (uint8_t)text[i];
            CHECK_CHARACTER(character, font);
            if (character == ' ') {
                clear_glyph_row(_textRow + i * glyph_size, glyph_size);
                continue;
            }
            if (font->upper_only) {
                FOLD_TO_UPPER(character);
            }
            copy_glyph_row(font, character, pixel_row, _textRow + i * glyph_size);
        }
        store_text_row(cache, (uint8_t)width, pixel_row);
    }
    END_TEXT_ROWS(cache);

    result->gram_x = (uint8_t)cache->next_x;
    result->gram_y = (uint8_t)cache->next_y;
    result->width = (uint8_t)width;
    result->height = glyph_size;
#if defined(__CC65__)
    result->sprite_slot = cache->sprite_slot;
#endif
    cache->next_x += width;
    if (glyph_size > cache->row_height) {
        cache->row_height = glyph_size;
    }
    return 1;
}
