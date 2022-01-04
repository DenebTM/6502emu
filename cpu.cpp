#include "cpu.h"
#include <string.h>

extern AddressSpace add_spc;
extern Emu6502 cpu;
extern QWord cycle;

Emu6502::Emu6502(bool *irq, bool *nmi) { 
    _irq = irq;
    _nmi = nmi;
    // done
    opcodes[0x00] = { "BRK impl ", &Emu6502::BRK, addr_imp, 0 };        opcodes[0x01] = { "ORA x,ind", &Emu6502::ORA, add_xind, 2 };
    opcodes[0x10] = { "BPL rel  ", &Emu6502::BPL, addr_rel, 2 };        opcodes[0x11] = { "ORA ind,y", &Emu6502::ORA, add_indy, 2 };
    opcodes[0x20] = { "JSR abs  ", &Emu6502::JSR, addr_imm, 0 };        opcodes[0x21] = { "AND x,ind", &Emu6502::AND, add_xind, 2 };
    opcodes[0x30] = { "BMI rel  ", &Emu6502::BMI, addr_rel, 2 };        opcodes[0x31] = { "AND ind,y", &Emu6502::AND, add_indy, 2 };
    opcodes[0x40] = { "RTI impl ", &Emu6502::RTI, addr_imp, 0 };        opcodes[0x41] = { "EOR x,ind", &Emu6502::EOR, add_xind, 2 };
    opcodes[0x50] = { "BVC rel  ", &Emu6502::BVC, addr_rel, 2 };        opcodes[0x51] = { "EOR ind,y", &Emu6502::EOR, add_indy, 2 };
    opcodes[0x60] = { "RTS impl ", &Emu6502::RTS, addr_imp, 1 };        opcodes[0x61] = { "ADC x,ind", &Emu6502::ADC, add_xind, 2 };
    opcodes[0x70] = { "BVS rel  ", &Emu6502::BVS, addr_rel, 2 };        opcodes[0x71] = { "ADC ind,y", &Emu6502::ADC, add_indy, 2 };
                                                                        opcodes[0x81] = { "STA x,ind", &Emu6502::STA, add_xind, 2 };
    opcodes[0x90] = { "BCC rel  ", &Emu6502::BCC, addr_rel, 2 };        opcodes[0x91] = { "STA ind,y", &Emu6502::STA, add_indy, 2 };
    opcodes[0xA0] = { "LDY #    ", &Emu6502::LDY, addr_imm, 2 };        opcodes[0xA1] = { "LDA x,ind", &Emu6502::LDA, add_xind, 2 };
    opcodes[0xB0] = { "BCS rel  ", &Emu6502::BCS, addr_rel, 2 };        opcodes[0xB1] = { "LDA ind,y", &Emu6502::LDA, add_indy, 2 };
    opcodes[0xC0] = { "CPY #    ", &Emu6502::CPY, addr_imm, 2 };        opcodes[0xC1] = { "CMP x,ind", &Emu6502::CMP, add_xind, 2 };
    opcodes[0xD0] = { "BNE rel  ", &Emu6502::BNE, addr_rel, 2 };        opcodes[0xD1] = { "CMP ind,y", &Emu6502::CMP, add_indy, 2 };
    opcodes[0xE0] = { "CPX #    ", &Emu6502::CPX, addr_imm, 2 };        opcodes[0xE1] = { "SBC x,ind", &Emu6502::SBC, add_xind, 2 };
    opcodes[0xF0] = { "BEQ rel  ", &Emu6502::BEQ, addr_rel, 2 };        opcodes[0xF1] = { "SBC ind,y", &Emu6502::SBC, add_indy, 2 };

                                                                        opcodes[0x24] = { "BIT zpg  ", &Emu6502::BIT, addr_zpg, 2 };
 
                                                                        opcodes[0x84] = { "STY zpg  ", &Emu6502::STY, addr_zpg, 2 };
                                                                        opcodes[0x94] = { "STY zpg,x", &Emu6502::STY, add_zpgx, 2 };
    opcodes[0xA2] = { "LDX #    ", &Emu6502::LDX, addr_imm, 2 };        opcodes[0xA4] = { "LDY zpg  ", &Emu6502::LDY, addr_zpg, 2 };
                                                                        opcodes[0xB4] = { "LDY zpg,x", &Emu6502::LDY, add_zpgx, 2 };
                                                                        opcodes[0xC4] = { "CPY zpg  ", &Emu6502::CPY, addr_zpg, 2 };
                                                                        opcodes[0xE4] = { "CPX zpg  ", &Emu6502::CPX, addr_zpg, 2 };

    opcodes[0x05] = { "ORA zpg  ", &Emu6502::ORA, addr_zpg, 2 };        opcodes[0x06] = { "ASL zpg  ", &Emu6502::ASL, addr_zpg, 2 };
    opcodes[0x15] = { "ORA zpg,x", &Emu6502::ORA, add_zpgx, 2 };        opcodes[0x16] = { "ASL zpg,x", &Emu6502::ASL, add_zpgx, 2 };
    opcodes[0x25] = { "AND zpg  ", &Emu6502::AND, addr_zpg, 2 };        opcodes[0x26] = { "ROL zpg  ", &Emu6502::ROL, addr_zpg, 2 };
    opcodes[0x35] = { "AND zpg,x", &Emu6502::AND, add_zpgx, 2 };        opcodes[0x36] = { "ROL zpg,x", &Emu6502::ROL, add_zpgx, 2 };
    opcodes[0x45] = { "EOR zpg  ", &Emu6502::EOR, addr_zpg, 2 };        opcodes[0x46] = { "LSR zpg  ", &Emu6502::LSR, addr_zpg, 2 };
    opcodes[0x55] = { "EOR zpg,x", &Emu6502::EOR, add_zpgx, 2 };        opcodes[0x56] = { "LSR zpg,x", &Emu6502::LSR, add_zpgx, 2 };
    opcodes[0x65] = { "ADC zpg  ", &Emu6502::ADC, addr_zpg, 2 };        opcodes[0x66] = { "ROR zpg  ", &Emu6502::ROR, addr_zpg, 2 };
    opcodes[0x75] = { "ADC zpg,x", &Emu6502::ADC, add_zpgx, 2 };        opcodes[0x76] = { "ROR zpg,x", &Emu6502::ROR, add_zpgx, 2 };
    opcodes[0x85] = { "STA zpg  ", &Emu6502::STA, addr_zpg, 2 };        opcodes[0x86] = { "STX zpg  ", &Emu6502::STX, addr_zpg, 2 };
    opcodes[0x95] = { "STA zpg,x", &Emu6502::STA, add_zpgx, 2 };        opcodes[0x96] = { "STX zpg,x", &Emu6502::STX, add_zpgy, 2 };
    opcodes[0xA5] = { "LDA zpg  ", &Emu6502::LDA, addr_zpg, 2 };        opcodes[0xA6] = { "LDX zpg  ", &Emu6502::LDX, addr_zpg, 2 };
    opcodes[0xB5] = { "LDA zpg,x", &Emu6502::LDA, add_zpgx, 2 };        opcodes[0xB6] = { "LDX zpg,x", &Emu6502::LDX, add_zpgy, 2 };
    opcodes[0xC5] = { "CMP zpg  ", &Emu6502::CMP, addr_zpg, 2 };        opcodes[0xC6] = { "DEC zpg  ", &Emu6502::DEC, addr_zpg, 2 };
    opcodes[0xD5] = { "CMP zpg,x", &Emu6502::CMP, add_zpgx, 2 };        opcodes[0xD6] = { "DEC zpg,x", &Emu6502::DEC, add_zpgx, 2 };
    opcodes[0xE5] = { "SBC zpg  ", &Emu6502::SBC, addr_zpg, 2 };        opcodes[0xE6] = { "INC zpg  ", &Emu6502::INC, addr_zpg, 2 };
    opcodes[0xF5] = { "SBC zpg,x", &Emu6502::SBC, add_zpgx, 2 };        opcodes[0xF6] = { "INC zpg,x", &Emu6502::INC, add_zpgx, 2 };

    opcodes[0x08] = { "PHP impl ", &Emu6502::push,auxsr_ph, 1 };        opcodes[0x09] = { "ORA #    ", &Emu6502::ORA, addr_imm, 2 };
    opcodes[0x18] = { "CLC impl ", &Emu6502::CLC, addr_imp, 1 };        opcodes[0x19] = { "ORA abs,y", &Emu6502::ORA, add_absy, 3 };
    opcodes[0x28] = { "PLP impl ", &Emu6502::pull,auxsr_pl, 1 };        opcodes[0x29] = { "AND #    ", &Emu6502::AND, addr_imm, 2 };
    opcodes[0x38] = { "SEC impl ", &Emu6502::SEC, addr_imp, 1 };        opcodes[0x39] = { "AND abs,y", &Emu6502::AND, add_absy, 3 };
    opcodes[0x48] = { "PHA impl ", &Emu6502::push,auxreg_a, 1 };        opcodes[0x49] = { "EOR #    ", &Emu6502::EOR, addr_imm, 2 };
    opcodes[0x58] = { "CLI impl ", &Emu6502::CLI, addr_imp, 1 };        opcodes[0x59] = { "EOR abs,y", &Emu6502::EOR, add_absy, 3 };
    opcodes[0x68] = { "PLA impl ", &Emu6502::pull,auxreg_a, 1 };        opcodes[0x69] = { "ADC #    ", &Emu6502::ADC, addr_imm, 2 };
    opcodes[0x78] = { "SEI impl ", &Emu6502::SEI, addr_imp, 1 };        opcodes[0x79] = { "ADC abs,y", &Emu6502::ADC, add_absy, 3 };
    opcodes[0x88] = { "DEY impl ", &Emu6502::DEC, auxreg_y, 1 };
    opcodes[0x98] = { "TYA impl ", &Emu6502::STY, auxreg_a, 1 };        opcodes[0x99] = { "STA abs,y", &Emu6502::STA, add_absy, 3 };
    opcodes[0xA8] = { "TAY impl ", &Emu6502::STA, auxreg_y, 1 };        opcodes[0xA9] = { "LDA #    ", &Emu6502::LDA, addr_imm, 2 };
    opcodes[0xB8] = { "CLV impl ", &Emu6502::CLV, addr_imp, 1 };        opcodes[0xB9] = { "LDA abs,y", &Emu6502::LDA, add_absy, 3 };
    opcodes[0xC8] = { "INY impl ", &Emu6502::INC, auxreg_y, 1 };        opcodes[0xC9] = { "CMP #    ", &Emu6502::CMP, addr_imm, 2 };
    opcodes[0xD8] = { "CLD impl ", &Emu6502::CLD, addr_imp, 1 };        opcodes[0xD9] = { "CMP abs,y", &Emu6502::CMP, add_absy, 3 };
    opcodes[0xE8] = { "INX impl ", &Emu6502::INC, auxreg_x, 1 };        opcodes[0xE9] = { "SBC #    ", &Emu6502::SBC, addr_imm, 2 };
    opcodes[0xF8] = { "SED impl ", &Emu6502::SED, addr_imp, 1 };        opcodes[0xF9] = { "SBC abs,y", &Emu6502::SBC, add_absy, 3 };
 
    opcodes[0x0A] = { "ASL acc  ", &Emu6502::ASL, auxreg_a, 1 };
    opcodes[0x2A] = { "ROL acc  ", &Emu6502::ROL, auxreg_a, 1 };        opcodes[0x2C] = { "BIT abs  ", &Emu6502::BIT, addr_abs, 3 };
    opcodes[0x4A] = { "LSR acc  ", &Emu6502::LSR, auxreg_a, 1 };        opcodes[0x4C] = { "JMP abs  ", &Emu6502::JMP, addr_imm, 0 }; // addr_abs (duct tape fix)
    opcodes[0x6A] = { "ROR acc  ", &Emu6502::ROR, auxreg_a, 1 };        opcodes[0x6C] = { "JMP ind  ", &Emu6502::JMP, addr_abs, 0 }; // addr_ind (same, but in anticipation)
    opcodes[0x8A] = { "TXA impl ", &Emu6502::STX, auxreg_a, 1 };        opcodes[0x8C] = { "STY abs  ", &Emu6502::STY, addr_abs, 3 };
    opcodes[0x9A] = { "TXS impl ", &Emu6502::STX, auxre_sp, 1 };
    opcodes[0xAA] = { "TAX impl ", &Emu6502::STA, auxreg_x, 1 };        opcodes[0xAC] = { "LDY abs  ", &Emu6502::LDY, addr_abs, 3 };
    opcodes[0xBA] = { "TSX impl ", &Emu6502::LDX, auxre_sp, 1 };        opcodes[0xBC] = { "LDY abs,x", &Emu6502::LDY, add_absx, 3 };
    opcodes[0xCA] = { "DEX impl ", &Emu6502::DEC, auxreg_x, 1 };        opcodes[0xCC] = { "CPY abs  ", &Emu6502::CPY, addr_abs, 3 };
    opcodes[0xEA] = { "NOP impl ",          NULL, addr_imp, 1 };        opcodes[0xEC] = { "CPX abs  ", &Emu6502::CPX, addr_abs, 3 };
 
    opcodes[0x0D] = { "ORA abs  ", &Emu6502::ORA, addr_abs, 3 };        opcodes[0x0E] = { "ASL abs  ", &Emu6502::ASL, addr_abs, 3 };
    opcodes[0x1D] = { "ORA abs,x", &Emu6502::ORA, add_absx, 3 };        opcodes[0x1E] = { "ASL abs,x", &Emu6502::ASL, add_absx, 3 };
    opcodes[0x2D] = { "AND abs  ", &Emu6502::AND, addr_abs, 3 };        opcodes[0x2E] = { "ROL abs  ", &Emu6502::ROL, addr_abs, 3 };
    opcodes[0x3D] = { "AND abs,x", &Emu6502::AND, add_absx, 3 };        opcodes[0x3E] = { "ROL abs,x", &Emu6502::ROL, add_absx, 3 };
    opcodes[0x4D] = { "EOR abs  ", &Emu6502::EOR, addr_abs, 3 };        opcodes[0x4E] = { "LSR abs  ", &Emu6502::LSR, addr_abs, 3 };
    opcodes[0x5D] = { "EOR abs,x", &Emu6502::EOR, add_absx, 3 };        opcodes[0x5E] = { "LSR abs,x", &Emu6502::LSR, add_absx, 3 };
    opcodes[0x6D] = { "ADC abs  ", &Emu6502::ADC, addr_abs, 3 };        opcodes[0x6E] = { "ROR abs  ", &Emu6502::ROR, addr_abs, 3 };
    opcodes[0x7D] = { "ADC abs,x", &Emu6502::ADC, add_absx, 3 };        opcodes[0x7E] = { "ROR abs,x", &Emu6502::ROR, add_absx, 3 };
    opcodes[0x8D] = { "STA abs  ", &Emu6502::STA, addr_abs, 3 };        opcodes[0x8E] = { "STX abs  ", &Emu6502::STX, addr_abs, 3 };
    opcodes[0x9D] = { "STA abs,x", &Emu6502::STA, add_absx, 3 };
    opcodes[0xAD] = { "LDA abs  ", &Emu6502::LDA, addr_abs, 3 };        opcodes[0xAE] = { "LDX abs  ", &Emu6502::LDX, addr_abs, 3 };
    opcodes[0xBD] = { "LDA abs,x", &Emu6502::LDA, add_absx, 3 };        opcodes[0xBE] = { "LDX abs,x", &Emu6502::LDX, add_absy, 3 };
    opcodes[0xCD] = { "CMP abs  ", &Emu6502::CMP, addr_abs, 3 };        opcodes[0xCE] = { "DEC abs  ", &Emu6502::DEC, addr_abs, 3 };
    opcodes[0xDD] = { "CMP abs,x", &Emu6502::CMP, add_absx, 3 };        opcodes[0xDE] = { "DEC abs,x", &Emu6502::DEC, add_absx, 3 };
    opcodes[0xED] = { "SBC abs  ", &Emu6502::SBC, addr_abs, 3 };        opcodes[0xEE] = { "INC abs  ", &Emu6502::INC, addr_abs, 3 };
    opcodes[0xFD] = { "SBC abs,x", &Emu6502::SBC, add_absx, 3 };        opcodes[0xFE] = { "INC abs,x", &Emu6502::INC, add_absx, 3 };
}

void Emu6502::do_instruction() {
    Byte cur_byte = *add_spc[reg_pc];
    cur_opc = opcodes + cur_byte;
    void (Emu6502::*fun)(void*) = cur_opc->func;
    void* tgt = get_target(cur_opc->mode);
    if (fun) (this->*fun)(tgt);
    reg_pc += cur_opc->length;

    cycle++;
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
void* Emu6502::get_target(Byte addrMode) {
    ushort addr = 0;
    switch (addrMode) {
        case addr_abs:
            addr = *(ushort*)add_spc[reg_pc+1];
            cycle += 3;
            return add_spc[addr];
        case add_absx:
            addr = *(ushort*)add_spc[reg_pc+1] + reg_x;
            cycle += 3;
            return add_spc[addr];
        case add_absy:
            addr = *(ushort*)add_spc[reg_pc+1] + reg_y;
            cycle += 3;
            return add_spc[addr];
        case addr_imm:
        case addr_rel:
            addr = reg_pc+1;
            cycle += 1;
            return add_spc[addr];
        /*case addr_ind:
            // TODO: wrap page boundary on outer lookup
            addr = *(ushort*)add_spc[*(ushort*)add_spc[reg_pc+1]];
            cycle += 4;
            return add_spc[addr];*/
        case add_xind:
            // TODO: wrap page boundary on outer lookup
            addr = *(ushort*)add_spc[*add_spc[reg_pc+1] + reg_x];
            cycle += 5;
            return add_spc[addr];
        case add_indy:
            // TODO: wrap page boundary on outer lookup
            addr = *(ushort*)add_spc[*add_spc[reg_pc+1]] + reg_y;
            cycle += 4;
            return add_spc[addr];
        case addr_zpg:
            addr = *add_spc[reg_pc+1];
            cycle += 2;
            return add_spc[addr];
        case add_zpgx:
            addr = *add_spc[reg_pc+1] + reg_x;
            cycle += 3;
            return add_spc[addr];
        case add_zpgy:
            addr = *add_spc[reg_pc+1] + reg_y;
            cycle += 3;
            return add_spc[addr];

        case addr_acc:
        case auxreg_a:
            cycle += 1;
            return &reg_a;
        case auxreg_x:
            cycle += 1;
            return &reg_x;
        case auxreg_y:
            cycle += 1;
            return &reg_y;
        case auxre_sp:
            cycle += 1;
            return &reg_sp;
        case auxsr_ph:
            cycle += 1;
            __reg_sr = reg_sr | flag_b|0x20;
            return &__reg_sr;
        case auxsr_pl:
            cycle += 1;
            return &__reg_sr;

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

void Emu6502::copy(void* src, void *dst) { *(Byte*)dst = *(Byte*)src; }
void Emu6502::push(void *src) {
    copy(src, add_spc[0x100+(reg_sp++)]);

    cycle += 1;
}
void Emu6502::pull(void *dst) {
    copy(add_spc[0x100+(--reg_sp)], dst);
    if (dst == &__reg_sr)
        reg_sr = (__reg_sr&~(flag_b|0x20)) | (reg_sr&(flag_b|0x20));

    cycle += 2;
}
void Emu6502::push(void *src, size_t count) {
    for (size_t i = 0; i < count; i++)
        push(((Byte*)src+(count-i-1)));
}
void Emu6502::pull(void *dst, size_t count) {
    for (size_t i = 0; i < count; i++)
        pull(((Byte*)dst+i));
}

void Emu6502::alu_op(const Byte op1, const Byte op2, Byte *dest, Byte op_id) {
    ushort res = 0;
    if (dest) res = *dest;
    switch (op_id) {
        case alu_adc:
            res = op1 + (op2 + SR_C);
            set_flags(op1, op2, res, flag_n|flag_v|flag_z|flag_c);
            break;
        case alu_sbc:
            alu_op(op1, ~op2, (Byte*)&res, alu_adc);
            break;
        case alu_add:
            res = op1 + op2;
            set_flags(op1, op2, res, flag_n|flag_z);
            break;
        case alu_cmp:
            res = op1 - op2;
            set_flags(op1, op2, res, flag_n|flag_z|flag_c);
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
    if(dest) *dest = (Byte)res;
}

void Emu6502::set_flags(const ushort res, const Byte flag_mask) {
    set_flags(0x0, 0x80, res, flag_mask);
}

void Emu6502::set_flags(const Byte op1, const Byte op2,
                        const ushort res,
                        const Byte flag_mask) {
    // Backup flags that should not be set
    const Byte flags_tmp = reg_sr & ~flag_mask;
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

void Emu6502::ADC(void* op) { alu_op(reg_a, *(Byte*)op, &reg_a, alu_adc); }
void Emu6502::SBC(void* op) { alu_op(reg_a, *(Byte*)op, &reg_a, alu_sbc); }
void Emu6502::ASL(void* op) { alu_op(*(Byte*)op, 1,  (Byte*)op, alu_asl); }
void Emu6502::LSR(void* op) { alu_op(*(Byte*)op, 1,  (Byte*)op, alu_lsr); }
void Emu6502::ROL(void* op) { alu_op(*(Byte*)op, 1,  (Byte*)op, alu_rol); }
void Emu6502::ROR(void* op) { alu_op(*(Byte*)op, 1,  (Byte*)op, alu_ror); }
void Emu6502::AND(void* op) { alu_op(reg_a, *(Byte*)op, &reg_a, alu_and); }
void Emu6502::ORA(void* op) { alu_op(reg_a, *(Byte*)op, &reg_a, alu_ora); }
void Emu6502::EOR(void* op) { alu_op(reg_a, *(Byte*)op, &reg_a, alu_eor); }
void Emu6502::INC(void* op) { alu_op(*(Byte*)op,  1, (Byte*)op, alu_add); }
void Emu6502::DEC(void* op) { alu_op(*(Byte*)op, -1, (Byte*)op, alu_add); }

void Emu6502::LDA(void* src) { alu_op(*(Byte*)src, 0, &reg_a, alu_add); }
void Emu6502::LDX(void* src) { alu_op(*(Byte*)src, 0, &reg_x, alu_add); }
void Emu6502::LDY(void* src) { alu_op(*(Byte*)src, 0, &reg_y, alu_add); }
void Emu6502::STA(void* dst) { copy(&reg_a, dst); }
void Emu6502::STX(void* dst) { copy(&reg_x, dst); }
void Emu6502::STY(void* dst) { copy(&reg_y, dst); }

void Emu6502::BIT(void* op) {
    CLZ(); CLN(); CLV();
    reg_sr |= flag_z*((reg_a&*(Byte*)op)==0);
    reg_sr |= (*(Byte*)op & (flag_n|flag_v));
}
void Emu6502::CMP(void* op) { alu_op(reg_a, *(Byte*)op, NULL, alu_cmp); }
void Emu6502::CPX(void* op) { alu_op(reg_x, *(Byte*)op, NULL, alu_cmp); }
void Emu6502::CPY(void* op) { alu_op(reg_y, *(Byte*)op, NULL, alu_cmp); }

void Emu6502::JMP(ushort loc){
    reg_pc = loc;
    cycle += 1;
}
void Emu6502::JMP(void *loc) {
    memcpy(&reg_pc, loc, 2);
}
void Emu6502::BCC(void *loc) { if (!SR_C) JMP(reg_pc+*(SByte*)loc); }
void Emu6502::BCS(void *loc) { if ( SR_C) JMP(reg_pc+*(SByte*)loc); }
void Emu6502::BVC(void *loc) { if (!SR_V) JMP(reg_pc+*(SByte*)loc); }
void Emu6502::BVS(void *loc) { if ( SR_V) JMP(reg_pc+*(SByte*)loc); }
void Emu6502::BNE(void *loc) { if (!SR_Z) JMP(reg_pc+*(SByte*)loc); }
void Emu6502::BEQ(void *loc) { if ( SR_Z) JMP(reg_pc+*(SByte*)loc); }
void Emu6502::BPL(void *loc) { if (!SR_N) JMP(reg_pc+*(SByte*)loc); }
void Emu6502::BMI(void *loc) { if ( SR_N) JMP(reg_pc+*(SByte*)loc); }

void Emu6502::JSR(void *loc) {
    reg_pc += 2;
    push(&reg_pc, 2);
    JMP(loc);
}

void Emu6502::RTS(void *ign) {
    pull(&reg_pc, 2);
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
    cycle += 6;
}

void Emu6502::NMI() {
    push(&reg_pc, 2);
    push(&reg_sr);
    SEI();
    reg_pc = add_spc.read_word(VEC_NMI);
}

void Emu6502::RESET() {
    reg_sp = 0x00;
    reg_a = 0;
    reg_x = 0;
    reg_y = 0;
    reg_sr = 0x20;
    reg_pc = add_spc.read_word(VEC_RST);

    cycle += 6;
}