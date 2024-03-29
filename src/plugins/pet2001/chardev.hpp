#pragma once
#include <SDL2/SDL.h>

#include "mem-dev.hpp"

class Chardev : public MemoryMappedDevice {
public:
  Chardev();
  ~Chardev();

  int sdl_init(SDL_Renderer *renderer);

  void update();

  void ui_render();
  void ui_handle_event(SDL_Event &event);

  bool load_success = false;

private:
  Byte *screen_mem;

  SDL_Renderer *renderer;

  char *char_rom;
  SDL_Texture **characters;

  SDL_Texture *render_tex;

  int load_char_rom();
  int create_char_textures();
};
