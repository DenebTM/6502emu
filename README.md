# 6502emu

## Build dependencies

- GCC supporting C++17 or higher
- GNU Make
- readline
- yaml-cpp

## Building

Run `make all` to build the `6502` executable and all included plugins.

Run `make rebuild` to perform a clean rebuild.

## I/O (`emu-stdio.so`)

### Build dependencies

- GCC supporting C++17 or higher
- GNU Make
- ncurses

Intended for use with Jeff Tranter's [Enhanced 6502 BASIC](https://github.com/jefftranter/6502/tree/master/asm/ehbasic).

- To write a character to `stdout`, store it at address `0xF001`.
- To read a character from `stdin`, read from address `0xF004` (non-blocking, returns `0` if there is none).

## Character display (`chardev.so`)

### Build dependencies

- GCC supporting C++17 or higher
- GNU Make
- SDL2

Provides a 40x25 character display, mapped to `0x8000-0x8FFF` (mirrored four times).
Requires a 2k PETSCII character ROM to be present at `roms/char_rom.bin`.

## TODO

- fix cycle counting
- improve memory-mapping process
- more plugins - CBM PET or VIC-20 emulation as potential goal
- UI for debugging
- documentation
