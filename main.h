#pragma once
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
// For delays
#include <chrono>
#include <thread>
// To catch SIGINT
#include <signal.h>
// Emulator I/O
#include <ncurses.h>

#include "common.h"
#include "cpu.h"
#include "emu-stdio.h"
#include "mem.h"

void signal_callback_handler(int signum);
std::list<ROM *> load_roms();
int main(void);
