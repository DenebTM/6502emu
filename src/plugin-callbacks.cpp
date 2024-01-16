#include <atomic>

#include "cpu.hpp"
#include "plugins/callbacks.hpp"

extern std::atomic_bool is_running;
extern int exit_status;
extern Emu6502 cpu;

void plugin_callbacks::emu_exit(int status) {
  exit_status = status;
  is_running = false;
}

void plugin_callbacks::assert_interrupt(bool nmi) { cpu.assert_interrupt(nmi); }
