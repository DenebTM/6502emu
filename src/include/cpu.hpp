#pragma once
#include <atomic>
#include <functional>
#include <string>
#include <tuple>

#include "emu-types.hpp"
#include "mem.hpp"

#define FLAG_N 0x80 // status register bit 7 - negative
#define FLAG_V 0x40 // status register bit 6 - signed operation overflow
#define FLAG_B 0x10 // status register bit 4 - break (set on stack when pushed by BRK)
#define FLAG_D 0x08 // status register bit 3 - BCD arithmetic
#define FLAG_I 0x04 // status register bit 2 - interrupt disable
#define FLAG_Z 0x02 // status register bit 1 - zero
#define FLAG_C 0x01 // status register bit 0 - carry

#define SR_N (Byte)(reg_sr & FLAG_N) // current bit 7 of status register
#define SR_V (Byte)(reg_sr & FLAG_V) // current bit 6 of status register
#define SR_D (Byte)(reg_sr & FLAG_D) // current bit 3 of status register
#define SR_I (Byte)(reg_sr & FLAG_I) // current bit 2 of status register
#define SR_Z (Byte)(reg_sr & FLAG_Z) // current bit 1 of status register
#define SR_C (Byte)(reg_sr & FLAG_C) // current bit 0 of status register

#define VEC_NMI 0xfffa // word at this address is loaded into program counter on NMI
#define VEC_RST 0xfffc // word at this address is loaded into program counter on RESET
#define VEC_IRQ 0xfffe // word at this address is loaded into program counter on IRQ/BRK

class Emu6502 {
public:
  enum AddressingMode { NONE, ACC, IMM, ZPG, ZPG_X, ZPG_Y, ABS, ABS_X, ABS_Y, IND, X_IND, IND_Y };

  Emu6502();
  ~Emu6502();

  // handle reset, interrupts, loading register values from `new_` variables, etc.
  void do_instruction_pre();

  // fetch and execute a single instruction from memory
  void do_instruction();

  // signal an interrupt to the CPU
  void assert_interrupt(bool nmi);

  Word reg_pc = 0;
  Byte reg_sp = 0x00;
  Byte reg_sr = 0x20;
  Byte reg_a = 0;
  Byte reg_x = 0;
  Byte reg_y = 0;

  Byte current_opcode;

  std::atomic_bool do_reset = true;
  std::atomic_int new_pc = -1;
  std::atomic_int new_sp = -1;
  std::atomic_int new_sr = -1;
  std::atomic_int new_a = -1;
  std::atomic_int new_x = -1;
  std::atomic_int new_y = -1;
  long long step_instructions = -1;

private:
  AddressingMode get_addr_mode(int opc_a, int opc_b, int opc_c);
  Word get_target(AddressingMode mode, bool index_always_adds_cycle = false);

  Byte read(Word addr);
  Word read_word(Word addr_lo, bool wrap_page = false);
  void write(Word addr, Byte val);

  void move(Byte val, Byte *target);

  void push(Byte data);
  void push_word(Word data);
  Byte pop();
  Word pop_word();

  void set_flags(Byte val, Byte flags);

  Byte get_sr();
  void set_reg(Byte *reg, Byte val, Byte flags = 0);

  void alu_add(Byte operand, Byte flags);
  void alu_add(Byte op1, Byte op2, Byte *target, Byte flags, Byte carry_in);
  void alu_bcd(Byte operand, Byte flags, bool sub);

  void handle_interrupt(bool brk);
  std::atomic_bool got_irq;
  std::atomic_bool got_nmi;

  void reset();

  std::tuple<std::string, std::function<void(Emu6502::AddressingMode)>> *opcode_map;
};
