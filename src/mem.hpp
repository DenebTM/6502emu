#pragma once
#include "common.hpp"
#include "mem-dev.hpp"
#include "rom.hpp"
#include <iostream>
#include <list>

class AddressSpace {
public:
  AddressSpace();
  AddressSpace(DWord mSize);
  AddressSpace(DWord mSize, std::list<ROM *> roms);

  Byte *operator[](DWord i);

  Byte *get(DWord i);
  Word read_word(DWord addr);
  Word read_word(DWord addr, bool wrap_page);
  void write_word(DWord addr, Word val);
  void clear_ram();
  void map_roms(std::list<ROM *> roms);
  void map_mem(ROM *rom);
  void map_mem(MemoryMappedDevice *dev, DWord addr);
  void map_mem(const void *bytes, DWord size, DWord start_addr);
  void map_mem(void *bytes, DWord size, DWord start_addr);
  void map_mem(void *bytes, DWord size, DWord start_addr, bool mask, bool read_only);
  void unmap_mem(DWord size, DWord start_addr);

  void free();

private:
  typedef struct mem_addr {
    void *memory = NULL;
    bool is_mapped = false;
    bool read_only = false;
    SByte dev_regidx = -1;
  } MemAddr;
  DWord mem_size;
  DWord tmpval;
  Byte *ram;
  DWord last_addr = -1;

  MemAddr *mapped_mem;
  void init_ram();
};
