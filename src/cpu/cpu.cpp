#include "cpu.hpp"
#include "sysclock.hpp"

extern AddressSpace add_spc;

// hacky but reduces code duplication
#include "cpu-common.cpp"

Emu6502::Emu6502() {}

Emu6502::~Emu6502() {}

void Emu6502::do_instruction() {
  // fetch the next instruction from memory
  current_opcode = read(reg_pc++);

  // determine the addressing mode
  int opc_a = (current_opcode & 0xe0) >> 5;
  int opc_b = (current_opcode & 0x1c) >> 2;
  int opc_c = (current_opcode & 0x03) >> 0;
  AddressingMode mode = get_addr_mode(opc_a, opc_b, opc_c);

  // execute the instruction
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
      write(get_target(mode, true), *reg);
      break;
    }

    // TAX
    case 0xaa:
      set_reg(&reg_x, reg_a, FLAG_N | FLAG_Z);
      sysclock_step();
      break;
    // TAY
    case 0xa8:
      set_reg(&reg_y, reg_a, FLAG_N | FLAG_Z);
      sysclock_step();
      break;
    // TSX
    case 0xba:
      set_reg(&reg_x, reg_sp, FLAG_N | FLAG_Z);
      sysclock_step();
      break;
    // TXA
    case 0x8a:
      set_reg(&reg_a, reg_x, FLAG_N | FLAG_Z);
      sysclock_step();
      break;
    // TXS
    case 0x9a:
      set_reg(&reg_sp, reg_x);
      sysclock_step();
      break;
    // TYA
    case 0x98:
      set_reg(&reg_a, reg_y, FLAG_N | FLAG_Z);
      sysclock_step();
      break;

    // PHA
    case 0x48:
      push(reg_a);
      sysclock_step();
      break;
    // PHP
    case 0x08:
      push(get_sr() | FLAG_B);
      sysclock_step();
      break;
    // PLA
    case 0x68:
      set_reg(&reg_a, pop(), FLAG_N | FLAG_Z);
      sysclock_step(2);
      break;
    // PLP
    case 0x28:
      set_reg(&reg_sr, pop());
      sysclock_step(2);
      break;

    // DEC
    case 0xc6:
    case 0xd6:
    case 0xce:
    case 0xde: {
      auto addr = get_target(mode, true);
      auto val = read(addr) - 1;
      write(addr, val);
      set_flags(val, FLAG_N | FLAG_Z);
      sysclock_step();
      break;
    }
    // DEX
    case 0xca:
      reg_x--;
      set_flags(reg_x, FLAG_N | FLAG_Z);
      sysclock_step();
      break;
    // DEY
    case 0x88:
      reg_y--;
      set_flags(reg_y, FLAG_N | FLAG_Z);
      sysclock_step();
      break;

    // INC
    case 0xe6:
    case 0xf6:
    case 0xee:
    case 0xfe: {
      auto addr = get_target(mode, true);
      auto val = read(addr) + 1;
      write(addr, val);
      set_flags(val, FLAG_N | FLAG_Z);
      sysclock_step();
      break;
    }
    // INX
    case 0xe8:
      reg_x++;
      set_flags(reg_x, FLAG_N | FLAG_Z);
      sysclock_step();
      break;
    // INY
    case 0xc8:
      reg_y++;
      set_flags(reg_y, FLAG_N | FLAG_Z);
      sysclock_step();
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
      Word addr = (mode != ACC) ? get_target(mode, true) : 0;
      Byte val = addr ? read(addr) : reg_a;
      Word res = (Word)val << 1;

      reg_sr &= ~FLAG_C;
      reg_sr |= (res & 0x100) >> 8;
      set_flags((Byte)res, FLAG_N | FLAG_Z);
      sysclock_step();

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
      Word addr = (mode != ACC) ? get_target(mode, true) : 0;
      Byte val = addr ? read(addr) : reg_a;
      Word res = (Word)val >> 1;

      reg_sr &= ~FLAG_C;
      reg_sr |= FLAG_C & val;
      set_flags((Byte)res, FLAG_N | FLAG_Z);
      sysclock_step();

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
      Word addr = (mode != ACC) ? get_target(mode, true) : 0;
      Byte val = addr ? read(addr) : reg_a;
      Word res = ((Word)val << 1) | SR_C;

      reg_sr &= ~FLAG_C;
      reg_sr |= (res & 0x100) >> 8;
      set_flags((Byte)res, FLAG_N | FLAG_Z);
      sysclock_step();

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
      Word addr = (mode != ACC) ? get_target(mode, true) : 0;
      Byte val = addr ? read(addr) : reg_a;
      Word res = ((Word)val >> 1) | (SR_C << 7);

      reg_sr &= ~FLAG_C;
      reg_sr |= FLAG_C & val;
      set_flags((Byte)res, FLAG_N | FLAG_Z);
      sysclock_step();

      if (mode == ACC) {
        reg_a = (Byte)res;
      } else {
        write(addr, (Byte)res);
      }
      break;
    }

    // CLC/CLD/CLI/CLV / SEC/SED/SEI
    case 0x18:
    case 0xd8:
    case 0x58:
    case 0xb8:
    case 0x38:
    case 0xf8:
    case 0x78: {
      Byte flag_affected[4] = {FLAG_C, FLAG_I, FLAG_V, FLAG_D};

      // clear the flag
      if (opc_a % 2 == 0 || opc_a == 5) {
        reg_sr &= ~flag_affected[opc_a / 2];
      }
      // set the flag
      else {
        reg_sr |= flag_affected[opc_a / 2];
      }

      sysclock_step();
      break;
    }

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
      Byte conditions[8] = {!SR_N, SR_N, !SR_V, SR_V, !SR_C, SR_C, !SR_Z, SR_Z};
      auto offset = read(reg_pc++);

      if (conditions[opc_a]) {
        auto new_pc = reg_pc + *(SByte *)&offset;
        sysclock_step();
        if ((reg_pc & 0xff00) != (new_pc & 0xff00))
          sysclock_step();
        reg_pc = new_pc;
      }
      break;
    }

    // JSR
    case 0x20:
      push_word(reg_pc + 1);
      sysclock_step();
    // JMP
    case 0x4c:
    case 0x6c:
      reg_pc = get_target(mode);
      break;

    // RTS
    case 0x60:
      reg_pc = pop_word() + 1;
      sysclock_step(3);
      break;

    // RTI
    case 0x40:
      set_reg(&reg_sr, pop());
      reg_pc = pop_word();
      sysclock_step(2);
      break;

    case 0xea:
      sysclock_step();
      break;

    // BRK
    case 0x00:
    default:
      reg_pc++;
      handle_interrupt(true);
      break;
  }
}
