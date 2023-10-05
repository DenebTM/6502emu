irq_vec = $fffa
res_vec = $fffc
nmi_vec = $fffe

start = $f000

*=res_vec
.word start

*=irq_vec
.word irq_target
*=nmi_vec
.word irq_target

*=start
          ADC #$69
          ADC $65
          ADC $75,X
          ADC $6D6D
          ADC $7D7D,X
          ADC $7979,Y
          ADC ($61,X)
          ADC ($71),Y
          AND #$29
          AND $25
          AND $35,X
          AND $2D2D
          AND $3D3D,X
          AND $3939,Y
          AND ($21,X)
          AND ($31),Y
          ASL A
          ASL $06
          ASL $16,X
          ASL $0E0E
          ASL $1E1E,X
          BIT $24
          BIT $2C2C
          CLC
          CLD
          CLI
          CLV
          CMP #$C9
          CMP $C5
          CMP $D5,X
          CMP $CDCD
          CMP $DDDD,X
          CMP $D9D9,Y
          CMP ($C1,X)
          CMP ($D1),Y
          CPX #$E0
          CPX $E4
          CPX $ECEC
          CPY #$C0
          CPY $C4
          CPY $CCCC
          DEC $C6
          DEC $D6,X
          DEC $CECE
          DEC $DEDE,X
          DEX
          DEY
          EOR #$49
          EOR $45
          EOR $55,X
          EOR $4D4D
          EOR $5D5D,X
          EOR $5959,Y
          EOR ($41,X)
          EOR ($51),Y
          INC $E6
          INC $F6,X
          INC $EEEE
          INC $FEFE,X
          INX
          INY
          LDA #$A9
          LDA $A5
          LDA $B5,X
          LDA $ADAD
          LDA $BDBD,X
          LDA $B9B9,Y
          LDA ($A1,X)
          LDA ($B1),Y
          LDX #$A2
          LDX $A6
          LDX $B6,Y
          LDX $AEAE
          LDX $BEBE,Y
          LDY #$A0
          LDY $A4
          LDY $B4,X
          LDY $ACAC
          LDY $BCBC,X
          LSR A
          LSR $46
          LSR $56,X
          LSR $4E4E
          LSR $5E5E,X
          NOP
          ORA #$09
          ORA $05
          ORA $15,X
          ORA $0D0D
          ORA $1D1D,X
          ORA $1919,Y
          ORA ($01,X)
          ORA ($11),Y
          PHA
          PHP
          PLA
          PLP
          ROL A
          ROL $26
          ROL $36,X
          ROL $2E2E
          ROL $3E3E,X
          ROR A
          ROR $66
          ROR $76,X
          ROR $6E6E
          ROR $7E7E,X
          SBC #$E9
          SBC $E5
          SBC $F5,X
          SBC $EDED
          SBC $FDFD,X
          SBC $F9F9,Y
          SBC ($E1,X)
          SBC ($F1),Y
          SEC
          SED
          SEI
          STA $85
          STA $95,X
          STA $8D8D
          STA $9D9D,X
          STA $9999,Y
          STA ($81,X)
          STA ($91),Y
          STX $86
          STX $96,Y
          STX $8E8E
          STY $84
          STY $94,X
          STY $8C8C
          TAX
          TAY
          TSX
          TXA
          TXS
          TYA

bpl_no:
          LDA #$80
          BPL bmi_no
bpl_yes:
          LDA #$00
          BPL bmi_no
bmi_no:
          LDA #$00
          BMI bvc_no
bmi_yes:
          LDA #$80
          BMI bvc_no
bvc_no:
          LDA #$40
          ADC #$40
          BVC bvs_no
bvc_yes:
          CLV
          BVC bvs_no
bvs_no:
          CLV
          BVS bcc_no
bvs_yes:
          LDA #$40
          ADC #$40
          BVS bcc_no
bcc_no:
          SEC
          BCC bcs_no
bcc_yes:
          CLC
          BCC bcs_no
bcs_no:
          CLC
          BCS bne_no
bcs_yes:
          SEC
          BCS bne_no
bne_no:
          LDA #$00
          BNE beq_no
bne_yes:
          LDA #$01
          BNE beq_no
beq_no:
          LDA #$01
          BEQ branch_end
beq_yes:
          LDA #$00
          BEQ branch_end

branch_end:
          JSR jsr_target
          JMP jmp_target

jsr_target:
          RTS

jmp_target:
          BRK

irq_target:
          RTI
