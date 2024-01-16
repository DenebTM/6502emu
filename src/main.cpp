#include <algorithm>
#include <atomic>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <readline/readline.h>
#include <thread>
using namespace std::chrono_literals;

#include "cpu.hpp"
#include "emu-config.hpp"
#include "emu-types.hpp"
#include "mem.hpp"
#include "plugin-loader.hpp"
#include "sysclock.hpp"

#include "ui/ui_thread.hpp"

void load_configured_roms();
void setup_configured_ram();
void signal_callback_handler(int signum);
void emu_exit(int status);

int exit_status = 0;
std::atomic_bool is_running = true;

AddressSpace add_spc;
Emu6502 cpu;

std::thread *main_window_thread;

char *input_config_file() {
  auto filename = readline("Enter config filename: ");

  // got EOF or empty string
  // TODO: handle empty string case separately
  if (!filename || strlen(filename) == 0)
    exit(0);

  // trim trailing spaces that may be added by `readline`
  return strtok(filename, " ");
}

int main(int argc, char **argv) {
  // load configuration
  char *config_filename = nullptr;
  if (argc >= 2) {
    config_filename = argv[1];
  } else {
    config_filename = input_config_file();
  }

  while (!(config_filename && std::filesystem::is_regular_file(config_filename))) {
    if (!std::filesystem::is_regular_file(config_filename)) {
      std::cerr << "File not found or invalid: '" << config_filename << "'" << std::endl;
    }
    config_filename = input_config_file();
  }

  config = new EmuConfig(config_filename);

  signal(SIGINT, signal_callback_handler);
  signal(SIGTERM, signal_callback_handler);
  std::cout << "Press Ctrl+C to quit." << std::endl;

  load_configured_roms();
  setup_configured_ram();
  load_configured_plugins();

  start_ui_thread();
  sysclock_init(config->clock_speed);
  init_plugins();

  // config may either use the reset vector (default) or set the pc to a defined value
  cpu.do_reset = config->init_reset;
  cpu.reg_pc = config->init_pc;

  // execution loop runs until `is_running` is cleared by either the signal handler or plugin callback
  while (is_running) {
    cpu.do_instruction_pre();

    if (!sysclock_paused) {
      cpu.do_instruction();
    } else {
      std::this_thread::sleep_for(1ms);

      if (cpu.step_instructions != 0)
        sysclock_resume();
    }
  }

  emu_exit(exit_status);
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
    add_spc.map_mem(nullptr, size, start_addr, false);
}

void signal_callback_handler(int signum) {
  if (signum == SIGINT || signum == SIGTERM)
    emu_exit(0);
}

void emu_exit(int status) {
  std::cout << std::endl << "Exiting." << std::endl;

  destroy_plugins();
  stop_ui_thread();
  delete config;

  exit(status);
}
