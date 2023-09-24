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

int Via::pre_read(Word offset) { return 0; }

int Via::post_write(Word offset) {
  if (mapped_regs + offset == ifr) {
    *ifr = 0;
  }

  return 0;
}

void Via::flag_interrupt(Byte irq) {
  *ifr |= (*ier & irq) | ((irq > 0) * 0x80);

  if ((*ier & 0x80) && (*ifr & ~0x80)) {
    plugin_callback(CPU_INTERRUPT, (void *)false);
  }
}
