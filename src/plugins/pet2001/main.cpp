#include <future>
#include <optional>
#include <thread>
using namespace std::chrono_literals;

#include "chardev.hpp"
#include "keyboard.hpp"
#include "mem-dev.hpp"
#include "mem.hpp"
#include "plugin-callback.hpp"
#include "plugins/6520-pia.hpp"

plugin_callback_t plugin_callback;

Chardev *chardev;
Pia *pia1;

extern "C" int plugin_load(plugin_callback_t callback) {
  plugin_callback = callback;

  chardev = new Chardev();
  if (!chardev->load_success) {
    delete chardev;
    return -1;
  }

  return 0;
}

extern "C" int plugin_init(AddressSpace &add_spc, Word addr) {
  addr = addr ? addr : 0x8000;

  if (chardev->sdl_init() != 0)
    return -1;

  // 40-column screen memory is mirrored four times
  add_spc.map_mem(chardev, addr + 0x000);
  add_spc.map_mem(chardev, addr + 0x400);
  add_spc.map_mem(chardev, addr + 0x800);
  add_spc.map_mem(chardev, addr + 0xc00);

  /**
   * PIA must be present in order for vblank interrupts and the keyboard to work
   * this plugin may be initialized before PIA1, therefore wait for it asynchronously and return early
   * so as not to block _its_ initialization
   */
  std::future<void> wait_for_pia1 = std::async(std::launch::async, [&] {
    std::optional<MemoryMappedDevice *> dev = std::nullopt;
    do {
      dev = add_spc.get_dev(0xe810);
      std::this_thread::sleep_for(10ms);
    } while (!dev.has_value());

    pia1 = dynamic_cast<Pia *>(dev.value());

    pia1->read_port_a = []() { return *pia1->port_a; };
    pia1->write_port_a = [](Byte val) {
      set_kb_row(val & 0x0f);
      return *pia1->port_a = (*pia1->port_a & 0xf0) | (val & ~0xf0);
    };

    pia1->read_port_b = get_kb_row_contents;
    pia1->write_port_b = [](Byte) { return 0; };
  });

  return 0;
}

extern "C" int plugin_destroy() {
  if (chardev)
    delete chardev;

  return 0;
}
