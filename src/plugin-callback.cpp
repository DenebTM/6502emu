#include <atomic>

#include "cpu.hpp"
#include "plugin-callback.hpp"

extern std::atomic_bool is_running;
extern int exit_status;
extern Emu6502 cpu;

/**
 * a pointer to this function should be passed to every plugin that
 * needs to be able to interrupt the CPU or cleanly shut down the emulator
 */
void plugin_callback_handler(PluginCallbackType type, void *arg) {
  switch (type) {
    case EMU_EXIT:
      exit_status = (int)(intptr_t)arg;
      is_running = false;
      break;

    case CPU_INTERRUPT: {
      bool nmi = (bool)arg;
      cpu.assert_interrupt(nmi);
      break;
    }
  }
}
