#include <algorithm>
#include <dlfcn.h>
#include <filesystem>
#include <string>

#include "emu-config.hpp"
#include "emu-types.hpp"
#include "mem.hpp"
#include "plugin-loader.hpp"

extern AddressSpace add_spc;
extern void plugin_callback_handler(PluginCallbackType type, void *arg);

// FIXME: consolidate these
std::vector<std::string> plugin_filenames;
std::vector<std::tuple<plugin_init_t, Word>> plugin_init_funcs;
std::vector<plugin_destroy_t> plugin_destroy_funcs;
std::vector<plugin_update_t> plugin_update_funcs;

void load_plugin(std::filesystem::path path, Word map_addr) {
  std::string loaded_filename = path.filename().string();

  // load plugin into base namespace by default
  Lmid_t lmid = LM_ID_BASE;
  auto mode = RTLD_GLOBAL | RTLD_NOW;
  // load plugin into new namespace if already loaded
  if (std::find(plugin_filenames.begin(), plugin_filenames.end(), loaded_filename) != plugin_filenames.end()) {
    lmid = LM_ID_NEWLM;
    mode = RTLD_NOW;
  }

  // attempt to open the plugin
  void *plugin = dlmopen(lmid, path.c_str(), mode);
  if (!plugin) {
    std::cerr << dlerror() << std::endl;
    return;
  }
  plugin_filenames.push_back(loaded_filename);

  // find and call the plugin's load function if present
  auto plugin_load_func = (plugin_load_t)dlsym(plugin, "plugin_load");
  if (plugin_load_func && plugin_load_func(&plugin_callback_handler) == -1) {
    std::cerr << "Plugin " << loaded_filename << " failed to load." << std::endl;
    dlclose(plugin);
    return;
  }

  auto plugin_init_func = (plugin_init_t)dlsym(plugin, "plugin_init");
  if (!plugin_init_func) {
    std::cerr << dlerror() << std::endl;
    dlclose(plugin);
    return;
  }
  plugin_init_funcs.push_back({plugin_init_func, map_addr});

  auto plugin_destroy_func = (plugin_destroy_t)dlsym(plugin, "plugin_destroy");
  if (!plugin_destroy_func) {
    std::cerr << dlerror() << std::endl;
    dlclose(plugin);
    return;
  }
  plugin_destroy_funcs.push_back(plugin_destroy_func);

  auto plugin_update_func = (plugin_update_t)dlsym(plugin, "plugin_update");
  if (plugin_update_func)
    plugin_update_funcs.push_back(plugin_update_func);
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
        std::string loaded_filename = entry.path().filename().string();

        // find configuration parameters for plugin, if present
        Word plugin_addr = 0;
        bool plugin_disable = false;
        for (auto [filename, start_addr, disable] : config->plugin_configs) {
          if (loaded_filename == filename) {
            plugin_addr = start_addr;
            plugin_disable = disable;
            break;
          }
        }

        if (plugin_disable)
          continue;

        load_plugin(entry.path(), plugin_addr);
      }
    }
  }

  // load all plugins listed in config in order of appearance
  else {
    for (auto [filename, plugin_addr, disable] : config->plugin_configs)
      load_plugin(PLUGIN_PATH + filename, plugin_addr);
  }
}

void init_plugins() {
  for (auto [plugin_init, addr] : plugin_init_funcs)
    plugin_init(add_spc, addr);
}

void destroy_plugins() {
  for (auto plugin_destroy : plugin_destroy_funcs)
    plugin_destroy();
}
