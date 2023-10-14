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

  return 0;
}

extern "C" EXPORT int plugin_init(AddressSpace &add_spc) {
  datasette = new Datasette;

  std::future<void> wait_for_pia1 = std::async(std::launch::async, [&] {
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
  return 0;
}

extern "C" EXPORT int plugin_update() {
  datasette->update();

  return 0;
}

extern "C" EXPORT int plugin_ui_render(/* SDL_Renderer *renderer */) {
  ImGui::Begin("PET 2001 Datasette");

  if (ImGui::Button("Load file...")) {
    nfdchar_t *outPath;
    nfdfilteritem_t filterItem[1] = {{"CBM TAP 1.0/1.1 File", "tap"}};
    if (NFD_OpenDialog(&outPath, filterItem, 1, NULL) == NFD_OKAY) {
      datasette->load_tap(std::string(outPath));
      NFD_FreePath(outPath);
    }
  }

  ImGui::Text("Status: %s", (datasette->tap_size > 0) ? (datasette->playing ? "Playing" : "Stopped") : "No file");
  ImGui::Text("tap_index: %ld / %ld", datasette->tap_size ? datasette->tap_index - TAP_HEADER_LEN : 0,
              datasette->tap_size ? datasette->tap_size - TAP_HEADER_LEN : 0);

  if (ImGui::Button("Play")) {
    datasette->play();
  }

  ImGui::SameLine();
  if (ImGui::Button("Stop")) {
    datasette->stop();
  }

  ImGui::SameLine();
  if (ImGui::Button("Rewind")) {
    datasette->rewind();
  }

  ImGui::End();

  return 0;
}
