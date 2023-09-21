#pragma once
#include <SDL2/SDL.h>

#include "mem-dev.hpp"

class Chardev : public MemoryMappedDevice {
public:
  Chardev();
  ~Chardev();

  int pre_read();
  int post_write();

  int init_sdl();

  void render();
  void handle_events();

  bool load_success = false;

private:
  Byte *screen_mem;

  SDL_Window *window;
  SDL_Renderer *renderer;

  char *char_rom;
  SDL_Texture **characters;

  int load_char_rom();
  void create_char_textures();
};
