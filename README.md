# 6502emu

## Build dependencies

- GCC supporting C++20 or higher
- GNU Make
- readline
- yaml-cpp

## Building

Run `make all` to build the `6502` executable and all included plugins.

Run `make rebuild` to perform a clean rebuild.

## I/O plugin (`emu-stdio.so`)

### Build dependencies

- GCC supporting C++20 or higher
- GNU Make
- ncurses

Intended for use with Jeff Tranter's [Enhanced 6502 BASIC](https://github.com/jefftranter/6502/tree/master/asm/ehbasic).

- To write a character to `stdout`, store it at address `0xF001`.
- To read a character from `stdin`, read from address `0xF004` (non-blocking, returns `0` if there is none).

## PET 2001 emulation (`pet2001.so`)

### Build dependencies

- GCC supporting C++20 or higher
- GNU Make
- SDL2

Incomplete implementation of the Commodore PET 2001 hardware, sufficient to boot into BASIC.

Use with [pet2001.yaml](configs/pet2001.yaml) - ROMs not included.
Additionally requires a 2k PETSCII character ROM to be present at `roms/char_rom.bin`

## TODO

- fix cycle counting
- improve memory-mapping process
- more plugins - CBM PET or VIC-20 emulation as potential goal
- UI for debugging
- documentation
