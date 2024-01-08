#include <SDL2/SDL.h>

#include "framebuf.hpp"
#include "plugin-callback.hpp"
#include "plugins/6520-pia.hpp"
#include "plugins/6522-via.hpp"

#include "imgui.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define MAP_SIZE 4096
#define BANK_SIZE 3840

extern plugin_callback_t plugin_callback;

extern Pia *pia1;
extern Via *via;

bool render_update;

Framebuf::Framebuf() : MemoryMappedDevice(false, MAP_SIZE) {
  screen_mem = new Byte[SCREEN_WIDTH * SCREEN_HEIGHT / 8]();
}

Framebuf::~Framebuf() {}

Byte Framebuf::read(Word offset) {
  if (offset >= BANK_SIZE)
    return active_bank;

  return screen_mem[active_bank * BANK_SIZE + offset];
}

Byte Framebuf::write(Word offset, Byte val) {
  if (offset >= BANK_SIZE)
    return active_bank = val;

  return screen_mem[active_bank * BANK_SIZE + offset] = val;
}

int Framebuf::sdl_init(SDL_Renderer *renderer) {
  this->renderer = renderer;

  render_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, //
                                 SCREEN_WIDTH, SCREEN_HEIGHT);

  if (!render_tex) {
    std::cerr << "Framebuf: Error creating render texture: " << SDL_GetError() << std::endl;
    return -1;
  }

  return 0;
}

SDL_Surface *Framebuf::create_surface() {
  SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(screen_mem, SCREEN_WIDTH, SCREEN_HEIGHT, 1,
                                                            SCREEN_WIDTH / 8, SDL_PIXELFORMAT_INDEX1MSB);
  static SDL_Color colors[2] = {{0, 0, 0, 255}, {255, 255, 255, 255}};
  SDL_SetPaletteColors(surface->format->palette, colors, 0, 2);

  return surface;
}

void Framebuf::ui_render() {
  static SDL_Surface *surface = create_surface();
  static SDL_Texture *texture;

  SDL_DestroyTexture(texture);
  texture = SDL_CreateTextureFromSurface(renderer, surface);

  // draw screen into an ImGui window
  {
    ImGui::Begin("Framebuffer", nullptr, ImGuiWindowFlags_NoResize);

    ImGui::Image(texture, ImVec2(SCREEN_WIDTH, SCREEN_HEIGHT));

    ImGui::End();
  }
}
