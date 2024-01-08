LDLIBS = `sdl2-config --libs` 
INCLUDE = `sdl2-config --cflags`
SOURCES = main.cpp framebuf.cpp
OUTFILE = ../framebuf.so

# ImGui
IMGUI_DIR = ../../../lib/imgui
INCLUDE += -I$(IMGUI_DIR)

include ../plugin-makefile-base
