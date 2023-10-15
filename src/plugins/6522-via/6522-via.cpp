#include <chrono>
#include <filesystem>
#include <fstream>
using namespace std::chrono_literals;

#include "emu-types.hpp"
#include "plugin-callback.hpp"
#include "plugins/6522-via.hpp"

Via::Via(plugin_callback_t callback) : MemoryMappedDevice(false, 16) {
  this->plugin_callback = callback;

  *ddr_a = 0;
  *ddr_b = 0;

  *ifr = 0;
  *ier = BIT7;

  *timer1_period = 0xffff;
  *timer2_period = 0;
}

Byte Via::read(Word offset) {
  if (offset == PortACA2) {
    offset = PortA;

    // TODO: perform CA2 handshake
  }

  else if (offset == Timer1PeriodLow || offset == Timer1LatchLow) {
    clear_interrupt(IRQ::TIMER1_ZERO);
  } else if (offset == Timer2PeriodLow) {
    clear_interrupt(IRQ::TIMER2_ZERO);
  }

  else if (offset == InterruptFlagReg) {
    clear_interrupt(IRQ::ANY);
  }

  return mapped_regs[offset];
}

Byte Via::write(Word offset, Byte val) {
  if (offset == PortACA2) {
    offset = PortA;

    // TODO: perform CA2 handshake
  }
  if (offset == PortA) {
    val = (*port_a & ~*ddr_a) | (val & *ddr_a);
  } else if (offset == PortB) {
    val = (*port_b & ~*ddr_b) | (val & *ddr_b);
  }

  else if (offset == InterruptFlagReg) {
    val = *ifr & ~(val & ~BIT7);
  }

  else if (offset == InterruptEnableReg) {
    bool enable_irqs = val >> 7;
    if (enable_irqs) {
      val = *ier | (val & ~BIT7);
    } else {
      val = *ier & ~(val & ~BIT7);
    }
  }

  else if (offset == Timer1PeriodLow) {
    offset = Timer1LatchLow;
  } else if (offset == Timer1PeriodHigh) {
    *t1c_lo = *t1l_lo;                 // transfer latched low-order period
    clear_interrupt(IRQ::TIMER1_ZERO); // clear IFR6
    timer1_irq_on_zero = true;         // enable IRQ on next zero-cross (in oneshot mode)
  } else if (offset == Timer1LatchHigh) {
    clear_interrupt(IRQ::TIMER1_ZERO); // just clear IFR6
  }

  else if (offset == Timer2PeriodLow) {
    return t2l_lo = val;
  } else if (offset == Timer2PeriodHigh) {
    *t2c_lo = t2l_lo;                  // transfer latched low-order period
    clear_interrupt(IRQ::TIMER2_ZERO); // clear IFR5
    timer2_irq_on_zero = true;         // enable IRQ on next zero-cross
  }

  return mapped_regs[offset] = val;
}

void Via::update() {
  static bool timer1_hit_zero = (timer1_period == 0);
  static bool timer1_reload = false;

  /**
   * timer 1 - once zero is reached:
   *  + 0 cycles -> IRQ, disable IRQ if ACR6 == 0
   *  + 1 cycle  -> prepare to reload latched value
   *  + 2 cycles -> load latched value into timer period
   *
   * TODO: PB7 pulse
   */
  (*timer1_period)--;
  if (*timer1_period == 0) {
    timer1_hit_zero = true;
    if (timer1_irq_on_zero) {
      flag_interrupt(IRQ::TIMER1_ZERO);

      // oneshot mode
      if (!(*acr & BIT6)) {
        timer1_irq_on_zero = false;
      }
    }
  } else if (timer1_hit_zero) {
    timer1_hit_zero = false;
    timer1_reload = true;
  } else if (timer1_reload) {
    timer1_reload = false;
    *timer1_period = *timer1_latch;
  }

  /**
   * timer 2 - always decrements, oneshot IRQ on reaching zero
   *
   * TODO: PB6 pulse
   */
  (*timer2_period)--;
  if (*timer2_period == 0 && timer2_irq_on_zero) {
    timer2_irq_on_zero = false;
    flag_interrupt(IRQ::TIMER2_ZERO);
  }
}

void Via::flag_interrupt(IRQ irq) {
  *ifr |= irq | ((irq > 0) && (*ier & ~BIT7)) << 7;

  if (*ifr & *ier & ~BIT7) {
    plugin_callback(CPU_INTERRUPT, (void *)false);
  }
}

void Via::clear_interrupt(IRQ irq) {
  *ifr &= ~irq;
  if (!(*ifr & ~BIT7))
    *ifr = 0;
}
