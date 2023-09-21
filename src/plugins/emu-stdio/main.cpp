#include <tuple>
#include <vector>

#include "emu-stdin.hpp"
#include "emu-stdio-common.hpp"
#include "emu-stdout.hpp"

OutChar *emu_out;
InChar *emu_in;

extern "C" int plugin_init(std::vector<std::pair<MemoryMappedDevice *, Word>> *devs) {
  emu_out = new OutChar();
  emu_in = new InChar();

  devs->push_back({emu_out, 0xF001});
  devs->push_back({emu_in, 0xF004});

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

  // raw();
  // addstr("# Intercepting Ctrl+C; press Ctrl+D to exit instead.\n");
  // refresh();

  ncurses_initialized = true;
}
