#include "mem.h"
#include "cpu.h"
#include <string.h>

extern AddressSpace add_spc;
extern Emu6502 cpu;

Emu6502::Emu6502(bool *irq, bool *nmi) { 
    _irq = irq;
    _nmi = nmi;
    // done
    /* BRK impl  */ opcodes[0x00] = { &Emu6502::BRK, addr_imp, 2 };     /* ORA x,ind */ opcodes[0x01] = { &Emu6502::ORA, add_xind, 2 };
    /* BPL rel   */ opcodes[0x10] = { &Emu6502::BPL, addr_rel, 2 };     /* ORA ind,y */ opcodes[0x11] = { &Emu6502::ORA, add_indy, 2 };
    /* JSR abs   */ opcodes[0x20] = { &Emu6502::JSR, addr_abs, 0 };     /* AND x,ind */ opcodes[0x21] = { &Emu6502::AND, add_xind, 2 };
    /* BMI rel   */ opcodes[0x30] = { &Emu6502::BMI, addr_rel, 2 };     /* AND ind,y */ opcodes[0x31] = { &Emu6502::AND, add_indy, 2 };
    /* RTI impl  */ opcodes[0x40] = { &Emu6502::RTI, addr_imp, 1 };     /* EOR x,ind */ opcodes[0x41] = { &Emu6502::EOR, add_xind, 2 };
    /* BVC rel   */ opcodes[0x50] = { &Emu6502::BVC, addr_rel, 2 };     /* EOR ind,y */ opcodes[0x51] = { &Emu6502::EOR, add_indy, 2 };
    /* RTS impl  */ opcodes[0x60] = { &Emu6502::RTS, addr_imp, 1 };     /* ADC x,ind */ opcodes[0x61] = { &Emu6502::ADC, add_xind, 2 };
    /* BVS rel   */ opcodes[0x70] = { &Emu6502::BVS, addr_rel, 2 };     /* ADC ind,y */ opcodes[0x71] = { &Emu6502::ADC, add_indy, 2 };
                                                                        /* STA x,ind */ opcodes[0x81] = { &Emu6502::STA, add_xind, 2 };
    /* BCC rel   */ opcodes[0x90] = { &Emu6502::BCC, addr_rel, 2 };     /* STA ind,y */ opcodes[0x91] = { &Emu6502::STA, add_indy, 2 };
    /* LDY #     */ opcodes[0xA0] = { &Emu6502::LDY, addr_imm, 2 };     /* LDA x,ind */ opcodes[0xA1] = { &Emu6502::LDA, add_xind, 2 };
    /* BCS rel   */ opcodes[0xB0] = { &Emu6502::BCS, addr_rel, 2 };     /* LDA ind,y */ opcodes[0xB1] = { &Emu6502::LDA, add_indy, 2 };
    /* CPY #     */ opcodes[0xC0] = { &Emu6502::CPY, addr_imm, 2 };     /* CMP x,ind */ opcodes[0xC1] = { &Emu6502::CMP, add_xind, 2 };
    /* BNE rel   */ opcodes[0xD0] = { &Emu6502::BNE, addr_rel, 2 };     /* CMP ind,y */ opcodes[0xD1] = { &Emu6502::CMP, add_indy, 2 };
    /* CPX #     */ opcodes[0xE0] = { &Emu6502::CPX, addr_imm, 2 };     /* SBC x,ind */ opcodes[0xE1] = { &Emu6502::SBC, add_xind, 2 };
    /* BEQ rel   */ opcodes[0xF0] = { &Emu6502::BEQ, addr_rel, 2 };     /* SBC ind,y */ opcodes[0xF1] = { &Emu6502::SBC, add_indy, 2 };

    // done
                                                                        /* BIT zpg   */ opcodes[0x24] = { &Emu6502::JSR, addr_abs, 2 };                                                                        

                                                                        /* STY zpg   */ opcodes[0x84] = { &Emu6502::STY, addr_zpg, 2 };
                                                                        /* STY zpg,x */ opcodes[0x94] = { &Emu6502::STY, add_zpgx, 2 };
    /* LDX #     */ opcodes[0xA2] = { &Emu6502::LDX, addr_imm, 2 };     /* LDY zpg   */ opcodes[0xA4] = { &Emu6502::LDY, addr_zpg, 2 };
                                                                        /* LDY zpg,x */ opcodes[0xB4] = { &Emu6502::LDY, add_zpgx, 2 };
                                                                        /* CPY zpg   */ opcodes[0xC4] = { &Emu6502::CPY, addr_zpg, 2 };
                                                                        /* CPX zpg   */ opcodes[0xE4] = { &Emu6502::CPX, addr_zpg, 2 };

    // done
    /* ORA zpg   */ opcodes[0x05] = { &Emu6502::ORA, addr_zpg, 2 };     /* ASL zpg   */ opcodes[0x06] = { &Emu6502::ASL, addr_zpg, 2 };
    /* ORA zpg,x */ opcodes[0x15] = { &Emu6502::ORA, add_zpgx, 2 };     /* ASL zpg,x */ opcodes[0x16] = { &Emu6502::ASL, add_zpgx, 2 };
    /* AND zpg   */ opcodes[0x25] = { &Emu6502::AND, addr_zpg, 2 };     /* ROL zpg   */ opcodes[0x26] = { &Emu6502::ROL, addr_zpg, 2 };
    /* AND zpg,x */ opcodes[0x35] = { &Emu6502::AND, add_zpgx, 2 };     /* ROL zpg,x */ opcodes[0x36] = { &Emu6502::ROL, add_zpgx, 2 };
    /* EOR zpg   */ opcodes[0x45] = { &Emu6502::EOR, addr_zpg, 2 };     /* LSR zpg   */ opcodes[0x46] = { &Emu6502::LSR, addr_zpg, 2 };
    /* EOR zpg,x */ opcodes[0x55] = { &Emu6502::EOR, add_zpgx, 2 };     /* LSR zpg,x */ opcodes[0x56] = { &Emu6502::LSR, add_zpgx, 2 };
    /* ADC zpg   */ opcodes[0x65] = { &Emu6502::ADC, addr_zpg, 2 };     /* ROR zpg   */ opcodes[0x66] = { &Emu6502::ROR, addr_zpg, 2 };
    /* ADC zpg,x */ opcodes[0x75] = { &Emu6502::ADC, add_zpgx, 2 };     /* ROR zpg,x */ opcodes[0x76] = { &Emu6502::ROR, add_zpgx, 2 };
    /* STA zpg   */ opcodes[0x85] = { &Emu6502::STA, addr_zpg, 2 };     /* STX zpg   */ opcodes[0x86] = { &Emu6502::STX, addr_zpg, 2 };                         
    /* STA zpg,x */ opcodes[0x95] = { &Emu6502::STA, add_zpgx, 2 };     /* STX zpg,x */ opcodes[0x96] = { &Emu6502::STX, add_zpgx, 2 };
    /* LDA zpg   */ opcodes[0xA5] = { &Emu6502::LDA, addr_zpg, 2 };     /* LDX zpg   */ opcodes[0xA6] = { &Emu6502::LDX, addr_zpg, 2 };
    /* LDA zpg,x */ opcodes[0xB5] = { &Emu6502::LDA, add_zpgx, 2 };     /* LDX zpg,x */ opcodes[0xB6] = { &Emu6502::LDX, add_zpgx, 2 };
    /* CMP zpg   */ opcodes[0xC5] = { &Emu6502::CMP, addr_zpg, 2 };     /* DEC zpg   */ opcodes[0xC6] = { &Emu6502::DEC, addr_zpg, 2 };
    /* CMP zpg,x */ opcodes[0xD5] = { &Emu6502::CMP, add_zpgx, 2 };     /* DEC zpg,x */ opcodes[0xD6] = { &Emu6502::DEC, add_zpgx, 2 };
    /* SBC zpg   */ opcodes[0xE5] = { &Emu6502::SBC, addr_zpg, 2 };     /* INC zpg   */ opcodes[0xE6] = { &Emu6502::INC, addr_zpg, 2 };
    /* SBC zpg,x */ opcodes[0xF5] = { &Emu6502::SBC, add_zpgx, 2 };     /* INC zpg,x */ opcodes[0xF6] = { &Emu6502::INC, add_zpgx, 2 };

    // done
    /* PHP impl  */ opcodes[0x08] = { &Emu6502::push,auxre_sr, 1 };     /* ORA #     */ opcodes[0x09] = { &Emu6502::ORA, addr_imm, 2 };
    /* CLC impl  */ opcodes[0x18] = { &Emu6502::CLC, addr_imp, 1 };     /* ORA abs,y */ opcodes[0x19] = { &Emu6502::ORA, add_absy, 3 };
    /* PLP impl  */ opcodes[0x28] = { &Emu6502::pull,auxre_sr, 1 };     /* AND #     */ opcodes[0x29] = { &Emu6502::AND, addr_imm, 2 };
    /* SEC impl  */ opcodes[0x38] = { &Emu6502::SEC, addr_imp, 1 };     /* AND abs,y */ opcodes[0x39] = { &Emu6502::AND, add_absy, 3 };
    /* PHA impl  */ opcodes[0x48] = { &Emu6502::push,auxreg_a, 1 };     /* EOR #     */ opcodes[0x49] = { &Emu6502::EOR, addr_imm, 2 };
    /* CLI impl  */ opcodes[0x58] = { &Emu6502::CLI, addr_imp, 1 };     /* EOR abs,y */ opcodes[0x59] = { &Emu6502::EOR, add_absy, 3 };
    /* PLA impl  */ opcodes[0x68] = { &Emu6502::pull,auxreg_a, 1 };     /* ADC #     */ opcodes[0x69] = { &Emu6502::ADC, addr_imm, 2 };
    /* SEI impl  */ opcodes[0x78] = { &Emu6502::SEI, addr_imp, 1 };     /* ADC abs,y */ opcodes[0x79] = { &Emu6502::ADC, add_absy, 3 };
    /* DEY impl  */ opcodes[0x88] = { &Emu6502::DEC, auxreg_y, 1 };
    /* TYA impl  */ opcodes[0x98] = { &Emu6502::STY, auxreg_a, 1 };     /* STA abs,y */ opcodes[0x99] = { &Emu6502::STA, add_absy, 3 };
    /* TAY impl  */ opcodes[0xA8] = { &Emu6502::STA, auxreg_y, 1 };     /* LDA #     */ opcodes[0xA9] = { &Emu6502::LDA, addr_imm, 2 };
    /* CLV impl  */ opcodes[0xB8] = { &Emu6502::CLV, addr_imp, 1 };     /* LDA abs,y */ opcodes[0xB9] = { &Emu6502::LDA, add_absy, 3 };
    /* INY impl  */ opcodes[0xC8] = { &Emu6502::INC, auxreg_y, 1 };     /* CMP #     */ opcodes[0xC9] = { &Emu6502::CMP, addr_imm, 2 };
    /* CLD impl  */ opcodes[0xD8] = { &Emu6502::CLD, addr_imp, 1 };     /* CMP abs,y */ opcodes[0xD9] = { &Emu6502::CMP, add_absy, 3 };
    /* INX impl  */ opcodes[0xE8] = { &Emu6502::INC, auxreg_x, 1 };     /* SBC #     */ opcodes[0xE9] = { &Emu6502::SBC, addr_imm, 2 };
    /* SED impl  */ opcodes[0xF8] = { &Emu6502::SED, addr_imp, 1 };     /* SBC abs,y */ opcodes[0xF9] = { &Emu6502::SBC, add_absy, 3 };

    // done
    /* ASL acc   */ opcodes[0x0A] = { &Emu6502::ASL, addr_acc, 2 };
    /* ROL acc   */ opcodes[0x2A] = { &Emu6502::ROL, addr_imp, 2 };     /* BIT abs   */ opcodes[0x2C] = { &Emu6502::BIT, addr_abs, 3 };
    /* LSR acc   */ opcodes[0x4A] = { &Emu6502::LSR, addr_imp, 2 };     /* JMP abs   */ opcodes[0x4C] = { &Emu6502::JMP, addr_imm, 0 }; // addr_abs (duct tape fix)
    /* ROR acc   */ opcodes[0x6A] = { &Emu6502::ROR, addr_imp, 2 };     /* JMP ind   */ opcodes[0x6C] = { &Emu6502::JMP, addr_abs, 0 }; // addr_ind (same, but in anticipation)
    /* TXA impl  */ opcodes[0x8A] = { &Emu6502::STX, auxreg_a, 1 };     /* STY abs   */ opcodes[0x9C] = { &Emu6502::STY, addr_abs, 3 };
    /* TXS impl  */ opcodes[0x9A] = { &Emu6502::STX, auxre_sp, 1 };                                                 
    /* TAX impl  */ opcodes[0xAA] = { &Emu6502::STA, auxreg_x, 1 };     /* LDY abs   */ opcodes[0xAC] = { &Emu6502::LDY, addr_abs, 3 };
    /* TSX impl  */ opcodes[0xBA] = { &Emu6502::LDX, auxre_sp, 1 };     /* LDY abs,x */ opcodes[0xBC] = { &Emu6502::LDY, add_absx, 3 };
    /* DEX impl  */ opcodes[0xCA] = { &Emu6502::DEC, auxreg_x, 1 };     /* CPY abs   */ opcodes[0xCC] = { &Emu6502::CPY, addr_abs, 3 };
    /* NOP impl  */ opcodes[0xEA] = { &Emu6502::NOP, addr_imp, 1 };     /* CPX abs   */ opcodes[0xEC] = { &Emu6502::CPX, addr_abs, 3 };

    // done
    /* ORA abs   */ opcodes[0x0D] = { &Emu6502::ORA, addr_abs, 3 };     /* ASL abs   */ opcodes[0x0E] = { &Emu6502::ASL, addr_abs, 3 };
    /* ORA abs,x */ opcodes[0x1D] = { &Emu6502::ORA, add_absx, 3 };     /* ASL abs,x */ opcodes[0x1E] = { &Emu6502::ASL, add_absx, 3 };
    /* AND abs   */ opcodes[0x2D] = { &Emu6502::AND, addr_abs, 3 };     /* ROL abs   */ opcodes[0x2E] = { &Emu6502::ROL, addr_abs, 3 };
    /* AND abs,x */ opcodes[0x3D] = { &Emu6502::AND, add_absx, 3 };     /* ROL abs,x */ opcodes[0x3E] = { &Emu6502::ROL, add_absx, 3 };
    /* EOR abs   */ opcodes[0x4D] = { &Emu6502::EOR, addr_abs, 3 };     /* LSR abs   */ opcodes[0x4E] = { &Emu6502::LSR, addr_abs, 3 };
    /* EOR abs,x */ opcodes[0x5D] = { &Emu6502::EOR, add_absx, 3 };     /* LSR abs,x */ opcodes[0x5E] = { &Emu6502::LSR, add_absx, 3 };
    /* ADC abs   */ opcodes[0x6D] = { &Emu6502::ADC, addr_abs, 3 };     /* ROR abs   */ opcodes[0x6E] = { &Emu6502::ROR, addr_abs, 3 };
    /* ADC abs,x */ opcodes[0x7D] = { &Emu6502::ADC, add_absx, 3 };     /* ROR abs,x */ opcodes[0x7E] = { &Emu6502::ROR, add_absx, 3 };
    /* STA abs   */ opcodes[0x8D] = { &Emu6502::STA, addr_abs, 3 };     /* STX abs   */ opcodes[0x8E] = { &Emu6502::STX, addr_abs, 3 };                         
    /* STA abs,x */ opcodes[0x9D] = { &Emu6502::STA, add_absx, 3 };
    /* LDA abs   */ opcodes[0xAD] = { &Emu6502::LDA, addr_abs, 3 };     /* LDX abs   */ opcodes[0xAE] = { &Emu6502::LDX, addr_abs, 3 };
    /* LDA abs,x */ opcodes[0xBD] = { &Emu6502::LDA, add_absx, 3 };     /* LDX abs,x */ opcodes[0xBE] = { &Emu6502::LDX, add_absy, 3 };
    /* CMP abs   */ opcodes[0xCD] = { &Emu6502::CMP, addr_abs, 3 };     /* DEC abs   */ opcodes[0xCE] = { &Emu6502::DEC, addr_abs, 3 };
    /* CMP abs,x */ opcodes[0xDD] = { &Emu6502::CMP, add_absx, 3 };     /* DEC abs,x */ opcodes[0xDE] = { &Emu6502::DEC, add_absx, 3 };
    /* SBC abs   */ opcodes[0xED] = { &Emu6502::SBC, addr_abs, 3 };     /* INC abs   */ opcodes[0xEE] = { &Emu6502::INC, addr_abs, 3 };
    /* SBC abs,x */ opcodes[0xFD] = { &Emu6502::SBC, add_absx, 3 };     /* INC abs,x */ opcodes[0xFE] = { &Emu6502::INC, add_absx, 3 };
}

void Emu6502::do_instruction() {
    cur_opc = opcodes + *add_spc[reg_pc];
    void* tgt = get_target(cur_opc->mode);
    void (Emu6502::*fun)(void*) = cur_opc->func;
    (this->*fun)(tgt);
    reg_pc += cur_opc->length;
}

/**
 * Addressing Modes
 *  0: A       -> Accumulator
 *     Operand is accumulator
 *  1: Abs     -> Absolute
 *     Operand is address
 *  2: Abs,X   -> Absolute, X-indexed
 *     Operand is address + X with carry
 *  3: Abs,Y   -> Absolute, Y-indexed
 *     Operand is address + Y with carry
 *  4: #       -> Immediate
 *     Operand is next byte
 *  5: Imp     -> Implied
 *     Operand is implied
 *  6: Ind     -> Indirect
 *     Operand is address referenced at address
 *  7: X,Ind   -> X-indexed, indirect
 *     Operand is address, referenced at given zeropage address offset by X
 *  8: Ind,Y   -> Indirect, Y-indexed
 *     Operand is address offset by Y, referenced at given zeropage address
 *  9: Rel     -> Relative
 *     Branch only; target is PC + offset
 * 10: Zpg     -> Zeropage
 *     Operand is address (1-byte)
 * 11: Zpg,X   -> Zeropage, X-indexed
 *     Operand is address (1-byte) + X with carry
 * 12: Zpg,Y   -> Zeropage, Y-indexed
 *     Operand is address (1-byte) + Y with carry
 */
void* Emu6502::get_target(char addrMode) {
    ushort addr = 0;
    switch (addrMode) {
        case addr_acc:
            addr = *add_spc[reg_pc+1];
            return add_spc[addr];
        case addr_abs:
            addr = *(ushort*)add_spc[reg_pc+1];
            return add_spc[addr];
        case add_absx:
            addr = *(ushort*)add_spc[reg_pc+1] + reg_x;
            return add_spc[addr];
        case add_absy:
            addr = *(ushort*)add_spc[reg_pc+1] + reg_y;
            return add_spc[addr];
        case addr_imm:
        case addr_rel:
            addr = reg_pc+1;
            return add_spc[addr];
        case addr_ind:
            // TODO: wrap page boundary on outer lookup
            addr = *(ushort*)add_spc[*(ushort*)add_spc[reg_pc+1]];
            return add_spc[addr];
        case add_xind:
            // TODO: wrap page boundary on outer lookup
            addr = *(ushort*)add_spc[*add_spc[reg_pc+1] + reg_x];
            return add_spc[addr];
        case add_indy:
            // TODO: wrap page boundary on outer lookup
            addr = *(ushort*)add_spc[*add_spc[reg_pc+1]] + reg_y;
            return add_spc[addr];
        case addr_zpg:
            addr = *add_spc[reg_pc+1];
            return add_spc[addr];
        case add_zpgx:
            addr = *add_spc[reg_pc+1 + reg_x];
            return add_spc[addr];
        case add_zpgy:
            addr = *add_spc[reg_pc+1 + reg_y];
            return add_spc[addr];

        case auxreg_a:
            return &reg_a;
        case auxreg_x:
            return &reg_x;
        case auxreg_y:
            return &reg_y;
        case auxre_sp:
            return &reg_sp;
        case auxre_sr:
            return &reg_sr;

        default:
            return NULL;
    }
}

// Flag operations
//   only CLV, SED, CLD, SEI, CLI, SEC and CLC actually exist on the 6502
//   the others are merely helper functions
void Emu6502::SEN() { reg_sr |=  flag_n; } void Emu6502::SEN(void* ign) { SEN(); }
void Emu6502::CLN() { reg_sr &= ~flag_n; } void Emu6502::CLN(void* ign) { CLN(); }
void Emu6502::SEV() { reg_sr |=  flag_n; } void Emu6502::SEV(void* ign) { SEV(); }
void Emu6502::CLV() { reg_sr &= ~flag_v; } void Emu6502::CLV(void* ign) { CLV(); }
void Emu6502::SED() { reg_sr |=  flag_d; } void Emu6502::SED(void* ign) { SED(); }
void Emu6502::CLD() { reg_sr &= ~flag_d; } void Emu6502::CLD(void* ign) { CLD(); }
void Emu6502::SEI() { reg_sr |=  flag_i; } void Emu6502::SEI(void* ign) { SEI(); }
void Emu6502::CLI() { reg_sr &= ~flag_i; } void Emu6502::CLI(void* ign) { CLI(); }
void Emu6502::SEZ() { reg_sr |=  flag_z; } void Emu6502::SEZ(void* ign) { SEZ(); }
void Emu6502::CLZ() { reg_sr &= ~flag_z; } void Emu6502::CLZ(void* ign) { CLZ(); }
void Emu6502::SEC() { reg_sr |=  flag_c; } void Emu6502::SEC(void* ign) { SEC(); }
void Emu6502::CLC() { reg_sr &= ~flag_c; } void Emu6502::CLC(void* ign) { CLC(); }

void Emu6502::comp(const char op1, const char op2) {
    ushort res = op1 - op2;
    set_flags(op1, op2, res, flag_n|flag_z|flag_c);
}

void Emu6502::copy(void* src, void *dst) {
    *(char*)dst = *(char*)src;
}

void Emu6502::copy(void* src, void *dst, char flag_mask) {
    copy(src, dst);
    set_flags(*(ushort*)dst, flag_mask);
}

void Emu6502::push(void *src) { copy(src, add_spc[reg_sp++]); }
void Emu6502::pull(void *dst) { copy(add_spc[reg_sp--], dst); }
void Emu6502::push(void *src, size_t count) {
    for (size_t i = 0; i < count; i++)
        push((char*)((char*)src+i));
}
void Emu6502::pull(void *dst, size_t count) {
    for (size_t i = 0; i < count; i++)
        pull((char*)((char*)dst+i));
}

void Emu6502::alu_op(const char op1, const char op2,
                     char *dest,
                     char op_id) {
    ushort res;
    if (dest) res = *dest;
    switch (op_id) {
        case alu_add:
            res = op1 + op2 + SR_C;
            set_flags(op1, op2, res, flag_n|flag_v|flag_z|flag_c);
            break;
        case alu_sub:
            alu_op(op1, ~op1, (char*)&res, alu_add);
            break;
        case alu_asl:
            res = op1 << 1;
            set_flags(res, flag_n|flag_z|flag_c);
            break;
        case alu_lsr:
            res = op1 >> 1;
            set_flags(res, flag_n|flag_z);
            CLC(); if (op1&flag_c) SEC();
            break;
        case alu_rol:
            res = op1<<1 | SR_C;
            set_flags(res, flag_n|flag_z|flag_c);
            break;
        case alu_ror:
            res = op1 >> 1 | (SR_C<<7);
            set_flags(res, flag_n|flag_z);
            CLC(); if (op1&flag_c) SEC();
            break;
        case alu_and:
            res = op1 & op2;
            set_flags(res, flag_n|flag_z);
            break;
        case alu_ora:
            res = op1 | op2;
            set_flags(res, flag_n|flag_z);
            break;
        case alu_eor:
            res = op1 ^ op2;
            set_flags(res, flag_n|flag_z);
        default:
            break;
    }
    if(dest) *dest = (char)res;
}

void Emu6502::set_flags(const ushort res, const char flag_mask) {
    set_flags(0x0, 0x80, res, flag_mask);
}

void Emu6502::set_flags(const char op1, const char op2,
                        const ushort res,
                        const char flag_mask) {
    // Backup flags that should not be set
    const char flags_tmp = reg_sr & ~flag_mask;
    // Carry
    CLC(); reg_sr |= res >> 8;
    // Negative
    CLN(); reg_sr |= res & flag_n;
    // Zero
    CLZ(); reg_sr |= flag_z * ((res&0xFF) == 0);
    // Arithmetic overflow
    CLV(); reg_sr |= flag_v * ((op1&flag_n) == (op2&flag_n) && (op1&flag_n) != (res&flag_n));
    // Restore flags that should not be set
    reg_sr &= flag_mask;
    reg_sr |= flags_tmp;
}

void Emu6502::incdec(char *op, char inc) {
    char carrybak = SR_C;
    alu_op(*op, inc, op, alu_add);
    reg_sr &= ~flag_c; reg_sr |= carrybak;
}

void Emu6502::NOP(void* ign) { }

void Emu6502::ADC(void* op) { alu_op(reg_a, *(char*)op, &reg_a, alu_add); }
void Emu6502::SBC(void* op) { alu_op(reg_a, *(char*)op, &reg_a, alu_sub); }
void Emu6502::ASL(void* op) { alu_op(*(char*)op, 1,  (char*)op, alu_asl); }
void Emu6502::LSR(void* op) { alu_op(*(char*)op, 1,  (char*)op, alu_lsr); }
void Emu6502::ROL(void* op) { alu_op(*(char*)op, 1,  (char*)op, alu_rol); }
void Emu6502::ROR(void* op) { alu_op(*(char*)op, 1,  (char*)op, alu_ror); }
void Emu6502::AND(void* op) { alu_op(reg_a, *(char*)op, &reg_a, alu_and); }
void Emu6502::ORA(void* op) { alu_op(reg_a, *(char*)op, &reg_a, alu_ora); }
void Emu6502::EOR(void* op) { alu_op(reg_a, *(char*)op, &reg_a, alu_eor); }
void Emu6502::INC(void* op) { incdec((char*)op, 1); }
void Emu6502::DEC(void* op) { incdec((char*)op, -1); }

void Emu6502::LDA(void* src) { copy(src, &reg_a, flag_n|flag_z); }
void Emu6502::LDX(void* src) { copy(src, &reg_x, flag_n|flag_z); }
void Emu6502::LDY(void* src) { copy(src, &reg_y, flag_n|flag_z); }
void Emu6502::STA(void* dst) { copy(&reg_a, dst); }
void Emu6502::STX(void* dst) { copy(&reg_x, dst); }
void Emu6502::STY(void* dst) { copy(&reg_y, dst); }

void Emu6502::BIT(void* op) {
    CLZ(); CLN(); CLV();
    reg_sp |= flag_z*((reg_a&*(char*)op)==0);
    reg_sp |= (*(char*)op & (flag_n|flag_v));
}
void Emu6502::CMP(void* op) { comp(reg_a, *(char*)op); }
void Emu6502::CPX(void* op) { comp(reg_x, *(char*)op); }
void Emu6502::CPY(void* op) { comp(reg_y, *(char*)op); }

void Emu6502::JMP(void *loc) { memcpy(&reg_pc, loc, 2); }
void Emu6502::JMP(ushort loc) {memcpy(&reg_pc,&loc, 2); }
void Emu6502::BCC(void *loc) { if (!SR_C) JMP(reg_pc+*(signed char*)loc); }
void Emu6502::BCS(void *loc) { if ( SR_C) JMP(reg_pc+*(signed char*)loc); }
void Emu6502::BVC(void *loc) { if (!SR_V) JMP(reg_pc+*(signed char*)loc); }
void Emu6502::BVS(void *loc) { if ( SR_V) JMP(reg_pc+*(signed char*)loc); }
void Emu6502::BNE(void *loc) { if (!SR_Z) JMP(reg_pc+*(signed char*)loc); }
void Emu6502::BEQ(void *loc) { if ( SR_Z) JMP(reg_pc+*(signed char*)loc); }
void Emu6502::BPL(void *loc) { if (!SR_N) JMP(reg_pc+*(signed char*)loc); }
void Emu6502::BMI(void *loc) { if ( SR_N) JMP(reg_pc+*(signed char*)loc); }

void Emu6502::JSR(void *loc) {
    reg_pc += 2;
    push(&reg_pc, 2);
    JMP(loc);
}

void Emu6502::RTS(void *ign) {
    pull(&reg_pc, 2);
    reg_pc++;
}

void Emu6502::BRK(void* ign) {
    reg_pc += 2;
    IRQ();
}

void Emu6502::RTI(void* ign) {
    pull(&reg_sr);
    pull(&reg_pc, 2);
}

void Emu6502::IRQ() {
    push(&reg_pc, 2);
    push(&reg_sr);
    SEI();
    reg_pc = add_spc.read_word(VEC_IRQ);
}

void Emu6502::NMI() {
    push(&reg_pc, 2);
    push(&reg_sr);
    SEI();
    reg_pc = add_spc.read_word(VEC_NMI);
}

void Emu6502::RESET() {
    reg_sp = 0xFF;
    reg_a = 0;
    reg_x = 0;
    reg_y = 0;
    reg_sr = 0x20;
    reg_pc = add_spc.read_word(VEC_RST);
}