#include <ncurses.h>
#include "common.h"
#include "mem.h"

struct OutChar: public MemoryMappedDevice {
    OutChar();
    Byte val;

    int pre_update();
    int post_update();
};

struct InChar: public MemoryMappedDevice {
    InChar();
    Byte val;

    int pre_update();
    int post_update();
};