; GameTank SDK src/main.s (replacing main.c). Needs gametank-font.s/.inc in src/, and
; ninja-gaiden-trilogy.bmp from gametank-fonts/gametank-sdk/ in assets/fonts/.

.setcpu "W65C02"

.include "gametank-font.inc"

.export _main
.import _queue_clear_screen, _await_drawing, _await_vsync, _flip_pages
.import _ASSET__fonts__ninja_gaiden_trilogy_bmp_load_list

BACKGROUND_COLOR = $A9      ; hue 5, saturation 1, luminance 1

.segment "BSS"
title_font:     .res FONT_RECORD_SIZE

.segment "RODATA"
HelloText:      .byte "HELLO, GAMETANK!", 0
ExampleText:    .byte "ca65 example", 0
DigitsText:     .byte "0123456789", 0
SymbolsText:    .byte "!", $22, "#$%&'()*+,-./", 0
QuickText:      .byte "The quick brown", 0
FoxText:        .byte "fox jumps over", 0

.segment "CODE"

_main:
    FONT_ALLOCATE title_font, _ASSET__fonts__ninja_gaiden_trilogy_bmp_load_list, FONT_DEFAULT
    bcs @frame
    stp                         ; no free graphics RAM quadrant
@frame:
    lda #BACKGROUND_COLOR
    jsr _queue_clear_screen
    FONT_DRAW_TEXT title_font, 0, 16, HelloText
    FONT_DRAW_TEXT title_font, 4, 32, ExampleText
    FONT_DRAW_TEXT title_font, 4, 56, DigitsText
    FONT_DRAW_TEXT title_font, 4, 72, SymbolsText
    FONT_DRAW_TEXT title_font, 4, 88, QuickText
    FONT_DRAW_TEXT title_font, 4, 96, FoxText
    lda #120
    sta font_x
    lda #112
    sta font_y
    lda #'?'
    jsr FontDrawCharacter
    jsr _await_drawing
    lda #1
    ldx #0
    jsr _await_vsync
    jsr _flip_pages
    jmp @frame
