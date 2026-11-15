// GameTank SDK src/main.c. Needs gametank-font.c/.h in src/, and ninja-gaiden-trilogy.bmp and
// text-cache.bmp from gametank-fonts/gametank-sdk/ in assets/fonts/.
#include "gametank.h"
#include "gfx/draw_queue.h"
#include "gen/assets/fonts.h"
#include "gametank-font.h"

#define BACKGROUND_COLOR 0xA9 // hue 5, saturation 1, luminance 1

static GtFont _titleFont;
static GtTextCache _textCache;
static GtText _quickText;
static GtText _foxText;
static uint16_t _frames;
static const uint8_t _bcdScore[] = { 0x00, 0x42, 0x50 };

static void halt() {
    while (1) {
    }
}

void main() {
    GT_FONT_ALLOCATE(&_titleFont, ASSET__fonts__ninja_gaiden_trilogy_bmp);
    if (!gt_font_allocated(&_titleFont)) halt();
    GT_TEXT_CACHE_ALLOCATE(&_textCache, ASSET__fonts__text_cache_bmp);
    if (!gt_text_cache_allocated(&_textCache)) halt();
    if (!gt_text_cache_add(&_textCache, &_quickText, &_titleFont, "The quick brown")) halt();
    if (!gt_text_cache_add(&_textCache, &_foxText, &_titleFont, "fox jumps over")) halt();

    while (1) {
        queue_clear_screen(BACKGROUND_COLOR);
        gt_font_draw(&_titleFont, 0, 16, "HELLO, GAMETANK!");
        gt_font_draw(&_titleFont, 4, 32, "cc65 example");
        gt_font_draw(&_titleFont, 4, 56, "0123456789");
        gt_font_draw(&_titleFont, 4, 72, "!\"#$%&'()*+,-./");
        gt_text_draw(&_quickText, 4, 88);
        gt_text_draw(&_foxText, 4, 96);
        gt_font_draw_char(&_titleFont, 120, 56, '?');
        gt_font_draw_u16(&_titleFont, 4, 106, _frames, 5, '0');      // 00123
        gt_font_draw_u16(&_titleFont, 64, 106, _frames, 5, ' ');     //   123
        gt_font_draw_u8(&_titleFont, 4, 116, 7, 3, '0');             // 007
        gt_font_draw_bcd(&_titleFont, 48, 116, _bcdScore, 3, ' ');   //   4250
        ++_frames;
        await_drawing();
        await_vsync(1);
        flip_pages();
    }
}
