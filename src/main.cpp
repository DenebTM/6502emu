#include <algorithm>
#include <atomic>
#include <chrono>
#include <dlfcn.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <readline/readline.h>
#include <signal.h>

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

AddressSpace add_spc;
Emu6502 cpu;

// FIXME: pull these out into their own translation unit (see also sysclock.cpp)
typedef int (*plugin_load_t)(void);
typedef int (*plugin_init_t)(AddressSpace &, plugin_callback_t);
typedef int (*plugin_destroy_t)(void);
typedef int (*plugin_update_t)(int cycles_elapsed);

std::vector<plugin_init_t> plugin_init_funcs;
std::vector<plugin_destroy_t> plugin_destroy_funcs;
std::vector<plugin_update_t> plugin_update_funcs;

int main(int argc, char **argv) {
  char *config_file_name;
  if (argc >= 2) {
    config_file_name = argv[1];
  } else {
    config_file_name = readline("Enter config filename: ");
  }

  // got EOF or empty string
  // TODO: handle empty string case separately
  if (!config_file_name || strlen(config_file_name) == 0)
    return 0;

  // trim trailing spaces that may be added by `readline`
  config_file_name = strtok(config_file_name, " ");
  config = new EmuConfig(config_file_name);

  signal(SIGINT, signal_callback_handler);

  load_roms();
  if (config->enumerate_plugins) {
    load_plugins();
  }

  std::cout << "Press Ctrl+C to quit." << std::endl;

  init_plugins();

  // config may either use the reset vector (default) or set the pc to a defined value
  if (config->init_reset) {
    cpu.reset();
  } else {
    cpu.reg_pc = config->init_pc;
  }

  // execution loop runs until is_running is cleared by either the signal handler or plugin callback
  while (is_running) {
    cpu.do_instruction();
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

    ROM rom = ROM((const Byte *)bytes, size, start_addr, read_only);
    add_spc.map_mem(rom);

    delete[] bytes;
  }
}

void load_plugins() {
  std::string plugin_path = "./plugins";
  if (!std::filesystem::exists(plugin_path))
    return;

  for (auto &entry : std::filesystem::directory_iterator(plugin_path, {})) {
    if ((entry.is_regular_file() || entry.is_symlink()) && entry.path().extension().string() == ".so") {

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
  for (auto plugin_init_func : plugin_init_funcs)
    plugin_init_func(add_spc, &plugin_callback_handler);
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
