stdout = $f001
stdin = $f004

*=$ff00
start:
; enable CB1 IRQ
  LDA #$90
  STA $e84e

  LDX -1
loop:
  INX
  LDA text,X
  STA stdout
  BNE loop
end:
  LDA stdin
  JMP end

irq:
  LDA #'A
  STA stdout
  LDA #1
  STA $e84d
  CLI
  RTI

text:
  .text "Running"
  .byte $0a
  .byte 0

*=$fffc
.word start

*=$fffe
.word irq
