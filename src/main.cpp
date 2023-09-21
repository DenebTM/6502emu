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
#include "plugin-callback.hpp"

std::list<ROM *> load_roms();
void load_plugins();
void init_plugins();
void signal_callback_handler(int signum);
void emu_exit(int code);

extern void plugin_callback_handler(PluginCallbackType, void *);

bool is_running = false, init_mode = true;

QWord cyclesToRun = -1, cycle = 0;

AddressSpace add_spc;
Emu6502 cpu;
std::list<ROM *> rom_list;

typedef int (*plugin_load_t)(void);
typedef int (*plugin_init_t)(std::vector<std::pair<MemoryMappedDevice *, Word>> *, plugin_callback_t);
typedef int (*plugin_destroy_t)(void);
typedef int (*plugin_update_t)(void);

std::vector<plugin_init_t> plugin_init_funcs;
std::vector<plugin_destroy_t> plugin_destroy_funcs;
std::vector<plugin_update_t> plugin_update_funcs;

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
  cpu.reset();
#else
  cpu.reg_pc = 0x0400;
#endif
  while (1) {
    cpu.do_instruction();

#ifndef FUNCTEST
    if (cycle > 3000) {
      cycle = 0;
      sleep_for(nanoseconds(833333));

      for (auto plugin_update_func : plugin_update_funcs) {
        plugin_update_func();
      }
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

      auto plugin_load_func = (plugin_load_t)dlsym(plugin, "plugin_load");
      if (plugin_load_func && plugin_load_func() == -1) {
        std::cerr << "Plugin " << entry.path().filename() << " failed to load." << std::endl;
        continue;
      }

      auto plugin_init_func = (plugin_init_t)dlsym(plugin, "plugin_init");
      if (!plugin_init_func) {
        std::cerr << dlerror() << std::endl;
        continue;
      }
      plugin_init_funcs.push_back(plugin_init_func);

      auto plugin_destroy_func = (plugin_destroy_t)dlsym(plugin, "plugin_destroy");
      if (!plugin_destroy_func) {
        std::cerr << dlerror() << std::endl;
        continue;
      }
      plugin_destroy_funcs.push_back(plugin_destroy_func);

      auto plugin_update_func = (plugin_update_t)dlsym(plugin, "plugin_update");
      if (plugin_update_func)
        plugin_update_funcs.push_back(plugin_update_func);
    }
  }
}

void init_plugins() {
  std::vector<std::pair<MemoryMappedDevice *, Word>> plugin_devs;
  for (auto plugin_init_func : plugin_init_funcs)
    plugin_init_func(&plugin_devs, &plugin_callback_handler);
  for (auto [dev, addr] : plugin_devs) {
    add_spc.map_mem(dev, addr);
  }
}

void signal_callback_handler(int signum) {
  if (signum == SIGINT || signum == SIGTERM)
    emu_exit(0);
}

void emu_exit(int code) {
  for (auto plugin_destroy_func : plugin_destroy_funcs)
    plugin_destroy_func();

  std::cout << std::endl << "Exiting." << std::endl;
  for (ROM *r : rom_list) {
    delete[] r->content;
    delete r;
  }
  rom_list.clear();
  exit(code);
}
