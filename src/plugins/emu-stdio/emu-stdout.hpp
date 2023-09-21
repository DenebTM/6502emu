#pragma once
#include "mem-dev.hpp"

struct OutChar : public MemoryMappedDevice {
  OutChar();
  Byte *val;

  int pre_read();
  int post_write();
};
