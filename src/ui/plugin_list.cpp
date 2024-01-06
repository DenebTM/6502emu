#include "imgui/imgui.h"
#include <future>

#include "plugin-loader.hpp"
#include "ui/plugin_list.hpp"
// #include "emu-config.hpp"

#include "nfd.h"

extern AddressSpace add_spc;

static bool nfd_initialized = false;
void load_plugin_modal(bool *modal_shown) {
  static bool load_allowed = false;

  if (!nfd_initialized) {
    NFD_Init();
    nfd_initialized = true;
  }

  ImGui::Begin("Load new plugin", modal_shown, ImGuiWindowFlags_Modal | ImGuiWindowFlags_NoResize);
  ImGui::SetWindowSize(ImVec2(250, 0));

  static std::string filename;
  if (ImGui::Button("Choose file...")) {
    static bool opening_file = false;
    static std::future<void> load_file;

    // prevent the dialog from being opened twice
    if (!opening_file) {
      opening_file = true;
      // run asynchronously so as not to block the UI thread
      load_file = std::async(std::launch::async, [&] {
        nfdchar_t *outPath;
        nfdfilteritem_t filterItem[1] = {{"Shared library", "so"}};
        if (NFD_OpenDialog(&outPath, filterItem, 1, nullptr) == NFD_OKAY) {
          filename = std::string(outPath);
          NFD_FreePath(outPath);
        }

        opening_file = false;
      });
    }
  }
  ImGui::SameLine();
  ImGui::SetNextItemWidth(150);
  ImGui::TextWrapped("%s", std::filesystem::path(filename).filename().c_str());

  static char id[16] = {0};
  ImGui::PushItemWidth(150);
  ImGui::InputText("ID", id, 16);

  static Word addr = 0;
  ImGui::PushItemWidth(50);
  ImGui::InputScalar("Load address", ImGuiDataType_U16, &addr, nullptr, nullptr, "%04X");

  ImGui::Separator();

  load_allowed = (filename.length() > 0) && (strnlen(id, 16) > 0) && !plugins.contains(id);

  if (!load_allowed)
    ImGui::BeginDisabled();
  if (ImGui::Button("Load and initialize")) {
    config->plugin_configs.push_back({
        .id = id,
        .filename = filename,
        .disable = false,
        .map_addr = addr,
    });
    load_plugin(PluginID(id), filename);
    plugins[id].init(add_spc, addr, config);
    *modal_shown = false;
  }
  if (!load_allowed)
    ImGui::EndDisabled();

  ImGui::End();
}

void show_plugin_list() {
  static std::vector<PluginID> to_be_erased;

  // modal dialog to load new plugin
  static bool load_plugin_modal_shown = false;
  if (load_plugin_modal_shown) {
    load_plugin_modal(&load_plugin_modal_shown);
  }

  // plugin list
  ImGui::Begin("Plugins", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

  if (ImGui::Button("Load plugin")) {
    load_plugin_modal_shown = true;
  }

  for (auto &[id, plugin] : plugins) {
    ImGui::Separator();

    ImGui::Text("%s", id.c_str());
    if (plugin.map_addr) {
      if (ImGui::BeginTable(("plugin-list/" + id).c_str(), 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableNextColumn();
        ImGui::Text("Map address");
        ImGui::TableNextColumn();
        ImGui::Text("%4X", plugin.map_addr);
      }

      ImGui::EndTable();
    }

    if (ImGui::Button(("Remove##" + id).c_str())) {
      to_be_erased.push_back(id);
    }
  }

  ImGui::End();

  for (auto id : to_be_erased)
    unload_plugin(id);
  to_be_erased.clear();
}
