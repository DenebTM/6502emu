start       = $ff00 ; ROM start address

FB_BANK_CNT = 10    ; number of framebuffer-internal screen memory banks
FB_PAGE_CNT = 15    ; number of memory pages taken up by framebuffer MMIO
FBADD_LO    = $00   ; framebuffer MMIO screen memory address (low byte)
FBADD_HI    = $90   ; framebuffer MMIO screen memory address (high byte)
FB_BANK_REG = $9f00 ; framebuffer MMIO address to set current bank (0..(FB_BANK_CNT-1))

addr        = $00   ; address holding low byte of framebuffer address
addr_hi     = $01   ; address holding high byte of framebuffer address
addr_offset = $02   ; address holding current framebuffer high byte offset (0..(FB_PAGE_CNT-1))
pattern     = $03   ; address holding 8px pattern to be written to framebuffer
; row_remain  = $04   ; address holding number of bytes remaining in current row

; setup reset and interrupt vectors
*=$fffc
.word start
.word start

*=start             
lda #$ff            ; initialize value to $ff (8 pixels of white)
; lda #$aa            ; initialize value to $aa (4+4 alternating black/white pixels)
sta pattern
; lda #80
; sta row_remain

restart:            ; jump here after first run
ldx #FB_BANK_CNT    ; start with highest memory bank used by screen memory
L1:
  dex
  stx FB_BANK_REG
  php

  lda #FB_PAGE_CNT
  sta addr_offset
  L2:
    dec addr_offset
    php
    jsr sub_set_addr

    lda pattern
    ldy #$00
    L3:
      dey
      sta (addr),y

    ; uncomment to invert the pattern after every scanline
    ;   php
    ;   dec row_remain
    ;   bne L3_end
    ; flip_row:
    ;   lda #80
    ;   sta row_remain
    ;   jsr sub_invert
    ;   lda pattern
    ;  plp

    L3_end:
      bne L3

  L2_end:
    plp
    bne L2

L1_end:
  plp
  bne L1

end:
; jmp end
jsr sub_invert      ; invert pattern after every pass
jmp restart

sub_invert:         ; flip all bits in pattern
  lda pattern
  eor #$ff
  sta pattern
  rts

sub_set_addr:       ; set base address to write to ($9000..$9900)
  lda #FBADD_LO
  sta addr
  lda #FBADD_HI
  clc
  adc addr_offset
  sta addr_hi
  rts
