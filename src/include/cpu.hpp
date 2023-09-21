#include <string>

#include "emu-common.hpp"
#include "mem.hpp"

#define FLAG_N 0x80
#define FLAG_V 0x40
#define FLAG_B 0x10
#define FLAG_D 0x08
#define FLAG_I 0x04
#define FLAG_Z 0x02
#define FLAG_C 0x01

#define SR_N (Byte)(reg_sr & FLAG_N)
#define SR_V (Byte)(reg_sr & FLAG_V)
#define SR_B (Byte)(reg_sr & FLAG_B)
#define SR_D (Byte)(reg_sr & FLAG_D)
#define SR_I (Byte)(reg_sr & FLAG_I)
#define SR_Z (Byte)(reg_sr & FLAG_Z)
#define SR_C (Byte)(reg_sr & FLAG_C)

#define VEC_NMI 0xFFFA
#define VEC_RST 0xFFFC
#define VEC_IRQ 0xFFFE

class Emu6502 {
public:
  enum AddressingMode { NONE, ACC, IMM, ZPG, ZPG_X, ZPG_Y, ABS, ABS_X, ABS_Y, IND, X_IND, IND_Y };
  enum CpuRegister { REG_A, REG_X, REG_Y, REG_SP, REG_SR };

  Emu6502();

  Word reg_pc = 0;
  Byte reg_sp = 0x00;
  Byte reg_sr = 0x20;
  Byte reg_a = 0;
  Byte reg_x = 0;
  Byte reg_y = 0;

  Byte current_opcode;

  void do_instruction();
  void reset();

  void assert_interrupt(bool nmi);

private:
  Word get_target(AddressingMode mode);

  Byte read(Word addr);
  Word read_word(Word addr_lo);
  Word read_word(Word addr_lo, bool wrap_page);
  void write(Word addr, Byte val);

  void move(Byte val, Byte *target);

  void push(Byte data);
  void push_word(Word data);
  Byte pop();
  Word pop_word();

  void set_flags(Byte val, Byte flags);

  Byte get_sr();
  void set_reg(Byte *reg, Byte val);
  void set_reg(Byte *reg, Byte val, Byte flags);

  void alu_add(Byte operand, Byte flags);
  void alu_add(Byte op1, Byte op2, Byte *target, Byte flags, Byte carry_in);

  void handle_interrupt();
  bool got_irq;
  bool got_nmi;
};