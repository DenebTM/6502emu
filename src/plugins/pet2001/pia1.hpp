#pragma once
#include <SDL2/SDL_keyboard.h>
#include <map>
#include <optional>
#include <vector>

#include "mem-dev.hpp"

class Pia1 : public MemoryMappedDevice {
public:
  Pia1();

  int pre_read(Word offset);
  int post_write(Word offset) { return 0; }

  Byte read(Word offset) override;
  Byte write(Word offset, Byte val) override;

  void key_down(SDL_Keysym &key);
  void key_up(SDL_Keysym &key);

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
};
