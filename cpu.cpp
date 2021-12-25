#include "mem.h"
#include "cpu.h"

extern AddressSpace add_spc;
extern Emu6502 cpu;

enum AddressingMode { addr_acc, addr_abs, addr_absx, addr_abs_y, addr_imm, addr_imp, addr_ind, addr_xind, addr_indy, addr_rel, addr_zpg, addr_zpgx, addr_zpgy };
enum AluOp { alu_add, alu_sub, alu_mul, alu_div, alu_asl, alu_lsr, alu_rol, alu_ror, alu_and, alu_ora };
enum flags { flag_n = 0x80, flag_v = 0x40, flag_b = 0x10, flag_d = 0x08, flag_i = 0x04, flag_z = 0x02, flag_c = 0x01 };

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
 *     Operand is address, referenced at given address offset by X
 *  8: Ind,Y   -> Indirect, Y-indexed
 *     Operand is address offset by Y, referenced at given address
 *  9: Rel     -> Relative
 *     Branch only; target is PC + offset
 * 10: Zpg     -> Zeropage
 *     Operand is address (1-byte)
 * 11: Zpg,X   -> Zeropage, X-indexed
 *     Operand is address (1-byte) + X with carry
 * 12: Zpg,Y   -> Zeropage, Y-indexed
 *     Operand is address (1-byte) + Y with carry
 */
ushort Emu6502::get_target_addr(char addrMode) {
    switch (addrMode) {
        case addr_acc:
            return add_spc.read_word((++reg_pc)++, false);
        case addr_abs:
            return add_spc.read_word((++reg_pc)++ + reg_x, false);
        case addr_absx:
            return add_spc.read_word((++reg_pc)++ + reg_y, false);
        case addr_imm:
        case addr_rel:
            return ++reg_pc;
        case addr_ind:
            return add_spc.read_word(add_spc.read_word((++reg_pc)++, false), true);
        case addr_xind:
            return add_spc.read_word(add_spc.read_word((++reg_pc)++ + reg_x, false), true);
        case addr_indy:
            return add_spc.read_word(add_spc.read_word((++reg_pc)++, false) + reg_y, true);
        case addr_zpg:
            return *add_spc[++reg_pc];
        case addr_zpgx:
            return *add_spc[++reg_pc] + reg_x;
        case addr_zpgy:
            return *add_spc[++reg_pc] + reg_y;
        default:
            return 0;
    }
}

// Flag operations
//   only CLV, SED, CLD, SEI, CLI, SEC and CLC actually exist on the 6502
//   the others are merely helper functions
void Emu6502::SEN() { reg_sr |=  flag_n; }
void Emu6502::CLN() { reg_sr &= ~flag_n; }
void Emu6502::SEV() { reg_sr |=  flag_n; }
void Emu6502::CLV() { reg_sr &= ~flag_v; }
void Emu6502::SED() { reg_sr |=  flag_d; }
void Emu6502::CLD() { reg_sr &= ~flag_d; }
void Emu6502::SEI() { reg_sr |=  flag_i; }
void Emu6502::CLI() { reg_sr &= ~flag_i; }
void Emu6502::SEZ() { reg_sr |=  flag_z; }
void Emu6502::CLZ() { reg_sr &= ~flag_z; }
void Emu6502::SEC() { reg_sr |=  flag_c; }
void Emu6502::CLC() { reg_sr &= ~flag_c; }

void Emu6502::comp(const char op1, const char op2) {
    ushort res = op1 - op2;
    set_flags(op1, op2, res, flag_n|flag_z|flag_c);
    reg_pc++;
}

void Emu6502::copy(const char *src, char *dst) {
    *dst = *src;
    reg_pc++;
}

void Emu6502::push(char *src) { copy(src, add_spc[reg_sp++]); }
void Emu6502::pull(char *dst) { copy(add_spc[reg_sp--], dst); }

void Emu6502::alu_op(const char op1, const char op2,
                     char *dest,
                     char op_id) {
    /**
     *  0: add
     *  1: sub
     *  2: multiply
     *  3: divide
     *  4: shift left
     *  5: shift right
     *  6: rotate left
     *  7: rotate right
     *  8: AND
     *  9: OR
     */
    //char tmp = 0;
    ushort res = *dest;
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
        case alu_and:
            res = op1 & op2;
            set_flags(res, flag_n|flag_z);
            break;
        case alu_ora:
            res = op1 | op2;
            set_flags(res, flag_n|flag_z);
            break;
        default:
            break;
    }
    *dest = (char)res;
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
    CLZ(); reg_sr |= flag_z * (res&0xFF == 0);
    // Arithmetic overflow
    CLV(); reg_sr |= flag_v * ((op1&flag_n == op2&flag_n) && (op1&flag_n != res&flag_n));
    // Restore flags that should not be set
    reg_sr &= flag_mask;
    reg_sr |= flags_tmp;
}

void Emu6502::incdec(char *op, char inc) {
    char carrybak = SR_C;
    alu_op(*op, inc, op, alu_add);
    reg_sr &= flag_c; reg_sr |= carrybak;
}

void Emu6502::ADC(const char *op) { alu_op(reg_a, *op, &reg_a, alu_add); }
void Emu6502::SBC(const char *op) { alu_op(reg_a, *op, &reg_a, alu_sub); }
void Emu6502::ASL(char *op) { alu_op(*op, 1, op, alu_asl); }
void Emu6502::LSR(char *op) { alu_op(*op, 1, op, alu_lsr); }
void Emu6502::ROL(char *op) { alu_op(*op, 1, op, alu_rol); }
void Emu6502::ROR(char *op) { alu_op(*op, 1, op, alu_ror); }
void Emu6502::AND(const char *op) { alu_op(reg_a, *op, &reg_a, alu_and); }
void Emu6502::ORA(const char *op) { alu_op(reg_a, *op, &reg_a, alu_ora); }
void Emu6502::INC(char *op) { incdec(op, 1); }
void Emu6502::DEC(char *op) { incdec(op, -1); }
void Emu6502::INX() { incdec(&reg_x, 1); }
void Emu6502::DEX() { incdec(&reg_x, -1); }
void Emu6502::INY() { incdec(&reg_y, 1); }
void Emu6502::DEY() { incdec(&reg_y, -1); }

