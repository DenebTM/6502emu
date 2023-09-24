#pragma once
#include <algorithm>

#include "emu-common.hpp"

struct MemoryMappedDevice {
  MemoryMappedDevice(bool ro, Word mapped_regs_count)
      : read_only(ro), mapped_regs_count(mapped_regs_count), mapped_regs(new Byte[mapped_regs_count]) {
    std::fill_n(mapped_regs, mapped_regs_count, 0xff);
  }
  virtual ~MemoryMappedDevice() { delete[] mapped_regs; }

  const bool read_only = false;
  const Word mapped_regs_count;
  Byte *mapped_regs;

  virtual int pre_read(Word offset) = 0;
  virtual int post_write(Word offset) = 0;
};
