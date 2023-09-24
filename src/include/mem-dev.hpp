#pragma once
#include <string.h>

#include "emu-common.hpp"

struct MemoryMappedDevice {
  MemoryMappedDevice(bool ro, DWord mapc) : read_only(ro), mapped_regs_count(mapc), mapped_regs(new Byte[mapc]) {
    memset(mapped_regs, 0xff, mapped_regs_count);
  }
  virtual ~MemoryMappedDevice() { delete[] mapped_regs; }

  const bool read_only = false;
  const Word mapped_regs_count;
  Byte *mapped_regs;

  virtual int pre_read(Word offset) = 0;
  virtual int post_write(Word offset) = 0;
};
