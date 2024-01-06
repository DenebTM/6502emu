#pragma once
#include "mem-dev.hpp"

struct OutChar : public MemoryMappedDevice {
  OutChar();
  Byte *val;

  int pre_read(Word offset) override;
  int post_write(Word offset) override;
};
