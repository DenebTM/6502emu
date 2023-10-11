#include <chrono>
#include <filesystem>
#include <fstream>
using namespace std::chrono_literals;

#include "plugin-callback.hpp"
#include "plugins/6522-via.hpp"

extern plugin_callback_t plugin_callback;

Via::Via() : MemoryMappedDevice(false, 16) {
  *ifr = 0x00;
  *ier = 0x80;
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

  return mapped_regs[offset];
}

Byte Via::write(Word offset, Byte val) {
  if (offset == PortACA2) {
    offset = PortA;

    // TODO: perform CA2 handshake
  }
  if (offset == PortA) {
    val = (*port_a & *ddra) | (val & ~(*ddra));
  } else if (offset == PortB) {
    val = (*port_b & *ddrb) | (val & ~(*ddrb));
  }

  else if (offset == InterruptFlagReg) {
    val = *ifr & ~(val & ~0x80);
  }

  else if (offset == InterruptEnableReg) {
    bool enable_irqs = val >> 7;
    if (enable_irqs) {
      val = *ier | (val & ~0x80);
    } else {
      val = *ier & ~(val & ~0x80);
    }
  }

  else if (offset == Timer1PeriodHigh || offset == Timer1LatchHigh) {
    clear_interrupt(IRQ::TIMER1_ZERO);
  } else if (offset == Timer2PeriodHigh) {
    clear_interrupt(IRQ::TIMER2_ZERO);
    timer2_active = true;
  }

  return mapped_regs[offset] = val;
}

void Via::update() {
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

  // timer 2 - always decrements, oneshot IRQ on reaching zero
  (*timer2_period)--;

  if (*timer2_period == 0 && timer2_active) {
    timer2_active = false;
    flag_interrupt(IRQ::TIMER2_ZERO);
  }

  // TODO: PB6 pulse counting mode
}

void Via::flag_interrupt(IRQ irq) {
  *ifr |= (*ier & irq) | ((irq > 0) << 7);

  if ((*ier & 0x80) && (*ifr & ~0x80)) {
    plugin_callback(CPU_INTERRUPT, (void *)false);
  }
}

void Via::clear_interrupt(IRQ irq) {
  *ifr &= ~irq;
  if (!(*ifr & ~0x80))
    *ifr = 0;
}
