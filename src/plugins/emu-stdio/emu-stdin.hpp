#pragma once
#include "mem-dev.hpp"

struct InChar : public MemoryMappedDevice {
  InChar();
  Byte *val;

  int pre_read(Word offset);
  int post_write(Word offset);
};
