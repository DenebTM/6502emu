#include "mem-dev.hpp"
#include "mem.hpp"
#include "plugin-callback.hpp"
#include "plugins/6520-pia.hpp"
#include "plugins/plugin-types.hpp"

Pia *pia;

AddressSpace *_add_spc;
Word _addr;

extern "C" EXPORT int plugin_load(plugin_callback_t callback) {
  pia = new Pia(callback);

  return 0;
}

extern "C" EXPORT int plugin_init(AddressSpace &add_spc, Word addr) {
  _add_spc = &add_spc;
  _addr = addr ? addr : 0xe810;

  _add_spc->map_mem(pia, _addr);
  _add_spc->map_mem(pia, _addr + 4);
  _add_spc->map_mem(pia, _addr + 8);
  _add_spc->map_mem(pia, _addr + 12);

  return 0;
}

extern "C" EXPORT int plugin_destroy() {
  if (pia) {
    _add_spc->unmap_mem(_addr);
    _add_spc->unmap_mem(_addr + 4);
    _add_spc->unmap_mem(_addr + 8);
    _add_spc->unmap_mem(_addr + 12);
    auto _pia = pia;
    pia = nullptr;
    delete _pia;
  }

  return 0;
}

extern "C" EXPORT int plugin_update() {
  if (pia) {
    pia->update();
  }

  return 0;
}
