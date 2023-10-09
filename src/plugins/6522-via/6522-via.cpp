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
  }

  return mapped_regs[offset] = val;
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
