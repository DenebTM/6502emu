#include <algorithm>
#include <filesystem>
#include <fstream>
using namespace std::chrono_literals;

#include "pia1.hpp"
#include "plugin-callback.hpp"

extern plugin_callback_t plugin_callback;

Pia1::Pia1() : MemoryMappedDevice(false, 4) {
  *port_a = 0xff;
  *port_b = 0xff;

  *ctrl_a = 0x80;
  *ctrl_b = 0x80;

  std::fill_n(keyboard_rows, 10, 0xff);
}

int Pia1::pre_read(Word offset) {
  if (mapped_regs + offset == port_b) {
    *port_b = keyboard_rows[get_row_request()];
  }

  return 0;
}

Byte Pia1::read(Word offset) {
  Byte val = mapped_regs[offset];

  if (*ctrl_a & 0x01) {
    *ctrl_a &= ~0x80;
  }
  if (*ctrl_b & 0x01) {
    *ctrl_b &= ~0x80;
  }

  return val;
}

Byte Pia1::write(Word offset, Byte val) {
  if (mapped_regs + offset == port_a) {
    val = (mapped_regs[offset] & 0xf0) | (val & ~0xf0);
  } else if (mapped_regs + offset == port_b) {
    val = mapped_regs[offset];
  } else if (mapped_regs + offset == ctrl_a || mapped_regs + offset == ctrl_b) {
    val = (mapped_regs[offset] & 0x80) | (val & ~0xc0);
  }

  return mapped_regs[offset] = val;
}

void Pia1::key_down(SDL_Keysym key) {
  auto maybe_modkey = modkey_map(key);

  if (maybe_modkey.has_value()) {
    auto [row, bit] = maybe_modkey.value();
    keyboard_rows[row] &= ~bit;
  } else {
    auto [row, bit] = keysym_map(key.sym);
    keyboard_rows[row] &= ~bit;
  }
}

void Pia1::key_up(SDL_Keysym key) {
  auto maybe_modkey = modkey_map(key);

  if (maybe_modkey.has_value()) {
    auto [row, bit] = maybe_modkey.value();
    if (bit) {
      keyboard_rows[row] |= bit;
    }
  } else {
    auto [row, bit] = keysym_map(key.sym);
    if (bit) {
      keyboard_rows[row] |= bit;
    }
  }
}

void Pia1::flag_interrupt() {
  if (*ctrl_b & 0x01) {
    *ctrl_b |= 0x80;
    plugin_callback(CPU_INTERRUPT, (void *)false);
  }
}

Byte Pia1::get_row_request() { return (*port_a & 0x0f) % 10; }
