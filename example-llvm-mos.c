// gametank-llvm-mos example. Add it and gametank-font.c to gametank_add_rom.
#include "gametank.h"
#include "gametank-font.h"

#define BACKGROUND_COLOR GT_COLOR(5, 1, 1)

// For an upper-only font, #embed "gametank-fonts/upper-only/ninja-gaiden-trilogy.bin".
static const unsigned char title_sheet[] = {
#embed "gametank-fonts/ninja-gaiden-trilogy.bin"
};

static GtFont _titleFont;
static GtTextCache _textCache;
static GtText _quickText;
static GtText _foxText;

int main(void) {
    GT_FONT_LOAD(&_titleFont, title_sheet, 0, 0);

    gt_text_cache_init(&_textCache, 0, 48, 128, 16);
    GT_ASSERT(gt_text_cache_add(&_textCache, &_quickText, &_titleFont, "The quick brown"));
    GT_ASSERT(gt_text_cache_add(&_textCache, &_foxText, &_titleFont, "fox jumps over"));

    for (;;) {
        gt_clear_screen(BACKGROUND_COLOR);
        gt_font_draw(&_titleFont, 0, 16, "HELLO, GAMETANK!");
        gt_font_draw(&_titleFont, 0, 32, "llvm-mos example");
        gt_font_draw(&_titleFont, 4, 56, "0123456789");
        gt_font_draw(&_titleFont, 4, 72, "!\"#$%&'()*+,-./");
        gt_text_draw(&_quickText, 4, 88);
        gt_text_draw(&_foxText, 4, 96);
        gt_font_draw_char(&_titleFont, 120, 112, '?');
        gt_show_frame();
    }
}
