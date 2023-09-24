#pragma once
#include "emu-common.hpp"

struct ROM {
  ROM(DWord rom_size, const Byte *content) : ROM(rom_size, content, 0) {}
  ROM(DWord rom_size, const Byte *content, DWord start_addr) : ROM(rom_size, content, start_addr, true) {}
  ROM(DWord rom_size, const Byte *content, DWord start_addr, bool read_only)
      : size(rom_size), content(content), start_addr(start_addr), read_only(read_only) {}

  const Byte *content;
  const DWord size;
  const DWord start_addr;
  bool read_only = true;
};
