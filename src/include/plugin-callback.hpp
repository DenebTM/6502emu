#pragma once
enum PluginCallbackType { EMU_EXIT, CPU_INTERRUPT };

typedef void (*plugin_callback_t)(PluginCallbackType, void *);
