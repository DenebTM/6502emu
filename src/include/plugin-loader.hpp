#pragma once

#include "mem.hpp"
#include "plugin-callback.hpp"

typedef int (*plugin_load_t)(plugin_callback_t);
typedef int (*plugin_init_t)(AddressSpace &, Word);
typedef int (*plugin_destroy_t)(void);
typedef int (*plugin_update_t)(unsigned int cycles_elapsed);

extern std::vector<std::tuple<plugin_init_t, Word>> plugin_init_funcs;
extern std::vector<plugin_destroy_t> plugin_destroy_funcs;
extern std::vector<plugin_update_t> plugin_update_funcs;

void load_plugins();
void init_plugins();
void destroy_plugins();
