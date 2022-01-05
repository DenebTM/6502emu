#include "mem.h"

ROM::ROM(DWord rom_size, Byte* rom_content) : ROM(rom_size, rom_content, 0) { }
ROM::ROM(DWord rom_size, Byte* rom_content, DWord start_addr):
    size(rom_size), content(rom_content), start_address(start_addr) { }
const Byte* ROM::operator[](DWord i) {
    if (i < size)
        return content + i;
    return NULL;
}

AddressSpace::AddressSpace() : AddressSpace(0x10000, std::list<ROM*>()) { }
AddressSpace::AddressSpace(DWord mSize) : AddressSpace(mSize, std::list<ROM*>()) { }
AddressSpace::AddressSpace(DWord mSize, std::list<ROM*> roms) {
    mem_size = mSize;
    mapped_mem = new MemAddr[mem_size];
    init_ram();
    map_roms(roms);
}

Byte* AddressSpace::operator[](DWord i) { return get(i); }
Byte* AddressSpace::get(DWord i) {
    // Make sure index isn't out of range
    if (i >= mem_size) return NULL;

    MemAddr* mem = mapped_mem + last_addr;
    // Handling of memory mapped devices
    // Make sure a memory access has occurred before
       // if so, and the memory address points to a device register,
       //   execute that device's post-update function
    if (last_addr != -1 && mem->dev_regidx != -1)
        ((MemoryMappedDevice*)mem->memory)->post_update();
    last_addr = i;
    mem = mapped_mem + i;
    // If the memory address points at a device-mapped register,
    //   run the device's pre-update function
    if (mem->dev_regidx >= 0) {
        MemoryMappedDevice* devptr = (MemoryMappedDevice*)mem->memory;
        devptr->pre_update();
        if (mem->read_only) {
            tmpval = *devptr->mapped_regs[mem->dev_regidx];
            return (Byte*)&tmpval;
        }
        return devptr->mapped_regs[mem->dev_regidx];
    }
    // If the memory address is read-only, copy its value to a temporary
    //   location and return a pointer to it
    if (mem->read_only) {
        tmpval = *(DWord*)mem->memory;
        return (Byte*)&tmpval;
    }
    // If the memory address is read-write, return a pointer to it
    return (Byte*)mem->memory;
}
ushort AddressSpace::read_word(DWord addr) { return read_word(addr, false); }
ushort AddressSpace::read_word(DWord addr, bool wrap_page) {
    Byte loByte, hiByte;
    loByte = *get(addr);
    if (!wrap_page || (addr&0xFF) != 0xFF)
        hiByte = *get(addr+1);
    else
        hiByte = *get(addr&0xFF00);

    return (ushort)(hiByte<<8) | loByte;
}

void AddressSpace::write_word(DWord addr, ushort val) {
    *get(addr)   = val;
    *get(addr+1) = val>>8;
}

void AddressSpace::map_roms(std::list<ROM*> roms) { for (ROM* r : roms) map_mem(r); }
void AddressSpace::map_mem(ROM* rom) { map_mem(rom->content, rom->size, rom->start_address); }

void AddressSpace::map_mem(MemoryMappedDevice* dev, DWord start_addr) {
    for (SByte i = 0; i < dev->num_mapped_regs && (start_addr+i) < mem_size; i++)
        mapped_mem[start_addr] = { dev, true, dev->read_only, i };
}
void AddressSpace::map_mem(const void* bytes, DWord size, DWord start_addr) {
    map_mem((void*)bytes, size, start_addr, true, false);
}
void AddressSpace::map_mem(void* bytes, DWord size, DWord start_addr) {
    map_mem(bytes, size, start_addr, true, false);
}
void AddressSpace::map_mem(void* bytes, DWord size, DWord start_addr, bool mask, bool read_only) {
    for (size_t i = 0; i < size && (start_addr+i) < mem_size; i++)
        if (!mapped_mem[start_addr + i].is_mapped)
            mapped_mem[start_addr + i] = { (Byte*)bytes+i, mask, read_only, -1 };
}

void AddressSpace::unmap_mem(DWord size, DWord start_addr) {
    for (size_t i = 0; i < size && (start_addr+i) < mem_size; i++)
        mapped_mem[start_addr+i].is_mapped = false;
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

void AddressSpace::free() {
    delete[] mapped_mem;
    delete[] ram;
}