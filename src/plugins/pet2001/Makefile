LDLIBS = `sdl2-config --libs` 
INCLUDE = `sdl2-config --cflags`
SOURCES = main.cpp chardev.cpp keyboard.cpp
OUTFILE = ../pet2001.so

# ImGui
IMGUI_DIR = ../../../lib/imgui
INCLUDE += -I$(IMGUI_DIR)

include ../plugin-makefile-base
