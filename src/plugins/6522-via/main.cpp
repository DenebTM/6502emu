#include "mem-dev.hpp"
#include "mem.hpp"
#include "plugin-callback.hpp"
#include "plugins/6522-via.hpp"

plugin_callback_t plugin_callback;

Via *via;

extern "C" int plugin_load(plugin_callback_t callback) {
  plugin_callback = callback;

  via = new Via();

  return 0;
}

extern "C" int plugin_init(AddressSpace &add_spc, Word addr) {
  addr = addr ? addr : 0xe840;

  add_spc.map_mem(via, addr);

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
