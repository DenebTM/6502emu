#include <chrono>
#include <thread>
using namespace std::chrono_literals;

#include "emu-config.hpp"
#include "plugin-loader.hpp"
#include "sysclock.hpp"

QWord cycle_current_period = 0;
QWord cycle_full = 0;

// TODO maybe: add a version of this for non-CPU devices and/or non-main threads
void step_cycle() {
  const static auto cycles_per_scanline = config->clock_speed / 200 / 60;
  const static auto scanline_time = 1000000000ns / (config->clock_speed / cycles_per_scanline);
  static auto next_wake = std::chrono::system_clock::now() + scanline_time;

  cycle_current_period++;
  cycle_full++;

  for (auto plugin_update_func : plugin_update_funcs)
    plugin_update_func(1);

  if (cycle_current_period >= cycles_per_scanline) {
    std::this_thread::sleep_until(next_wake);

    cycle_current_period = 0;
    next_wake += scanline_time;
  }
}
