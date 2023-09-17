#include "common.h"
#include "mem.h"
#include <ncurses.h>

#define SC_EOF 4    // Ctrl+D
#define SC_BKSP 8   // ASCII backspace
#define SC_DEL 127  // ncurses backspace for some reason
#define SC_DELB 330 // delete key
#define SC_CR 13    // carriage return
#define SC_LF 10    // newline (linefeed)

#define ARR_D 258 // down/up/left/right arrows
#define ARR_U 259
#define ARR_L 260
#define ARR_R 261

#define NUMCOLS getmaxx(stdscr)
#define NOCHAR 0

struct OutChar : public MemoryMappedDevice {
  OutChar();
  Byte val;

  int pre_update();
  int post_update();
};

struct InChar : public MemoryMappedDevice {
  InChar();
  Byte val;

  int pre_update();
  int post_update();
};
