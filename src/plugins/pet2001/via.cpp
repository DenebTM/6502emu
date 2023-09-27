#include <chrono>
#include <filesystem>
#include <fstream>
using namespace std::chrono_literals;

#include "plugin-callback.hpp"
#include "via.hpp"

extern plugin_callback_t plugin_callback;

Via::Via() : MemoryMappedDevice(false, 16) {
  *ifr = 0x00;
  *ier = 0x80;
}

Byte Via::write(Word offset, Byte val) {
  if (mapped_regs + offset == ifr) {
    val = *ifr & ~(val & ~0x80);
  }

  else if (mapped_regs + offset == ier) {
    bool enable_irqs = val >> 7;
    if (enable_irqs) {
      val = *ier | (val & ~0x80);
    } else {
      val = *ier & ~(val & ~0x80);
    }
  }

  return mapped_regs[offset] = val;
}

void Via::flag_interrupt(Byte irq) {
  *ifr |= (*ier & irq) | ((irq > 0) * 0x80);

  if ((*ier & 0x80) && (*ifr & ~0x80)) {
    plugin_callback(CPU_INTERRUPT, (void *)false);
  }
}

void Via::update(int cycles_taken) {
  // TODO: implement timers
}
