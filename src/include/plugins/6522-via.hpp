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
    CA2_ACTEDGE = BIT0,
    CA1_ACTEDGE = BIT1,
    SHIFTREG_8S = BIT2,
    CB2_ACTEDGE = BIT3,
    CB1_ACTEDGE = BIT4,
    TIMER2_ZERO = BIT5,
    TIMER1_ZERO = BIT6,

    ANY = 0xff,
  };

  Via(plugin_callback_t callback);

  Byte read(Word offset) override;
  Byte write(Word offset, Byte val) override;

  void update();

private:
  void flag_interrupt(IRQ irq);
  void clear_interrupt(IRQ irq);

  plugin_callback_t plugin_callback;

  bool timer1_irq_on_zero = false;
  bool timer2_irq_on_zero = false;

  Byte *port_b = mapped_regs + PortB;
  Byte *port_a_ca2 = mapped_regs + PortACA2;
  Byte *ddr_b = mapped_regs + DDRB;
  Byte *ddr_a = mapped_regs + DDRA;

  Byte *t1c_lo = mapped_regs + Timer1PeriodLow;
  Byte *t1c_hi = mapped_regs + Timer1PeriodHigh;
  Word *timer1_period = (Word *)t1c_lo;

  Byte *t1l_lo = mapped_regs + Timer1LatchLow;
  Byte *t1l_hi = mapped_regs + Timer1LatchHigh;
  Word *timer1_latch = (Word *)t1l_lo;

  Byte *t2c_lo = mapped_regs + Timer2PeriodLow;
  Byte *t2c_hi = mapped_regs + Timer2PeriodHigh;
  Word *timer2_period = (Word *)t2c_lo;
  Byte t2l_lo;

  Byte *shift_register = mapped_regs + ShiftReg;

  Byte *acr = mapped_regs + AuxControlReg;
  Byte *pcr = mapped_regs + PeripheralControlReg;
  Byte *ifr = mapped_regs + InterruptFlagReg;
  Byte *ier = mapped_regs + InterruptEnableReg;

  Byte *port_a = mapped_regs + PortA;
};
