#pragma once
#include <iostream>
#include <fstream>
#include <list>
#include <limits>
// For delays
#include <chrono>
#include <thread>
// To catch SIGINT
#include <signal.h>
// Emulator I/O
#include <ncurses.h>

#include "common.h"
#include "mem.h"
#include "cpu.h"
#include "emu-stdio.h"

void signal_callback_handler(int signum);
std::list<ROM*> load_roms();
int main(void);