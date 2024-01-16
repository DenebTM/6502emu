#include "ui/choose_file.hpp"

#include <future>
#include <nfd.h>

void ui::choose_file(std::function<void(std::string)> func) {
  static bool nfd_initialized = false;
  if (!nfd_initialized) {
    NFD_Init();
    nfd_initialized = true;
  }

  static bool opening_file = false;
  static std::future<void> load_file;

  // run asynchronously so as not to block the calling (usually UI) thread
  load_file = std::async(std::launch::async, [func] {
    // prevent the dialog from being opened more than once
    if (!opening_file) {
      opening_file = true;

      nfdchar_t *nfd_path;
      nfdfilteritem_t filterItem[1] = {{"CBM TAP 1.0/1.1 File", "tap"}};
      if (NFD_OpenDialog(&nfd_path, filterItem, 1, nullptr) == NFD_OKAY) {
        func(std::string(nfd_path));

        NFD_FreePath(nfd_path);
      }

      opening_file = false;
    }
  });

  // TODO: call `NFD_Quit()` somewhere during clean-up
}

void ui::choose_file(std::string &out) {
  choose_file([&out](std::string _out) { out = _out; });
}
