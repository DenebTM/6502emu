#include "main.h"
#include "mem.h"
#include "cpu.h"

bool _irq = true, _nmi = true,
     is_running = false,
     init_mode = true;

unsigned long long cyclesToRun = -1, cycle = 0;

AddressSpace add_spc(0x10000);
Emu6502 cpu(&_irq, &_nmi);

int main(void) {
    std::cout << "hewwo\n";

    return 0;
}

void LoadRoms() {
    while (true) {
        std::cout << "Enter path of a ROM to be mapped into memory\n";
    }
}