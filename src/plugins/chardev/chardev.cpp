#include "chardev.hpp"
#include <SDL2/SDL.h>

SDL_Window *window;
SDL_Renderer *renderer;

Chardev::Chardev() : MemoryMappedDevice(false, 1024) {
  screen_mem = new Byte[1024];
  mapped_regs = screen_mem;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cout << "Failed to initialize SDL2" << std::endl;
    return;
  }

  window = SDL_CreateWindow("Chardev", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 960, 600, 0);
  if (!window) {
    std::cout << "Failed to create SDL2 window" << std::endl;
    return;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    std::cout << "Failed to create SDL2 renderer" << std::endl;
  }

  SDL_RenderSetIntegerScale(renderer, SDL_TRUE);
  SDL_RenderSetScale(renderer, 3., 3.);
}

Chardev::~Chardev() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
}

int Chardev::pre_read() { return 0; }

int Chardev::post_write() { return 0; }

void Chardev::handle_events() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {}
  }
}

void Chardev::render() {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  for (int row = 0; row < 25; row++) {
    for (int col = 0; col < 40; col++) {
      if (screen_mem[row * 40 + col]) {
        SDL_Rect h{.x = 8 * col, .y = 8 * row, .w = 8, .h = 8};
        SDL_RenderFillRect(renderer, &h);
      }
    }
  }

  SDL_RenderPresent(renderer);
}
