#include <dlfcn.h>
#include <filesystem>
#include <string>

#include "emu-config.hpp"
#include "emu-types.hpp"
#include "mem.hpp"
#include "plugin-loader.hpp"

extern AddressSpace add_spc;
extern void plugin_callback_handler(PluginCallbackType type, void *arg);

std::vector<std::tuple<plugin_init_t, Word>> plugin_init_funcs;
std::vector<plugin_destroy_t> plugin_destroy_funcs;
std::vector<plugin_update_t> plugin_update_funcs;

std::optional<std::tuple<plugin_init_t, plugin_destroy_t, plugin_update_t>> load_plugin(std::filesystem::path path) {
  std::string loaded_filename = path.filename().string();

  void *plugin = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
  if (!plugin) {
    std::cerr << dlerror() << std::endl;
    return std::nullopt;
  }

  auto plugin_load_func = (plugin_load_t)dlsym(plugin, "plugin_load");
  if (plugin_load_func && plugin_load_func(&plugin_callback_handler) == -1) {
    std::cerr << "Plugin " << loaded_filename << " failed to load." << std::endl;
    return std::nullopt;
  }

  auto plugin_init_func = (plugin_init_t)dlsym(plugin, "plugin_init");
  if (!plugin_init_func) {
    std::cerr << dlerror() << std::endl;
    return std::nullopt;
  }

  auto plugin_destroy_func = (plugin_destroy_t)dlsym(plugin, "plugin_destroy");
  if (!plugin_destroy_func) {
    std::cerr << dlerror() << std::endl;
    return std::nullopt;
  }

  auto plugin_update_func = (plugin_update_t)dlsym(plugin, "plugin_update");

  return std::make_optional<std::tuple<plugin_init_t, plugin_destroy_t, plugin_update_t>>(
      {plugin_init_func, plugin_destroy_func, plugin_update_func});
}

void load_configured_plugins() {
  if (!std::filesystem::exists(PLUGIN_PATH)) {
    std::cerr << "Could not find plugin directory (" << PLUGIN_PATH << ")" << std::endl;
    return;
  }

  // load all plugins found in ./plugins directory
  // note: certain plugins may need to load ahead of other ones, so this can currently cause some issues
  if (config->enumerate_plugins) {
    for (auto &entry : std::filesystem::directory_iterator(PLUGIN_PATH, {})) {
      if ((entry.is_regular_file() || entry.is_symlink()) && entry.path().extension().string() == ".so") {
        std::string loaded_filename = entry.path().filename().string();

        bool has_config = false;
        Word plugin_addr = 0;
        bool plugin_disable = false;
        for (auto [filename, start_addr, disable] : config->plugin_configs) {
          if (loaded_filename == filename) {
            has_config = true;
            plugin_addr = start_addr;
            plugin_disable = disable;
            break;
          }
        }

        if ((!config->enumerate_plugins && !has_config) || plugin_disable)
          continue;

        auto loaded_plugin = load_plugin(entry.path());
        if (loaded_plugin.has_value()) {
          auto [init, destroy, update] = loaded_plugin.value();
          plugin_init_funcs.push_back({init, plugin_addr});
          plugin_destroy_funcs.push_back(destroy);
          if (update) {
            plugin_update_funcs.push_back(update);
          }
        }
      }
    }
  } else {
    for (auto [filename, plugin_addr, disable] : config->plugin_configs) {
      auto loaded_plugin = load_plugin(PLUGIN_PATH + filename);
      if (loaded_plugin.has_value()) {
        auto [init, destroy, update] = loaded_plugin.value();
        plugin_init_funcs.push_back({init, plugin_addr});
        plugin_destroy_funcs.push_back(destroy);
        if (update) {
          plugin_update_funcs.push_back(update);
        }
      }
    }
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
