#include <atomic>
#include <filesystem>
#include <future>
#include <nfd.h>
#include <optional>

#include "cpu.hpp"
#include "plugin-callback-handler.hpp"

extern std::atomic_bool is_running;
extern int exit_status;
extern Emu6502 cpu;

void choose_file(std::function<void(std::string)> *func) {
  static bool opening_file = false;
  static std::future<void> load_file;

  // run asynchronously so as not to block the calling (usually UI) thread
  load_file = std::async(std::launch::async, [/* path */ func] {
    // prevent the dialog from being opened more than once
    if (!opening_file) {
      opening_file = true;

      nfdchar_t *nfd_path;
      nfdfilteritem_t filterItem[1] = {{"CBM TAP 1.0/1.1 File", "tap"}};
      if (NFD_OpenDialog(&nfd_path, filterItem, 1, nullptr) == NFD_OKAY) {
        (*func)(std::string(nfd_path));
        NFD_FreePath(nfd_path);
      }

      opening_file = false;
    }
  });
}

/**
 * a pointer to this function should be passed to every plugin that
 * needs to be able to interrupt the CPU or cleanly shut down the emulator
 */
void plugin_callback_handler(PluginCallbackType type, void *arg) {
  switch (type) {
    case EMU_EXIT: {
      exit_status = (int)(intptr_t)arg;
      is_running = false;
      break;
    }

    case CPU_INTERRUPT: {
      bool nmi = (bool)arg;
      cpu.assert_interrupt(nmi);
      break;
    }

    case CHOOSE_FILE: {
      choose_file((std::function<void(std::string)> *)arg);
      break;
    }
  }
}
