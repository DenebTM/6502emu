// #include <chrono>
#include <chrono>
#include <thread>
#include <tuple>
#include <vector>

#include "mem-dev.hpp"
#include "pia1.hpp"
#include "plugin-callback.hpp"

Pia1 *pia1;
plugin_callback_t plugin_callback;

extern "C" int plugin_load() {
  pia1 = new Pia1();

  return 0;
}

extern "C" int plugin_init(std::vector<std::pair<MemoryMappedDevice *, Word>> *devs, plugin_callback_t callback) {
  plugin_callback = callback;

  devs->push_back({pia1, 0xe810});
  pia1->start();

  return 0;
}

extern "C" int plugin_destroy() {
  if (pia1)
    delete pia1;

  return 0;
}
