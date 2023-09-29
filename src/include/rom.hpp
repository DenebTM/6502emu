#pragma once
#include "emu-common.hpp"

struct ROM {
  ROM(const Byte *content, DWord rom_size) : ROM(content, rom_size, 0) {}
  ROM(const Byte *content, DWord rom_size, DWord start_addr) : ROM(content, rom_size, start_addr, true) {}
  ROM(const Byte *content, DWord rom_size, DWord start_addr, bool read_only)
      : content(content), size(rom_size), start_addr(start_addr), read_only(read_only) {}

  const Byte *content;
  const DWord size;
  const DWord start_addr;
  bool read_only = true;
};
