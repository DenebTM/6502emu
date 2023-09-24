#include <algorithm>
#include <atomic>
#include <chrono>
#include <dlfcn.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <readline/readline.h>
#include <signal.h>
#include <thread>
#include <tuple>
#include <vector>
using namespace std::chrono_literals;

#include "cpu.hpp"
#include "emu-common.hpp"
#include "emu-config.hpp"
#include "mem.hpp"
#include "plugin-callback.hpp"

void load_roms();
void load_plugins();
void init_plugins();
void signal_callback_handler(int signum);
void emu_exit(int code);

extern void plugin_callback_handler(PluginCallbackType, void *);

int exit_code = 0;
std::atomic_bool is_running = true;

#define CYCLES_UNTIL_PAUSE 100
QWord cyclesToRun = -1, cycle = 0;

AddressSpace add_spc;
Emu6502 cpu;

EmuConfig *config;

typedef int (*plugin_load_t)(void);
typedef int (*plugin_init_t)(std::vector<std::pair<MemoryMappedDevice *, Word>> *, plugin_callback_t);
typedef int (*plugin_destroy_t)(void);
typedef int (*plugin_update_t)(void);

std::vector<plugin_init_t> plugin_init_funcs;
std::vector<plugin_destroy_t> plugin_destroy_funcs;
std::vector<plugin_update_t> plugin_update_funcs;

int main(int argc, char **argv) {
  using namespace std::this_thread;
  using namespace std::chrono;

  char *config_file_name;
  if (argc >= 2) {
    config_file_name = argv[1];
  } else {
    config_file_name = readline("Enter config filename: ");
  }
  if (!config_file_name || strlen(config_file_name) == 0) {
    return 0;
  }

  config_file_name = strtok(config_file_name, " ");
  config = new EmuConfig(config_file_name);

  const auto sleep_time = 1000000000ns / (config->clock_speed / CYCLES_UNTIL_PAUSE);

  signal(SIGINT, signal_callback_handler);

  load_roms();
  if (config->enumerate_plugins) {
    load_plugins();
  }

  std::cout << "Press Ctrl+C to quit." << std::endl;

  init_plugins();

  // Start execution loop
  if (config->init_reset) {
    cpu.reset();
  } else {
    cpu.reg_pc = config->init_pc;
  }

  while (is_running.load()) {
    cpu.do_instruction();

    if (cycle > CYCLES_UNTIL_PAUSE) {
      cycle = 0;
      sleep_for(sleep_time);

      for (auto plugin_update_func : plugin_update_funcs) {
        plugin_update_func();
      }
    }
  }

  emu_exit(exit_code);
}

void load_roms() {
  for (auto [file_name, start_addr, read_only] : config->roms) {
    std::ifstream file(file_name, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    char *bytes = new char[size];
    file.read(bytes, size);

    ROM rom = ROM(size, (const Byte *)bytes, start_addr, read_only);
    add_spc.map_mem(rom);

    delete bytes;
  }
}

void load_plugins() {
  std::string plugin_path = "./plugins";
  if (!std::filesystem::exists(plugin_path))
    return;

  for (auto &entry : std::filesystem::directory_iterator(plugin_path, {})) {
    if (entry.is_regular_file() || entry.is_symlink() && entry.path().extension().string() == "so") {

      std::string filename = entry.path().filename().string();
      if (std::find(config->disabled_plugins.begin(), config->disabled_plugins.end(), filename) !=
          config->disabled_plugins.end())
        continue;

      void *plugin = dlopen(entry.path().c_str(), RTLD_NOW | RTLD_GLOBAL);
      if (!plugin) {
        std::cerr << dlerror() << std::endl;
        break;
      }

      auto plugin_load_func = (plugin_load_t)dlsym(plugin, "plugin_load");
      if (plugin_load_func && plugin_load_func() == -1) {
        std::cerr << "Plugin " << filename << " failed to load." << std::endl;
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
  std::cout << std::endl << "Exiting." << std::endl;

  for (auto plugin_destroy_func : plugin_destroy_funcs)
    plugin_destroy_func();

  delete config;

  exit(code);
}
