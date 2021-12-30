//#ifndef CPU_H
//#define CPU_H
//#ifndef BASICLIBS
//#define BASICLIBS
#include <iostream>
#include <list>

#define flag_n 0x80
#define flag_v 0x40
#define flag_b 0x10
#define flag_d 0x08
#define flag_i 0x04
#define flag_z 0x02
#define flag_c 0x01

#define SR_N (reg_sr & flag_n)
#define SR_V (reg_sr & flag_v)
#define SR_B (reg_sr & flag_b)
#define SR_I (reg_sr & flag_i)
#define SR_Z (reg_sr & flag_z)
#define SR_C (reg_sr & flag_c)

#define VEC_NMI 0xFFFA
#define VEC_RST 0xFFFC
#define VEC_IRQ 0xFFFE

enum AddressingMode { addr_acc, addr_abs, add_absx, add_absy, addr_imm, add_immw, addr_imp, addr_ind, add_xind, add_indy, addr_rel, addr_zpg, add_zpgx, add_zpgy,
                      auxreg_a, auxreg_x, auxreg_y, auxre_sp, auxre_sr /* some auxiliary modes */ };
enum AluOp { alu_add, alu_sub, alu_mul, alu_div, alu_asl, alu_lsr, alu_rol, alu_ror, alu_and, alu_ora, alu_eor };

//using opcode = void(Emu6502::*)(void*);

//typedef void (*opcode)(void* op);

//#endif
class Emu6502 {
    //using opcode = void (Emu6502::*)(void*);
    public:
        typedef struct instruction {
            void (Emu6502::*func)(void*);
            char mode;
            char length;
        } instruction;
        instruction opcodes[256];

        //char *cur_byte;
        instruction *cur_opc;

        Emu6502(bool *irq, bool *nmi);

        //typedef void (Emu6502::*opcode)(void*);

        bool* _irq, _nmi;

        // Registers
        ushort  reg_pc = 0;
        char    reg_sp = 0xFF,
                reg_sr = 0x20,
                reg_a  = 0,
                reg_x  = 0,
                reg_y  = 0;
        char current_opcode;
        
        void do_instruction();
        void RESET();

    private:
        //static opcode inst_map[];

        void* get_target(char addrMode);

        void SEN(); void CLN();  void SEN(void* ign); void CLN(void* ign);
        void SEV(); void CLV();  void SEV(void* ign); void CLV(void* ign);
        void SED(); void CLD();  void SED(void* ign); void CLD(void* ign);
        void SEI(); void CLI();  void SEI(void* ign); void CLI(void* ign);
        void SEZ(); void CLZ();  void SEZ(void* ign); void CLZ(void* ign);
        void SEC(); void CLC();  void SEC(void* ign); void CLC(void* ign);

        void comp(const char op1, const char op2);
        void copy(void* src, void* dst);
        void copy(void* src, void* dst, char flag_mask);
        void push(void* src);
        void pull(void* dst);
        void push(void* src, size_t count);
        void pull(void* dst, size_t count);
        void alu_op(const char op1, const char op2, char *dest, char op_id);
        void set_flags(const ushort res, const char flag_mask);
        void set_flags(const char op1, const char op2, const ushort res, const char flag_mask);
        void incdec(char* op, char inc);

        void NOP(void* ign);
        
        void ADC(void* op);
        void SBC(void* op);
        void ASL(void* op);
        void LSR(void* op);
        void ROL(void* op);
        void ROR(void* op);
        void AND(void* op);
        void ORA(void* op);
        void EOR(void* op);
        void INC(void* op);
        void DEC(void* op);
        /*void INX();
        void DEX();
        void INY();
        void DEY();*/
        
        void LDA(void* src);
        void LDX(void* src);
        void LDY(void* src);
        void STA(void* dst);
        void STX(void* dst);
        void STY(void* dst);
        /*void TSX();
        void TSY();
        void TXA();
        void TXS();
        void TYA();
        void TAX();
        void TAY();
        void PHA();
        void PLA();
        void PHP();
        void PLP();*/
        
        void BIT(void* op);
        void CMP(void* op);
        void CPX(void* op);
        void CPY(void* op);
        
        void JMP(void *loc);
        void JMP(ushort loc);
        void BCC(void *loc);
        void BCS(void *loc);
        void BVC(void *loc);
        void BVS(void *loc);
        void BNE(void *loc);
        void BEQ(void *loc);
        void BPL(void *loc);
        void BMI(void *loc);
        void JSR(void *loc);
        void RTS(void *ign);
        void IRQ(void *ign);
        void NMI(void *ign);
        void BRK(void *ign);
        void RTI(void *ign);
};
//#endif
