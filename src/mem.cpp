#include "mem.hpp"

AddressSpace::AddressSpace() : AddressSpace(0x10000, std::list<ROM *>()) {}
AddressSpace::AddressSpace(DWord mSize) : AddressSpace(mSize, std::list<ROM *>()) {}
AddressSpace::AddressSpace(DWord mSize, std::list<ROM *> roms) {
  mem_size = mSize;
  mapped_mem = new MemAddr[mem_size];
  init_ram();
  map_roms(roms);
}

Byte *AddressSpace::operator[](DWord i) { return get(i); }
Byte *AddressSpace::get(DWord i) {
  // Wrap around to beginning of memory space if out of range.
  i %= mem_size;

#ifndef FUNCTEST
  MemAddr *mem = mapped_mem + last_addr;
  // Handling of memory mapped devices
  // Make sure a memory access has occurred before
  // if so, and the memory address points to a device register,
  //   execute that device's post-update function
  if (last_addr != -1 && mem->dev_regidx != -1)
    ((MemoryMappedDevice *)mem->memory)->post_update();
  last_addr = i;
  mem = mapped_mem + i;
  // If the memory address points at a device-mapped register,
  //   run the device's pre-update function
  if (mem->dev_regidx >= 0) {
    MemoryMappedDevice *devptr = (MemoryMappedDevice *)mem->memory;
    devptr->pre_update();
    if (mem->read_only) {
      tmpval = *devptr->mapped_regs[mem->dev_regidx];
      return (Byte *)&tmpval;
    }
    return devptr->mapped_regs[mem->dev_regidx];
  }
  // If the memory address is read-only, copy its value to a temporary
  //   location and return a pointer to it
  if (mem->read_only) {
    tmpval = *(DWord *)mem->memory;
    return (Byte *)&tmpval;
  }
#else
  MemAddr *mem = mapped_mem + i;
#endif
  // If the memory address is read-write, return a pointer to it
  return (Byte *)mem->memory;
}
Word AddressSpace::read_word(DWord addr) { return read_word(addr, false); }
Word AddressSpace::read_word(DWord addr, bool wrap_page) {
  Byte loByte, hiByte;
  loByte = *get(addr);
  if (!wrap_page || (addr & 0xFF) != 0xFF)
    hiByte = *get(addr + 1);
  else
    hiByte = *get(addr & 0xFF00);

  return (Word)(hiByte << 8) | loByte;
}

void AddressSpace::write_word(DWord addr, Word val) {
  *get(addr) = val;
  *get(addr + 1) = val >> 8;
}

void AddressSpace::map_roms(std::list<ROM *> roms) {
  for (ROM *r : roms)
    map_mem(r);
}
void AddressSpace::map_mem(ROM *rom) { map_mem(rom->content, rom->size, rom->start_address); }

void AddressSpace::map_mem(MemoryMappedDevice *dev, DWord start_addr) {
  for (SByte i = 0; i < dev->num_mapped_regs && (start_addr + i) < mem_size; i++)
    mapped_mem[start_addr + i] = {dev, true, dev->read_only, i};
}
void AddressSpace::map_mem(const void *bytes, DWord size, DWord start_addr) {
  map_mem((void *)bytes, size, start_addr, true, true);
}
void AddressSpace::map_mem(void *bytes, DWord size, DWord start_addr) { map_mem(bytes, size, start_addr, true, false); }
void AddressSpace::map_mem(void *bytes, DWord size, DWord start_addr, bool mask, bool read_only) {
  for (size_t i = 0; i < size && (start_addr + i) < mem_size; i++)
    if (!mapped_mem[start_addr + i].is_mapped)
      mapped_mem[start_addr + i] = {(Byte *)bytes + i, mask, read_only, -1};
}

void AddressSpace::unmap_mem(DWord size, DWord start_addr) {
  for (size_t i = 0; i < size && (start_addr + i) < mem_size; i++)
    mapped_mem[start_addr + i].is_mapped = false;
}

void AddressSpace::clear_ram() {
  for (size_t i = 0; i < mem_size; i++)
    ram[i] = 0;
}
void AddressSpace::init_ram() {
  ram = new Byte[mem_size];
  clear_ram();
  map_mem(ram, mem_size, 0, false, false);
}

AddressSpace::~AddressSpace() {
  delete[] mapped_mem;
  delete[] ram;
}
