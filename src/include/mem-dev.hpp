#pragma once
#include <algorithm>

#include "emu-types.hpp"

/**
 * abstract base class representing a peripheral device accessible via MMIO
 */
struct MemoryMappedDevice {
protected:
  MemoryMappedDevice(bool ro, Word mapped_regs_count)
      : read_only(ro), mapped_regs_count(mapped_regs_count), mapped_regs(new Byte[mapped_regs_count]) {
    std::fill_n(mapped_regs, mapped_regs_count, 0xff);
  }

public:
  virtual ~MemoryMappedDevice() { delete[] mapped_regs; }

  const bool read_only = false;
  const Word mapped_regs_count;
  Byte *mapped_regs;

  virtual Byte read(Word offset) { return mapped_regs[offset]; }
  virtual Byte write(Word offset, Byte val) { return mapped_regs[offset] = val; }
};
