#include <filesystem>
#include <future>
#include <optional>
#include <readline/readline.h>
#include <string.h>
using namespace std::chrono_literals;

#include "cassette.hpp"
#include "mem.hpp"
#include "plugins/6520-pia.hpp"
#include "plugins/plugin-types.hpp"
#include "ui/choose_file.hpp"

#include <imgui.h>

Datasette *datasette;
Pia *pia1;

extern "C" EXPORT int plugin_init(AddressSpace &add_spc) {
  datasette = new Datasette();

  static std::future<void> wait_for_pia1 = std::async(std::launch::async, [&add_spc] {
    std::optional<MemoryMappedDevice *> dev_pia1 = std::nullopt;
    do {
      dev_pia1 = add_spc.get_dev(0xe810);
      std::this_thread::sleep_for(10ms);
    } while (!dev_pia1.has_value());

    pia1 = dynamic_cast<Pia *>(dev_pia1.value());
  });

  return 0;
}

extern "C" EXPORT int plugin_destroy() {
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

  static std::string file;
  if (ImGui::Button("Load file...")) {
    ui::choose_file([](std::string new_file) {
      file = new_file;
      datasette->load_tap(file);
    });
  }

  ImGui::SameLine();
  ImGui::TextWrapped("%s", file.c_str());

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
