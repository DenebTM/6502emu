#include <SDL2/SDL.h>
#include <filesystem>
#include <fstream>

#include "chardev.hpp"
#include "plugin-callback.hpp"

#define COL_WIDTH 8
#define ROW_HEIGHT 8
#define COLS 40
#define ROWS 25

#define RENDER_SCALE 2.
constexpr int RENDER_WIDTH = COL_WIDTH * COLS * RENDER_SCALE;
constexpr int RENDER_HEIGHT = ROW_HEIGHT * ROWS * RENDER_SCALE;

extern plugin_callback_t plugin_callback;

Chardev::Chardev() : MemoryMappedDevice(false, 1024) {
  screen_mem = new Byte[1024];
  mapped_regs = screen_mem;

  if (load_char_rom() == 0)
    load_success = true;
}

Chardev::~Chardev() {
  render_thread_exit = true;
  if (render_thread.joinable())
    render_thread.join();

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  if (characters) {
    delete[] characters;
  }
  if (char_rom)
    delete[] char_rom;
}

int Chardev::pre_read() { return 0; }

int Chardev::post_write() { return 0; }

int Chardev::init_sdl() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "Failed to initialize SDL2" << std::endl;
    return -1;
  }

  window =
      SDL_CreateWindow("Chardev", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, RENDER_WIDTH, RENDER_HEIGHT, 0);
  if (!window) {
    std::cerr << "Failed to create SDL2 window" << std::endl;
    return -2;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!renderer) {
    std::cerr << "Failed to create SDL2 renderer" << std::endl;
    return -3;
  }

  create_char_textures();

  SDL_RenderSetIntegerScale(renderer, SDL_TRUE);
  SDL_RenderSetScale(renderer, RENDER_SCALE, RENDER_SCALE);
  render_thread = std::thread(&Chardev::render_thread_func, this);

  return 0;
}

void Chardev::handle_events() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        plugin_callback(EMU_EXIT, (void *)0);
        break;
    }
  }
}

void Chardev::render() {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  for (int row = 0; row < 25; row++) {
    for (int col = 0; col < 40; col++) {
      auto screen_addr = row * 40 + col;
      SDL_Rect dst{.x = 8 * col, .y = 8 * row, .w = 8, .h = 8};
      SDL_RenderCopy(renderer, characters[screen_mem[screen_addr]], NULL, &dst);
    }
  }

  SDL_RenderPresent(renderer);
}

void Chardev::render_thread_func() {
  while (!render_thread_exit) {
    handle_events();
    render();
  }
}

int Chardev::load_char_rom() {
  characters = new SDL_Texture *[256];

  std::string fname = "roms/char_rom.bin";
  if (!std::filesystem::exists(fname)) {
    std::cerr << "Chardev: Missing character ROM (" << fname << ")" << std::endl;
    return -1;
  }

  std::ifstream file(fname, std::ios::binary | std::ios::ate);

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  char_rom = new char[2048];
  file.read(char_rom, size <= 2048 ? size : 2048);

  return 0;
}

void Chardev::create_char_textures() {
  SDL_Color colors[2] = {{0, 0, 0, 255}, {255, 255, 255, 255}};
  for (int i = 0; i < 256; i++) {
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(char_rom + 8 * i, 8, 8, 1, 1, SDL_PIXELFORMAT_INDEX1MSB);
    SDL_SetPaletteColors(surface->format->palette, colors, 0, 2);
    characters[i] = SDL_CreateTextureFromSurface(renderer, surface);
    delete surface;
  }
}
