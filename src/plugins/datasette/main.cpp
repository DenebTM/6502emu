#include <filesystem>
#include <future>
#include <optional>
#include <readline/readline.h>
#include <string.h>
using namespace std::chrono_literals;

#include "cassette.hpp"
#include "mem.hpp"
#include "plugin-callback.hpp"
#include "plugins/6520-pia.hpp"
#include "plugins/plugin-types.hpp"

#include "imgui.h"
#include "nfdx/src/include/nfd.h"

plugin_callback_t plugin_callback;

Datasette *datasette;
Pia *pia1;

extern "C" EXPORT int plugin_load(plugin_callback_t callback) {
  plugin_callback = callback;

  datasette = new Datasette();

  return 0;
}

extern "C" EXPORT int plugin_init(AddressSpace &add_spc) {
  static std::future<void> wait_for_pia1 = std::async(std::launch::async, [&] {
    std::optional<MemoryMappedDevice *> dev_pia1 = std::nullopt;
    do {
      dev_pia1 = add_spc.get_dev(0xe810);
      std::this_thread::sleep_for(10ms);
    } while (!dev_pia1.has_value());

    pia1 = dynamic_cast<Pia *>(dev_pia1.value());
  });

  NFD_Init();

  return 0;
}

extern "C" EXPORT int plugin_destroy() {
  NFD_Quit();

  if (datasette) {
    auto _datasette = datasette;
    datasette = nullptr;
    delete _datasette;
  }

  return 0;
}

extern "C" EXPORT int plugin_update() {
  if (datasette) {
    datasette->update();
  }

  return 0;
}

extern "C" EXPORT int plugin_ui_render(/* SDL_Renderer *renderer */) {
  if (!datasette)
    return -1;

  ImGui::Begin("PET 2001 Datasette");

  static std::filesystem::path file;
  if (ImGui::Button("Load file...")) {
    static bool opening_file = false;
    static std::future<void> load_file;

    // prevent the dialog from being opened twice
    if (!opening_file) {
      opening_file = true;
      // run asynchronously so as not to block the UI thread
      load_file = std::async(std::launch::async, [&] {
        nfdchar_t *outPath;
        nfdfilteritem_t filterItem[1] = {{"CBM TAP 1.0/1.1 File", "tap"}};
        if (NFD_OpenDialog(&outPath, filterItem, 1, nullptr) == NFD_OKAY) {
          file = std::filesystem::path(outPath);
          datasette->load_tap(std::string(outPath));
          NFD_FreePath(outPath);
        }

        opening_file = false;
      });
    }
  }

  ImGui::SameLine();
  ImGui::TextWrapped("%s", file.filename().c_str());

  ImGui::Text("Status: %s", (datasette->tap_size > 0) ? (datasette->playing ? "Playing" : "Stopped") : "No file");
  ImGui::Text("tap_index: %ld / %ld", datasette->tap_size ? datasette->tap_index - TAP_HEADER_LEN : 0,
              datasette->tap_size ? datasette->tap_size - TAP_HEADER_LEN : 0);

  bool play_disabled = datasette->playing || datasette->tap_size == 0;
  if (play_disabled)
    ImGui::BeginDisabled();
  if (ImGui::Button("Play")) {
    datasette->play();
  }
  if (play_disabled)
    ImGui::EndDisabled();

  ImGui::SameLine();
  bool stop_disabled = !datasette->playing;
  if (stop_disabled)
    ImGui::BeginDisabled();
  if (ImGui::Button("Stop")) {
    datasette->stop();
  }
  if (stop_disabled)
    ImGui::EndDisabled();

  ImGui::SameLine();
  bool rewind_disabled = datasette->tap_index == TAP_HEADER_LEN;
  if (rewind_disabled)
    ImGui::BeginDisabled();
  if (ImGui::Button("Rewind")) {
    datasette->rewind();
  }
  if (rewind_disabled)
    ImGui::EndDisabled();

  ImGui::End();

  return 0;
}
