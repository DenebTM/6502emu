#pragma once

#include "emu-common.hpp"

extern QWord cycle_current_period;
extern QWord cycle_full;

void step_cycle();
inline void step_cycle(unsigned int count) {
  for (unsigned int i = 0; i < count; i++)
    step_cycle();
}
