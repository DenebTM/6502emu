#include "imgui/imgui.h"
#include "imgui_memory_editor.h"
#include <chrono>
using namespace std::chrono_literals;

#include "cpu.hpp"
#include "emu-config.hpp"
#include "emu-types.hpp"
#include "sysclock.hpp"
#include "ui/debug_window.hpp"

extern AddressSpace add_spc;
extern Emu6502 cpu;

void cpu_section() {
  if (ImGui::BeginTable("##cpu-control", 2)) {
    if (ImGui::TableNextColumn()) {
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

      bool cpu_paused = (cpu.step_instructions == 0);

      static int step_cycles = 1;
      ImGui::SetNextItemWidth(60);
      ImGui::InputScalar("##step-cycles", ImGuiDataType_S32, &step_cycles);

      ImGui::SameLine();
      if (!cpu_paused)
        ImGui::BeginDisabled();
      if (ImGui::Button("Step")) {
        cpu.step_instructions = step_cycles;
      }
      if (!cpu_paused)
        ImGui::EndDisabled();

      ImGui::SameLine();
      if (!cpu_paused) {
        if (ImGui::Button("Pause"))
          cpu.step_instructions = 0;
      } else if (ImGui::Button("Run")) {
        cpu.step_instructions = -1;
      }
    }

    if (ImGui::TableNextColumn()) {
      uint64_t clock_tgt = config->clock_speed;

      static uint64_t clock_eff_mean = config->clock_speed;

      // calculate effective clock speed once every 100ms, average over 1s
      const static auto CLOCK_EFF_UPD_INTERVAL = 100ms;
      const static int CLOCK_EFF_SAMPLE_COUNT = 10;
      static uint64_t clock_eff_samples[CLOCK_EFF_SAMPLE_COUNT];

      static auto clock_eff_upd_next = std::chrono::system_clock::now();
      static uint64_t cycles_passed;
      static uint64_t cycles_prev = sysclock_cycle;
      if (std::chrono::system_clock::now() >= clock_eff_upd_next) {
        static int sample_idx = 0;
        cycles_passed = sysclock_cycle - cycles_prev;
        cycles_prev = sysclock_cycle;
        clock_eff_samples[sample_idx] = cycles_passed;
        clock_eff_mean = 0;
        for (int i = 0; i < CLOCK_EFF_SAMPLE_COUNT; i++)
          clock_eff_mean += clock_eff_samples[i];
        clock_eff_mean /= CLOCK_EFF_SAMPLE_COUNT;
        clock_eff_mean *= CLOCK_EFF_SAMPLE_COUNT;
        clock_eff_upd_next += CLOCK_EFF_UPD_INTERVAL;
        sample_idx = (sample_idx + 1) % CLOCK_EFF_SAMPLE_COUNT;
      }

      ImGui::Text("Target clock:    %lu Hz", clock_tgt);
      ImGui::Text("Effective clock: %lu Hz", clock_eff_mean);
    }

    ImGui::EndTable();
  }

  ImGui::PushItemWidth(60);

  static Word new_pc;
  ImGui::InputScalar("PC", ImGuiDataType_U16, &new_pc, nullptr, nullptr, "%04X");
  if (ImGui::IsItemDeactivatedAfterEdit()) {
    cpu.new_pc = new_pc;
  } else {
    new_pc = cpu.reg_pc;
  }

  static Byte new_sp;
  ImGui::SameLine();
  ImGui::InputScalar("SP", ImGuiDataType_U8, &new_sp, nullptr, nullptr, "%02X");
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
      // bit 5 of the SR does not exist, so do not change it
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
  ImGui::InputScalar("SR##sr-hex", ImGuiDataType_U8, &new_sr, nullptr, nullptr, "%02X");
  if (ImGui::IsItemDeactivatedAfterEdit()) {
    cpu.new_sr = new_sr;
  } else {
    new_sr = cpu.reg_sr;
  }

  ImGui::Separator();

  static Byte new_a;
  ImGui::InputScalar("A", ImGuiDataType_U8, &new_a, nullptr, nullptr, "%02X");
  if (ImGui::IsItemDeactivatedAfterEdit()) {
    cpu.new_a = new_a;
  } else {
    new_a = cpu.reg_a;
  }

  static Byte new_x;
  ImGui::SameLine();
  ImGui::InputScalar("X", ImGuiDataType_U8, &new_x, nullptr, nullptr, "%02X");
  if (ImGui::IsItemDeactivatedAfterEdit()) {
    cpu.new_x = new_x;
  } else {
    new_x = cpu.reg_x;
  }

  static Byte new_y;
  ImGui::SameLine();
  ImGui::InputScalar("Y", ImGuiDataType_U8, &new_y, nullptr, nullptr, "%02X");
  if (ImGui::IsItemDeactivatedAfterEdit()) {
    cpu.new_y = new_y;
  } else {
    new_y = cpu.reg_y;
  }
}

void memory_view_section() {
  static MemoryEditor mem_edit;
  mem_edit.WriteFn = [](ImU8 *_, size_t addr, ImU8 val) { add_spc.write(addr, val); };
  mem_edit.ReadFn = [](const ImU8 *_, size_t addr) { return add_spc.re_read(addr); };
  mem_edit.DrawContents(nullptr, 65536);
}

void show_debug_window() {
  ImGui::Begin("Debug");

  if (ImGui::CollapsingHeader("CPU", ImGuiTreeNodeFlags_DefaultOpen)) {
    cpu_section();
  }
  if (ImGui::CollapsingHeader("Memory", ImGuiTreeNodeFlags_DefaultOpen)) {
    memory_view_section();
  }

  ImGui::End();
}
