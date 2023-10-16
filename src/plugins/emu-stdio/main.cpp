#include "emu-stdin.hpp"
#include "emu-stdio-common.hpp"
#include "emu-stdout.hpp"
#include "mem.hpp"
#include "plugin-callback.hpp"
#include "plugins/plugin-types.hpp"

OutChar *emu_out;
InChar *emu_in;

plugin_callback_t plugin_callback;

AddressSpace *_add_spc;
Word _addr;

extern "C" EXPORT int plugin_load(plugin_callback_t callback) {
  plugin_callback = callback;
  return 0;
}

extern "C" EXPORT int plugin_init(AddressSpace &add_spc, Word addr) {
  _add_spc = &add_spc;
  _addr = addr ? addr : 0xe840;

  emu_out = new OutChar();
  emu_in = new InChar();

  _add_spc->map_mem(emu_out, _addr);
  _add_spc->map_mem(emu_in, _addr + 3);

  return 0;
}

extern "C" EXPORT int plugin_destroy() {
  if (emu_out) {
    _add_spc->unmap_mem(_addr);
    auto _emu_out = emu_out;
    emu_out = NULL;
    delete _emu_out;
  }
  if (emu_in) {
    _add_spc->unmap_mem(_addr + 3);
    auto _emu_in = emu_in;
    emu_in = NULL;
    delete _emu_in;
  }
  if (ncurses_initialized)
    endwin();

  return 0;
}

bool ncurses_initialized = false;
void init_ncurses() {
  if (ncurses_initialized)
    return;

  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  scrollok(stdscr, TRUE);

  raw();
  addstr("# Intercepting Ctrl+C; press Ctrl+D to exit instead.\n");
  refresh();

  ncurses_initialized = true;
}
