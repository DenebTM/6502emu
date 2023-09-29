#include <chrono>
#include <thread>
using namespace std::chrono_literals;

#include "emu-config.hpp"
#include "sysclock.hpp"

// FIXME: pull these out into their own translation unit (see also main.cpp)
typedef int (*plugin_update_t)(int cycles_elapsed);
extern std::vector<plugin_update_t> plugin_update_funcs;

QWord cycle_current_period = 0;
QWord cycle_full = 0;

// TODO maybe: add a version of this for non-CPU devices and/or non-main threads
void step_cycle() {
  const static auto cycles_per_scanline = config->clock_speed / 200 / 60;
  const static auto sleep_time = 1000000000ns / (config->clock_speed / cycles_per_scanline);

  cycle_current_period++;
  cycle_full++;

  for (auto plugin_update_func : plugin_update_funcs)
    plugin_update_func(1);

  if (cycle_current_period >= cycles_per_scanline) {
    cycle_current_period = 0;
    std::this_thread::sleep_for(sleep_time);
  }
}
