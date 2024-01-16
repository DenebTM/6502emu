#pragma once
#include <functional>
#include <string>

namespace plugin_callbacks {

void emu_exit(int status = 0);

void assert_interrupt(bool nmi = false);

void choose_file(std::function<void(std::string)> *func);

} // namespace plugin_callbacks
