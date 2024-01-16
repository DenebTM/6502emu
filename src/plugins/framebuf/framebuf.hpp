#pragma once
#include <SDL2/SDL.h>

#include "mem-dev.hpp"

class Framebuf : public MemoryMappedDevice {
public:
  Framebuf();
  ~Framebuf();

  Byte read(Word offset) override;
  Byte write(Word offset, Byte val) override;

  int sdl_init(SDL_Renderer *renderer);
  SDL_Surface *create_surface();

  void ui_render();

private:
  Byte *screen_mem;
  size_t active_bank = 0;

  SDL_Renderer *renderer;
  SDL_Texture *render_tex;
};
