#pragma once

namespace plugin_callbacks {

void emu_exit(int status = 0);

void assert_interrupt(bool nmi = false);

} // namespace plugin_callbacks
