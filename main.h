#pragma once
//#ifndef MAIN_H
//#define MAIN_H
//#ifndef BASICLIBS
//#define BASICLIBS
#include <iostream>
#include <fstream>
#include <list>
#include <limits>
#include <signal.h>

#include "mem.h"
#include "cpu.h"
//#endif

void signal_callback_handler(int signum);
std::list<ROM> load_roms();
int main(void);
//#endif