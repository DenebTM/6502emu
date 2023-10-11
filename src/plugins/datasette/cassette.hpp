#pragma once
#include <string>

#include "plugins/6520-pia.hpp"

#define TAP_HEADER_LEN 0x14

struct Datasette {
  void load_tap(std::string filename);
  void play();
  void stop();
  void rewind();

  // positive non-zero return: data was read
  void update();

  unsigned char *tap;
  size_t tap_size = 0;
  bool playing = false;
  size_t tap_index = 0;
};
