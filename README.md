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