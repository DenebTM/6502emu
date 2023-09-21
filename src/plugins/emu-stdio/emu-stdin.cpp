#include "emu-stdin.hpp"
#include "emu-stdio-common.hpp"

InChar::InChar() : MemoryMappedDevice(true, 1) {
  init_ncurses();
  mapped_regs[0] = &val;
}
int InChar::pre_update() {
  int ch = getch();
  switch (ch) {
    case ARR_D:
    case ARR_U:
    case ARR_L:
    case ARR_R: {
      int x = getcurx(stdscr), y = getcury(stdscr);
      switch (ch) {
        case ARR_D:
          y++;
          break;
        case ARR_U:
          y--;
          break;
        case ARR_L:
          x--;
          break;
        case ARR_R:
          x++;
          break;
      }
      if (x < 0) {
        y--;
        x = NUMCOLS - 1;
      }
      if (x >= NUMCOLS) {
        y++;
        x = 0;
      }
      move(y, x);
      val = NOCHAR;
      break;
    }
    case KEY_BACKSPACE:
    case SC_BKSP:
    case SC_DEL:
      val = SC_BKSP;
      break;
    case SC_LF:
      addch(SC_LF);
      val = SC_CR;
      break;

      // #ifdef EHBASIC
    case SC_EOF:
      // emu_exit(0);
      break;
      // #endif
    case ERR:
      val = 0;
      return ERR;
    default:
      // #ifdef EHBASIC
      // swap upper- and lowercase because EHBasic does not recognise lowercase
      // commands
      if ((ch | 0x20) >= 'a' && (ch | 0x20) <= 'z')
        ch ^= 32;
      // #endif
      val = (Byte)ch;
  }
  return ch;
}
int InChar::post_update() { return 0; }
