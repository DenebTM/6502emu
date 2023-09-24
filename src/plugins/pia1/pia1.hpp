#pragma once
#include <thread>

#include "mem-dev.hpp"

class Pia1 : public MemoryMappedDevice {
public:
  Pia1();

  int pre_read();
  int post_write();
};
