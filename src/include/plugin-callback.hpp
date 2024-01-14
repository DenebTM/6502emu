#pragma once

enum PluginCallbackType { EMU_EXIT, CPU_INTERRUPT, CHOOSE_FILE };

typedef void (*plugin_callback_t)(PluginCallbackType, void *);
