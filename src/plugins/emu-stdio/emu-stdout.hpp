#pragma once
#include "mem-dev.hpp"

struct OutChar : public MemoryMappedDevice {
  OutChar();
  Byte *val;

  int pre_read(Word offset);
  int post_write(Word offset);
};
