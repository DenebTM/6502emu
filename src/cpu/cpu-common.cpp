#include "cpu.hpp"
#include "sysclock.hpp"

extern AddressSpace add_spc;
extern Emu6502 cpu;

Emu6502::AddressingMode Emu6502::get_addr_mode(int opc_a, int opc_b, int opc_c) {
  if (current_opcode == 0x6c) {
    return IND;
  } else if (opc_b == 2 && opc_c == 2 /* && opc_a < 4 */) {
    return ACC;
  } else if ((opc_b == 2 && opc_c == 1) || (opc_b == 0 && opc_c != 1 && opc_a >= 4)) {
    return IMM;
  } else if (opc_b == 1) {
    return ZPG;
  } else if (opc_b == 5) {
    if (opc_c == 2 && (opc_a == 4 || opc_a == 5)) {
      return ZPG_Y;
    } else {
      return ZPG_X;
    }
  } else if (opc_b == 3 || current_opcode == 0x20) {
    return ABS;
  } else if (opc_b == 6 || current_opcode == 0xbe) {
    return ABS_Y;
  } else if (opc_b == 7) {
    return ABS_X;
  } else if (opc_b == 0 && opc_c == 1) {
    return X_IND;
  } else if (opc_b == 4 && opc_c == 1) {
    return IND_Y;
  }

  return NONE;
}

/**
 * get memory address of instruction operand based on addressing mode
 * also advances the program counter to point at the following instruction
 */
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

    case ACC:
    case NONE:
      return 0;
  }
  return 0;
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
  step_cycle();
  return add_spc.read(addr);
}
inline Word Emu6502::read_word(Word addr_lo) { return read_word(addr_lo, false); }
inline Word Emu6502::read_word(Word addr_lo, bool wrap_page) {
  auto addr_hi = addr_lo + 1;
  if (wrap_page) {
    addr_hi &= 0x00ff;
    addr_hi |= addr_lo & 0xff00;
  } else if ((addr_lo & 0xff00) != (addr_hi & 0xff00)) {
    step_cycle();
  }

  return (Word)read(addr_lo) + ((Word)read(addr_hi) << 8);
}

inline void Emu6502::write(Word addr, Byte val) {
  step_cycle();
  add_spc.write(addr, val);
}

inline void Emu6502::move(Byte val, Byte *target) { *target = val; }

void Emu6502::push(Byte val) { write(0x100 + (reg_sp--), val); }
void Emu6502::push_word(Word data) {
  push((Byte)(data >> 8));
  push((Byte)(data & 0xff));
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
  bool carry_out = (res_bcd > 99) | (res_bcd < 0);
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

  for (int i = 0; i < 4; i++)
    step_cycle();
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

  for (int i = 0; i < 2; i++)
    step_cycle();
}
