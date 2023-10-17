#include <chrono>
#include <thread>
using namespace std::chrono_literals;

#include "plugin-loader.hpp"
#include "sysclock.hpp"

uint64_t sysclock_cycle = 0;

uint64_t cycles_per_scanline;
std::chrono::system_clock::duration scanline_time;
std::chrono::system_clock::time_point next_wake;

bool sysclock_paused = false;
std::chrono::system_clock::time_point last_pause;

void sysclock_init(uint64_t clock_speed) {
  cycles_per_scanline = clock_speed / 220 / 60;
  scanline_time = 1000000000ns / (clock_speed / cycles_per_scanline);
  next_wake = std::chrono::system_clock::now() + scanline_time;
}

void sysclock_step() {
  static uint64_t cycle_current_scanline = 0;

  cycle_current_scanline++;
  sysclock_cycle++;

  update_plugins();

  if (cycle_current_scanline >= cycles_per_scanline) {
    std::this_thread::sleep_until(next_wake);

    cycle_current_scanline = 0;
    next_wake += scanline_time;
  }
}

void sysclock_pause() {
  sysclock_paused = true;
  last_pause = std::chrono::system_clock::now();
}

void sysclock_resume() {
  sysclock_paused = false;
  next_wake += (std::chrono::system_clock::now() - last_pause);
}
