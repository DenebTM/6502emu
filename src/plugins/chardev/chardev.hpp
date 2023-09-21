#pragma once
#include <SDL2/SDL.h>

#include "mem-dev.hpp"

class Chardev : public MemoryMappedDevice {
public:
  Chardev();
  ~Chardev();

  int pre_read();
  int post_write();

  void render();
  void handle_events();

private:
  Byte *screen_mem;
};
