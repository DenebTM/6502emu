#include "keyboard.hpp"
#include "keymap-normal.hpp"

extern Pia *pia1;
Byte kb_rows[10] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
Byte kb_active_row = 0;

void set_kb_row(Byte row) { kb_active_row = row; }
Byte get_kb_row_contents() { return (kb_active_row < 10) ? kb_rows[kb_active_row] : 0xff; }

void handle_key_down(SDL_Keysym &key) {
  if (key.mod & KMOD_SHIFT && shiftmap.count(key.sym)) {
    auto &[row, bit] = shiftmap.at(key.sym);

    if (key.mod & KMOD_LSHIFT) {
      kb_rows[8] |= 0x01;
    }
    if (key.mod & KMOD_RSHIFT) {
      kb_rows[8] |= 0x20;
    }

    kb_rows[row] &= ~bit;
  } else if (key.mod & KMOD_CTRL && ctrlmap.count(key.sym)) {
    auto &[row, bit] = ctrlmap.at(key.sym);
    kb_rows[row] &= ~bit;
  } else if (keymap.count(key.sym)) {
    for (auto &[row, bit] : keymap.at(key.sym)) {
      kb_rows[row] &= ~bit;
    }
  }
}

void handle_key_up(SDL_Keysym &key) {
  if (key.sym == SDLK_LSHIFT || key.sym == SDLK_RSHIFT) {
    for (auto &[key, value] : shiftmap) {
      auto &[row, bit] = value;
      kb_rows[row] |= bit;
    }
  } else if (key.sym == SDLK_LCTRL || key.sym == SDLK_RCTRL) {
    for (auto &[key, value] : ctrlmap) {
      auto &[row, bit] = value;
      kb_rows[row] |= bit;
    }
  }

  if (keymap.count(key.sym)) {
    for (auto &[row, bit] : keymap.at(key.sym)) {
      kb_rows[row] |= bit;
    }
  }
}
