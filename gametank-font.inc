FONT_GLYPH_SIZE         = 8
FONT_FIRST_CHARACTER    = $20
FONT_LAST_CHARACTER     = $7F
FONT_RECORD_SIZE        = 4

; FONT_ALLOCATE kinds.
FONT_DEFAULT            = 0     ; <name>.bmp
FONT_UPPER_ONLY         = $80   ; <name>-upper-only.bmp

.ifndef GAMETANK_FONT_IMPLEMENTATION

.importzp font_record, font_pointer, font_x, font_y

; A = kind, font_pointer = load list. Carry clear when there's no free quadrant.
.import FontAllocate

; font_pointer = zero-terminated text.
.import FontDrawText

; A = character.
.import FontDrawCharacter

.endif

.macro FONT_ALLOCATE record, load_list, kind
    .assert ((kind) = FONT_DEFAULT) .or ((kind) = FONT_UPPER_ONLY), error, "kind is FONT_DEFAULT or FONT_UPPER_ONLY"
    lda #<(record)
    sta font_record
    lda #>(record)
    sta font_record + 1
    lda #<(load_list)
    sta font_pointer
    lda #>(load_list)
    sta font_pointer + 1
    lda #(kind)
    jsr FontAllocate
.endmacro

.macro FONT_DRAW_TEXT record, x_position, y_position, text
    lda #<(record)
    sta font_record
    lda #>(record)
    sta font_record + 1
    lda #<(text)
    sta font_pointer
    lda #>(text)
    sta font_pointer + 1
    lda #(x_position)
    sta font_x
    lda #(y_position)
    sta font_y
    jsr FontDrawText
.endmacro
