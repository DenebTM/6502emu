#include "emu-stdio.h"
#define SC_BKSP 0x08
#define SC_DEL  0x7F
#define SC_LF   0x0A
#define SC_CR   0x0D
#define ARR_D   258
#define ARR_U   259
#define ARR_L   260
#define ARR_R   261

#define NUMCOLS getmaxx(stdscr)
#define NOCHAR  0

OutChar::OutChar() : MemoryMappedDevice(false, 1) {
    mapped_regs[0] = &val;
}
int OutChar::pre_update() { return 0; }
int OutChar::post_update() {
    int x = getcurx(stdscr),
        y = getcury(stdscr);
    switch (val) {
        case NOCHAR:
            return 1;
        case SC_BKSP:
            x--;
            if (x < 0) { y--; x = NUMCOLS-1; }
            move(y, x);
            delch();
            refresh();
            break;
        case SC_CR:
            break;

        default:
            addch(val);
            refresh();
    }
    val = NOCHAR;
    return 0;
}

InChar::InChar() : MemoryMappedDevice(true, 1) {
    mapped_regs[0] = &val;
}
int InChar::pre_update() {
    int ch;
    ch = getch();
    switch (ch) {
        case ARR_D: case ARR_U: case ARR_L: case ARR_R: {
            int x = getcurx(stdscr),
                y = getcury(stdscr);
            switch (ch) {
                case ARR_D: y++; break;
                case ARR_U: y--; break;
                case ARR_L: x--; break;
                case ARR_R: x++; break;
            }
            if (x < 0) { y--; x = NUMCOLS-1; }
            if (x >= NUMCOLS) { y++; x = 0; }
            move(y, x);
            val = NOCHAR;
            break;
        }
        case KEY_BACKSPACE: case SC_BKSP: case SC_DEL:
            val = SC_BKSP;
            break;
        case SC_LF:
            val = SC_CR;
            break;

#ifdef EHBASIC
        case 0x04:      // catch Ctrl+D
            emu_exit(0);
            break;
#endif
        case ERR:
            val = 0;
            return ERR;
        default:
#ifdef EHBASIC
            // convert to uppercase because EHBasic does not work with lowercase characters
            if (ch >= 'a' && ch <= 'z')
                ch -= 32;
#endif
            val = (Byte)ch;
    }
    return ch;
}
int InChar::post_update() { return 0; }