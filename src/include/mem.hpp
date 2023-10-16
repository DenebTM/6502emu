#pragma once
#include <iostream>
#include <optional>
#include <tuple>
#include <vector>

#include "emu-types.hpp"
#include "mem-dev.hpp"

/**
 * a remappable 16-bit address space
 *
 * each address may be either read-only or  r/w
 */
class AddressSpace {
public:
  const static DWord SIZE = 0x10000;

  AddressSpace() {
    this->memory = new Byte[SIZE]();
    this->mem_info = new AddressInfo[SIZE]();
  }

  ~AddressSpace() {
    delete[] this->mem_info;
    delete[] this->memory;
  }

  /**
   * read a byte from mapped RAM, ROM or device
   */
  Byte read(Word addr) {
    AddressInfo info = mem_info[addr];

    // nothing at address
    if (!info.mapped)
      return 0xff;

    // redirect request to mapped device
    if (info.dev.has_value()) {
      auto [dev, dev_idx] = info.dev.value();
      dev->pre_read(dev_idx);
      return dev->read(dev_idx);
    }

    // read directly from RAM/ROM
    return memory[addr];
  }

  /**
   * read a word from memory (little-endian byte order)
   *
   * @param addr_lo location from which to read low byte
   * @param wrap_page when `true`, `addr_hi`'s high byte will not be incremented for the second read
   */
  Word read_word(Word addr_lo, bool wrap_page = false) {
    auto addr_hi = addr_lo + 1;
    if (wrap_page) {
      addr_hi &= 0x00ff;
      addr_hi |= addr_lo & 0xff00;
    }

    return (Word)read(addr_lo) + ((Word)read(addr_hi) << 8);
  }

  /**
   * write a byte to mapped RAM, ROM or device
   */
  void write(Word addr, Byte val) {
    AddressInfo info = mem_info[addr];

    // nothing at address
    if (!info.mapped)
      return;

    // redirect request to mapped device
    if (info.dev.has_value()) {
      auto [dev, dev_idx] = info.dev.value();
      if (!info.read_only)
        dev->write(dev_idx, val);
      dev->post_write(dev_idx);
    }

    // write directly to RAM
    else if (!info.read_only)
      memory[addr] = val;
  }

  /**
   * write a word to memory (little-endian byte order)
   *
   * @param addr_lo location at which to store low byte of `val`
   * @param wrap_page when `true`, `addr_hi`'s high byte will not be incremented for the second write
   */
  void write_word(Word addr_lo, Word val, bool wrap_page = false) {
    auto addr_hi = addr_lo + 1;
    if (wrap_page) {
      addr_hi &= 0x00ff;
      addr_hi |= addr_lo & 0xff00;
    }

    write(addr_lo, val), write(addr_hi, val >> 8);
  }

  /**
   * mark a region in the address space as mapped and read-only, optionally copy a byte array into it
   *
   * @param bytes ROM data or NULL
   */
  void map_mem(const Byte *bytes, DWord size, Word start_addr) { map_mem((Byte *)bytes, size, start_addr, true); }

  /**
   * mark a region in the address space as mapped, optionally copy a byte array into it
   *
   * @param bytes binary data or NULL
   */
  void map_mem(Byte *bytes, DWord size, Word start_addr, bool read_only = false) {
    for (size_t i = 0; i < size && (start_addr + i) < SIZE; i++) {
      mem_info[start_addr + i].mapped = true;
      mem_info[start_addr + i].read_only = read_only;

      if (bytes)
        memory[start_addr + i] = bytes[i];
    }
  }

  /**
   * map a peripheral device into the address space
   */
  void map_mem(MemoryMappedDevice *dev, Word start_addr) {
    for (size_t i = 0; i < dev->mapped_regs_count && (start_addr + i) < SIZE; i++) {
      mem_info[start_addr + i].dev.emplace<std::pair<MemoryMappedDevice *, size_t>>({dev, i});
      mem_info[start_addr + i].mapped = true;
      mem_info[start_addr + i].read_only = dev->read_only;
    }
  }

  /**
   * unmap all RAM, ROM and peripheral devices from a memory region
   *
   * if count == 0 and a device is found at the specified address,
   * the whole device is unmapped
   *
   * @param start_addr first address to unmap
   * @param count number of addresses to unmap
   */
  void unmap_mem(Word start_addr, DWord count = 0) {
    if (count == 0) {
      if (mem_info[start_addr].dev.has_value()) {
        auto &[dev, offset] = mem_info[start_addr].dev.value();

        start_addr -= offset;
        count = dev->mapped_regs_count;
      } else {
        count = 1;
      }
    }

    for (size_t i = 0; i < count && (start_addr + i) < SIZE; i++) {
      mem_info[start_addr + i].mapped = false;
      mem_info[start_addr + i].read_only = true;
      mem_info[start_addr + i].dev = std::nullopt;
    }
  }

  /**
   * unmap all RAM, ROM and peripheral devices
   */
  void clear() { unmap_mem(0, SIZE); }

  /**
   * get pointer to peripheral device mapped at address, if present
   *
   * use `dynamic_cast` to access device-specific functions
   */
  std::optional<MemoryMappedDevice *> get_dev(Word addr) {
    auto dev = mem_info[addr].dev;
    if (dev.has_value()) {
      return std::get<MemoryMappedDevice *>(dev.value());
    }

    return std::nullopt;
  }

private:
  Byte *memory;

  struct AddressInfo {
    /**
     * information about mapped peripheral device, if present
     *
     * @param _0 mapped device
     * @param _1 offset from base address for the mapping
     */
    std::optional<std::pair<MemoryMappedDevice *, size_t>> dev = std::nullopt;

    bool mapped = false;
    bool read_only = true;
  } *mem_info;
};
