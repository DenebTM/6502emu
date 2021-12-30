#include "main.h"

using namespace std;

bool _irq = true, _nmi = true,
     is_running = false,
     init_mode = true;

unsigned long long cyclesToRun = -1, cycle = 0;

AddressSpace add_spc;
Emu6502 cpu(&_irq, &_nmi);
OutChar out;
list<ROM> rom_list;

void signal_callback_handler(int signum) {
    if(signum == SIGINT) {
        std::cout << "\nCaught SIGINT, exiting.\n";
        add_spc.clear();
        exit(0);
    }
}

int main(void) {
    signal(SIGINT, signal_callback_handler);
    std::cout << "hewwo\n";

    add_spc.map_mem(&(out.val), 1, 0xF001);
    rom_list = load_roms();
    add_spc.map_roms(rom_list);

    cout << "Beginning execution! Output follows.\n";
    // Start execution
    cpu.RESET();
    while(1) {
        cpu.do_instruction();
        out.check_changed();
    }

    return 0;
}

list<ROM> load_roms() {
    list rom_list = std::list<ROM>();
    string fname = "";
    while (true) {
        cout << "Enter path of a ROM to be mapped, or press Return when done: ";
        getline(cin, fname);
        if (fname.empty()) return rom_list;

        ifstream file(fname, ios::binary | ios::ate);
        streamsize size = file.tellg();
        file.seekg(0, ios::beg);
        char *bytes = new char[size];
        file.read(bytes, size);

        ROM* rom = new ROM(size, bytes, 0xC000);
        cout << "Where should this ROM be mapped? (Enter in hex, default 0xC000): 0x";
        string inAddr;
        uint loc = 0xC000;
        bool valid = false;
        do {
            try {
                getline(cin, inAddr);
                if (!inAddr.empty())
                    loc = stoi(inAddr, NULL, 16);
                valid = true;
            }
            catch(const exception& e) {
                cout << "Invalid input\n0x";
            }
        } while (!valid);
        
        rom->start_address = loc;

        rom_list.push_back(*rom);
        delete rom;
    }
}