#include "cpu.hpp"

extern AddressSpace add_spc;
extern Emu6502 cpu;
extern QWord cycle;
extern QWord cycle_real;

Emu6502::Emu6502() {}

Word Emu6502::get_target(AddressingMode mode) {
  switch (mode) {
    case IMM:
      return reg_pc++;
    case ZPG:
      return read(reg_pc++);
    case ZPG_X:
      return (Byte)(read(reg_pc++) + reg_x);
    case ZPG_Y:
      return (Byte)(read(reg_pc++) + reg_y);
    case ABS: {
      auto addr = read_word(reg_pc);
      reg_pc += 2;
      return addr;
    }
    case ABS_X: {
      auto addr = read_word(reg_pc) + reg_x;
      reg_pc += 2;
      return addr;
    }
    case ABS_Y: {
      auto addr = read_word(reg_pc) + reg_y;
      reg_pc += 2;
      return addr;
    }
    case IND: {
      return read_word(read_word(reg_pc));
    }
    case X_IND: {
      auto addr_zpg = (Byte)(read(reg_pc++) + reg_x);
      return read_word(addr_zpg, true);
    }
    case IND_Y: {
      auto addr_zpg = read(reg_pc++);
      return read_word(addr_zpg, true) + reg_y;
    }
  }
  return 0;
}

void Emu6502::do_instruction() {
  if (got_irq || got_nmi) {
    got_irq = got_nmi = false;
    handle_interrupt(false);
  }

  current_opcode = read(reg_pc++);

  char opc_a = (current_opcode & 0xe0) >> 5;
  char opc_b = (current_opcode & 0x1c) >> 2;
  char opc_c = (current_opcode & 0x03) >> 0;

  AddressingMode mode = NONE;
  if (current_opcode == 0x6c) {
    mode = IND;
  } else if (opc_b == 2 && opc_c == 2 /* && opc_a < 4 */) {
    mode = ACC;
  } else if ((opc_b == 2 && opc_c == 1) || (opc_b == 0 && opc_c != 1 && opc_a >= 4)) {
    mode = IMM;
  } else if (opc_b == 1) {
    mode = ZPG;
  } else if (opc_b == 5) {
    if (opc_c == 2 && (opc_a == 4 || opc_a == 5)) {
      mode = ZPG_Y;
    } else {
      mode = ZPG_X;
    }
  } else if (opc_b == 3 || current_opcode == 0x20) {
    mode = ABS;
  } else if (opc_b == 6 || current_opcode == 0xbe) {
    mode = ABS_Y;
  } else if (opc_b == 7) {
    mode = ABS_X;
  } else if (opc_b == 0 && opc_c == 1) {
    mode = X_IND;
  } else if (opc_b == 4 && opc_c == 1) {
    mode = IND_Y;
  }

  switch (current_opcode) {
    // LDA/LDX/LDY
    case 0xa9:
    case 0xa5:
    case 0xb5:
    case 0xad:
    case 0xbd:
    case 0xb9:
    case 0xa1:
    case 0xb1:
    case 0xa2:
    case 0xa6:
    case 0xb6:
    case 0xae:
    case 0xbe:
    case 0xa0:
    case 0xa4:
    case 0xb4:
    case 0xac:
    case 0xbc: {
      Byte *reg = opc_c == 0 ? &reg_y : opc_c == 1 ? &reg_a : &reg_x;
      set_reg(reg, read(get_target(mode)), FLAG_N | FLAG_Z);
      break;
    }

    // STA/STX/STY
    case 0x85:
    case 0x95:
    case 0x8d:
    case 0x9d:
    case 0x99:
    case 0x81:
    case 0x91:
    case 0x86:
    case 0x96:
    case 0x8e:
    case 0x84:
    case 0x94:
    case 0x8c: {
      Byte *reg = opc_c == 0 ? &reg_y : opc_c == 1 ? &reg_a : &reg_x;
      write(get_target(mode), *reg);
      break;
    }

    // TAX
    case 0xaa:
      set_reg(&reg_x, reg_a, FLAG_N | FLAG_Z);
      break;
    // TAY
    case 0xa8:
      set_reg(&reg_y, reg_a, FLAG_N | FLAG_Z);
      break;
    // TSX
    case 0xba:
      set_reg(&reg_x, reg_sp, FLAG_N | FLAG_Z);
      break;
    // TXA
    case 0x8a:
      set_reg(&reg_a, reg_x, FLAG_N | FLAG_Z);
      break;
    // TXS
    case 0x9a:
      set_reg(&reg_sp, reg_x);
      break;
    // TYA
    case 0x98:
      set_reg(&reg_a, reg_y, FLAG_N | FLAG_Z);
      break;

    // PHA
    case 0x48:
      push(reg_a);
      break;
    // PHP
    case 0x08:
      push(get_sr() | FLAG_B);
      break;
    // PLA
    case 0x68:
      set_reg(&reg_a, pop(), FLAG_N | FLAG_Z);
      break;
    // PLP
    case 0x28:
      set_reg(&reg_sr, pop());
      break;

    // DEC
    case 0xc6:
    case 0xd6:
    case 0xce:
    case 0xde: {
      auto addr = get_target(mode);
      auto val = read(addr) - 1;
      write(addr, val);
      set_flags(val, FLAG_N | FLAG_Z);
      break;
    }
    // DEX
    case 0xca:
      reg_x--;
      set_flags(reg_x, FLAG_N | FLAG_Z);
      break;
    // DEY
    case 0x88:
      reg_y--;
      set_flags(reg_y, FLAG_N | FLAG_Z);
      break;

    // INC
    case 0xe6:
    case 0xf6:
    case 0xee:
    case 0xfe: {
      auto addr = get_target(mode);
      auto val = read(addr) + 1;
      write(addr, val);
      set_flags(val, FLAG_N | FLAG_Z);
      break;
    }
    // INX
    case 0xe8:
      reg_x++;
      set_flags(reg_x, FLAG_N | FLAG_Z);
      break;
    // INY
    case 0xc8:
      reg_y++;
      set_flags(reg_y, FLAG_N | FLAG_Z);
      break;

    // ADC
    case 0x69:
    case 0x65:
    case 0x75:
    case 0x6d:
    case 0x7d:
    case 0x79:
    case 0x61:
    case 0x71: {
      auto operand = read(get_target(mode));
      if (SR_D)
        alu_bcd(operand, FLAG_N | FLAG_Z | FLAG_C | FLAG_V, false);
      else
        alu_add(operand, FLAG_N | FLAG_Z | FLAG_C | FLAG_V);
      break;
    }

    // SBC
    case 0xe9:
    case 0xe5:
    case 0xf5:
    case 0xed:
    case 0xfd:
    case 0xf9:
    case 0xe1:
    case 0xf1: {
      auto operand = read(get_target(mode));
      if (SR_D)
        alu_bcd(operand, FLAG_N | FLAG_Z | FLAG_C | FLAG_V, true);
      else
        alu_add(~operand, FLAG_N | FLAG_Z | FLAG_C | FLAG_V);
      break;
    }

    // AND
    case 0x29:
    case 0x25:
    case 0x35:
    case 0x2d:
    case 0x3d:
    case 0x39:
    case 0x21:
    case 0x31: {
      auto operand = read(get_target(mode));
      set_reg(&reg_a, reg_a & operand, FLAG_N | FLAG_Z);
      break;
    }

    // EOR
    case 0x49:
    case 0x45:
    case 0x55:
    case 0x4d:
    case 0x5d:
    case 0x59:
    case 0x41:
    case 0x51: {
      auto operand = read(get_target(mode));
      set_reg(&reg_a, reg_a ^ operand, FLAG_N | FLAG_Z);
      break;
    }

    // ORA
    case 0x09:
    case 0x05:
    case 0x15:
    case 0x0d:
    case 0x1d:
    case 0x19:
    case 0x01:
    case 0x11: {
      auto operand = read(get_target(mode));
      set_reg(&reg_a, reg_a | operand, FLAG_N | FLAG_Z);
      break;
    }

    // ASL
    case 0x0a:
    case 0x06:
    case 0x16:
    case 0x0e:
    case 0x1e: {
      Word addr = (mode != ACC) ? get_target(mode) : 0;
      Byte val = addr ? read(addr) : reg_a;
      Word res = (Word)val << 1;

      reg_sr &= ~FLAG_C;
      reg_sr |= (res & 0x100) >> 8;
      set_flags((Byte)res, FLAG_N | FLAG_Z);

      if (mode == ACC) {
        reg_a = (Byte)res;
      } else {
        write(addr, (Byte)res);
      }
      break;
    }

    // LSR
    case 0x4a:
    case 0x46:
    case 0x56:
    case 0x4e:
    case 0x5e: {
      Word addr = (mode != ACC) ? get_target(mode) : 0;
      Byte val = addr ? read(addr) : reg_a;
      Word res = (Word)val >> 1;

      reg_sr &= ~FLAG_C;
      reg_sr |= FLAG_C & val;
      set_flags((Byte)res, FLAG_N | FLAG_Z);

      if (mode == ACC) {
        reg_a = (Byte)res;
      } else {
        write(addr, (Byte)res);
      }
      break;
    }

    // ROL
    case 0x2a:
    case 0x26:
    case 0x36:
    case 0x2e:
    case 0x3e: {
      Word addr = (mode != ACC) ? get_target(mode) : 0;
      Byte val = addr ? read(addr) : reg_a;
      Word res = ((Word)val << 1) | SR_C;

      reg_sr &= ~FLAG_C;
      reg_sr |= (res & 0x100) >> 8;
      set_flags((Byte)res, FLAG_N | FLAG_Z);

      if (mode == ACC) {
        reg_a = (Byte)res;
      } else {
        write(addr, (Byte)res);
      }
      break;
    }

    // ROR
    case 0x6a:
    case 0x66:
    case 0x76:
    case 0x6e:
    case 0x7e: {
      Word addr = (mode != ACC) ? get_target(mode) : 0;
      Byte val = addr ? read(addr) : reg_a;
      Word res = ((Word)val >> 1) | (SR_C << 7);

      reg_sr &= ~FLAG_C;
      reg_sr |= FLAG_C & val;
      set_flags((Byte)res, FLAG_N | FLAG_Z);

      if (mode == ACC) {
        reg_a = (Byte)res;
      } else {
        write(addr, (Byte)res);
      }
      break;
    }

    // CLC
    case 0x18:
      reg_sr &= ~FLAG_C;
      break;
    // CLD
    case 0xd8:
      reg_sr &= ~FLAG_D;
      break;
    // CLI
    case 0x58:
      reg_sr &= ~FLAG_I;
      break;
    // CLV
    case 0xb8:
      reg_sr &= ~FLAG_V;
      break;
    // SEC
    case 0x38:
      reg_sr |= FLAG_C;
      break;
    // SED
    case 0xf8:
      reg_sr |= FLAG_D;
      break;
    // SEI
    case 0x78:
      reg_sr |= FLAG_I;
      break;

    // CMP/CPX/CPY
    case 0xc9:
    case 0xc5:
    case 0xd5:
    case 0xcd:
    case 0xdd:
    case 0xd9:
    case 0xc1:
    case 0xd1:
    case 0xe0:
    case 0xe4:
    case 0xec:
    case 0xc0:
    case 0xc4:
    case 0xcc: {
      auto reg = (opc_c == 1) ? reg_a : (opc_a == 7) ? reg_x : reg_y;
      auto operand = read(get_target(mode));

      alu_add(reg, ~operand, NULL, FLAG_C | FLAG_Z | FLAG_N, 1);
      break;
    }

    // BIT
    case 0x24:
    case 0x2c: {
      auto operand = read(get_target(mode));

      reg_sr &= ~(FLAG_V | FLAG_Z | FLAG_N);
      reg_sr |= FLAG_Z * !(reg_a & operand);
      reg_sr |= operand & (FLAG_N | FLAG_V);
      break;
    }

    // BPL/BMI/BVC/BVS/BCC/BCS/BNE/BEQ
    case 0x10:
    case 0x30:
    case 0x50:
    case 0x70:
    case 0x90:
    case 0xb0:
    case 0xd0:
    case 0xf0: {
      Byte flags[8] = {!SR_N, SR_N, !SR_V, SR_V, !SR_C, SR_C, !SR_Z, SR_Z};
      auto offset = read(reg_pc++);

      if (flags[opc_a]) {
        auto new_pc = reg_pc + *(SByte *)&offset;
        cycle++;
        if (reg_pc & 0xff00 != new_pc & 0xff00)
          cycle++;
        reg_pc = new_pc;
      }
      break;
    }

    // JSR
    case 0x20:
      push_word(reg_pc + 1);
    // JMP
    case 0x4c:
    case 0x6c:
      reg_pc = get_target(mode);
      break;

    // RTS
    case 0x60:
      reg_pc = pop_word() + 1;
      break;

    // RTI
    case 0x40:
      set_reg(&reg_sr, pop());
      reg_pc = pop_word();
      break;

    case 0xea:
      break;

    // BRK
    case 0x00:
    default:
      reg_pc++;
      handle_interrupt(true);
      break;
  }
}

void Emu6502::set_flags(Byte val, Byte flags) {
  reg_sr &= ~flags;
  reg_sr |= (flags & FLAG_Z) * (val == 0);
  reg_sr |= (flags & FLAG_N) & val;
}

Byte Emu6502::get_sr() { return reg_sr | 0x20; }

void Emu6502::set_reg(Byte *reg, Byte val) { set_reg(reg, val, 0); }
void Emu6502::set_reg(Byte *reg, Byte val, Byte flags) {
  if (reg == &reg_sr) {
    reg_sr = (reg_sr & (FLAG_B | 0x20)) | (val & ~(FLAG_B | 0x20));
  } else {
    set_flags(val, flags);
    if (reg)
      *reg = val;
  }
}

inline Byte Emu6502::read(Word addr) {
  cycle++;
  return add_spc.read(addr);
}
inline Word Emu6502::read_word(Word addr_lo) { return read_word(addr_lo, false); }
inline Word Emu6502::read_word(Word addr_lo, bool wrap_page) {
  auto addr_hi = addr_lo + 1;
  if (wrap_page) {
    addr_hi &= 0x00ff;
    addr_hi |= addr_lo & 0xff00;
  } else if (addr_lo & 0xFF00 != addr_hi & 0xFF00) {
    cycle++;
  }

  return (Word)read(addr_lo) + ((Word)read(addr_hi) << 8);
}

inline void Emu6502::write(Word addr, Byte val) {
  cycle++;
  add_spc.write(addr, val);
}

inline void Emu6502::move(Byte val, Byte *target) { *target = val; }

void Emu6502::push(Byte val) { write(0x100 + (reg_sp--), val); }
void Emu6502::push_word(Word data) {
  push((Byte)(data >> 8));
  push((Byte)(data & 0xFF));
}

Byte Emu6502::pop() { return read(0x100 + (++reg_sp)); }
Word Emu6502::pop_word() { return (Word)pop() + ((Word)pop() << 8); }

void Emu6502::alu_add(Byte operand, Byte flags) { alu_add(reg_a, operand, &reg_a, flags, SR_C); }
void Emu6502::alu_add(Byte op1, Byte op2, Byte *target, Byte flags, Byte carry_in) {
  Word res = (Word)op1 + op2 + carry_in;

  reg_sr &= ~flags;
  reg_sr |= (flags & FLAG_C) & (res >> 8);
  reg_sr |= (flags & FLAG_V) * ((op1 & FLAG_N) == (op2 & FLAG_N) && (op1 & FLAG_N) != (res & FLAG_N));

  set_reg(target, (Byte)res, flags & ~(FLAG_C | FLAG_V));
}

void Emu6502::alu_bcd(Byte operand, Byte flags, bool sub) {
  if (sub) // use 9's complement for subtraction
    operand = (9 - (operand & 0x0f)) | ((9 - (operand >> 4)) << 4);

  Byte op1_bcd = (reg_a & 0x0f) + 10 * (reg_a >> 4);
  signed char op2_bcd = (operand & 0x0f) + 10 * (operand >> 4);
  short res_bcd = (short)op1_bcd + op2_bcd + SR_C;
  bool carry_out = res_bcd > 99 | res_bcd < 0;
  res_bcd %= 100;
  Byte res = (res_bcd % 10) | ((res_bcd / 10) << 4);

  reg_sr &= ~flags;
  reg_sr |= (flags & FLAG_C) & carry_out;
  reg_sr |= (flags & FLAG_V) * ((reg_a & FLAG_N) == (operand & FLAG_N) && (reg_a & FLAG_N) != (res & FLAG_N));

  set_reg(&reg_a, (Byte)res, flags & ~(FLAG_C | FLAG_V));
}

void Emu6502::reset() {
  reg_a = 0;
  reg_x = 0;
  reg_y = 0;
  reg_sr = 0x20;
  reg_sp = 0xff;
  reg_pc = read_word(VEC_RST);
  cycle += 4;
}

void Emu6502::assert_interrupt(bool nmi) {
  if (nmi)
    got_nmi = true;
  else if (!SR_I)
    got_irq = true;
}

void Emu6502::handle_interrupt(bool brk) {
  push_word(reg_pc);
  push(get_sr() | (brk * FLAG_B));
  reg_sr |= FLAG_I;
  reg_pc = read_word(got_nmi ? VEC_NMI : VEC_IRQ);

  (got_nmi ? got_nmi : got_irq) = false;
}
