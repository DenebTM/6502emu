#include "emu-stdout.hpp"
#include "emu-stdio-common.hpp"

OutChar::OutChar() : MemoryMappedDevice(false, 1) {
  init_ncurses();
  last_char = mapped_regs;
  *last_char = NOCHAR;
}

Byte OutChar::write(Word offset, Byte val) {
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
  return val;
}
