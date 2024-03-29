#include <SDL2/SDL_events.h>
#include <future>
#include <optional>
#include <thread>
using namespace std::chrono_literals;

#include "chardev.hpp"
#include "emu-config.hpp"
#include "keyboard.hpp"
#include "mem-dev.hpp"
#include "mem.hpp"
#include "plugins/6520-pia.hpp"
#include "plugins/6522-via.hpp"
#include "plugins/plugin-types.hpp"

uint64_t system_clock_speed = 1000000;

Chardev *chardev;
Pia *pia1;
Via *via;

AddressSpace *_add_spc;
Word _addr;

extern "C" EXPORT int plugin_load() {
  chardev = new Chardev();
  if (!chardev->load_success) {
    delete chardev;
    return -1;
  }

  return 0;
}

extern "C" EXPORT int plugin_init(AddressSpace &add_spc, Word addr, EmuConfig *config) {
  system_clock_speed = config->clock_speed;
  _add_spc = &add_spc;
  _addr = addr ? addr : 0x8000;

  // 40-column screen memory is mirrored four times
  _add_spc->map_mem(chardev, _addr + 0x000);
  _add_spc->map_mem(chardev, _addr + 0x400);
  _add_spc->map_mem(chardev, _addr + 0x800);
  _add_spc->map_mem(chardev, _addr + 0xc00);

  /**
   * PIA must be present in order for vblank interrupts and the keyboard to work
   * this plugin may be initialized before PIA1, therefore wait for it asynchronously and return early
   * so as not to block _its_ initialization
   */
  static std::future<void> wait_for_pia1 = std::async(std::launch::async, [] {
    std::optional<MemoryMappedDevice *> dev_pia1 = std::nullopt;
    do {
      dev_pia1 = _add_spc->get_dev(0xe810);
      std::this_thread::sleep_for(10ms);
    } while (!dev_pia1.has_value());

    pia1 = dynamic_cast<Pia *>(dev_pia1.value());

    // temporary: set all Port A input bits to 1
    pia1->mapped_regs[Pia::ORA] = 0xff;

    pia1->on_write_port_a = [](Byte val) { set_kb_row(val & 0x0f); };
    pia1->read_port_b = get_kb_row_contents;
  });

  // same for VIA
  static std::future<void> wait_for_via = std::async(std::launch::async, [] {
    std::optional<MemoryMappedDevice *> dev_via = std::nullopt;
    do {
      dev_via = _add_spc->get_dev(0xe840);
      std::this_thread::sleep_for(10ms);
    } while (!dev_via.has_value());

    via = dynamic_cast<Via *>(dev_via.value());
  });

  return 0;
}

extern "C" EXPORT int plugin_destroy() {
  if (chardev) {
    _add_spc->unmap_mem(_addr + 0x000);
    _add_spc->unmap_mem(_addr + 0x400);
    _add_spc->unmap_mem(_addr + 0x800);
    _add_spc->unmap_mem(_addr + 0xc00);
    auto _chardev = chardev;
    chardev = nullptr;
    delete _chardev;
  }

  return 0;
}

extern "C" EXPORT int plugin_update() {
  if (chardev) {
    chardev->update();
  }

  return 0;
}

extern "C" EXPORT int plugin_ui_event(SDL_Event &event) {
  if (chardev) {
    chardev->ui_handle_event(event);
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

    if (chardev->sdl_init(renderer) != 0)
      sdl_init_failed = true;
  }

  if (chardev) {
    chardev->ui_render();
  }

  return 0;
}
