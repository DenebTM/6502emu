#pragma once

extern uint64_t sysclock_cycle;
extern bool sysclock_paused;

void sysclock_init(uint64_t clock_speed);

void sysclock_step();
inline void sysclock_step(unsigned int count) {
  for (unsigned int i = 0; i < count; i++)
    sysclock_step();
}

void sysclock_pause();
void sysclock_resume();
