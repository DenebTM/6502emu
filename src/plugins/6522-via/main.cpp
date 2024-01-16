#include <thread>
using namespace std::chrono_literals;

#include "mem-dev.hpp"
#include "mem.hpp"
#include "plugins/6522-via.hpp"
#include "plugins/plugin-types.hpp"

Via *via;

AddressSpace *_add_spc;
Word _addr;

extern "C" EXPORT int plugin_init(AddressSpace &add_spc, Word addr) {
  _add_spc = &add_spc;
  _addr = addr ? addr : 0xe840;

  via = new Via();
  _add_spc->map_mem(via, _addr);

  return 0;
}

extern "C" EXPORT int plugin_destroy() {
  if (via) {
    _add_spc->unmap_mem(_addr);
    auto _via = via;
    via = nullptr;
    delete _via;
  }

  return 0;
}

extern "C" EXPORT int plugin_update() {
  if (via) {
    via->update();
  }

  return 0;
}
