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

  // FIXME: consolidate these
  virtual int pre_read(Word offset) = 0;
  virtual int post_write(Word offset) = 0;
  virtual Byte read(Word offset) { return mapped_regs[offset]; }
  virtual Byte write(Word offset, Byte val) { return mapped_regs[offset] = val; }
};
