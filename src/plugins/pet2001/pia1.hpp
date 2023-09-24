#pragma once
#include <thread>

#include "mem-dev.hpp"

class Pia1 : public MemoryMappedDevice {
public:
  Pia1();
  ~Pia1();

  int pre_read(Word offset);
  int post_write(Word offset);

  void start();

private:
  bool pia1_running;
  std::thread irq_thread;
  void irq_thread_func();
};
