#pragma once
#include "../../mem-dev.hpp"
#include "common.hpp"

struct OutChar : public MemoryMappedDevice {
  OutChar();
  Byte val;

  int pre_update();
  int post_update();
};
