#include <atomic>
#include <thread>

#include "emu-stdin.hpp"
#include "emu-stdio-common.hpp"
#include "plugin-callback.hpp"

extern plugin_callback_t plugin_callback;

std::atomic_bool stdin_thread_running = true;
std::thread *stdin_thread;

InChar::InChar() : MemoryMappedDevice(true, 1) {
  init_ncurses();
  val = mapped_regs;
  *val = 0;

  stdin_thread = new std::thread([&] {
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
          *val = NOCHAR;
          break;
        }
        case KEY_BACKSPACE:
        case SC_BKSP:
        case SC_DEL:
          *val = SC_BKSP;
          break;
        case SC_LF:
          addch(SC_LF);
          *val = SC_CR;
          break;

        case SC_EOF:
          plugin_callback(EMU_EXIT, (void *)0);
          return;
        case ERR:
          *val = 0;
          return;
        default:
          // swap upper- and lowercase because EHBasic does not recognise lowercase
          // commands
          if ((ch | 0x20) >= 'a' && (ch | 0x20) <= 'z')
            ch ^= 32;
          *val = (Byte)ch;
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
