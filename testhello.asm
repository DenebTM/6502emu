stdout = $f001
start = $ff00

*=$fffc
.word start
.word start

*=start
LDX #$ff
print:
INX
LDA string,X
STA stdout
BNE print

done:
JMP done

string:
.text "Hello world"
.byte 0
