#pragma once
#include <SDL2/SDL_keyboard.h>
#include <thread>

#include "mem-dev.hpp"

class Pia1 : public MemoryMappedDevice {
public:
  Pia1();
  ~Pia1();

  int pre_read(Word offset);
  int post_write(Word offset);

  void start();

  void key_down(SDL_Keysym key);
  void key_up(SDL_Keysym key);

  // keyboard row select (bits 0-3)
  Byte *port_a;

  // keyboard row contents
  Byte *port_b;

private:
  bool pia1_running;
  std::thread irq_thread;
  void irq_thread_func();

  Byte get_row_request();

  Byte keyboard_rows[10];
};
