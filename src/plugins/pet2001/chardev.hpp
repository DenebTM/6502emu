#pragma once
#include <SDL2/SDL.h>
#include <thread>

#include "mem-dev.hpp"

class Chardev : public MemoryMappedDevice {
public:
  Chardev();
  ~Chardev();

  int pre_read(Word offset);
  int post_write(Word offset);

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

  bool sdl_initialized = false;

  bool render_thread_exit = false;
  std::thread render_thread;
  void render_thread_func();

  int load_char_rom();
  void create_char_textures();
};
