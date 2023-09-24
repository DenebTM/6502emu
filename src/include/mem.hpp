#pragma once
#include <iostream>
#include <optional>
#include <tuple>
#include <vector>

#include "emu-common.hpp"
#include "mem-dev.hpp"
#include "rom.hpp"

class AddressSpace {
public:
  AddressSpace();
  AddressSpace(DWord mSize);
  AddressSpace(DWord mSize, std::vector<ROM> roms);

  ~AddressSpace();

  Byte read(DWord addr);
  Word read_word(DWord addr_lo);
  Word read_word(DWord addr_lo, bool wrap_page);

  void write(DWord addr, Byte val);
  void write_word(DWord addr_lo, Word val);
  void write_word(DWord addr_lo, Word val, bool wrap_page);

  void clear_ram();
  void map_roms(std::vector<ROM> roms);
  void map_mem(ROM rom);
  void map_mem(MemoryMappedDevice *dev, DWord addr);
  void map_mem(const Byte *bytes, DWord size, DWord start_addr);
  void map_mem(Byte *bytes, DWord size, DWord start_addr);
  void map_mem(Byte *bytes, DWord size, DWord start_addr, bool read_only);
  void unmap_mem(DWord size, DWord start_addr);

private:
  DWord mem_size;
  // DWord last_addr = -1;

  Byte *memory;

  struct AddressInfo {
    std::optional<std::pair<MemoryMappedDevice *, size_t>> dev = std::nullopt;
    bool read_only = false;
  } *mem_info;
};
