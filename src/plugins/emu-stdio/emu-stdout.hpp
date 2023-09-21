#pragma once
#include "mem-dev.hpp"

struct OutChar : public MemoryMappedDevice {
  OutChar();
  Byte val;

  int pre_update();
  int post_update();
};
