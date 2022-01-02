outloc = 0xF001
string = 0xc010
*=string
.text "Hello world"
.byte 0

.org 0xc000
LDX #0xFF
print:
INX
LDA string,X
STA outloc
BNE print

exit:
JMP exit 
