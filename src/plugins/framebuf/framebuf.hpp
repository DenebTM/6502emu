#pragma once
#include <SDL2/SDL.h>

#include "mem-dev.hpp"

class Framebuf : public MemoryMappedDevice {
public:
  Framebuf();
  ~Framebuf();

  virtual Byte read(Word offset) override;
  virtual Byte write(Word offset, Byte val) override;

  int pre_read(Word offset) override { return 0; }
  int post_write(Word offset) override { return 0; }

  int sdl_init(SDL_Renderer *renderer);
  SDL_Surface *create_surface();

  void ui_render();

private:
  Byte *screen_mem;
  size_t active_bank = 0;

  SDL_Renderer *renderer;
  SDL_Texture *render_tex;
};
