#include "emu-stdin.hpp"
#include "emu-stdio-common.hpp"
#include "emu-stdout.hpp"
#include "mem.hpp"
#include "plugin-callback.hpp"

OutChar *emu_out;
InChar *emu_in;

plugin_callback_t plugin_callback;

extern "C" int plugin_init(AddressSpace &add_spc, Word addr, plugin_callback_t callback) {
  addr = addr ? addr : 0xf001;
  plugin_callback = callback;

  emu_out = new OutChar();
  emu_in = new InChar();

  add_spc.map_mem(emu_out, addr);
  add_spc.map_mem(emu_in, addr + 3);

  return 0;
}

extern "C" int plugin_destroy() {
  if (emu_out)
    delete emu_out;
  if (emu_in)
    delete emu_in;
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
