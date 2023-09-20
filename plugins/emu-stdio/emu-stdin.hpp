#pragma once
// #include "../../mem.hpp"
#include "../../mem-dev.hpp"
#include "common.hpp"

struct InChar : public MemoryMappedDevice {
  InChar();
  Byte val;

  int pre_update();
  int post_update();
};
