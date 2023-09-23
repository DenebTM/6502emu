#include "mem.hpp"

AddressSpace::AddressSpace() : AddressSpace(0x10000, std::list<ROM *>()) {}
AddressSpace::AddressSpace(DWord mSize) : AddressSpace(mSize, std::list<ROM *>()) {}
AddressSpace::AddressSpace(DWord mSize, std::list<ROM *> roms) {
  mem_size = mSize;
  memory = new Byte[mem_size];
  mem_info = new AddressInfo[mem_size];
  clear_ram();
  map_roms(roms);
}

Byte AddressSpace::read(DWord addr) {
  // Wrap around to beginning of memory space if out of range.
  addr %= mem_size;

  AddressInfo info = mem_info[addr];
  if (info.dev.has_value()) {
    auto [dev, dev_idx] = info.dev.value();
    dev->pre_read();
    return dev->mapped_regs[dev_idx];
  }

  return memory[addr];
}
Word AddressSpace::read_word(DWord addr_lo) { return read_word(addr_lo, false); }
Word AddressSpace::read_word(DWord addr_lo, bool wrap_page) {
  auto addr_hi = addr_lo + 1;
  if (wrap_page) {
    addr_hi &= 0x00ff;
    addr_hi |= addr_lo & 0xff00;
  }

  return (Word)read(addr_lo) + ((Word)read(addr_hi) << 8);
}

void AddressSpace::write(DWord addr, Byte val) {
  AddressInfo info = mem_info[addr];
  if (info.dev.has_value()) {
    auto [dev, dev_idx] = info.dev.value();
    if (!info.read_only)
      dev->mapped_regs[dev_idx] = val;
    dev->post_write();
  }

  else if (!info.read_only) {
    memory[addr] = val;
  }
}

void AddressSpace::write_word(DWord addr_lo, Word val) { write_word(addr_lo, val, false); }

void AddressSpace::write_word(DWord addr_lo, Word val, bool wrap_page) {
  auto addr_hi = addr_lo + 1;
  if (wrap_page) {
    addr_hi &= 0x00ff;
    addr_hi |= addr_lo & 0xff00;
  }

  write(addr_lo, val);
  write(addr_hi, val >> 8);
}

void AddressSpace::map_roms(std::list<ROM *> roms) {
  for (ROM *r : roms)
    map_mem(r);
}
void AddressSpace::map_mem(ROM *rom) {
  map_mem(
#ifdef FUNCTEST // functional test takes up 64k, but needs memory to be R/W
      (Byte *)
#endif
          rom->content,
      rom->size, rom->start_address);
}

void AddressSpace::map_mem(MemoryMappedDevice *dev, DWord start_addr) {
  for (size_t i = 0; i < dev->num_mapped_regs && (start_addr + i) < mem_size; i++) {
    mem_info[start_addr + i].dev.emplace<std::pair<MemoryMappedDevice *, size_t>>({dev, i});
    mem_info[start_addr + i].read_only = dev->read_only;
  }
}
void AddressSpace::map_mem(const Byte *bytes, DWord size, DWord start_addr) {
  map_mem((Byte *)bytes, size, start_addr, true);
}
void AddressSpace::map_mem(Byte *bytes, DWord size, DWord start_addr) { map_mem(bytes, size, start_addr, false); }
void AddressSpace::map_mem(Byte *bytes, DWord size, DWord start_addr, bool read_only) {
  for (size_t i = 0; i < size && (start_addr + i) < mem_size; i++) {
    memory[start_addr + i] = bytes[i];
    mem_info[start_addr + i].read_only = read_only;
  }
}

void AddressSpace::unmap_mem(DWord size, DWord start_addr) {
  for (size_t i = 0; i < size && (start_addr + i) < mem_size; i++)
    mem_info[i].dev.reset();
}

void AddressSpace::clear_ram() {
  for (size_t i = 0; i < mem_size; i++)
    memory[i] = 0;
}

AddressSpace::~AddressSpace() {
  delete[] mem_info;
  delete[] memory;
}
