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
  AddressSpace() : AddressSpace(0x10000, std::vector<ROM>()) {}
  AddressSpace(DWord mSize) : AddressSpace(mSize, std::vector<ROM>()) {}
  AddressSpace(DWord mSize, std::vector<ROM> roms) {
    mem_size = mSize;
    memory = new Byte[mem_size];
    mem_info = new AddressInfo[mem_size];
    clear_ram();
    map_roms(roms);
  }

  ~AddressSpace() {
    delete[] mem_info;
    delete[] memory;
  };

  Byte read(DWord addr) {
    // Wrap around to beginning of memory space if out of range.
    addr %= mem_size;

    AddressInfo info = mem_info[addr];
    if (info.dev.has_value()) {
      auto [dev, dev_idx] = info.dev.value();
      dev->pre_read(dev_idx);
      return dev->read(dev_idx);
    }

    return memory[addr];
  }
  Word read_word(DWord addr_lo) { return read_word(addr_lo, false); }
  Word read_word(DWord addr_lo, bool wrap_page) {
    auto addr_hi = addr_lo + 1;
    if (wrap_page) {
      addr_hi &= 0x00ff;
      addr_hi |= addr_lo & 0xff00;
    }

    return (Word)read(addr_lo) + ((Word)read(addr_hi) << 8);
  }

  void write(DWord addr, Byte val) {
    AddressInfo info = mem_info[addr];
    if (info.dev.has_value()) {
      auto [dev, dev_idx] = info.dev.value();
      if (!info.read_only)
        dev->write(dev_idx, val);
      dev->post_write(dev_idx);
    }

    else if (!info.read_only) {
      memory[addr] = val;
    }
  }

  void write_word(DWord addr_lo, Word val) { write_word(addr_lo, val, false); }

  void write_word(DWord addr_lo, Word val, bool wrap_page) {
    auto addr_hi = addr_lo + 1;
    if (wrap_page) {
      addr_hi &= 0x00ff;
      addr_hi |= addr_lo & 0xff00;
    }

    write(addr_lo, val);
    write(addr_hi, val >> 8);
  }

  void clear_ram() {
    for (size_t i = 0; i < mem_size; i++)
      memory[i] = 0;
  }

  void map_roms(std::vector<ROM> roms) {
    for (ROM r : roms)
      map_mem(r);
  }
  void map_mem(ROM rom) { map_mem((Byte *)rom.content, rom.size, rom.start_addr, rom.read_only); }

  void map_mem(MemoryMappedDevice *dev, DWord start_addr) {
    for (size_t i = 0; i < dev->mapped_regs_count && (start_addr + i) < mem_size; i++) {
      mem_info[start_addr + i].dev.emplace<std::pair<MemoryMappedDevice *, size_t>>({dev, i});
      mem_info[start_addr + i].read_only = dev->read_only;
    }
  }
  void map_mem(const Byte *bytes, DWord size, DWord start_addr) { map_mem((Byte *)bytes, size, start_addr, true); }
  void map_mem(Byte *bytes, DWord size, DWord start_addr) { map_mem(bytes, size, start_addr, false); }
  void map_mem(Byte *bytes, DWord size, DWord start_addr, bool read_only) {
    for (size_t i = 0; i < size && (start_addr + i) < mem_size; i++) {
      memory[start_addr + i] = bytes[i];
      mem_info[start_addr + i].read_only = read_only;
    }
  }

  void unmap_mem(DWord size, DWord start_addr) {
    for (size_t i = 0; i < size && (start_addr + i) < mem_size; i++)
      mem_info[i].dev.reset();
  }

  std::optional<MemoryMappedDevice *> get_dev(DWord addr) {
    auto dev = mem_info[addr].dev;
    if (dev.has_value()) {
      return std::get<MemoryMappedDevice *>(dev.value());
    }

    return std::nullopt;
  }

private:
  DWord mem_size;

  Byte *memory;

  struct AddressInfo {
    std::optional<std::tuple<MemoryMappedDevice *, size_t>> dev = std::nullopt;
    bool read_only = false;
  } *mem_info;
};
