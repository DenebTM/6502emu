#pragma once
#include <thread>

#include "mem-dev.hpp"

struct InChar : public MemoryMappedDevice {
  InChar();
  ~InChar();

  Byte *last_char;
  std::thread *stdin_thread;
  std::atomic_bool stdin_thread_running = true;

  Byte read(Word offset) override;
};
