#pragma once
#include <SDL2/SDL_keyboard.h>
#include <map>

#include "mem-dev.hpp"

class Pia1 : public MemoryMappedDevice {
public:
  Pia1();

  int pre_read(Word offset);
  int post_write(Word offset) { return 0; }

  Byte read(Word offset) override;
  Byte write(Word offset, Byte val) override;

  void key_down(SDL_Keysym key);
  void key_up(SDL_Keysym key);

  void flag_interrupt();

  // keyboard row select (bits 0-3)
  Byte *port_a = mapped_regs + 0;

  Byte *ctrl_a = mapped_regs + 1;

  // keyboard row contents
  Byte *port_b = mapped_regs + 2;

  Byte *ctrl_b = mapped_regs + 3;

private:
  Byte get_row_request();

  Byte keyboard_rows[10];

  inline std::pair<int, int> keysym_map(SDL_Keycode key) {
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
};
