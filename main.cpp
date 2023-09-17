#include "main.h"

bool _irq = true, _nmi = true, is_running = false, init_mode = true;

QWord cyclesToRun = -1, cycle = 0;

AddressSpace add_spc;
Emu6502 cpu(&_irq, &_nmi);
OutChar emu_out;
InChar emu_in;
std::list<ROM *> rom_list;

void emu_exit(int code) {
  endwin();
  std::cout << "\nExiting.\n";
  delete[] emu_out.mapped_regs;
  delete[] emu_in.mapped_regs;
  add_spc.free();
  for (ROM *r : rom_list) {
    delete[] r->content;
    delete r;
  }
  rom_list.clear();
  exit(code);
}

void signal_callback_handler(int signum) {
  if (signum == SIGINT)
    emu_exit(0);
}

int main(void) {
  using namespace std::this_thread;
  using namespace std::chrono;

  rom_list = load_roms();
  add_spc.map_roms(rom_list);
#ifndef FUNCTEST
  add_spc.map_mem(&emu_out, 0xF001);
  add_spc.map_mem(&emu_in, 0xF004);

#ifdef EHBASIC
  std::cout << "Beginning execution in 1 second! Press Ctrl+D to quit at any "
               "point.\n";
#else
  std::cout << "Beginning execution in 1 second! Press Ctrl+C to quit at any "
               "point.\n";
#endif
  sleep_for(seconds(1));

  // Initialize ncurses
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  scrollok(stdscr, TRUE);
  setlocale(LC_ALL, "de_DE.UTF-8");
#ifdef EHBASIC
  raw();
#else
  signal(SIGINT, signal_callback_handler);
#endif
#endif

  // Start execution loop
  cpu.RESET();
  while (1) {
    cpu.do_instruction();
#ifndef FUNCTEST
    if (cycle > 3000) {
      cycle = 0;
      sleep_for(nanoseconds(833333));
    }
#endif
  }

  return 0;
}

std::list<ROM *> load_roms() {
  std::list rom_list = std::list<ROM *>();
  std::string fname = "";
  while (1) {
#ifndef FUNCTEST
    std::cout
        << "Enter path of a ROM to be mapped, or press Return when done: ";
    getline(std::cin, fname);
    if (fname.empty())
      return rom_list;
#else
    fname = "roms/6502_functional_test.bin";
#endif
    std::ifstream file(fname, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    Byte *bytes = new Byte[size];
    file.read(bytes, size);

#ifdef FUNCTEST
    DWord start_addr = 0;
#else
    DWord start_addr = 0xC000;
    std::cout << "Where should this ROM be mapped? (Enter in hex, default "
                 "0xC000): 0x";
    std::string inAddr = "";
    bool valid = false;
    do {
      try {
        getline(std::cin, inAddr);
        if (!inAddr.empty())
          start_addr = stoi(inAddr, NULL, 16);
        valid = true;
      } catch (const std::exception &e) {
        std::cout << "Invalid input\n0x";
      }
    } while (!valid);
#endif
    ROM *rom = new ROM(size, bytes, start_addr);
    rom_list.push_back(rom);
#ifdef FUNCTEST
    return rom_list;
#endif
  }
}
