# 6502emu

## Dependencies
- gcc supporting C++11 or higher
- ncurses

## Building
To build the ```6502``` executable, run ```make```.

To perform a clean build, run ```make rebuild```.

## Usage
Upon running the built executable, you will be asked to provide paths to 6502-compatible (no 65C02 nor 65C816 support... yet?) binary files and a start address for each.

To begin execution, press return after providing the last start address. Stop execution at any time by pressing Ctrl+C.

### I/O
I/O for now is possible via the console, on a per-character basis. This is achieved by taking advantage of ncurses.

To write an ASCII character to the console, simply store it at address ```0xF001```.

To read an ASCII character from the console, read from address ```0xF004```. When there are no characters to be read, this will yield `0`.

There are no plans to extend this console I/O scheme, as it only exists in order to work with EHBasic (in the roms directory).

### BASIC
Included in the ```roms``` directory is an assembled version of Jeff Tranter's [Enhanced 6502 BASIC](https://github.com/jefftranter/6502/tree/master/asm/ehbasic) - or EhBASIC for short - version 2.22.

You may load this binary file at the default starting address of ```$c000``` to get something interactive you can play with.

All credit for EhBASIC goes to Jeff, of course.

## Future plans
- BCD arithmetic is currently not supported - this will change.

- With any luck I'll be able to add other hardware to this and turn it into a more useful full-system emulator.

- A GUI is sorely needed, both for debugging and for I/O.

- Resource usage is okay, but could be a lot better. I'll probably refactor much of this in future, once I figure out a way of doing things that I'm happy with.