#pragma once
#include "mem-dev.hpp"

struct InChar : public MemoryMappedDevice {
  InChar();
  Byte val;

  int pre_update();
  int post_update();
};
