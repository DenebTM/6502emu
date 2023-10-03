#include "chardev.hpp"
#include "mem-dev.hpp"
#include "mem.hpp"
#include "pia1.hpp"
#include "plugin-callback.hpp"

plugin_callback_t plugin_callback;

Chardev *chardev;
Pia1 *pia1;

extern "C" int plugin_load() {
  chardev = new Chardev();
  if (!chardev->load_success) {
    delete chardev;
    return -1;
  }

  pia1 = new Pia1();

  return 0;
}

extern "C" int plugin_init(AddressSpace &add_spc, Word addr, plugin_callback_t callback) {
  addr = addr ? addr : 0x8000;
  plugin_callback = callback;

  chardev->sdl_init();
  add_spc.map_mem(chardev, addr + 0x000);
  add_spc.map_mem(chardev, addr + 0x400);
  add_spc.map_mem(chardev, addr + 0x800);
  add_spc.map_mem(chardev, addr + 0xc00);

  add_spc.map_mem(pia1, 0xe810);
  add_spc.map_mem(pia1, 0xe814);
  add_spc.map_mem(pia1, 0xe818);
  add_spc.map_mem(pia1, 0xe81c);

  return 0;
}

extern "C" int plugin_destroy() {
  if (chardev)
    delete chardev;

  if (pia1)
    delete pia1;

  return 0;
}
