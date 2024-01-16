#include <SDL2/SDL.h>
#include <algorithm>
#include <dlfcn.h>
#include <filesystem>
#include <string>

#include "emu-config.hpp"
#include "emu-types.hpp"
#include "mem.hpp"
#include "plugin-loader.hpp"

extern AddressSpace add_spc;

std::map<PluginID, Plugin> plugins;

bool is_dlib_loaded(std::string filename) {
  static std::vector<std::filesystem::path> loaded_libs;
  auto path = std::filesystem::path(PLUGIN_PATH + filename);

  if (std::find(loaded_libs.begin(), loaded_libs.end(), path) != loaded_libs.end())
    return true;

  loaded_libs.push_back(path);
  return false;
}

void load_plugin(PluginID id, std::string filename) {
  auto path = std::filesystem::path(filename);

  // load plugin into base namespace by default
  Lmid_t lmid = LM_ID_BASE;
  auto mode = RTLD_GLOBAL | RTLD_NOW;
  // load plugin into new namespace if loaded before
  if (is_dlib_loaded(path)) {
    lmid = LM_ID_NEWLM;
    mode = RTLD_NOW;
  }

  // attempt to open the plugin
  void *plugin_handle = dlmopen(lmid, path.c_str(), mode);
  if (!plugin_handle) {
    std::cerr << dlerror() << std::endl;
    return;
  }

  // find and call the plugin's load function if present
  auto plugin_load_func = (plugin_load_t)dlsym(plugin_handle, "plugin_load");
  if (plugin_load_func && plugin_load_func() == -1) {
    std::cerr << "Plugin " << filename << " failed to load." << std::endl;
    dlclose(plugin_handle);
    return;
  }

  auto plugin_init_func = (plugin_init_t)dlsym(plugin_handle, "plugin_init");
  if (!plugin_init_func) {
    std::cerr << dlerror() << std::endl;
    dlclose(plugin_handle);
    return;
  }

  auto plugin_destroy_func = (plugin_destroy_t)dlsym(plugin_handle, "plugin_destroy");
  if (!plugin_destroy_func) {
    std::cerr << dlerror() << std::endl;
    dlclose(plugin_handle);
    return;
  }

  auto plugin_update_func = (plugin_update_t)dlsym(plugin_handle, "plugin_update");
  auto plugin_ui_event_func = (plugin_ui_event_t)dlsym(plugin_handle, "plugin_ui_event");
  auto plugin_ui_render_func = (plugin_ui_render_t)dlsym(plugin_handle, "plugin_ui_render");

  Word map_addr = 0;
  auto maybe_plugin_config = config->get_plugin_config(id);
  if (maybe_plugin_config.has_value()) {
    map_addr = maybe_plugin_config.value().map_addr;
  }

  plugins[id] = {.filename = path,
                 .dlib_handle = plugin_handle,
                 .init = plugin_init_func,
                 .destroy = plugin_destroy_func,
                 .update = plugin_update_func,
                 .ui_event = plugin_ui_event_func,
                 .ui_render = plugin_ui_render_func,
                 .map_addr = map_addr};
}

void unload_plugin(PluginID id) {
  if (plugins.contains(id)) {
    auto plugin = plugins[id];
    plugins.erase(id);
    plugin.destroy();
    dlclose(plugin.dlib_handle);
  }
}

void load_configured_plugins() {
  if (!std::filesystem::exists(PLUGIN_PATH)) {
    std::cerr << "Could not find plugin directory (" << PLUGIN_PATH << ")" << std::endl;
    return;
  }

  // load all shared objects found in `PLUGIN_PATH` in filesystem order
  // note: some plugins may need to load ahead of others, so this can currently cause some issues
  if (config->enumerate_plugins) {
    for (auto &entry : std::filesystem::directory_iterator(PLUGIN_PATH, {})) {
      if ((entry.is_regular_file() || entry.is_symlink()) && entry.path().extension().string() == ".so") {
        std::string filename = entry.path().filename().string();

        // find configuration parameters for plugin, if present
        for (auto &plugin_config : config->plugin_configs) {
          if (filename == plugin_config.filename) {
            if (plugin_config.disable)
              break;

            load_plugin(plugin_config.id, PLUGIN_PATH + plugin_config.filename);
            break;
          }
        }
      }
    }
  }

  // load all plugins listed in config in order of appearance
  else {
    for (auto &plugin_config : config->plugin_configs) {
      if (plugin_config.disable)
        continue;

      load_plugin(plugin_config.id, PLUGIN_PATH + plugin_config.filename);
    }
  }
}

void init_plugins() {
  for (auto &[_, plugin] : plugins)
    plugin.init(add_spc, plugin.map_addr, config);
}
void destroy_plugins() {
  for (auto &[_, plugin] : plugins)
    plugin.destroy();
}
void update_plugins() {
  for (auto &[_, plugin] : plugins)
    if (plugin.update)
      plugin.update();
}
void handle_plugin_ui_events(SDL_Event &event) {
  for (auto &[_, plugin] : plugins)
    if (plugin.ui_event)
      plugin.ui_event(event);
}
void render_plugin_uis(SDL_Renderer *renderer) {
  for (auto &[_, plugin] : plugins)
    if (plugin.ui_render)
      plugin.ui_render(renderer);
}
