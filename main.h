#pragma once
#include <iostream>
#include <fstream>
#include <list>
#include <limits>
#include <signal.h>
#include <ncurses.h>

#include "common.h"
#include "mem.h"
#include "cpu.h"
#include "emu-stdio.h"

void signal_callback_handler(int signum);
std::list<ROM> load_roms();
int main(void);