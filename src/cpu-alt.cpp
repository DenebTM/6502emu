#include <iomanip>
#include <iostream>

#include "cpu.hpp"
#include "sysclock.hpp"

extern AddressSpace add_spc;
extern Emu6502 cpu;

/**
 * set up functions to handle instructions
 * illegal opcodes are logged to stderr and behave like NOP
 */
Emu6502::Emu6502() {
  opcode_map = new std::tuple<std::string, std::function<void(Emu6502::AddressingMode)>>[256];

  static std::function<void(AddressingMode)> illegal_opcode_handler = [&](AddressingMode) {
    std::cerr << std::hex << std::setw(2) << std::setfill('0') << "Encountered illegal opcode " << (int)current_opcode
              << " at pc=" << std::setw(4) << (int)reg_pc << std::endl;
  };
  for (int i = 0; i < 256; i++) {
    opcode_map[i] = {"???", illegal_opcode_handler};
  }

  static std::function<void(Byte *, AddressingMode)> load_reg_fn = [&](Byte *reg, AddressingMode mode) {
    set_reg(reg, read(get_target(mode)), FLAG_N | FLAG_Z);
  };
  opcode_map[0xa9] = {"LDA #", [&](AddressingMode mode) { load_reg_fn(&reg_a, mode); }};
  opcode_map[0xa5] = {"LDA zpg", [&](AddressingMode mode) { load_reg_fn(&reg_a, mode); }};
  opcode_map[0xb5] = {"LDA zpg,x", [&](AddressingMode mode) { load_reg_fn(&reg_a, mode); }};
  opcode_map[0xad] = {"LDA abs", [&](AddressingMode mode) { load_reg_fn(&reg_a, mode); }};
  opcode_map[0xbd] = {"LDA abs,x", [&](AddressingMode mode) { load_reg_fn(&reg_a, mode); }};
  opcode_map[0xb9] = {"LDA abs,y", [&](AddressingMode mode) { load_reg_fn(&reg_a, mode); }};
  opcode_map[0xa1] = {"LDA (ind,x)", [&](AddressingMode mode) { load_reg_fn(&reg_a, mode); }};
  opcode_map[0xb1] = {"LDA (ind),y", [&](AddressingMode mode) { load_reg_fn(&reg_a, mode); }};
  opcode_map[0xa2] = {"LDX #", [&](AddressingMode mode) { load_reg_fn(&reg_x, mode); }};
  opcode_map[0xa6] = {"LDX zpg", [&](AddressingMode mode) { load_reg_fn(&reg_x, mode); }};
  opcode_map[0xb6] = {"LDX zpg,y", [&](AddressingMode mode) { load_reg_fn(&reg_x, mode); }};
  opcode_map[0xae] = {"LDX abs", [&](AddressingMode mode) { load_reg_fn(&reg_x, mode); }};
  opcode_map[0xbe] = {"LDX abs,y", [&](AddressingMode mode) { load_reg_fn(&reg_x, mode); }};
  opcode_map[0xa0] = {"LDY #", [&](AddressingMode mode) { load_reg_fn(&reg_y, mode); }};
  opcode_map[0xa4] = {"LDY zpg", [&](AddressingMode mode) { load_reg_fn(&reg_y, mode); }};
  opcode_map[0xb4] = {"LDY zpg,x", [&](AddressingMode mode) { load_reg_fn(&reg_y, mode); }};
  opcode_map[0xac] = {"LDY abs", [&](AddressingMode mode) { load_reg_fn(&reg_y, mode); }};
  opcode_map[0xbc] = {"LDY abs,x", [&](AddressingMode mode) { load_reg_fn(&reg_y, mode); }};

  static std::function<void(Byte *, AddressingMode)> store_reg_fn = [&](Byte *reg, AddressingMode mode) {
    write(get_target(mode), *reg);
  };
  opcode_map[0x85] = {"STA zpg", [&](AddressingMode mode) { store_reg_fn(&reg_a, mode); }};
  opcode_map[0x95] = {"STA zpg,x", [&](AddressingMode mode) { store_reg_fn(&reg_a, mode); }};
  opcode_map[0x8d] = {"STA abs", [&](AddressingMode mode) { store_reg_fn(&reg_a, mode); }};
  opcode_map[0x9d] = {"STA abs,x", [&](AddressingMode mode) { store_reg_fn(&reg_a, mode); }};
  opcode_map[0x99] = {"STA abs,y", [&](AddressingMode mode) { store_reg_fn(&reg_a, mode); }};
  opcode_map[0x81] = {"STA (ind,x)", [&](AddressingMode mode) { store_reg_fn(&reg_a, mode); }};
  opcode_map[0x91] = {"STA (ind),y", [&](AddressingMode mode) { store_reg_fn(&reg_a, mode); }};
  opcode_map[0x86] = {"STX zpg", [&](AddressingMode mode) { store_reg_fn(&reg_x, mode); }};
  opcode_map[0x96] = {"STX zpg,y", [&](AddressingMode mode) { store_reg_fn(&reg_x, mode); }};
  opcode_map[0x8e] = {"STX abs", [&](AddressingMode mode) { store_reg_fn(&reg_x, mode); }};
  opcode_map[0x84] = {"STY zpg", [&](AddressingMode mode) { store_reg_fn(&reg_y, mode); }};
  opcode_map[0x94] = {"STY zpg,x", [&](AddressingMode mode) { store_reg_fn(&reg_y, mode); }};
  opcode_map[0x8c] = {"STY abs", [&](AddressingMode mode) { store_reg_fn(&reg_y, mode); }};

  opcode_map[0xaa] = {"TAX", [&](AddressingMode) { set_reg(&reg_x, reg_a, FLAG_N | FLAG_Z); }};
  opcode_map[0xa8] = {"TAY", [&](AddressingMode) { set_reg(&reg_y, reg_a, FLAG_N | FLAG_Z); }};
  opcode_map[0xba] = {"TSX", [&](AddressingMode) { set_reg(&reg_x, reg_sp, FLAG_N | FLAG_Z); }};
  opcode_map[0x8a] = {"TXA", [&](AddressingMode) { set_reg(&reg_a, reg_x, FLAG_N | FLAG_Z); }};
  opcode_map[0x9a] = {"TXS", [&](AddressingMode) { set_reg(&reg_sp, reg_x); }};
  opcode_map[0x98] = {"TYA", [&](AddressingMode) { set_reg(&reg_a, reg_y, FLAG_N | FLAG_Z); }};

  opcode_map[0x48] = {"PHA", [&](AddressingMode) { push(reg_a); }};
  opcode_map[0x08] = {"PHP", [&](AddressingMode) { push(get_sr() | FLAG_B); }};
  opcode_map[0x68] = {"PLA", [&](AddressingMode) { set_reg(&reg_a, pop(), FLAG_N | FLAG_Z); }};
  opcode_map[0x28] = {"PLP", [&](AddressingMode) { set_reg(&reg_sr, pop()); }};

  static std::function<void(AddressingMode)> dec_fn = [&](AddressingMode mode) {
    auto addr = get_target(mode);
    auto val = read(addr) - 1;
    write(addr, val);
    set_flags(val, FLAG_N | FLAG_Z);
  };
  opcode_map[0xc6] = {"DEC zpg", dec_fn};
  opcode_map[0xd6] = {"DEC zpg,x", dec_fn};
  opcode_map[0xce] = {"DEC abs", dec_fn};
  opcode_map[0xde] = {"DEC abs,x", dec_fn};
  opcode_map[0xca] = {"DEX", [&](AddressingMode) { reg_x--, set_flags(reg_x, FLAG_N | FLAG_Z); }};
  opcode_map[0x88] = {"DEY", [&](AddressingMode) { reg_y--, set_flags(reg_y, FLAG_N | FLAG_Z); }};

  static std::function<void(AddressingMode)> inc_fn = [&](AddressingMode mode) {
    auto addr = get_target(mode);
    auto val = read(addr) + 1;
    write(addr, val);
    set_flags(val, FLAG_N | FLAG_Z);
  };
  opcode_map[0xe6] = {"INC zpg", inc_fn};
  opcode_map[0xf6] = {"INC zpg,x", inc_fn};
  opcode_map[0xee] = {"INC abs", inc_fn};
  opcode_map[0xfe] = {"INC abs,x", inc_fn};
  opcode_map[0xe8] = {"INX", [&](AddressingMode) { reg_x++, set_flags(reg_x, FLAG_N | FLAG_Z); }};
  opcode_map[0xc8] = {"INY", [&](AddressingMode) { reg_y++, set_flags(reg_y, FLAG_N | FLAG_Z); }};

  static std::function<void(AddressingMode)> adc_fn = [&](AddressingMode mode) {
    auto operand = read(get_target(mode));
    if (SR_D)
      alu_bcd(operand, FLAG_N | FLAG_Z | FLAG_C | FLAG_V, false);
    else
      alu_add(operand, FLAG_N | FLAG_Z | FLAG_C | FLAG_V);
  };
  opcode_map[0x69] = {"ADC #", adc_fn};
  opcode_map[0x65] = {"ADC zpg", adc_fn};
  opcode_map[0x75] = {"ADC zpg,x", adc_fn};
  opcode_map[0x6d] = {"ADC abs", adc_fn};
  opcode_map[0x7d] = {"ADC abs,x", adc_fn};
  opcode_map[0x79] = {"ADC abs,y", adc_fn};
  opcode_map[0x61] = {"ADC (ind,x)", adc_fn};
  opcode_map[0x71] = {"ADC (ind),y", adc_fn};

  static std::function<void(AddressingMode)> sbc_fn = [&](AddressingMode mode) {
    auto operand = read(get_target(mode));
    if (SR_D)
      alu_bcd(operand, FLAG_N | FLAG_Z | FLAG_C | FLAG_V, true);
    else
      alu_add(~operand, FLAG_N | FLAG_Z | FLAG_C | FLAG_V);
  };
  opcode_map[0xe9] = {"SBC #", sbc_fn};
  opcode_map[0xe5] = {"SBC zpg", sbc_fn};
  opcode_map[0xf5] = {"SBC zpg,x", sbc_fn};
  opcode_map[0xed] = {"SBC abs", sbc_fn};
  opcode_map[0xfd] = {"SBC abs,x", sbc_fn};
  opcode_map[0xf9] = {"SBC abs,y", sbc_fn};
  opcode_map[0xe1] = {"SBC (ind,x)", sbc_fn};
  opcode_map[0xf1] = {"SBC (ind),y", sbc_fn};

  static std::function<void(AddressingMode)> and_fn = [&](AddressingMode mode) {
    auto operand = read(get_target(mode));
    set_reg(&reg_a, reg_a & operand, FLAG_N | FLAG_Z);
  };
  opcode_map[0x29] = {"AND #", and_fn};
  opcode_map[0x25] = {"AND zpg", and_fn};
  opcode_map[0x35] = {"AND zpg,x", and_fn};
  opcode_map[0x2d] = {"AND abs", and_fn};
  opcode_map[0x3d] = {"AND abs,x", and_fn};
  opcode_map[0x39] = {"AND abs,y", and_fn};
  opcode_map[0x21] = {"AND (ind,x)", and_fn};
  opcode_map[0x31] = {"AND (ind),y", and_fn};

  static std::function<void(AddressingMode)> eor_fn = [&](AddressingMode mode) {
    auto operand = read(get_target(mode));
    set_reg(&reg_a, reg_a ^ operand, FLAG_N | FLAG_Z);
  };
  opcode_map[0x49] = {"EOR #", eor_fn};
  opcode_map[0x45] = {"EOR zpg", eor_fn};
  opcode_map[0x55] = {"EOR zpg,x", eor_fn};
  opcode_map[0x4d] = {"EOR abs", eor_fn};
  opcode_map[0x5d] = {"EOR abs,x", eor_fn};
  opcode_map[0x59] = {"EOR abs,y", eor_fn};
  opcode_map[0x41] = {"EOR (ind,x)", eor_fn};
  opcode_map[0x51] = {"EOR (ind),y", eor_fn};

  static std::function<void(AddressingMode)> ora_fn = [&](AddressingMode mode) {
    auto operand = read(get_target(mode));
    set_reg(&reg_a, reg_a | operand, FLAG_N | FLAG_Z);
  };
  opcode_map[0x09] = {"ORA #", ora_fn};
  opcode_map[0x05] = {"ORA zpg", ora_fn};
  opcode_map[0x15] = {"ORA zpg,x", ora_fn};
  opcode_map[0x0d] = {"ORA abs", ora_fn};
  opcode_map[0x1d] = {"ORA abs,x", ora_fn};
  opcode_map[0x19] = {"ORA abs,y", ora_fn};
  opcode_map[0x01] = {"ORA (ind,x)", ora_fn};
  opcode_map[0x11] = {"ORA (ind),y", ora_fn};

  static std::function<void(AddressingMode)> asl_fn = [&](AddressingMode mode) {
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
  };
  opcode_map[0x0a] = {"ASL A", asl_fn};
  opcode_map[0x06] = {"ASL zpg", asl_fn};
  opcode_map[0x16] = {"ASL zpg,x", asl_fn};
  opcode_map[0x0e] = {"ASL abs", asl_fn};
  opcode_map[0x1e] = {"ASL abs,x", asl_fn};

  static std::function<void(AddressingMode)> lsr_fn = [&](AddressingMode mode) {
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
  };
  opcode_map[0x4a] = {"LSR A", lsr_fn};
  opcode_map[0x46] = {"LSR zpg", lsr_fn};
  opcode_map[0x56] = {"LSR zpg,x", lsr_fn};
  opcode_map[0x4e] = {"LSR abs", lsr_fn};
  opcode_map[0x5e] = {"LSR abs,x", lsr_fn};

  static std::function<void(AddressingMode)> rol_fn = [&](AddressingMode mode) {
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
  };
  opcode_map[0x2a] = {"ROL A", rol_fn};
  opcode_map[0x26] = {"ROL zpg", rol_fn};
  opcode_map[0x36] = {"ROL zpg,x", rol_fn};
  opcode_map[0x2e] = {"ROL abs", rol_fn};
  opcode_map[0x3e] = {"ROL abs,x", rol_fn};

  static std::function<void(AddressingMode)> ror_fn = [&](AddressingMode mode) {
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
  };
  opcode_map[0x6a] = {"ROR A", ror_fn};
  opcode_map[0x66] = {"ROR zpg", ror_fn};
  opcode_map[0x76] = {"ROR zpg,x", ror_fn};
  opcode_map[0x6e] = {"ROR abs", ror_fn};
  opcode_map[0x7e] = {"ROR abs,x", ror_fn};

  opcode_map[0x18] = {"CLC", [&](AddressingMode) { reg_sr &= ~FLAG_C; }};
  opcode_map[0xd8] = {"CLD", [&](AddressingMode) { reg_sr &= ~FLAG_D; }};
  opcode_map[0x58] = {"CLI", [&](AddressingMode) { reg_sr &= ~FLAG_I; }};
  opcode_map[0xb8] = {"CLV", [&](AddressingMode) { reg_sr &= ~FLAG_V; }};
  opcode_map[0x38] = {"SEC", [&](AddressingMode) { reg_sr |= FLAG_C; }};
  opcode_map[0xf8] = {"SED", [&](AddressingMode) { reg_sr |= FLAG_D; }};
  opcode_map[0x78] = {"SEI", [&](AddressingMode) { reg_sr |= FLAG_I; }};

  static std::function<void(Byte *, AddressingMode)> cmp_fn = [&](Byte *reg, AddressingMode mode) {
    auto operand = read(get_target(mode));

    alu_add(*reg, ~operand, NULL, FLAG_C | FLAG_Z | FLAG_N, 1);
  };
  opcode_map[0xc9] = {"CMP #", [&](AddressingMode mode) { cmp_fn(&reg_a, mode); }};
  opcode_map[0xc5] = {"CMP zpg", [&](AddressingMode mode) { cmp_fn(&reg_a, mode); }};
  opcode_map[0xd5] = {"CMP zpg,x", [&](AddressingMode mode) { cmp_fn(&reg_a, mode); }};
  opcode_map[0xcd] = {"CMP abs", [&](AddressingMode mode) { cmp_fn(&reg_a, mode); }};
  opcode_map[0xdd] = {"CMP abs,x", [&](AddressingMode mode) { cmp_fn(&reg_a, mode); }};
  opcode_map[0xd9] = {"CMP abs,y", [&](AddressingMode mode) { cmp_fn(&reg_a, mode); }};
  opcode_map[0xc1] = {"CMP (ind,x)", [&](AddressingMode mode) { cmp_fn(&reg_a, mode); }};
  opcode_map[0xd1] = {"CMP (ind),y", [&](AddressingMode mode) { cmp_fn(&reg_a, mode); }};
  opcode_map[0xe0] = {"CPX #", [&](AddressingMode mode) { cmp_fn(&reg_x, mode); }};
  opcode_map[0xe4] = {"CPX zpg", [&](AddressingMode mode) { cmp_fn(&reg_x, mode); }};
  opcode_map[0xec] = {"CPX abs", [&](AddressingMode mode) { cmp_fn(&reg_x, mode); }};
  opcode_map[0xc0] = {"CPY #", [&](AddressingMode mode) { cmp_fn(&reg_y, mode); }};
  opcode_map[0xc4] = {"CPY zpg", [&](AddressingMode mode) { cmp_fn(&reg_y, mode); }};
  opcode_map[0xcc] = {"CPY abs", [&](AddressingMode mode) { cmp_fn(&reg_y, mode); }};

  static std::function<void(AddressingMode)> bit_fn = [&](AddressingMode mode) {
    auto operand = read(get_target(mode));

    reg_sr &= ~(FLAG_V | FLAG_Z | FLAG_N);
    reg_sr |= FLAG_Z * !(reg_a & operand);
    reg_sr |= operand & (FLAG_N | FLAG_V);
  };
  opcode_map[0x24] = {"BIT zpg", bit_fn};
  opcode_map[0x2c] = {"BIT abs", bit_fn};

  static std::function<void(Byte)> branch = [&](Byte flag) {
    auto offset = read(reg_pc++);
    if (flag) {
      auto new_pc = reg_pc + *(SByte *)&offset;
      step_cycle();
      if ((reg_pc & 0xff00) != (new_pc & 0xff00))
        step_cycle();
      reg_pc = new_pc;
    }
  };

  opcode_map[0x10] = {"BPL", [&](AddressingMode) { branch(!SR_N); }};
  opcode_map[0x30] = {"BMI", [&](AddressingMode) { branch(SR_N); }};
  opcode_map[0x50] = {"BVC", [&](AddressingMode) { branch(!SR_V); }};
  opcode_map[0x70] = {"BVS", [&](AddressingMode) { branch(SR_V); }};
  opcode_map[0x90] = {"BCC", [&](AddressingMode) { branch(!SR_C); }};
  opcode_map[0xb0] = {"BCS", [&](AddressingMode) { branch(SR_C); }};
  opcode_map[0xd0] = {"BNE", [&](AddressingMode) { branch(!SR_Z); }};
  opcode_map[0xf0] = {"BEQ", [&](AddressingMode) { branch(SR_Z); }};

  static std::function<void(AddressingMode)> jmp_fn = [&](AddressingMode mode) { reg_pc = get_target(mode); };
  opcode_map[0x4c] = {"JMP abs", jmp_fn};
  opcode_map[0x6c] = {"JMP (ind)", jmp_fn};

  opcode_map[0x20] = {"JSR abs", [&](AddressingMode mode) { push_word(reg_pc + 1), jmp_fn(mode); }};
  opcode_map[0x60] = {"RTS", [&](AddressingMode mode) { reg_pc = pop_word() + 1; }};

  opcode_map[0x00] = {"BRK", [&](AddressingMode mode) { reg_pc++, handle_interrupt(true); }};
  opcode_map[0x40] = {"RTI", [&](AddressingMode mode) { set_reg(&reg_sr, pop()), reg_pc = pop_word(); }};

  opcode_map[0xea] = {"NOP", [&](AddressingMode mode) {}};
}

Emu6502::~Emu6502() { delete[] opcode_map; }

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

void Emu6502::do_instruction() {
  if (got_irq || got_nmi) {
    got_irq = got_nmi = false;
    handle_interrupt(false);
  }

  // fetch the next instruction from memory
  current_opcode = read(reg_pc++);

  // determine addressing mode for instruction
  int opc_a = (current_opcode & 0xe0) >> 5;
  int opc_b = (current_opcode & 0x1c) >> 2;
  int opc_c = (current_opcode & 0x03) >> 0;

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

  // execute the instruction
  auto [func_name, func_exec] = opcode_map[current_opcode];
  func_exec(mode);
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
