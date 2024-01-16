#include <atomic>
#include <future>
#include <nfd.h>
#include <string>

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

void plugin_callbacks::choose_file(std::function<void(std::string)> *func) {
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
