start    = $ff00
fbadd_lo = $00
fbadd_hi = $90
fbpage   = $9a00
addr     = $00
addr_hi  = $01
addr_offset = $02

*=$fffc
.word start
.word start

*=start
lda #10
sta addr_offset

ldx #16
L1:
  dex
  stx fbpage
  php

  lda #10
  sta addr_offset
  L2:
    dec addr_offset
    php
    jsr set_addr
  
    lda #$ff
    ldy #$00
    L3:
      dey
      sta (addr),y
    L3_end:
      bne L3

  L2_end:
    plp
    bne L2

L1_end:
  plp
  bne L1

end:
  jmp end

set_addr:
  lda #fbadd_lo
  sta addr
  lda #fbadd_hi
  clc
  adc addr_offset
  sta addr_hi
  rts
