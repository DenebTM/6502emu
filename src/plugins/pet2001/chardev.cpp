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

#include "imgui.h"

#define COL_WIDTH 8
#define ROW_HEIGHT 8
#define COLS 40
#define ROWS 25

#define RENDER_SCALE 2.
constexpr int SCREEN_WIDTH = COL_WIDTH * COLS;
constexpr int SCREEN_HEIGHT = ROW_HEIGHT * ROWS;

extern plugin_callback_t plugin_callback;
extern uint64_t system_clock_speed;

extern Pia *pia1;
extern Via *via;

std::condition_variable sdl_wake_cond;
std::mutex sdl_wake_mutex;

Chardev::Chardev() : MemoryMappedDevice(false, 1024) {
  screen_mem = mapped_regs;

  if (load_char_rom() == 0)
    load_success = true;
}

Chardev::~Chardev() {
  // allow ongoing render to complete
  sdl_wake_cond.notify_all();

  if (characters) {
    delete[] characters;
  }

  if (char_rom) {
    delete[] char_rom;
  }
}

/**
 * this function ties the SDL thread to the system clock - every ~16.7ms
 * a condition variable is used to signal the SDL thread to wake up
 *
 * additionally signals vblank to the system for 1.5ms after each frame
 */
void Chardev::update() {
  // 60Hz refresh rate at full clock speed
  static constexpr auto frame_time = 1s / 60.0;

  // from http://www.zimmers.net/anonftp/pub/cbm/schematics/computers/pet/2001/video-1.gif (graphic 2) and
  // http://www.zimmers.net/anonftp/pub/cbm/schematics/computers/pet/2001/notes-2.gif
  static constexpr auto vblank_time = 1.5ms;
  static constexpr auto visible_time = frame_time - vblank_time;

  static const unsigned cycles_frame = frame_time * system_clock_speed / 1s;
  static const unsigned cycles_visible = (visible_time / frame_time) * cycles_frame;

  static QWord thisframe_cycle = 0;
  static bool frame_vblank_done = false;

  thisframe_cycle++;
  if (thisframe_cycle > cycles_visible && !frame_vblank_done) {
    // visible portion over, vblank begins
    if (pia1) {
      pia1->set_cb1(0);
    }
    if (via) {
      via->mapped_regs[Via::PortB] &= ~BIT5;
    }

    frame_vblank_done = true;
  } else if (thisframe_cycle > cycles_frame) {
    // vblank over, visible portion begins
    if (pia1) {
      pia1->set_cb1(1);
    }
    if (via) {
      via->mapped_regs[Via::PortB] |= BIT5;
    }

    // use a condition variable to wake the SDL thread
    sdl_wake_cond.notify_all();

    // begin next frame
    thisframe_cycle = 0;
    frame_vblank_done = false;
  }
}

int Chardev::sdl_init(SDL_Renderer *renderer) {
  this->renderer = renderer;
  create_char_textures();

  render_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, //
                                 SCREEN_WIDTH, SCREEN_HEIGHT);

  if (!render_tex) {
    std::cerr << "Chardev: Error creating render texture: " << SDL_GetError() << std::endl;
    return -1;
  }

  return 0;
}

static bool imgui_window_focused = false;
void Chardev::ui_handle_event(SDL_Event &event) {
  if (!imgui_window_focused)
    return;

  switch (event.type) {
    case SDL_KEYDOWN:
      if (!event.key.repeat)
        handle_key_down(event.key.keysym);
      break;

    case SDL_KEYUP:
      handle_key_up(event.key.keysym);
      break;
  }
}

void Chardev::ui_render() {
  // wait until `update` signals wake-up
  // FIXME: this blocks the UI thread
  std::unique_lock<std::mutex> sdl_wake_lock(sdl_wake_mutex);
  sdl_wake_cond.wait(sdl_wake_lock);

  // save current renderer state
  SDL_Texture *old_render_target = SDL_GetRenderTarget(renderer);
  // SDL_bool old_integer_scale = SDL_RenderGetIntegerScale(renderer);
  // float old_scale_x, old_scale_y;
  // SDL_RenderGetScale(renderer, &old_scale_x, &old_scale_y);

  // prepare renderer to draw to the texture
  SDL_SetRenderTarget(renderer, render_tex);
  // SDL_RenderSetIntegerScale(renderer, SDL_TRUE);
  // SDL_RenderSetScale(renderer, RENDER_SCALE, RENDER_SCALE);

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

  // restore old renderer state
  SDL_SetRenderTarget(renderer, old_render_target);
  // SDL_RenderSetIntegerScale(renderer, old_integer_scale);
  // SDL_RenderSetScale(renderer, old_scale_x, old_scale_y);

  // draw PET screen into an ImGui window
  {
    ImGui::Begin("PET 2001 Display", NULL, ImGuiWindowFlags_NoResize);
    imgui_window_focused = ImGui::IsWindowFocused();

    static constexpr auto IMG_WIDTH = SCREEN_WIDTH * RENDER_SCALE;
    static constexpr auto IMG_HEIGHT = SCREEN_HEIGHT * RENDER_SCALE;
    ImGui::Image((void *)render_tex, ImVec2(IMG_WIDTH, IMG_HEIGHT));

    ImGui::End();
  }
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

int Chardev::create_char_textures() {
  characters = new SDL_Texture *[512];

  SDL_Color colors[2] = {{0, 0, 0, 255}, {255, 255, 255, 255}};
  SDL_Color colors_inv[2] = {{255, 255, 255, 255}, {0, 0, 0, 255}};
  for (int i = 0; i < 128; i++) {
    // character set 1
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(char_rom + 8 * i, 8, 8, 1, 1, SDL_PIXELFORMAT_INDEX1MSB);
    if (!surface) {
      std::cerr << "Chardev: Error creating surface: " << SDL_GetError() << std::endl;
      return -1;
    }

    SDL_SetPaletteColors(surface->format->palette, colors, 0, 2);
    characters[i] = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetPaletteColors(surface->format->palette, colors_inv, 0, 2);
    characters[i + 128] = SDL_CreateTextureFromSurface(renderer, surface);

    delete surface;

    // character set 2
    surface = SDL_CreateRGBSurfaceWithFormatFrom(char_rom + 8 * (i + 128), 8, 8, 1, 1, SDL_PIXELFORMAT_INDEX1MSB);
    if (!surface) {
      std::cerr << "Chardev: Error creating surface: " << SDL_GetError() << std::endl;
      return -1;
    }

    SDL_SetPaletteColors(surface->format->palette, colors, 0, 2);
    characters[i + 256] = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetPaletteColors(surface->format->palette, colors_inv, 0, 2);
    characters[i + 256 + 128] = SDL_CreateTextureFromSurface(renderer, surface);

    delete surface;
  }

  return 0;
}
