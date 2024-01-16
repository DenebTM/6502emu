#include "ui/choose_file.hpp"

#include <future>
#include <nfd.h>

void ui::choose_file(std::function<void(std::string)> func, std::vector<filteritem_t> filters) {
  static bool nfd_initialized = false;
  if (!nfd_initialized) {
    NFD_Init();
    nfd_initialized = true;
  }

  static bool opening_file = false;
  static std::future<void> load_file;

  // run asynchronously so as not to block the calling (usually UI) thread
  load_file = std::async(std::launch::async, [func, filters] {
    // prevent the dialog from being opened more than once
    if (!opening_file) {
      opening_file = true;

      nfdchar_t *nfd_path;
      if (NFD_OpenDialog(&nfd_path, (nfdfilteritem_t *)&filters[0], filters.size(), nullptr) == NFD_OKAY) {
        func(std::string(nfd_path));

        NFD_FreePath(nfd_path);
      }

      opening_file = false;
    }
  });

  // TODO: call `NFD_Quit()` somewhere during clean-up
}

void ui::choose_file(std::string &out, std::vector<filteritem_t> filters) {
  choose_file([&out](std::string _out) { out = _out; }, filters);
}
