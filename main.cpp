#include "main.h"

bool _irq = true, _nmi = true,
     is_running = false,
     init_mode = true;

QWord cyclesToRun = -1, cycle = 0;

AddressSpace add_spc;
Emu6502 cpu(&_irq, &_nmi);
OutChar emu_out;
InChar  emu_in;
std::list<ROM> rom_list;

void signal_callback_handler(int signum) {
    if(signum == SIGINT) {
        endwin();
        std::cout << "\nCaught SIGINT, exiting.\n";
        add_spc.free();
        for (ROM r : rom_list)
            delete[] r.content;
        rom_list.clear();
        exit(0);
    }
}

int main(void) {
    signal(SIGINT, signal_callback_handler);
    std::cout << "hewwo\n";

    add_spc.map_mem(&emu_out, 0xF001);
    add_spc.map_mem(&emu_in, 0xF004);
    /*add_spc.map_mem(&emu_out.val, 1, 0xF001);
    add_spc.map_mem(&emu_in.val, 1, 0xF004);*/
    rom_list = load_roms();
    add_spc.map_roms(rom_list);

    std::cout << "Beginning execution! Output follows.\n";

    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);

    // Start execution
    cpu.RESET();
    while(1) {
        cpu.do_instruction();
        //emu_out.update();
        //emu_in.update();
    }

    return 0;
}

std::list<ROM> load_roms() {
    std::list rom_list = std::list<ROM>();
    std::string fname = "";
    while (true) {
        std::cout << "Enter path of a ROM to be mapped, or press Return when done: ";
        getline(std::cin, fname);
        if (fname.empty()) return rom_list;

        std::ifstream file(fname, std::ios::binary | std::ios::ate);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        Byte *bytes = new Byte[size];
        file.read(bytes, size);

        std::cout << "Where should this ROM be mapped? (Enter in hex, default 0xC000): 0x";
        std::string inAddr;
        DWord start_addr = 0xC000;
        bool valid = false;
        do {
            try {
                getline(std::cin, inAddr);
                if (!inAddr.empty())
                    start_addr = stoi(inAddr, NULL, 16);
                valid = true;
            }
            catch(const std::exception& e) {
                std::cout << "Invalid input\n0x";
            }
        } while (!valid);
        ROM* rom = new ROM(size, bytes, start_addr);

        rom_list.push_back(*rom);
        delete rom;
    }
}