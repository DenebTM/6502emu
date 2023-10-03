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
  Byte *reg = mapped_regs + offset;

  if (reg == port_a_ca2) {
    reg = port_a;

    // TODO: perform CA2 handshake
  }

  else if (reg == t1c_lo || reg == t1l_lo) {
    clear_interrupt(IRQ::TIMER1_ZERO);
  }

  return mapped_regs[offset];
}

Byte Via::write(Word offset, Byte val) {
  Byte *reg = mapped_regs + offset;

  if (reg == port_a_ca2) {
    reg = port_a;

    // TODO: perform CA2 handshake
  }
  if (reg == port_a) {
    val = (*port_a & *ddra) | (val & ~(*ddra));
  } else if (reg == port_b) {
    val = (*port_b & *ddrb) | (val & ~(*ddrb));
  }

  else if (reg == ifr) {
    val = *ifr & ~(val & ~0x80);
  }

  else if (reg == ier) {
    bool enable_irqs = val >> 7;
    if (enable_irqs) {
      val = *ier | (val & ~0x80);
    } else {
      val = *ier & ~(val & ~0x80);
    }
  }

  else if (reg == t1c_hi || reg == t1l_hi) {
    clear_interrupt(IRQ::TIMER1_ZERO);
  }

  return *reg = val;
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
