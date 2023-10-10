#include "mem-dev.hpp"
#include "mem.hpp"
#include "plugin-callback.hpp"
#include "plugins/6520-pia.hpp"
#include "plugins/plugin-types.hpp"

Pia *pia;

extern "C" EXPORT int plugin_load(plugin_callback_t callback) {
  pia = new Pia(callback);

  return 0;
}

extern "C" EXPORT int plugin_init(AddressSpace &add_spc, Word addr) {
  addr = addr ? addr : 0xe810;

  add_spc.map_mem(pia, addr);
  add_spc.map_mem(pia, addr + 4);
  add_spc.map_mem(pia, addr + 8);
  add_spc.map_mem(pia, addr + 12);

  return 0;
}

extern "C" EXPORT int plugin_destroy() {
  if (pia)
    delete pia;

  return 0;
}
