#include <algorithm>
#include <atomic>
#include <chrono>
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
#include "plugin-loader.hpp"

void load_configured_roms();
void setup_configured_ram();
void signal_callback_handler(int signum);
void emu_exit(int code);

extern void plugin_callback_handler(PluginCallbackType, void *);

int exit_code = 0;
std::atomic_bool is_running = true;

AddressSpace add_spc;
Emu6502 cpu;

int main(int argc, char **argv) {
  // load configuration
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
  signal(SIGTERM, signal_callback_handler);
  std::cout << "Press Ctrl+C to quit." << std::endl;

  load_configured_roms();
  setup_configured_ram();
  load_configured_plugins();

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

void load_configured_roms() {
  for (auto [file_name, start_addr, read_only] : config->roms) {
    // load ROM and determine size in bytes
    std::ifstream file(file_name, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // get ROM contents
    char *bytes = new char[size];
    file.read(bytes, size);

    // copy ROM into address space
    add_spc.map_mem((Byte *)bytes, size, start_addr, read_only);
    delete[] bytes;
  }
}

void setup_configured_ram() {
  for (auto [start_addr, size] : config->ram)
    add_spc.map_mem(NULL, size, start_addr, false);
}

void signal_callback_handler(int signum) {
  if (signum == SIGINT || signum == SIGTERM)
    emu_exit(0);
}

void emu_exit(int code) {
  std::cout << std::endl << "Exiting." << std::endl;

  destroy_plugins();
  delete config;

  exit(code);
}
