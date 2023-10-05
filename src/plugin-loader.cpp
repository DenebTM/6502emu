#include <dlfcn.h>
#include <filesystem>
#include <string>

#include "emu-common.hpp"
#include "emu-config.hpp"
#include "mem.hpp"
#include "plugin-loader.hpp"

extern AddressSpace add_spc;
extern void plugin_callback_handler(PluginCallbackType type, void *arg);

std::vector<std::tuple<plugin_init_t, Word>> plugin_init_funcs;
std::vector<plugin_destroy_t> plugin_destroy_funcs;
std::vector<plugin_update_t> plugin_update_funcs;

void load_plugins() {
  std::string plugin_path = "./plugins";
  if (!std::filesystem::exists(plugin_path))
    return;

  for (auto &entry : std::filesystem::directory_iterator(plugin_path, {})) {
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

      void *plugin = dlopen(entry.path().c_str(), RTLD_NOW | RTLD_GLOBAL);
      if (!plugin) {
        std::cerr << dlerror() << std::endl;
        break;
      }

      auto plugin_load_func = (plugin_load_t)dlsym(plugin, "plugin_load");
      if (plugin_load_func && plugin_load_func(&plugin_callback_handler) == -1) {
        std::cerr << "Plugin " << loaded_filename << " failed to load." << std::endl;
        continue;
      }

      auto plugin_init_func = (plugin_init_t)dlsym(plugin, "plugin_init");
      if (!plugin_init_func) {
        std::cerr << dlerror() << std::endl;
        continue;
      }
      plugin_init_funcs.push_back({plugin_init_func, plugin_addr});

      auto plugin_destroy_func = (plugin_destroy_t)dlsym(plugin, "plugin_destroy");
      if (!plugin_destroy_func) {
        std::cerr << dlerror() << std::endl;
        continue;
      }
      plugin_destroy_funcs.push_back(plugin_destroy_func);

      auto plugin_update_func = (plugin_update_t)dlsym(plugin, "plugin_update");
      if (plugin_update_func)
        plugin_update_funcs.push_back(plugin_update_func);
    }
  }
}

void init_plugins() {
  for (auto [plugin_init_func, plugin_addr] : plugin_init_funcs)
    plugin_init_func(add_spc, plugin_addr);
}

void destroy_plugins() {
  for (auto plugin_destroy_func : plugin_destroy_funcs)
    plugin_destroy_func();
}
