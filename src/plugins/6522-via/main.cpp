#include "mem-dev.hpp"
#include "mem.hpp"
#include "plugin-callback.hpp"
#include "plugins/6522-via.hpp"
#include "plugins/plugin-types.hpp"

Via *via;

extern "C" EXPORT int plugin_load(plugin_callback_t callback) {
  via = new Via(callback);

  return 0;
}

extern "C" EXPORT int plugin_init(AddressSpace &add_spc, Word addr) {
  addr = addr ? addr : 0xe840;

  add_spc.map_mem(via, addr);

  return 0;
}

extern "C" EXPORT int plugin_destroy() {
  if (via)
    delete via;

  return 0;
}

extern "C" EXPORT int plugin_update() {
  if (via) {
    via->update();
  }

  return 0;
}
