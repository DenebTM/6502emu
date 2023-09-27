#pragma once
#include <thread>

#include "mem-dev.hpp"

class Via : public MemoryMappedDevice {
public:
  Via();

  int pre_read(Word offset) { return 0; }
  int post_write(Word offset) { return 0; }

  Byte write(Word offset, Byte val) override;

  void flag_interrupt(Byte irq);

  void update(int cycles_taken);

private:
  Byte *ifr = mapped_regs + 0xd;
  Byte *ier = mapped_regs + 0xe;

  ushort *timer1_period = (ushort *)(mapped_regs + 0x4);
};
