// #include <chrono>
#include <tuple>
#include <vector>

#include "chardev.hpp"
#include "mem-dev.hpp"
#include "pia1.hpp"
#include "plugin-callback.hpp"
#include "via.hpp"

plugin_callback_t plugin_callback;

Chardev *chardev;
Pia1 *pia1;
Via *via;

extern "C" int plugin_load() {
  chardev = new Chardev();
  if (!chardev->load_success) {
    delete chardev;
    return -1;
  }

  pia1 = new Pia1();

  via = new Via();

  return 0;
}

extern "C" int plugin_init(std::vector<std::pair<MemoryMappedDevice *, Word>> *devs, plugin_callback_t callback) {
  plugin_callback = callback;

  chardev->init_sdl();
  devs->push_back({chardev, 0x8000});
  devs->push_back({chardev, 0x8400});
  devs->push_back({chardev, 0x8800});
  devs->push_back({chardev, 0x8c00});

  pia1->start();
  devs->push_back({pia1, 0xe810});

  devs->push_back({via, 0xe840});

  return 0;
}

extern "C" int plugin_destroy() {
  if (chardev)
    delete chardev;

  if (pia1)
    delete pia1;

  return 0;
}

// extern "C" int plugin_update() {
//   static auto min_render_interval = 8.333ms;
//   static auto last_render = std::chrono::system_clock::now();

//   if (chardev) {
//     chardev->handle_events();

//     auto now = std::chrono::system_clock::now();
//     if ((now - last_render) >= min_render_interval) {
//       chardev->render();
//       last_render = now;
//     }
//   }

//   return 0;
// }
