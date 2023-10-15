#include "imgui/imgui.h"

#include "plugin-loader.hpp"
#include "plugin_list.hpp"
// #include "emu-config.hpp"

void show_plugin_list() {
  static std::vector<PluginID> to_be_erased;

  ImGui::Begin("Plugins");

  for (auto &[id, plugin] : plugins) {
    ImGui::Separator();

    ImGui::Text(id.c_str());
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
      plugin.destroy();
      to_be_erased.push_back(id);
    }
  }

  ImGui::End();

  for (auto id : to_be_erased)
    plugins.erase(id);
  to_be_erased.clear();
}
