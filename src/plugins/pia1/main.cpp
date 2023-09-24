// #include <chrono>
#include <chrono>
#include <thread>
#include <tuple>
#include <vector>
using namespace std::chrono_literals;

#include "mem-dev.hpp"
#include "pia1.hpp"
#include "plugin-callback.hpp"

Pia1 *pia1;
plugin_callback_t plugin_callback;

std::thread pia1_thread;
bool pia1_running = true;
void thread_func() {
  std::this_thread::sleep_for(2s);
  while (pia1_running) {
    std::this_thread::sleep_for(1000000us / 60);
    plugin_callback(CPU_INTERRUPT, (void *)false);
  }
}

extern "C" int plugin_load() {
  pia1 = new Pia1();

  return 0;
}

extern "C" int plugin_init(std::vector<std::pair<MemoryMappedDevice *, Word>> *devs, plugin_callback_t callback) {
  plugin_callback = callback;

  devs->push_back({pia1, 0xe810});

  pia1_thread = std::thread(&thread_func);

  return 0;
}

extern "C" int plugin_destroy() {
  pia1_running = false;
  pia1_thread.join();
  return 0;
}
