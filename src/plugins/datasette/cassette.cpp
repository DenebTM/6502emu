#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include "cassette.hpp"
#include "emu-types.hpp"
#include "plugins/6520-pia.hpp"

extern Pia *pia1;

void Datasette::load_tap(std::string filename) {
  if (!std::filesystem::exists(filename)) {
    std::cerr << "Could not find file '" << filename << "'" << std::endl;
    return;
  }

  std::ifstream file(filename, std::ios::binary | std::ios::ate);

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  tap = new unsigned char[size];
  tap_size = size;
  tap_index = TAP_HEADER_LEN;
  file.read((char *)tap, size);

  std::cout << "Loaded file '" << filename << "' with size " << (tap_size - TAP_HEADER_LEN) << std::endl;
}

void Datasette::play() {
  this->playing = true;
  pia1->mapped_regs[Pia::ORA] &= ~BIT4;
}

void Datasette::stop() {
  this->playing = false;
  pia1->mapped_regs[Pia::ORA] |= BIT4;
}

void Datasette::update() {
  static size_t cycles_since_last_pulse = 0;

  bool motor_en = (pia1->cb2 == 0);

  if (playing && motor_en && tap_index < tap_size) {
    size_t num_cycles = tap[tap_index] * 8;

    bool nc_zero = (num_cycles == 0);
    if (nc_zero) {
      num_cycles = tap[tap_index + 1] | (tap[tap_index + 2] << 8) | (tap[tap_index + 3] << 16);
    }

    if (cycles_since_last_pulse >= num_cycles) {
      pia1->set_ca1(0);
      pia1->set_ca1(1);

      tap_index += nc_zero ? 4 : 1;
      cycles_since_last_pulse = 0;

      static const char *CSI = "\33[";
      std::cout << std::endl << (tap_index - TAP_HEADER_LEN) << " / " << (tap_size - TAP_HEADER_LEN);
      std::cout << CSI << "1F> ";
      fflush(stdout);
    }

    cycles_since_last_pulse++;
  }
}

void Datasette::rewind() { tap_index = TAP_HEADER_LEN; }
