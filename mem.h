#pragma once
#include "common.h"
#include <iostream>
#include <list>
#include <ncurses.h>

struct MemoryMappedDevice {
    MemoryMappedDevice(bool ro, SByte mapc):
        read_only(ro), num_mapped_regs(mapc) {
            mapped_regs = new Byte*[mapc];
        }
    const bool read_only = false;
    const SByte num_mapped_regs;
    Byte** mapped_regs;

    virtual int pre_update() = 0;
    virtual int post_update() = 0;
};

struct ROM {
    ROM(DWord rom_size, Byte* rom_content);
    ROM(DWord rom_size, Byte* rom_content, DWord start_addr);
    const Byte* operator[](DWord i);

    const Byte *content;
    const DWord size;
    const DWord start_address;
};

class AddressSpace {
    public:
        AddressSpace();
        AddressSpace(DWord mSize);
        AddressSpace(DWord mSize, std::list<ROM*> roms);

        Byte* operator[](DWord i);

        Byte* get(DWord i);
        ushort read_word(DWord addr);
        ushort read_word(DWord addr, bool wrap_page);
        void write_word(DWord addr, ushort val);
        void clear_ram();
        void map_roms(std::list<ROM*> roms);
        void map_mem(ROM* rom);
        void map_mem(MemoryMappedDevice* dev, DWord addr);
        void map_mem(const void* bytes, DWord size, DWord start_addr);
        void map_mem(void* bytes, DWord size, DWord start_addr);
        void map_mem(void* bytes, DWord size, DWord start_addr, bool mask, bool read_only);
        void unmap_mem(DWord size, DWord start_addr);

        void free();

    private:
        typedef struct mem_addr {
            void* memory     = NULL;
            bool  is_mapped  = false;
            bool  read_only  = false;
            SByte dev_regidx = -1;
        } MemAddr;
        DWord mem_size;
        DWord tmpval;
        Byte *ram;
        DWord last_addr = -1;

        MemAddr* mapped_mem;
        void init_ram();
};