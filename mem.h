//#ifndef MEM_H
//#define MEM_H
//#ifndef BASICLIBS
//#define BASICLIBS
#include <iostream>
#include <list>
//#endif

class ROM {
    public:
        char *content;
        uint size;
        uint start_address;
        ROM(uint rom_size, char *rom_content) : ROM(rom_size, rom_content, 0) { }
        ROM(uint rom_size, char *rom_content, uint start_addr) {
            size = rom_size;
            content = rom_content;
            start_address = start_addr;
        }

        char *operator[](uint i);
};

class AddressSpace {
    public:
        AddressSpace() : AddressSpace(0x10000, std::list<ROM>()) { }
        AddressSpace(uint mSize) : AddressSpace(mSize, std::list<ROM>()) { }
        AddressSpace(uint mSize, std::list<ROM> roms) {
            mem_size = mSize;
            init_ram();
            for (ROM r : roms)
                map_mem(r);
        }

        char *operator[](uint i);
        ushort read_word(uint addr, bool wrap_page);
        void write_word(uint addr, ushort val);
        void clear_ram();
        void map_mem(ROM rom);
        void map_mem(char *bytes, uint size, uint start_addr);

    private:
        uint mem_size;
        char *ram;
        char **mapped_mem;
        void init_ram();
};
//#endif