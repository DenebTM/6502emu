#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <map>
using namespace std::chrono_literals;

#include "pia1.hpp"
#include "plugin-callback.hpp"
#include "via.hpp"

extern plugin_callback_t plugin_callback;

extern Via *via;

Pia1::Pia1() : MemoryMappedDevice(true, 16) {
  mapped_regs[2] = 0xff;
  mapped_regs[3] = 0x80;

  port_a = mapped_regs + 0;
  port_b = mapped_regs + 2;

  std::fill_n(keyboard_rows, 10, 0xff);
}

Pia1::~Pia1() {
  pia1_running = false;
  if (irq_thread.joinable())
    irq_thread.join();
}

int Pia1::pre_read(Word offset) {
  if (mapped_regs + offset == port_b) {
    *port_b = keyboard_rows[get_row_request()];
  }

  return 0;
}

int Pia1::post_write(Word offset) { return 0; }

void Pia1::start() {
  pia1_running = true;
  irq_thread = std::thread(&Pia1::irq_thread_func, this);
}

std::pair<int, int> keysym_map(SDL_Keycode key) {
  static std::map<SDL_Keycode, std::pair<int, int>> keymap = {
      {SDLK_EQUALS, {9, 0x80}},      //
      {SDLK_PERIOD, {9, 0x40}},      //
      {SDLK_LESS, {9, 0x08}},        //
      {SDLK_SPACE, {9, 0x04}},       //
      {SDLK_LEFTBRACKET, {9, 0x02}}, //

      {SDLK_n, {7, 0x04}}, //
      {SDLK_v, {7, 0x02}}, //
      {SDLK_x, {7, 0x01}}, //

      {SDLK_m, {6, 0x08}}, //
      {SDLK_b, {6, 0x04}}, //
      {SDLK_c, {6, 0x02}}, //
      {SDLK_z, {6, 0x01}}, //

      {SDLK_k, {5, 0x08}}, //
      {SDLK_h, {5, 0x04}}, //
      {SDLK_f, {5, 0x02}}, //
      {SDLK_s, {5, 0x01}}, //

      {SDLK_l, {4, 0x10}}, //
      {SDLK_j, {4, 0x08}}, //
      {SDLK_g, {4, 0x04}}, //
      {SDLK_d, {4, 0x02}}, //
      {SDLK_a, {4, 0x01}}, //

      {SDLK_p, {3, 0x10}}, //
      {SDLK_i, {3, 0x08}}, //
      {SDLK_y, {3, 0x04}}, //
      {SDLK_r, {3, 0x02}}, //
      {SDLK_w, {3, 0x01}}, //

      {SDLK_o, {2, 0x10}}, //
      {SDLK_u, {2, 0x08}}, //
      {SDLK_t, {2, 0x04}}, //
      {SDLK_e, {2, 0x02}}, //
      {SDLK_q, {2, 0x01}}, //
  };

  return keymap[key];
}

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

void Pia1::irq_thread_func() {
  std::this_thread::sleep_for(2s);
  while (pia1_running) {
    std::this_thread::sleep_for(1000000us / 60);
    via->flag_interrupt(0x10);
  }
}

Byte Pia1::get_row_request() { return (*port_a & 0x0f) % 10; }
