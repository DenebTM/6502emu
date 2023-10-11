#include <SDL2/SDL.h>
#include <chrono>
#include <filesystem>
#include <fstream>
using namespace std::chrono_literals;

#include "chardev.hpp"
#include "keyboard.hpp"
#include "plugin-callback.hpp"
#include "plugins/6520-pia.hpp"
#include "plugins/6522-via.hpp"

#define COL_WIDTH 8
#define ROW_HEIGHT 8
#define COLS 40
#define ROWS 25

#define RENDER_SCALE 2.
constexpr int RENDER_WIDTH = COL_WIDTH * COLS * RENDER_SCALE;
constexpr int RENDER_HEIGHT = ROW_HEIGHT * ROWS * RENDER_SCALE;

extern plugin_callback_t plugin_callback;

extern Pia *pia1;
extern Via *via;

Chardev::Chardev() : MemoryMappedDevice(false, 1024) {
  screen_mem = new Byte[1024];
  mapped_regs = screen_mem;

  if (load_char_rom() == 0)
    load_success = true;
}

Chardev::~Chardev() {
  if (sdl_initialized) {
    sdl_thread_exit = true;
    if (sdl_thread.joinable())
      sdl_thread.join();
  }

  if (characters) {
    delete[] characters;
  }

  if (char_rom) {
    delete[] char_rom;
  }
}

int Chardev::sdl_init() {
  std::promise<int> sdl_init_promise;
  auto sdl_init_future = sdl_init_promise.get_future();

  sdl_thread = std::thread(&Chardev::sdl_thread_fn, this, std::move(sdl_init_promise));

  sdl_init_future.wait();
  auto sdl_init_return = sdl_init_future.get();

  sdl_initialized = (sdl_init_return == 0);
  return sdl_init_return;
}

void Chardev::sdl_handle_events() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        plugin_callback(EMU_EXIT, (void *)0);
        break;

      case SDL_KEYDOWN:
        if (!event.key.repeat)
          handle_key_down(event.key.keysym);
        break;

      case SDL_KEYUP:
        handle_key_up(event.key.keysym);
        break;
    }
  }
}

void Chardev::sdl_render() {
  // assume 60Hz refresh rate for simplicity
  static constexpr auto frame_time = 1s / 60.0;

  // from http://www.zimmers.net/anonftp/pub/cbm/schematics/computers/pet/2001/video-1.gif (graphic 2) and
  // http://www.zimmers.net/anonftp/pub/cbm/schematics/computers/pet/2001/notes-2.gif
  static constexpr auto vblank_time = 1.5ms;
  static constexpr auto visible_time = frame_time - vblank_time;

  // vblank over, visible portion begins
  auto frame_start = std::chrono::system_clock::now();
  if (pia1) {
    pia1->set_cb1(1);
  }
  if (via) {
    via->mapped_regs[Via::PortB] |= 0b00100000;
  }

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

  // draw screen, simulate that this takes until vblank begins
  SDL_RenderPresent(renderer);
  std::this_thread::sleep_until(frame_start + visible_time);

  // visible portion over, vblank begins
  if (pia1) {
    pia1->set_cb1(0);
  }
  if (via) {
    via->mapped_regs[Via::PortB] &= ~0b00100000;
  }

  std::this_thread::sleep_until(frame_start + frame_time);
}

void Chardev::sdl_thread_fn(std::promise<int> &&ret) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "Failed to initialize SDL2" << std::endl;
    ret.set_value(-1);
    return;
  }

  SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");

  window = SDL_CreateWindow("Chardev", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, RENDER_WIDTH, RENDER_HEIGHT, 0);
  if (!window) {
    std::cerr << "Failed to create SDL2 window" << std::endl;
    ret.set_value(-2);
    return;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    std::cerr << "Failed to create SDL2 renderer" << std::endl;
    ret.set_value(-3);
    return;
  }

  ret.set_value(0);

  create_char_textures();

  SDL_RenderSetIntegerScale(renderer, SDL_TRUE);
  SDL_RenderSetScale(renderer, RENDER_SCALE, RENDER_SCALE);

  while (!sdl_thread_exit) {
    sdl_handle_events();
    sdl_render();
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
}

int Chardev::load_char_rom() {
  std::string filename = "roms/char_rom.bin";
  if (!std::filesystem::exists(filename)) {
    std::cerr << "Chardev: Missing character ROM (" << filename << ")" << std::endl;
    return -1;
  }

  std::ifstream file(filename, std::ios::binary | std::ios::ate);

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  char_rom = new char[2048];
  file.read(char_rom, size <= 2048 ? size : 2048);

  return 0;
}

void Chardev::create_char_textures() {
  characters = new SDL_Texture *[512];

  SDL_Color colors[2] = {{0, 0, 0, 255}, {255, 255, 255, 255}};
  SDL_Color colors_inv[2] = {{255, 255, 255, 255}, {0, 0, 0, 255}};
  for (int i = 0; i < 128; i++) {
    // character set 1
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(char_rom + 8 * i, 8, 8, 1, 1, SDL_PIXELFORMAT_INDEX1MSB);

    SDL_SetPaletteColors(surface->format->palette, colors, 0, 2);
    characters[i] = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetPaletteColors(surface->format->palette, colors_inv, 0, 2);
    characters[i + 128] = SDL_CreateTextureFromSurface(renderer, surface);

    delete surface;

    // character set 2
    surface = SDL_CreateRGBSurfaceWithFormatFrom(char_rom + 8 * (i + 128), 8, 8, 1, 1, SDL_PIXELFORMAT_INDEX1MSB);

    SDL_SetPaletteColors(surface->format->palette, colors, 0, 2);
    characters[i + 256] = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetPaletteColors(surface->format->palette, colors_inv, 0, 2);
    characters[i + 256 + 128] = SDL_CreateTextureFromSurface(renderer, surface);

    delete surface;
  }
}
