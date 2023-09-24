#include <filesystem>
#include <fstream>

#include "pia1.hpp"

Pia1::Pia1() : MemoryMappedDevice(true, 16) { mapped_regs[3] = 0x80; }

int Pia1::pre_read() { return 0; }

int Pia1::post_write() { return 0; }
