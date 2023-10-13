#pragma once
#include <SDL2/SDL.h>
#include <filesystem>
#include <tuple>

#include "emu-config.hpp"
#include "mem.hpp"
#include "plugin-callback.hpp"

#define PLUGIN_PATH std::string("./plugins/")

/**
 * extern "C" int plugin_load - OPTIONAL
 *
 * a plugin may provide a function to be called immediately upon load, e.g.
 * to load data from another file (and abort early if that fails).
 *
 * should plugin_callback be required by the plugin, that is passed here.
 *
 * @return 0: success; non-zero: failure
 */
typedef int (*plugin_load_t)(plugin_callback_t);

/**
 * extern "C" int plugin_init - REQUIRED
 *
 * a plugin must provide a function to set itself up before emulation starts,
 * e.g. to map a device into memory. for this purpose, a reference to the
 * address space and a load address are passed in.
 *
 * @return ignored
 */
typedef int (*plugin_init_t)(AddressSpace &, Word, EmuConfig *);

/**
 * extern "C" int plugin_destroy - REQUIRED
 *
 * a plugin must provide a clean-up function to be called after emulation is
 * terminated, e.g. to deallocate memory.
 *
 * @return ignored
 */
typedef int (*plugin_destroy_t)(void);

/**
 * extern "C" int plugin_update - OPTIONAL
 *
 * a plugin may provide a function to be called after each cycle of the system
 * clock, e.g. to update internal timers.
 *
 * @return ignored
 */
typedef int (*plugin_update_t)(void);

/**
 * extern "C" int plugin_ui_event - OPTIONAL
 * extern "C" int plugin_ui_render - OPTIONAL
 *
 * a plugin may provide functions to display an ImGui window and handle SDL events
 *
 * @return ignored
 */
typedef int (*plugin_ui_event_t)(SDL_Event &);
typedef int (*plugin_ui_render_t)(SDL_Renderer *);

extern std::vector<std::string> plugin_filenames;
extern std::vector<std::tuple<plugin_init_t, Word>> plugin_init_funcs;
extern std::vector<plugin_destroy_t> plugin_destroy_funcs;
extern std::vector<plugin_update_t> plugin_update_funcs;
extern std::vector<plugin_ui_event_t> plugin_ui_event_funcs;
extern std::vector<plugin_ui_render_t> plugin_ui_render_funcs;

/**
 * load a plugin from a shared object file
 *
 * any given plugin may be loaded multiple times.
 * the first plugin is loaded into the base namespace so that its exported
 * symbols can be used by other plugins, each subsequently loaded instance is
 * given its own namespace.
 *
 * note: at most 16 duplicate plugins can be loaded.
 *
 * @param path filesystem location of shared object to be loaded
 * @param map_addr location in the address space to map plugin at
 */
void load_plugin(std::filesystem::path path, Word map_addr);

void load_configured_plugins();
void init_plugins();
void destroy_plugins();
