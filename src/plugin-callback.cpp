#include <atomic>

#include "cpu.hpp"
#include "plugin-callback.hpp"

extern std::atomic_bool is_running;
extern int exit_code;
extern Emu6502 cpu;

void plugin_callback_handler(PluginCallbackType type, void *arg) {
  switch (type) {
    case EMU_EXIT:
      exit_code = (int)arg;
      is_running = false;
      break;

    case CPU_INTERRUPT: {
      bool nmi = (bool)arg;
      cpu.assert_interrupt(nmi);
      break;
    }
  }
}
