# 6502emu

## Dependencies
- gcc supporting C++11 or higher
- ncurses

## Building
To build the ```6502``` executable, run ```make```.

To perform a clean build, run ```make rebuild```.

## Usage
Upon starting the emulator, you will be asked to provide filenames of 6502-compatible binary files, and a start address for each.

To begin execution, press return after providing the last start address. Stop execution at any time by pressing either Ctrl+D or Ctrl+C, depending on whether the ```EHBASIC``` preprocessor variable is set.

### I/O
For now, I/O is only possible via the console on a per-character basis.

- To write an ASCII character to the console, simply store it at address ```0xF001```.

- To get an ASCII character from the console input, read from address ```0xF004```. If there are no more characters to be read, this will yield `0`.

### EhBASIC
Included in the ```roms``` directory is an assembled version of Jeff Tranter's [Enhanced 6502 BASIC](https://github.com/jefftranter/6502/tree/master/asm/ehbasic) - or EhBASIC for short - version 2.22.

You may load this binary file at the default starting address of ```$C000``` to get something interactive you can play with.

All credit for EhBASIC goes to its creator, of course.

## Future plans
- I intend to add secondary hardware components to this to turn it into a more useful, full-system emulator of a.. I'm not decided yet.

- BCD arithmetic is currently not supported - this will change.

- A GUI is sorely needed, both for debugging and for I/O.

- Resource usage is okay, but could be a lot better. I'll probably refactor much of this in future, once I figure out a way of doing things that I'm happy with.