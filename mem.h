#include <iostream>
#include <list>

class OutChar {
    public:
        OutChar() { val = 0; lastVal = 0;}
        char val;
        void check_changed();
    private:
        char lastVal;
};

class ROM {
    public:
        char *content;
        uint size;
        uint start_address;
        ROM(uint rom_size, char* rom_content);
        ROM(uint rom_size, char* rom_content, uint start_addr);
        
        char* operator[](uint i);
};

class AddressSpace {
    public:
        AddressSpace();
        AddressSpace(uint mSize);
        AddressSpace(uint mSize, std::list<ROM> roms);

        ushort read_word(uint addr);
        ushort read_word(uint addr, bool wrap_page);
        void write_word(uint addr, ushort val);
        void clear_ram();
        void map_roms(std::list<ROM> roms);
        void map_mem(ROM rom);
        void map_mem(void* bytes, uint size, uint start_addr);
        void map_mem(void* bytes, uint size, uint start_addr, bool mask);
        void unmap_mem(uint size, uint start_addr);

        char* operator[](uint i);

        void clear();

    private:
        uint mem_size;
        char *ram;
        void **mapped_mem;
        bool *is_mapped;
        void init_ram();
};