#pragma once
#include "mem-dev.hpp"

struct InChar : public MemoryMappedDevice {
  InChar();
  ~InChar();
  Byte *val;

  inline int pre_read(Word offset) { return 0; };
  inline int post_write(Word offset) { return 0; };

  inline Byte read(Word offset) {
    auto _val = *val;
    *val = 0;
    return _val;
  }
};
