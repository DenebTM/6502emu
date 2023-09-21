#pragma once
#include "emu-common.hpp"

struct MemoryMappedDevice {
  MemoryMappedDevice(bool ro, DWord mapc) : read_only(ro), num_mapped_regs(mapc), mapped_regs(new Byte[mapc]) {}
  virtual ~MemoryMappedDevice() { delete[] mapped_regs; }

  const bool read_only = false;
  const DWord num_mapped_regs;
  Byte *mapped_regs;

  virtual int pre_read() = 0;
  virtual int post_write() = 0;
};
