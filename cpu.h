//#ifndef CPU_H
//#define CPU_H
//#ifndef BASICLIBS
//#define BASICLIBS
#include <iostream>
#include <list>

#define SR_N (reg_sr & flag_n)
#define SR_V (reg_sr & flag_v)
#define SR_B (reg_sr & flag_b)
#define SR_I (reg_sr & flag_i)
#define SR_Z (reg_sr & flag_z)
#define SR_C (reg_sr & flag_c)

//#endif
class Emu6502 {
    public:
        Emu6502(bool *irq, bool *nmi) { _irq = irq; _nmi = nmi; }

        bool* _irq, _nmi;

        // Registers
        ushort  reg_pc = 0;
        char    reg_sp = 0xFF,
                reg_sr = 0x20,
                reg_a  = 0,
                reg_x  = 0,
                reg_y  = 0;
        char current_opcode;
        
        void run();
        void instruction();

    private:
        ushort get_target_addr(char addrMode);

        void SEN(); void CLN();
        void SEV(); void CLV();
        void SED(); void CLD();
        void SEI(); void CLI();
        void SEZ(); void CLZ();
        void SEC(); void CLC();

        void comp(const char op1, const char op2);
        void copy(const char *src, char *dst);
        void push(char *src);
        void pull(char *dst);
        void alu_op(const char op1, const char op2, char *dest, char op_id);
        void set_flags(const ushort res, const char flag_mask);
        void set_flags(const char op1, const char op2, const ushort res, const char flag_mask);
        
        void ADC(const char *op);
        void SBC(const char *op);
        void ASL(char *op);
        void LSR(char *op);
        void ROL(char *op);
        void ROR(char *op);
        void AND(const char *op);
        void ORA(const char *op);
        void incdec(char *op, char inc);
        void INC(char *op);
        void DEC(char *op);
        void INX(); void DEX();
        void INY(); void DEY();
        
        void LDA(const char *op);
        void LDX(const char *op);
        void LDY(const char *op);
        void STA(char *loc);
        void STX(char *loc);
        void STY(char *loc);
        void TSX();
        void TSY();
        void TXA();
        void TXS();
        void TYA();
        void PHA();
        void PLA();
        void PHP();
        void PLP();
        
        void BIT(const char *op);
        void CMP(const char *op);
        void CPX(const char *op);
        void CPY(const char *op);
        
        void JMP(char *loc);
        void JSR(char *loc);
        void RTS();
        void BCC(const char *loc);
        void BCS(const char *loc);
        void BNE(const char *loc);
        void BEQ(const char *loc);
        void BVC(const char *loc);
        void BVS(const char *loc);
        void BPL(const char *loc);
        void BMI(const char *loc);
        void IRQ();
        void NMI();
        void BRK();
        void RTI();
        void RESET();
};
//#endif
