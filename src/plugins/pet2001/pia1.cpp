#include <chrono>
#include <filesystem>
#include <fstream>
using namespace std::chrono_literals;

#include "pia1.hpp"
#include "plugin-callback.hpp"
#include "via.hpp"

extern plugin_callback_t plugin_callback;

extern Via *via;

Pia1::Pia1() : MemoryMappedDevice(true, 16) {
  mapped_regs[2] = 0xff;
  mapped_regs[3] = 0x80;
}

Pia1::~Pia1() {
  pia1_running = false;
  if (irq_thread.joinable())
    irq_thread.join();
}

int Pia1::pre_read(Word offset) { return 0; }

int Pia1::post_write(Word offset) { return 0; }

void Pia1::start() {
  pia1_running = true;
  irq_thread = std::thread(&Pia1::irq_thread_func, this);
}

void Pia1::irq_thread_func() {
  std::this_thread::sleep_for(2s);
  while (pia1_running) {
    std::this_thread::sleep_for(1000000us / 60);
    via->flag_interrupt(0x10);
  }
}
