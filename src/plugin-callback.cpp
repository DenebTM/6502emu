#include "plugin-callback.hpp"
#include "cpu.hpp"

extern void emu_exit(int exit_code);

extern Emu6502 cpu;

void plugin_callback_handler(PluginCallbackType type, void *arg) {
  switch (type) {
    case EMU_EXIT: {
      int exit_code = (int)arg;
      emu_exit(exit_code);
      break;
    }

    case CPU_INTERRUPT: {
      bool nmi = (bool)arg;
      cpu.assert_interrupt(nmi);
      break;
    }
  }
}
