#include "emu-stdio.h"

OutChar::OutChar() : MemoryMappedDevice(false, 1) {
    mapped_regs[0] = &val;
}
int OutChar::pre_update() { return 0; }
int OutChar::post_update() {
    //Byte* valptr = getval();
    if (val == 0)
        return 1;
    addch(val);
    refresh();
    val = 0;
    return 0;
}

InChar::InChar() : MemoryMappedDevice(true, 1) {
    mapped_regs[0] = &val;
}
int InChar::pre_update() {
    int ch = getch();
#ifdef EHBASIC
    if (ch == 4) emu_exit(0); // Ctrl+D
#endif
    if (ch == ERR) { val = 0; return ERR; }
    val = (Byte)ch;
    return ch;
}
int InChar::post_update() { return 0; }