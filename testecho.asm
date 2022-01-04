stdout = $f001
stdin  = $f004
start  = $ff00

*=$fffc
.word start
.word start

*=start
LDA #$00
LDX #$ff
print:
INX
LDA string,X
STA stdout
BNE print

read:
LDA stdin
BEQ read
echo:
STA stdout
JMP read

string:
.text "Awaiting input"
.byte $0a
