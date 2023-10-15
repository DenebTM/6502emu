#include "imgui/imgui.h"

#include "cpu.hpp"
#include "cpu_debug.hpp"
#include "emu-config.hpp"
#include "emu-types.hpp"

extern AddressSpace add_spc;
extern Emu6502 cpu;

void show_cpu_debug_window() {
  ImGui::Begin("CPU");

  if (ImGui::Button("Reset")) {
    cpu.do_reset = true;
  }
  ImGui::SameLine();
  if (ImGui::Button("IRQ")) {
    cpu.assert_interrupt(false);
  }
  ImGui::SameLine();
  if (ImGui::Button("NMI")) {
    cpu.assert_interrupt(true);
  }

  if (ImGui::BeginTable("cpu-debug", 2)) {
    ImGui::TableNextColumn();
    ImGui::Text("Clock");
    ImGui::TableNextColumn();
    ImGui::Text("%lu Hz", config->clock_speed);

    static Word new_pc;
    ImGui::TableNextColumn();
    ImGui::Text("PC");
    ImGui::TableNextColumn();
    ImGui::InputScalar("##cpu-debug/reg_pc", ImGuiDataType_U16, &new_pc, NULL, NULL, "%04X");
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      cpu.new_pc = new_pc;
    } else {
      new_pc = cpu.reg_pc;
    }

    static Byte new_sp;
    ImGui::TableNextColumn();
    ImGui::Text("SP");
    ImGui::TableNextColumn();
    ImGui::InputScalar("##cpu-debug/reg_sp", ImGuiDataType_U8, &new_sp, NULL, NULL, "%02X");
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      cpu.new_sp = new_sp;
    } else {
      new_sp = cpu.reg_sp;
    }

    static Byte new_sr;
    ImGui::TableNextColumn();
    ImGui::Text("SR");
    ImGui::TableNextColumn();
    ImGui::InputScalar("##cpu-debug/reg_sr", ImGuiDataType_U8, &new_sr, NULL, NULL, "%02X");
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      cpu.new_sr = new_sr;
    } else {
      new_sr = cpu.reg_sr;
    }

    static Byte new_a;
    ImGui::TableNextColumn();
    ImGui::Text("A");
    ImGui::TableNextColumn();
    ImGui::InputScalar("##cpu-debug/reg_a", ImGuiDataType_U8, &new_a, NULL, NULL, "%02X");
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      cpu.new_a = new_a;
    } else {
      new_a = cpu.reg_a;
    }

    static Byte new_x;
    ImGui::TableNextColumn();
    ImGui::Text("X");
    ImGui::TableNextColumn();
    ImGui::InputScalar("##cpu-debug/reg_x", ImGuiDataType_U8, &new_x, NULL, NULL, "%02X");
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      cpu.new_x = new_x;
    } else {
      new_x = cpu.reg_x;
    }

    static Byte new_y;
    ImGui::TableNextColumn();
    ImGui::Text("Y");
    ImGui::TableNextColumn();
    ImGui::InputScalar("##cpu-debug/reg_y", ImGuiDataType_U8, &new_y, NULL, NULL, "%02X");
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      cpu.new_y = new_y;
    } else {
      new_y = cpu.reg_y;
    }

    ImGui::EndTable();
  }

  ImGui::End();
}
