#pragma once
#include <thread>

#include "mem-dev.hpp"

class Via : public MemoryMappedDevice {
public:
  Via();

  int pre_read(Word offset);
  int post_write(Word offset);

  void flag_interrupt(Byte irq);

private:
  Byte *ifr = mapped_regs + 0xd;
  Byte *ier = mapped_regs + 0xe;
};
