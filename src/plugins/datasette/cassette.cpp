#include <filesystem>
#include <fstream>
#include <iostream>

#include "cassette.hpp"
#include "emu-types.hpp"
#include "plugins/6520-pia.hpp"

extern Pia *pia1;

void Datasette::load_tap(std::string filename) {
  if (!std::filesystem::is_regular_file(filename)) {
    std::cerr << "File not found or invalid: '" << filename << "'" << std::endl;
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
  if (pia1)
    pia1->mapped_regs[Pia::ORA] &= ~BIT4;
}

void Datasette::stop() {
  this->playing = false;
  if (pia1)
    pia1->mapped_regs[Pia::ORA] |= BIT4;
}

int Datasette::update() {
  int retval = 0;

  static unsigned int cycles_since_last_pulse = 0;
  static unsigned int autostop_cycles = 0;

  bool motor_en = (pia1 != nullptr) && (pia1->cb2 == 0);

  if (playing && motor_en && tap_index < tap_size) {
    unsigned int num_cycles = tap[tap_index] * 8;

    bool nc_zero = (num_cycles == 0);
    if (nc_zero) {
      num_cycles = tap[tap_index + 1] | (tap[tap_index + 2] << 8) | (tap[tap_index + 3] << 16);
    }

    if (cycles_since_last_pulse >= num_cycles) {
      pia1->set_ca1(0);
      pia1->set_ca1(1);

      int step = nc_zero ? 4 : 1;
      tap_index += step;
      cycles_since_last_pulse = 0;

      retval = step;
    }

    cycles_since_last_pulse++;
  } else if (playing) {
    autostop_cycles++;

    if (autostop_cycles >= 1000000) {
      autostop_cycles = 0;
      stop();
    }
  }

  return retval;
}

void Datasette::rewind() { tap_index = TAP_HEADER_LEN; }
