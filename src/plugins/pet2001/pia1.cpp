#include <algorithm>
#include <filesystem>
#include <fstream>
using namespace std::chrono_literals;

#include "pia1.hpp"
#include "plugin-callback.hpp"

extern plugin_callback_t plugin_callback;

Pia1::Pia1() : MemoryMappedDevice(true, 16) {
  mapped_regs[2] = 0xff;
  mapped_regs[3] = 0x80;

  port_a = mapped_regs + 0;
  port_b = mapped_regs + 2;

  std::fill_n(keyboard_rows, 10, 0xff);
}

int Pia1::pre_read(Word offset) {
  if (mapped_regs + offset == port_b) {
    *port_b = keyboard_rows[get_row_request()];
  }

  return 0;
}

int Pia1::post_write(Word offset) { return 0; }

void Pia1::key_down(SDL_Keysym key) {
  if (key.mod & KMOD_CTRL) {
    switch (key.sym) {
      case SDLK_c:
        keyboard_rows[9] &= ~0x10;
        break;
    }
  }

  else {
    auto [row, bit] = keysym_map(key.sym);
    keyboard_rows[row] &= ~bit;
  }
}

void Pia1::key_up(SDL_Keysym key) {
  if (key.mod & KMOD_CTRL) {
    switch (key.scancode) {
      case SDLK_c:
        keyboard_rows[9] |= 0x10;
        break;
    }
  }

  else {
    auto [row, bit] = keysym_map(key.sym);
    keyboard_rows[row] |= bit;
  }
}

Byte Pia1::get_row_request() { return (*port_a & 0x0f) % 10; }
