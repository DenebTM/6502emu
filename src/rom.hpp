#include "common.hpp"

struct ROM {
  ROM(DWord rom_size, Byte *rom_content) : ROM(rom_size, rom_content, 0) {}
  ROM(DWord rom_size, Byte *rom_content, DWord start_addr)
      : size(rom_size), content(rom_content), start_address(start_addr) {}
  const Byte *operator[](DWord i) {
    if (i < size)
      return content + i;
    return NULL;
  }

  const Byte *content;
  const DWord size;
  const DWord start_address;
};
