stdout = $f001

*=$ff00
start:
  LDX -1
loop:
  INX
  LDA text,X
  STA stdout
  BNE loop
end:
  JMP end

irq:
  LDA #'A
  STA stdout
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
