#include <chrono>
#include <dlfcn.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <signal.h>
#include <thread>
#include <tuple>
#include <vector>

#include "common.hpp"
#include "cpu.hpp"
#include "mem.hpp"

std::list<ROM *> load_roms();
void signal_callback_handler(int signum);
extern "C" void emu_exit(int code);

bool is_running = false, init_mode = true;

QWord cyclesToRun = -1, cycle = 0;

AddressSpace add_spc;
Emu6502 cpu;
std::list<ROM *> rom_list;

int (*plug_init_func)(std::vector<std::pair<MemoryMappedDevice *, Word>> *);
int (*plug_destroy_func)();

int main(void) {
  using namespace std::this_thread;
  using namespace std::chrono;

  // load plugins
  // TODO: enumerate and load all plugins
  do {
    void *plugin = dlopen("plugins/emu-stdio.so", RTLD_NOW | RTLD_GLOBAL);
    if (!plugin) {
      std::cerr << dlerror() << "\n";
      break;
    }

    plug_init_func = (int (*)(std::vector<std::pair<MemoryMappedDevice *, Word>> *))dlsym(plugin, "plugin_init");
    if (!plug_init_func) {
      std::cerr << dlerror() << "\n";
      break;
    }

    plug_destroy_func = (int (*)())dlsym(plugin, "plugin_destroy");
    if (!plug_destroy_func) {
      std::cerr << dlerror() << "\n";
      break;
    }
  } while (0);

  rom_list = load_roms();
  add_spc.map_roms(rom_list);
#ifndef FUNCTEST
  std::cout << "Beginning execution in 1 second! Press Ctrl+C to quit.\n";
  sleep_for(seconds(1));

  // initialize plugins
  std::vector<std::pair<MemoryMappedDevice *, Word>> plugin_devs;
  do {
    // TODO: initialize all plugins
    plug_init_func(&plugin_devs);

    for (auto [dev, addr] : plugin_devs) {
      add_spc.map_mem(dev, addr);
    }
  } while (0);
#endif

  // Start execution loop
#ifndef FUNCTEST
  cpu.RESET();
#else
  cpu.reg_pc = 0x0400;
#endif
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
    std::cout << "Enter path of a ROM to be mapped, or press Return when done: ";
    getline(std::cin, fname);
    if (fname.empty())
      return rom_list;
#else
    fname = "roms/6502_functional_test.bin";
#endif
    std::ifstream file(fname, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    char *bytes = new char[size];
    file.read(bytes, size);

#ifdef FUNCTEST
    DWord start_addr = 0;
#else
    DWord start_addr = 0xC000;
    std::cout << "Where should this ROM be mapped? (Enter in hex, default 0xC000): 0x";
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
    ROM *rom = new ROM(size, (Byte *)bytes, start_addr);
    rom_list.push_back(rom);
#ifdef FUNCTEST
    return rom_list;
#endif
  }
}

extern "C" void emu_exit(int code) {
  if (plug_destroy_func)
    plug_destroy_func();

  std::cout << "\nExiting.\n";
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
