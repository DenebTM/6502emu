#include <chrono>
#include <dlfcn.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <readline/readline.h>
#include <signal.h>
#include <thread>
#include <tuple>
#include <vector>

#include "cpu.hpp"
#include "emu-common.hpp"
#include "mem.hpp"

std::list<ROM *> load_roms();
void load_plugins();
void init_plugins();
void signal_callback_handler(int signum);
extern "C" void emu_exit(int code);

bool is_running = false, init_mode = true;

QWord cyclesToRun = -1, cycle = 0;

AddressSpace add_spc;
Emu6502 cpu;
std::list<ROM *> rom_list;

typedef int (*plug_init_t)(std::vector<std::pair<MemoryMappedDevice *, Word>> *);
typedef int (*plug_destroy_t)(void);

std::vector<plug_init_t> plug_init_funcs;
std::vector<plug_destroy_t> plug_destroy_funcs;

int main(void) {
  using namespace std::this_thread;
  using namespace std::chrono;

  signal(SIGINT, signal_callback_handler);

  load_plugins();
  rom_list = load_roms();
  add_spc.map_roms(rom_list);
#ifndef FUNCTEST
  std::cout << "Beginning execution in 1 second! Press Ctrl+C to quit." << std::endl;
  sleep_for(seconds(1));

  init_plugins();
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
  while (1) {
#ifndef FUNCTEST
    char *fname = readline("Enter path of a ROM to be mapped, or press Return when done: ");
    if (strlen(fname) == 0)
      return rom_list;
    if (char *fname_first = strtok(fname, " "))
      fname = fname_first;
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
        std::cout << "Invalid input" << std::endl << "0x";
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

void load_plugins() {
  std::string plugin_path = "./plugins";
  for (auto &entry : std::filesystem::directory_iterator(plugin_path, {})) {
    if (entry.is_regular_file() || entry.is_symlink() && entry.path().extension().string() == "so") {
      void *plugin = dlopen(entry.path().c_str(), RTLD_NOW | RTLD_GLOBAL);
      if (!plugin) {
        std::cerr << dlerror() << std::endl;
        break;
      }

      auto plug_init_func = (plug_init_t)dlsym(plugin, "plugin_init");
      if (!plug_init_func) {
        std::cerr << dlerror() << std::endl;
        break;
      }
      plug_init_funcs.push_back(plug_init_func);

      auto plug_destroy_func = (plug_destroy_t)dlsym(plugin, "plugin_destroy");
      if (!plug_destroy_func) {
        std::cerr << dlerror() << std::endl;
        break;
      }
      plug_destroy_funcs.push_back(plug_destroy_func);
    }
  }
}

void init_plugins() {
  std::vector<std::pair<MemoryMappedDevice *, Word>> plugin_devs;
  for (auto plug_init_func : plug_init_funcs)
    plug_init_func(&plugin_devs);
  for (auto [dev, addr] : plugin_devs) {
    add_spc.map_mem(dev, addr);
  }
}

void signal_callback_handler(int signum) {
  if (signum == SIGINT || signum == SIGTERM)
    emu_exit(0);
}

void emu_exit(int code) {
  for (auto plug_destroy_func : plug_destroy_funcs)
    plug_destroy_func();

  std::cout << std::endl << "Exiting." << std::endl;
  for (ROM *r : rom_list) {
    delete[] r->content;
    delete r;
  }
  rom_list.clear();
  exit(code);
}
