#pragma once
#include <SDL2/SDL.h>
#include <future>
#include <thread>

#include "mem-dev.hpp"

class Chardev : public MemoryMappedDevice {
public:
  Chardev();
  ~Chardev();

  int pre_read(Word offset);
  int post_write(Word offset);

  int sdl_init();

  void sdl_render();
  void sdl_handle_events();

  bool load_success = false;

private:
  Byte *screen_mem;

  SDL_Window *window;
  SDL_Renderer *renderer;

  char *char_rom;
  SDL_Texture **characters;

  bool sdl_initialized = false;
  bool sdl_thread_exit = false;

  std::thread sdl_thread;
  void sdl_thread_fn(std::promise<int> &&ret);

  int load_char_rom();
  void create_char_textures();
};
