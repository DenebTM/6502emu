#include "imgui/imgui.h"
#include "imgui_memory_editor.h"

#include "cpu.hpp"
#include "emu-config.hpp"
#include "emu-types.hpp"
#include "ui/debug_window.hpp"

extern AddressSpace add_spc;
extern Emu6502 cpu;

#include <printf.h>

void cpu_section() {
  if (ImGui::CollapsingHeader("CPU")) {
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

    ImGui::Text("Clock speed: %lu Hz", config->clock_speed);

    ImGui::PushItemWidth(60);

    static Word new_pc;
    ImGui::InputScalar("PC", ImGuiDataType_U16, &new_pc, NULL, NULL, "%04X");
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      cpu.new_pc = new_pc;
    } else {
      new_pc = cpu.reg_pc;
    }

    static Byte new_sp;
    ImGui::SameLine();
    ImGui::InputScalar("SP", ImGuiDataType_U8, &new_sp, NULL, NULL, "%02X");
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      cpu.new_sp = new_sp;
    } else {
      new_sp = cpu.reg_sp;
    }

    ImGui::Separator();

    static const auto style = ImGui::GetStyle();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + style.ItemInnerSpacing.x - 1);
    ImGui::PushItemWidth(80);

    // annotated binary input for status register
    static Byte new_sr;
    static char new_sr_bin[9] = {0};
    ImGui::Text("NV-BDIZC");
    ImGui::InputText("##sr-bin", new_sr_bin, 9, ImGuiInputTextFlags_AlwaysOverwrite);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      for (int i = 0; i < 8; i++) {
        // Bit 5 of the SR does not exist, so do not change it
        i += (i == 5);
        int bit = 1 << i;
        new_sr = (new_sr & ~bit) | (bit * (new_sr_bin[7 - i] != '0'));
      }
      cpu.new_sr = new_sr;
    } else {
      for (int i = 0; i < 8; i++) {
        new_sr_bin[7 - i] = '0' + ((cpu.reg_sr & (1 << i)) != 0);
      }
    }

    ImGui::PushItemWidth(30);

    // hex input for status register
    ImGui::SameLine();
    ImGui::InputScalar("SR##sr-hex", ImGuiDataType_U8, &new_sr, NULL, NULL, "%02X");
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      cpu.new_sr = new_sr;
    } else {
      new_sr = cpu.reg_sr;
    }

    ImGui::Separator();

    static Byte new_a;
    ImGui::InputScalar("A", ImGuiDataType_U8, &new_a, NULL, NULL, "%02X");
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      cpu.new_a = new_a;
    } else {
      new_a = cpu.reg_a;
    }

    static Byte new_x;
    ImGui::SameLine();
    ImGui::InputScalar("X", ImGuiDataType_U8, &new_x, NULL, NULL, "%02X");
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      cpu.new_x = new_x;
    } else {
      new_x = cpu.reg_x;
    }

    static Byte new_y;
    ImGui::SameLine();
    ImGui::InputScalar("Y", ImGuiDataType_U8, &new_y, NULL, NULL, "%02X");
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      cpu.new_y = new_y;
    } else {
      new_y = cpu.reg_y;
    }
  }
}

void memory_view_section() {
  if (ImGui::CollapsingHeader("Memory")) {
    static MemoryEditor mem_edit;
    mem_edit.WriteFn = [](ImU8 *_, size_t addr, ImU8 val) { add_spc.write(addr, val); };
    mem_edit.ReadFn = [](const ImU8 *_, size_t addr) { return add_spc.re_read(addr); };
    mem_edit.DrawContents(NULL, 65536);
  }
}

void show_debug_window() {
  ImGui::Begin("Debug");

  cpu_section();
  memory_view_section();

  ImGui::End();
}
