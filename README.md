# 6502emu

## Build dependencies

- GCC supporting C++17 or higher
- GNU Make
- libreadline

## Building

Run `make all` to build the `6502` executable and all included plugins.

Run `make rebuild` to perform a clean rebuild.

## I/O (`emu-stdio.so`)

### Build dependencies

- GCC supporting C++17 or higher
- GNU Make
- libncurses

Intended for use with Jeff Tranter's [Enhanced 6502 BASIC](https://github.com/jefftranter/6502/tree/master/asm/ehbasic).

- To write a character to `stdout`, store it at address `0xF001`.
- To read a character from `stdin`, read from address `0xF004` (non-blocking, returns `0` if there is none).

## TODO

- interrupts
- support decimal mode
- fix cycle counting
- improve memory-mapping process
- more plugins - VIC-20 emulation as potential goal
