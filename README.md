# 6502emu

![Screenshot of the emulator running CBM PET Basic 4.0](https://github.com/DenebTM/6502emu/assets/7706853/98698322-f893-4471-8460-2e2f12b0dc75)

## Build dependencies

- Clang supporting C++20 or higher
- GNU Make
- readline
- yaml-cpp
- dbus-glib
- SDL2

## Building

Run `make all` to build the `6502` executable and all included plugins.

Run `make rebuild` to perform a clean rebuild.

## Some plugins

### I/O plugin (`emu-stdio.so`)

#### Additional dependencies

- ncurses

Intended for use with Jeff Tranter's [Enhanced 6502 BASIC](https://github.com/jefftranter/6502/tree/master/asm/ehbasic).

- To write a character to `stdout`, store it at address `0xF001`.
- To read a character from `stdin`, read from address `0xF004` (non-blocking, returns `0` if there is none).

### PET 2001 emulation (`pet2001.so`)

Incomplete implementation of the Commodore PET 2001 hardware, sufficient to boot into BASIC and load software from .tap files.

Use with [configs/pet2001.yaml](configs/pet2001.yaml) or [configs/pet2001.yaml](configs/basic4.yaml) - ROMs not included.
Additionally requires a 2k PETSCII character ROM to be present at `roms/char_rom.bin`.

### framebuffer (`framebuf.so`)

Provides a 640x480, 1bpp framebuffer at address `0x9000-0x9FFF`.

- `0x9000 - 0x9EFF` maps to the currently visible portion of screen memory (48 scanlines at a time)
- `0x9F00 - 0x9FFF` all map to a single, 8-bit register determining the currently active portion of screen memory. Valid values are `0-9`, higher values will result in a segfault because currently no bounds-checking is performed.

Some demo code is provided in [roms/fbclear.asm](roms/fbclear.asm), used by [configs/fb.yaml](configs/fb.yaml) .

## TODO

- more plugins - full CBM PET or VIC-20 emulation as potential goal
- UI for debugging (ongoing)
- documentation (ongoing)
- rework the plugin system
  - decouple plugins defining devices from instances of those devices
  - allow for other kinds of plugins, e.g. UI addons
