#include "mem.h"

char* ROM::operator[](uint i) {
    if (i < size)
        return content + i;
    return NULL;
}

char* AddressSpace::operator[](uint i) {
    if (i < mem_size)
        return *(mapped_mem + i);
    return NULL;
}
ushort AddressSpace::read_word(uint addr, bool wrap_page) {
    char loByte, hiByte;
    loByte = *mapped_mem[addr];
    if (!wrap_page || (addr&0xFF) != 0xFF)
        hiByte = *mapped_mem[addr+1];
    else
        hiByte = *mapped_mem[addr&0xFF00];

    return (ushort)(hiByte<<8) | loByte;
}
void AddressSpace::write_word(uint addr, ushort val) {
    *mapped_mem[addr++] = val;
    *mapped_mem[addr] = val;
}
void AddressSpace::clear_ram() {
    for (uint i = 0; i < mem_size; i++)
        ram[i] = 0;
}
void AddressSpace::map_mem(ROM rom) { map_mem(rom.content, rom.size, rom.start_address); }
void AddressSpace::map_mem(char *bytes, uint size, uint start_addr) {
    for (uint i = 0; i < size; i++)
        mapped_mem[start_addr + i] = bytes + i;
}
void AddressSpace::init_ram() {
    ram = new char[mem_size];
    mapped_mem = new char*[mem_size];
    clear_ram();
    map_mem(ram, mem_size, 0);
}
