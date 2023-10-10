#pragma once

#include "mem-dev.hpp"
#include "plugins/plugin-types.hpp"

class EXPORT Via : public MemoryMappedDevice {
public:
  enum ViaRegister {
    PortB,
    PortACA2,
    DDRB,
    DDRA,
    Timer1PeriodLow,
    Timer1PeriodHigh,
    Timer1LatchLow,
    Timer1LatchHigh,
    Timer2PeriodLow,
    Timer2PeriodHigh,
    ShiftReg,
    AuxControlReg,
    PeripheralControlReg,
    InterruptFlagReg,
    InterruptEnableReg,
    PortA,
  };

  enum IRQ {
    CA2_ACTEDGE = 0x01,
    CA1_ACTEDGE = 0x02,
    SHIFTREG_8S = 0x04,
    CB2_ACTEDGE = 0x08,
    CB1_ACTEDGE = 0x10,
    TIMER2_ZERO = 0x20,
    TIMER1_ZERO = 0x40,
  };

  Via();

  int pre_read(Word offset) { return 0; }
  int post_write(Word offset) { return 0; }

  Byte read(Word offset) override;
  Byte write(Word offset, Byte val) override;

  inline void update() {
    /**
     * timer 1 - once zero is reached:
     *  + 1 cycle  -> IRQ
     *  + 2 cycles -> restart (if so configured)
     */
    if (timer1_running) {
      (*timer1_period)--;

      if (*timer1_period == 0) {
        timer1_running = false;
      }
    } else {
      timer1_running = true;
      flag_interrupt(IRQ::TIMER1_ZERO);

      if (*acr & 0x40) {
        *timer1_period = *timer1_latch + 1;
      }

      // TODO: PB7 pulse
    }

    if (timer2_running) {
      (*timer2_period)--;

      if (*timer2_period == 0) {
        timer2_running = false;
        flag_interrupt(IRQ::TIMER2_ZERO);
      }

      // TODO: PB6 pulse counting mode
    }
  }

private:
  void flag_interrupt(IRQ irq);
  void clear_interrupt(IRQ irq);

  bool timer1_running = true;
  bool timer2_running = true;

  Byte *port_b = mapped_regs + PortB;
  Byte *port_a_ca2 = mapped_regs + PortACA2;
  Byte *ddrb = mapped_regs + DDRB;
  Byte *ddra = mapped_regs + DDRA;

  Byte *t1c_lo = mapped_regs + Timer1PeriodLow;
  Byte *t1c_hi = mapped_regs + Timer1PeriodHigh;
  Word *timer1_period = (Word *)t1c_lo;

  Byte *t1l_lo = mapped_regs + Timer1LatchLow;
  Byte *t1l_hi = mapped_regs + Timer1LatchHigh;
  Word *timer1_latch = (Word *)t1l_lo;

  Byte *t2c_lo = mapped_regs + Timer2PeriodLow;
  Byte *t2c_hi = mapped_regs + Timer2PeriodHigh;
  Word *timer2_period = (Word *)t2c_lo;

  Byte *shift_register = mapped_regs + ShiftReg;

  Byte *acr = mapped_regs + AuxControlReg;
  Byte *pcr = mapped_regs + PeripheralControlReg;
  Byte *ifr = mapped_regs + InterruptFlagReg;
  Byte *ier = mapped_regs + InterruptEnableReg;

  Byte *port_a = mapped_regs + PortA;
};
