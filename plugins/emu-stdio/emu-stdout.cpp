#include "emu-stdout.hpp"

OutChar::OutChar() : MemoryMappedDevice(false, 1) {
  init_ncurses();
  mapped_regs[0] = &val;
}
int OutChar::pre_update() { return 0; }
int OutChar::post_update() {
  switch (val) {
    case NOCHAR:
      return 1;
    case SC_BKSP: {
      int x = getcurx(stdscr), y = getcury(stdscr);
      x--;
      if (x < 0) {
        y--;
        x = NUMCOLS - 1;
      }
      move(y, x);
      delch();
      refresh();
      break;
    }
    case SC_CR:
      break;

    default:
      addch(val);
      refresh();
  }
  val = NOCHAR;
  return 0;
}
