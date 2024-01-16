#pragma once
#include "mem-dev.hpp"

struct OutChar : public MemoryMappedDevice {
  OutChar();

  Byte *last_char;

  Byte write(Word offset, Byte val) override;
};
