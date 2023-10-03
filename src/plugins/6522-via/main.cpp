#include "mem-dev.hpp"
#include "mem.hpp"
#include "plugin-callback.hpp"
#include "plugins/6522-via.hpp"

plugin_callback_t plugin_callback;

Via *via;

extern "C" int plugin_load() {
  via = new Via();

  return 0;
}

extern "C" int plugin_init(AddressSpace &add_spc, plugin_callback_t callback) {
  plugin_callback = callback;

  add_spc.map_mem(via, 0xe840);

  return 0;
}

extern "C" int plugin_destroy() {
  if (via)
    delete via;

  return 0;
}

extern "C" int plugin_update(int cycles_elapsed) {
  if (via) {
    for (int i = 0; i < cycles_elapsed; i++)
      via->update();
  }

  return 0;
}
