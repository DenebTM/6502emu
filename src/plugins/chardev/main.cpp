#include <tuple>
#include <vector>

#include "chardev.hpp"
#include "mem-dev.hpp"
#include "plugin-callback.hpp"

Chardev *chardev;
plugin_callback_t plugin_callback;

extern "C" int plugin_load() {
  chardev = new Chardev();
  if (!chardev->load_success) {
    delete chardev;
    return -1;
  }

  return 0;
}

extern "C" int plugin_init(std::vector<std::pair<MemoryMappedDevice *, Word>> *devs, plugin_callback_t callback) {
  plugin_callback = callback;

  chardev->init_sdl();

  devs->push_back({chardev, 0x8000});
  devs->push_back({chardev, 0x8400});
  devs->push_back({chardev, 0x8800});
  devs->push_back({chardev, 0x8c00});

  return 0;
}

extern "C" int plugin_destroy() {
  if (chardev)
    delete chardev;
  return 0;
}

extern "C" int plugin_update() {
  if (chardev) {
    chardev->handle_events();
    chardev->render();
  }

  return 0;
}
