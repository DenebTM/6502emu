#include <atomic>
#include <thread>
using namespace std::chrono_literals;

#include "emu-config.hpp"
#include "framebuf.hpp"
#include "mem-dev.hpp"
#include "mem.hpp"
#include "plugin-callback.hpp"
#include "plugins/plugin-types.hpp"

plugin_callback_t plugin_callback;
uint64_t system_clock_speed = 1000000;

Framebuf *framebuf;

AddressSpace *_add_spc;
Word _addr;

std::thread *render_thread;
std::atomic_bool render_thread_running = true;

extern "C" EXPORT int plugin_load(plugin_callback_t callback) {
  plugin_callback = callback;

  framebuf = new Framebuf();

  return 0;
}

extern "C" EXPORT int plugin_init(AddressSpace &add_spc, Word addr, EmuConfig *config) {
  system_clock_speed = config->clock_speed;
  _add_spc = &add_spc;
  _addr = addr ? addr : 0x9000;

  _add_spc->map_mem(framebuf, _addr);

  return 0;
}

extern "C" EXPORT int plugin_destroy() {
  if (framebuf) {
    _add_spc->unmap_mem(_addr);
    auto _framebuf = framebuf;
    framebuf = nullptr;
    delete _framebuf;
  }

  return 0;
}

extern "C" EXPORT int plugin_ui_render(SDL_Renderer *renderer) {
  static bool sdl_initialized = false;
  static bool sdl_init_failed = false;

  if (sdl_init_failed)
    return -1;

  if (!sdl_initialized) {
    sdl_initialized = true;

    if (framebuf->sdl_init(renderer) != 0)
      sdl_init_failed = true;
  }

  if (framebuf) {
    framebuf->ui_render();
  }

  return 0;
}
