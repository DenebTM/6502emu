#include "mem.h"

void OutChar::check_changed() {
    if (val != 0) {
        std::cout << val;
        std::cout.setf(std::ios::unitbuf);
        val = 0;
    }
}

ROM::ROM(uint rom_size, char* rom_content) : ROM(rom_size, rom_content, 0) { }
ROM::ROM(uint rom_size, char* rom_content, uint start_addr) {
    size = rom_size;
    content = rom_content;
    start_address = start_addr;
}
char* ROM::operator[](uint i) {
    if (i < size)
        return content + i;
    return NULL;
}

AddressSpace::AddressSpace() : AddressSpace(0x10000, std::list<ROM>()) { }
AddressSpace::AddressSpace(uint mSize) : AddressSpace(mSize, std::list<ROM>()) { }
AddressSpace::AddressSpace(uint mSize, std::list<ROM> roms) {
    mem_size = mSize;
    mapped_mem = new void*[mem_size];
    is_mapped = new bool[mem_size] { false };
    init_ram();
    map_roms(roms);
}
char* AddressSpace::operator[](uint i) {
    if (i < mem_size)
        return (char*)*(mapped_mem + i);
    return NULL;
}
ushort AddressSpace::read_word(uint addr) {
    return read_word(addr, false);
}
ushort AddressSpace::read_word(uint addr, bool wrap_page) {
    char loByte, hiByte;
    loByte = *(char*)mapped_mem[addr];
    if (!wrap_page || (addr&0xFF) != 0xFF)
        hiByte = *(char*)mapped_mem[addr+1];
    else
        hiByte = *(char*)mapped_mem[addr&0xFF00];

    return (ushort)(hiByte<<8) | loByte;
}
void AddressSpace::write_word(uint addr, ushort val) {
    *(char*)mapped_mem[addr++] = val;
    *(char*)mapped_mem[addr] = val>>8;
}
void AddressSpace::clear_ram() {
    for (uint i = 0; i < mem_size; i++)
        ram[i] = 0;
}
void AddressSpace::map_mem(ROM rom) { map_mem(rom.content, rom.size, rom.start_address); }
void AddressSpace::map_mem(void* bytes, uint size, uint start_addr) {
    map_mem(bytes, size, start_addr, true);
}
void AddressSpace::map_mem(void* bytes, uint size, uint start_addr, bool mask) {
    for (uint i = 0; i < size; i++) {
        if (!is_mapped[start_addr + i]) {
            mapped_mem[start_addr + i] = (char*)bytes + i;
            if (mask) is_mapped[start_addr + i] = true;
        }
    }
}
void AddressSpace::unmap_mem(uint size, uint start_addr) {
    for (uint i = 0; i < size; i++)
        is_mapped[start_addr + i] = false;
}
void AddressSpace::map_roms(std::list<ROM> roms) {
    for (ROM r : roms)
        map_mem(r);
}
void AddressSpace::init_ram() {
    ram = new char[mem_size];
    clear_ram();
    map_mem(ram, mem_size, 0, false);
}

void AddressSpace::clear() {
    delete[] mapped_mem;
    delete[] is_mapped;
    delete[] ram;
}