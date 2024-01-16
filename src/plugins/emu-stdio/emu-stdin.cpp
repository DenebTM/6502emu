#include <atomic>
#include <thread>

#include "emu-stdin.hpp"
#include "emu-stdio-common.hpp"
#include "plugin-callback.hpp"

extern plugin_callback_t plugin_callback;

InChar::InChar() : MemoryMappedDevice(true, 1) {
  init_ncurses();
  last_char = mapped_regs;
  *last_char = 0;

  stdin_thread = new std::thread([this] {
    while (stdin_thread_running) {
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
          *last_char = NOCHAR;
          break;
        }
        case KEY_BACKSPACE:
        case SC_BKSP:
        case SC_DEL:
          *last_char = SC_BKSP;
          break;
        case SC_LF:
          addch(SC_LF);
          *last_char = SC_CR;
          break;

        case SC_EOF:
          plugin_callback(EMU_EXIT, (void *)0);
          return;
        case ERR:
          *last_char = 0;
          return;
        default:
          // swap upper- and lowercase because EHBasic does not recognise lowercase
          // commands
          if ((ch | 0x20) >= 'a' && (ch | 0x20) <= 'z')
            ch ^= 32;
          *last_char = (Byte)ch;
      }
    }
  });
}

InChar::~InChar() {
  stdin_thread_running = false;

  if (stdin_thread->joinable())
    stdin_thread->join();

  delete stdin_thread;
}

Byte InChar::read(Word offset) {
  auto val = *last_char;
  *last_char = 0;
  return val;
}
