#include "common.h"
#include "mem.h"

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

enum AddressingMode { addr_acc, addr_abs, add_absx, add_absy, addr_imm, addr_imp, addr_ind, add_xind, add_indy, addr_rel, addr_zpg, add_zpgx, add_zpgy,
                      anone, auxreg_a, auxreg_x, auxreg_y, auxre_sp, auxsr_ph, auxsr_pl /* some auxiliary modes */ };
enum AluOp { alu_adc, alu_sbc, alu_add, alu_cmp, alu_mul, alu_div, alu_asl, alu_lsr, alu_rol, alu_ror, alu_and, alu_ora, alu_eor };

class Emu6502 {
    public:
        typedef struct instruction {
            Byte name[16];
            void (Emu6502::*func)(void*);
            Byte mode;
            Byte length;
        } instruction;
        instruction opcodes[256];

        instruction *cur_opc;

        Emu6502(bool *irq, bool *nmi);

        bool* _irq, _nmi;

        // Registers
        Word  reg_pc = 0;
        Byte    reg_sp = 0x00,
                reg_sr = 0x20,
                reg_a  = 0,
                reg_x  = 0,
                reg_y  = 0;
        Byte current_opcode;
        
        void do_instruction();
        void RESET();

    private:
        Byte __reg_sr = 0x20;
        //static opcode inst_map[];

        void* get_target(Byte addrMode);

        void SEN(); void CLN();  void SEN(void* ign); void CLN(void* ign);
        void SEV(); void CLV();  void SEV(void* ign); void CLV(void* ign);
        void SED(); void CLD();  void SED(void* ign); void CLD(void* ign);
        void SEI(); void CLI();  void SEI(void* ign); void CLI(void* ign);
        void SEZ(); void CLZ();  void SEZ(void* ign); void CLZ(void* ign);
        void SEC(); void CLC();  void SEC(void* ign); void CLC(void* ign);

        void copy(void* src, void* dst);
        void push(void* src);
        void pull(void* dst);
        void push(void* src, size_t count);
        void pull(void* dst, size_t count);
        void push_sr(void* ign);
        void pull_sr(void* ign);
        void alu_op(const Byte op1, const Byte op2, Byte *dest, Byte op_id);
        void set_flags(const Word res, const Byte flag_mask);
        void set_flags(const Byte op1, const Byte op2, const Word res, const Byte flag_mask);

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
        
        void LDA(void* src);
        void LDX(void* src);
        void LDY(void* src);
        void STA(void* dst);
        void STX(void* dst);
        void STY(void* dst);
        
        void BIT(void* op);
        void CMP(void* op);
        void CPX(void* op);
        void CPY(void* op);
        
        void JMP(Word loc);
        void JMP(void *loc);
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
        void BRK(void *ign);
        void RTI(void *ign);
        void IRQ();
        void NMI();
};