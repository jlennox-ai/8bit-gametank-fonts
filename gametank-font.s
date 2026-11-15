.setcpu "W65C02"

GAMETANK_FONT_IMPLEMENTATION = 1
.include "gametank-font.inc"

.exportzp font_record, font_pointer, font_x, font_y
.export FontAllocate, FontDrawText, FontDrawCharacter
.import _allocate_sprite, _direct_prepare_sprite_mode, _await_drawing, _draw_busy

DMA_VX          = $4000
DMA_VY          = $4001
DMA_GX          = $4002
DMA_GY          = $4003
DMA_WIDTH       = $4004
DMA_HEIGHT      = $4005
DMA_START       = $4006

SPRITE_OFFSET_X_MASK    = %00001000
SPRITE_OFFSET_Y_MASK    = %00010000
SPRITE_SLOT_INVALID_MASK = %11100000
QUADRANT_SIZE           = 128

RECORD_SHEET_X      = 0
RECORD_ROW_ORIGIN   = 1
RECORD_UPPER_ONLY   = 2
RECORD_SPRITE_SLOT  = 3
.assert RECORD_SPRITE_SLOT + 1 = FONT_RECORD_SIZE, error, "FONT_RECORD_SIZE is out of date"

.segment "ZEROPAGE"
font_record:    .res 2
font_pointer:   .res 2
font_x:         .res 1
font_y:         .res 1
sheet_x:        .res 1
row_origin:     .res 1
upper_only:     .res 1
sprite_slot:    .res 1
character:      .res 1
text_index:     .res 1

.segment "CODE"

.macro check_character
    cmp #FONT_FIRST_CHARACTER
    bcs :+
    stp
:
    cmp #(FONT_LAST_CHARACTER + 1)
    bcc :+
    stp
:
.endmacro

.macro fold_to_upper
    bit upper_only
    bpl :+
    cmp #$60
    bcc :+
    and #<~$20
:
.endmacro

.macro prepare_font
    ldy #RECORD_SHEET_X
    lda (font_record), y
    sta sheet_x
    ldy #RECORD_ROW_ORIGIN
    lda (font_record), y
    sta row_origin
    ldy #RECORD_UPPER_ONLY
    lda (font_record), y
    sta upper_only
    ldy #RECORD_SPRITE_SLOT
    lda (font_record), y
    sta sprite_slot
    jsr _direct_prepare_sprite_mode
.endmacro

.macro blit_glyph
    fold_to_upper
    sta character
    jsr _await_drawing
    lda character
    and #$0F
    asl
    asl
    asl
    clc
    adc sheet_x
    sta DMA_GX
    lda character
    lsr
    and #$78
    clc
    adc row_origin
    sta DMA_GY
    lda font_x
    sta DMA_VX
    lda font_y
    sta DMA_VY
    lda #FONT_GLYPH_SIZE
    sta DMA_WIDTH
    sta DMA_HEIGHT
    lda #1
    sta _draw_busy
    sta DMA_START
.endmacro

FontAllocate:
    ldy #RECORD_UPPER_ONLY
    sta (font_record), y
    lda font_pointer
    ldx font_pointer + 1
    jsr _allocate_sprite
    ldy #RECORD_SPRITE_SLOT
    sta (font_record), y
    tax
    and #SPRITE_OFFSET_X_MASK
    beq :+
    lda #QUADRANT_SIZE
:
    ldy #RECORD_SHEET_X
    sta (font_record), y
    txa
    and #SPRITE_OFFSET_Y_MASK
    beq :+
    lda #QUADRANT_SIZE
:
    ldy #RECORD_ROW_ORIGIN
    sta (font_record), y
    txa
    and #SPRITE_SLOT_INVALID_MASK
    beq @allocated
    clc
    rts
@allocated:
    sec
    rts

FontDrawText:
    prepare_font
    stz text_index
@next:
    ldy text_index
    lda (font_pointer), y
    beq @done
    check_character
    cmp #' '
    beq @advance
    blit_glyph
@advance:
    lda font_x
    clc
    adc #FONT_GLYPH_SIZE
    sta font_x
    inc text_index
    bne @next
    stp                         ; longer than 255 characters
@done:
    rts

FontDrawCharacter:
    check_character
    pha
    prepare_font
    pla
    cmp #' '
    beq @advance
    blit_glyph
@advance:
    lda font_x
    clc
    adc #FONT_GLYPH_SIZE
    sta font_x
    rts
